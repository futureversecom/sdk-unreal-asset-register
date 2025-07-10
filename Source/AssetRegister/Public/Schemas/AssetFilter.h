#pragma once
#include "EqFilter.h"

#include "AssetFilter.generated.h"

USTRUCT(BlueprintType)
struct FAssetFilter
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "eqFilters"))
	TArray<FEqFilter> EqFilters;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "hasFilters"))
	TArray<FString> HasFilters;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "search"))
	FString Search;
};
