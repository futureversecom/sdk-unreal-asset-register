#pragma once
#include "AssetConnection.generated.h"

USTRUCT(BlueprintType)
struct FAssetConnection
{
	GENERATED_BODY()
	
	FAssetConnection(){}

	UPROPERTY(EditAnywhere, meta=(QueryName = "addresses"))
	TArray<FString> Addresses;

	UPROPERTY(EditAnywhere,meta=(QueryName = "collectionIds"))
	TArray<FString> CollectionIds;

	UPROPERTY(EditAnywhere, meta=(QueryName = "first"))
	float First = 0;
};
