#pragma once
#include "Schemas/AssetFilter.h"
#include "Schemas/FSort.h"
#include "AssetConnection.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetConnection
{
	GENERATED_BODY()
	
	FAssetConnection(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "removeDuplicates"))
	bool RemoveDuplicates = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "sort"))
	TArray<FSort> Sort;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "filter"))
	FAssetFilter Filter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "schemaId"))
	FString SchemaId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "collectionIds"))
	TArray<FString> CollectionIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "addresses"))
	TArray<FString> Addresses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "before"))
	FString Before;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "after"))
	FString After;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "first"))
	float First = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "last"))
	float Last = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "chainId"))
	FString ChainId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "chainType"))
	FString ChainType;
};
