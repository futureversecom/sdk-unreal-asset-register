#pragma once
#include "AssetConnection.generated.h"

USTRUCT()
struct FAssetConnection
{
	GENERATED_BODY()
	
	FAssetConnection(){}

	FAssetConnection(const TArray<FString>& Addresses, const TArray<FString>& CollectionIds)
		: Addresses(Addresses), CollectionIds(CollectionIds){}

	UPROPERTY(EditAnywhere, meta=(QueryName = "addresses"))
	TArray<FString> Addresses;

	UPROPERTY(EditAnywhere, meta=(QueryName = "collectionIds"))
	TArray<FString> CollectionIds;
};
