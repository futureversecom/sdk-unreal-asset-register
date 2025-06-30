#pragma once
#include "QueryStringUtil.h"

class IQueryNode : public TSharedFromThis<IQueryNode>
{
public:
	IQueryNode() {}
	
	IQueryNode(const FString& InName, bool bIsUnion = false)
	: Name(InName), bIsUnion(bIsUnion) {}
	
	FString GetArgumentsString() 
	{
		TArray<FString> ArgumentStrings;
		for (const FString& ArgumentString : Arguments)
		{
			ArgumentStrings.Add(QueryStringUtil::ConvertJsonToGraphQLFriendlyString(ArgumentString));
		}
         
		return FString::Join(ArgumentStrings, TEXT(","));
	}

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

	FString GetQueryString()
	{
		return "query {\n" + SerializeNode(AsShared(), 0) + "\n}";
	}

protected:
	TArray<FString> Arguments;
	TMap<FString, TSharedPtr<IQueryNode>> ChildrenMap;
	FString Name;
	bool bIsUnion = false;
};

template <typename TModel>
class FQueryNode : public IQueryNode
{
public:
	FQueryNode() {}
	
	FQueryNode(const FString& InName, bool bIsUnion = false) : IQueryNode(InName, bIsUnion) {}

	FString GetModelString() const
	{
		return QueryStringUtil::GetQueryName<TModel>();
	}
	
	void AddArgument(const FString& Argument)
	{
		Arguments.Add(Argument);
	}
	
	template<typename T>
	FQueryNode<TModel>* AddArgument(const T& Argument)
	{
		FString OutJson;
		const UScriptStruct* ArgumentDefinition = TBaseStructure<T>::Get();

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

	void AddArgument(const FString& ArgName, const FString& Value)
	{
		AddArgument(FString::Printf(TEXT("%s:\"%s\""), *ArgName, *Value));
	}

	void AddArgument(const FString& ArgName, const int32 Value)
	{
		AddArgument(FString::Printf(TEXT("%s:%s"), *ArgName, *FString::FromInt(Value)));
	}

	void AddArgument(const FString& ArgName, const bool Value)
	{
		AddArgument(FString::Printf(TEXT("%s:%s"), *ArgName, Value ? TEXT("true") : TEXT("false")));
	}
	
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

	template<typename TDerived, typename TParent>
	requires std::is_base_of_v<std::remove_pointer_t<TParent>, std::remove_pointer_t<TDerived>>
	&& std::is_same_v<std::remove_pointer_t<TParent>, std::remove_pointer_t<TModel>>
	FQueryNode<std::remove_pointer_t<TDerived>>* OnUnion()
	{
		using Derived = std::remove_pointer_t<TDerived>;
		
		FString FieldName = QueryStringUtil::GetQueryName<Derived>();
		if (!ChildrenMap.Contains(FieldName))
		{
			ChildrenMap.Add(FieldName, MakeShared<FQueryNode<Derived>>(FieldName, true));
		}
		
		return StaticCastSharedPtr<FQueryNode<Derived>>(ChildrenMap[FieldName]).Get();
	}

	template<typename TParent, typename TElement>
	requires std::is_same_v<std::remove_pointer_t<TParent>, std::remove_pointer_t<TModel>>
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

