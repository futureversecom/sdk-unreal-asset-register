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

	void AddArgument(const FString& Argument)
	{
		Arguments.Add(Argument);
	}
	
	template<typename TArgument>
	FQueryNode<TModel>* AddArgument(const TArgument& Argument)
	{
		const UScriptStruct* ArgumentDefinition = TBaseStructure<TArgument>::Get();

		FString OutJson;
		if (FJsonObjectConverter::UStructToJsonObjectString(ArgumentDefinition, &Argument, OutJson,
			0,0,0, nullptr, false))
		{
			AddArgument(OutJson);
		}
		return this;
	}

	void AddArgument(const FString& Name, const FString& Value)
	{
		AddArgument(FString::Printf(TEXT("%s:\"%s\""), *Name, *Value));
	}

	void AddArgument(const FString& Name, const int32 Value)
	{
		AddArgument(FString::Printf(TEXT("%s:%s"), *Name, *FString::FromInt(Value)));
	}

	void AddArgument(const FString& Name, const bool Value)
	{
		AddArgument(FString::Printf(TEXT("%s:%s"), *Name, Value ? TEXT("true") : TEXT("false")));
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
	requires std::is_base_of_v<TParent, TDerived> && std::is_same_v<TParent, TModel>
	FQueryNode<TDerived>* OnUnion()
	{
		FString FieldName = QueryStringUtil::GetQueryName<TDerived>();
		if (!ChildrenMap.Contains(FieldName))
		{
			ChildrenMap.Add(FieldName, MakeShared<FQueryNode<TDerived>>(FieldName, true));
		}
		
		return StaticCastSharedPtr<FQueryNode<TDerived>>(ChildrenMap[FieldName]).Get();
	}

	template<typename TParent, typename TElement> requires std::is_same_v<TParent, TModel>
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

