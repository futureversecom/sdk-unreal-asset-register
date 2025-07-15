#pragma once
#include "Account.h"

#include "SFTBalance.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FSFTBalance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Balance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAccount Owner;
};
