#pragma once

#include "EqFilter.generated.h"

USTRUCT(BlueprintType)
struct FEqFilter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "name"))
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "value"))
	FString Value;
};
