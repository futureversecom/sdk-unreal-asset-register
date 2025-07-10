#pragma once

#include "ProfileProperties.generated.h"

USTRUCT(BlueprintType)
struct FProfileProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "description"))
	FString Description;
};
