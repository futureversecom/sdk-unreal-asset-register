#pragma once

#include "EqFilter.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FEqFilter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;
};
