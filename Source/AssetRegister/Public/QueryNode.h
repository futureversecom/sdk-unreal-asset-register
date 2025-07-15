#pragma once
#include "QueryStringUtil.h"

/**
 * Base interface for constructing a GraphQL query node.
 * Supports nesting and argument encoding.
 */
class IQueryNode : public TSharedFromThis<IQueryNode>
{
public:
	IQueryNode() {}
	
	IQueryNode(const FString& InName, bool bIsUnion = false)
	: Name(InName), bIsUnion(bIsUnion) {}

	/**
	 * Builds the GraphQL-formatted string for all arguments on this node.
	 */
	FString GetArgumentsString() 
	{
		TArray<FString> ArgumentStrings;
		for (const FString& ArgumentString : Arguments)
		{
			ArgumentStrings.Add(QueryStringUtil::ConvertJsonToGraphQLFriendlyString(ArgumentString));
		}
         
		return FString::Join(ArgumentStrings, TEXT(","));
	}
	
	/**
	 * Recursively serializes a node (and its children) into a valid GraphQL query string.
	 *
	 * @param Node The root node to serialize.
	 * @param IndentLevel The current indent level for formatting.
	 * @return The full GraphQL string for the node and its subtree.
	 */
	static FString SerializeNode(const TSharedPtr<IQueryNode>& Node, int IndentLevel)
	{
		FString Indent = FString::ChrN(IndentLevel * 2, ' ');
		FString Output;
		
		if (Node->bIsUnion)
		{
			Output += Indent + TEXT("... on ") + Node->Name;
		}
		else
		{
			Output += Indent + Node->Name;
		}

		if (!Node->Arguments.IsEmpty())
		{
			Output += "(" + Node->GetArgumentsString() + ")";
		}

		if (Node->ChildrenMap.IsEmpty())
		{
			Output += TEXT("\n");
			return Output;
		}
		
		Output += TEXT(" {\n");
		
		for (const auto& ChildPair : Node->ChildrenMap)
		{
			Output += SerializeNode(ChildPair.Value, IndentLevel + 1);
		}
		
		Output += Indent + TEXT("}\n");

		return Output;
	}
	
	/**
	 * Returns the complete raw GraphQL query string starting from this node.
	 */
	FString GetQueryString()
	{
		return "query {\n" + SerializeNode(AsShared(), 0) + "\n}";
	}

	/**
	 * Returns the complete GraphQL query json string.
	 */
	FString GetQueryJsonString()
	{
		FString OutJson;
		TSharedRef<FJsonObject> JsonBody = MakeShared<FJsonObject>();
		JsonBody->SetStringField("query", GetQueryString());
	
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
		FJsonSerializer::Serialize(JsonBody, Writer);
		return OutJson;
	}

protected:
	/** GraphQL argument strings (usually JSON-formatted). */
	TArray<FString> Arguments;
	
	/** Child nodes representing nested fields. */
	TMap<FString, TSharedPtr<IQueryNode>> ChildrenMap;
	
	/** The GraphQL field or type name for this node. */
	FString Name;

	/** Whether this node represents a union type selection. */
	bool bIsUnion = false;
};

/**
 * Strongly-typed query node for a model struct.
 *
 * @tparam TModel The struct type this query node corresponds to.
 */
template <typename TModel>
class FQueryNode : public IQueryNode
{
public:
	FQueryNode() {}
	
	FQueryNode(const FString& InName, bool bIsUnion = false) : IQueryNode(InName, bIsUnion) {}

	/** Returns the GraphQL name for the model type. */
	FString GetModelString() const
	{
		return QueryStringUtil::GetQueryName<TModel>();
	}

	/** Adds a raw GraphQL argument string to this node. */
	void AddArgument(const FString& Argument)
	{
		Arguments.Add(Argument);
	}

	/**
	 * Serializes a struct to JSON and adds it as a GraphQL argument, omitting default values.
	 *
	 * @param Argument The struct to serialize.
	 * @return This query node for chaining.
	 */
	template<typename T>
	FQueryNode<TModel>* AddArgument(const T& Argument)
	{
		FString OutJson;
		
		if (TSharedPtr<FJsonObject> ArgumentJson = FJsonObjectConverter::UStructToJsonObject<T>(Argument))
		{
			T DefaultStruct;
			TSharedPtr<FJsonObject> DefaultJson = FJsonObjectConverter::UStructToJsonObject<T>(DefaultStruct);
			
			for (const auto& Pair : DefaultJson->Values)
			{
				if (ArgumentJson->HasField(Pair.Key) &&
					FJsonValue::CompareEqual(*ArgumentJson->Values[Pair.Key].Get(), *Pair.Value.Get()))
				{
					ArgumentJson->RemoveField(Pair.Key);
				}
			}
			
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
			FJsonSerializer::Serialize(ArgumentJson.ToSharedRef(), Writer);

			AddArgument(OutJson);
		}
		return this;
	}

	/** Adds a string-valued argument in the format ArgName:"Value". */
	void AddArgument(const FString& ArgName, const FString& Value)
	{
		AddArgument(FString::Printf(TEXT("%s:\"%s\""), *ArgName, *Value));
	}

	/** Adds an integer argument in the format ArgName:Value. */
	void AddArgument(const FString& ArgName, const int32 Value)
	{
		AddArgument(FString::Printf(TEXT("%s:%s"), *ArgName, *FString::FromInt(Value)));
	}

	/** Adds a boolean argument in the format ArgName:true/false. */
	void AddArgument(const FString& ArgName, const bool Value)
	{
		AddArgument(FString::Printf(TEXT("%s:%s"), *ArgName, Value ? TEXT("true") : TEXT("false")));
	}

	/**
	 * Adds a nested field to this node, based on a member pointer of TModel.
	 *
	 * @param FieldPtr Pointer to the field to include.
	 * @return This node (chaining).
	 */
	template< typename TParent, typename TField> requires std::is_same_v<TParent, TModel>
	FQueryNode* AddField(TField TParent::* FieldPtr)
	{
		FString FieldName = QueryStringUtil::GetQueryName<TParent>(FieldPtr);
		
		if (!ChildrenMap.Contains(FieldName))
		{
			ChildrenMap.Add(FieldName, MakeShared<FQueryNode<TField>>(FieldName));
		}
		
		return this;
	}

	/**
	 * Adds or retrieves a child node for a field on TModel.
	 *
	 * @param FieldPtr Pointer to the field on the model.
	 * @return The child query node for the field.
	 */
	template<typename TParent, typename TField> requires std::is_same_v<TParent, TModel>
	FQueryNode<TField>* OnMember(TField TParent::* FieldPtr)
	{
		FString FieldName = QueryStringUtil::GetQueryName<TParent>(FieldPtr);
		if (!ChildrenMap.Contains(FieldName))
		{
			ChildrenMap.Add(FieldName, MakeShared<FQueryNode<TField>>(FieldName));
		}
		
		return StaticCastSharedPtr<FQueryNode<TField>>(ChildrenMap[FieldName]).Get();
	}

	/**
	 * Adds or retrieves a child node for a union subtype of the model.
	 *
	 * @tparam TDerived The derived (union) type.
	 * @return The query node for the union subtype.
	 */
	template<typename TDerived>
	requires std::is_base_of_v<TModel, TDerived>
	FQueryNode<std::remove_pointer_t<TDerived>>* OnUnion()
	{
		using Derived = std::remove_pointer_t<TDerived>;
		
		FString FieldName = QueryStringUtil::GetQueryName<Derived>(false);
		if (!ChildrenMap.Contains(FieldName))
		{
			ChildrenMap.Add(FieldName, MakeShared<FQueryNode<Derived>>(FieldName, true));
		}
		
		return StaticCastSharedPtr<FQueryNode<Derived>>(ChildrenMap[FieldName]).Get();
	}

	/**
	 * Adds or retrieves a child node for an array field on the model.
	 *
	 * @param ArrayPtr Pointer to the array field.
	 * @return The query node representing the array element type.
	 */
	template<typename TParent, typename TElement>
	requires std::is_same_v<TParent, TModel>
	FQueryNode<TElement>* OnArray(TArray<TElement> TParent::* ArrayPtr)
	{
		FString FieldName = QueryStringUtil::GetQueryName<TParent>(ArrayPtr);
		if (!ChildrenMap.Contains(FieldName))
		{
			ChildrenMap.Add(FieldName, MakeShared<FQueryNode<TElement>>(FieldName));
		}
		
		return StaticCastSharedPtr<FQueryNode<TElement>>(ChildrenMap[FieldName]).Get();
	}
};

