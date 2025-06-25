#pragma once

#include "JsonObjectConverter.h"
#include "QueryNode.h"
#include "QueryStringUtil.h"

template<typename TModel>
class FQueryBuilder
{
public:
	TSharedPtr<FQueryNode> RootNode;

	FQueryBuilder()
	{
		RootNode = MakeShared<FQueryNode>("");
	}

	explicit FQueryBuilder(const TSharedPtr<FQueryNode>& InNode)
		: RootNode(InNode) {}
	
	static FQueryBuilder* Create()
	{
		return new FQueryBuilder();
	}
	
	FString GetModelString() const
	{
		return QueryStringUtil::GetQueryName<TModel>();
	}
	
	template<typename T>
	void AddArgument(const T& Argument)
	{
		const UScriptStruct* ArgumentDefinition = TBaseStructure<T>::Get();

		FString OutJson;
		if (FJsonObjectConverter::UStructToJsonObjectString(ArgumentDefinition, &Argument, OutJson,
			0,0,0, nullptr, false))
		{
			Arguments.Add(OutJson);
		}
	}

    void AddArgument(const FString& Name, const FString& Value)
    {
        Arguments.Add(FString::Printf(TEXT("%s:\"%s\""), *Name, *Value));
    }

    void AddArgument(const FString& Name, const int32 Value)
    {
        Arguments.Add(FString::Printf(TEXT("%s:%s"), *Name, *FString::FromInt(Value)));
    }

    void AddArgument(const FString& Name, const bool Value)
    {
        Arguments.Add(FString::Printf(TEXT("%s:%s"), *Name, Value ? TEXT("true") : TEXT("false")));
    }
     
    FString GetArgumentsString() const
    {
        TArray<FString> ArgumentStrings;
        for (const FString& ArgumentString : Arguments)
        {
            ArgumentStrings.Add(QueryStringUtil::ConvertJsonToGraphQLFriendlyString(ArgumentString));
        }
         
        return FString::Join(ArgumentStrings, TEXT(","));
    }
	
	template<typename TParent, typename TField>
	FQueryBuilder AddField(TField TParent::* FieldPtr)
	{
		FString FieldName = QueryStringUtil::GetQueryName<TParent>(FieldPtr);
		RootNode->AddChild(MakeShared<FQueryNode>(FieldName));
		return *this;
	}
	
	template<typename TParent, typename TField>
    FQueryBuilder OnMember(TField TParent::* FieldPtr)
    {
    	FString FieldName = QueryStringUtil::GetQueryName<TParent>(FieldPtr);
    	auto Node = MakeShared<FQueryNode>(FieldName);
    	RootNode->AddChild(Node);
    	return FQueryBuilder(Node);
    }
	
	template<typename TDerived, typename TParent>
	FQueryBuilder OnUnion(TParent TModel::* FieldPtr)
	{
		static_assert(std::is_base_of_v<TParent, TDerived>, "TDerived must derive from TParent");
		
		FString FieldName = QueryStringUtil::GetQueryName<TModel>(FieldPtr);
	
		auto UnionNode = MakeShared<FQueryNode>(FieldName);
		auto FragmentNode = MakeShared<FQueryNode>(TEXT(""), TDerived::StaticStruct()->GetName());
	
		UnionNode->AddChild(FragmentNode);
		RootNode->AddChild(UnionNode);
	
		return FQueryBuilder(FragmentNode);
	}

	FString GetQueryString() const
	{
		FString QueryString = "query {\n";
		QueryString += GetModelString() + "(" + GetArgumentsString() + ")";
		QueryString += SerializeNode(RootNode, 0) + "\n }";
		
		return QueryString;
	}

	FString GetFieldString() const
	{
		return SerializeNode(RootNode, 0);
	}

private:
	FString SerializeNode(const TSharedPtr<FQueryNode>& Node, int IndentLevel) const
	{
		FString Indent = FString::ChrN(IndentLevel * 2, ' ');
		FString Output;

		FString NodeName = Node->Children.IsEmpty() ? Indent + Node->Name + TEXT("\n"): Indent + Node->Name + TEXT(" {\n");
		Output += NodeName;
		
		for (const auto& Child : Node->Children)
		{
			if (!Child->FragmentType.IsEmpty())
			{
				Output += FString::Printf(TEXT("%s  ... on %s {\n"), *Indent, *Child->FragmentType);
				Output += SerializeNode(Child, IndentLevel + 2);
				Output += FString::Printf(TEXT("%s  }\n"), *Indent);
			}
			else
			{
				Output += SerializeNode(Child, IndentLevel + 1);
			}
		}
	
		if (!Node->Children.IsEmpty())
			Output += Indent + TEXT("}\n");

		return Output;
	}
	
	TArray<FString> Arguments;
};
