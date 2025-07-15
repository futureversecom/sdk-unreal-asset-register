#pragma once

#include "ProfileProperties.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FProfileProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};
