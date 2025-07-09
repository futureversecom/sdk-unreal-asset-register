#pragma once
#include "AssetConnection.generated.h"

USTRUCT(BlueprintType)
struct FAssetConnection
{
	GENERATED_BODY()
	
	FAssetConnection(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "addresses"))
	TArray<FString> Addresses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "collectionIds"))
	TArray<FString> CollectionIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "first"))
	float First = 0;
};
