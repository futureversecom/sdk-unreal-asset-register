#pragma once
#include "EqFilter.h"

#include "AssetFilter.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetFilter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEqFilter> EqFilters;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> HasFilters;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Search;
};
