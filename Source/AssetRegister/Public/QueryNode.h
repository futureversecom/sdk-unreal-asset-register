#pragma once
#include "QueryNode.generated.h"

USTRUCT()
struct FQueryNode
{
	GENERATED_BODY()
	
	FString Name;
	FString FragmentType;
	TArray<TSharedPtr<FQueryNode>> Children;

	FQueryNode() {}
	
	FQueryNode(const FString& InName, const FString& InFragmentType = TEXT(""))
		: Name(InName), FragmentType(InFragmentType) {}

	void AddChild(const TSharedPtr<FQueryNode>& Child)
	{
		Children.Add(Child);
	}
};
