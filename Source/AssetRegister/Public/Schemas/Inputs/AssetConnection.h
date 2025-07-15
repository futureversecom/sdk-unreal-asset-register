#pragma once
#include "Schemas/AssetFilter.h"
#include "Schemas/FSort.h"
#include "AssetConnection.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetConnection
{
	GENERATED_BODY()
	
	FAssetConnection(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RemoveDuplicates = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSort> Sort;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAssetFilter Filter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SchemaId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CollectionIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Addresses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Before;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString After;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float First = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Last = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainType;
};
