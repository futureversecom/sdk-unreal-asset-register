#pragma once
#include "Account.h"

#include "SFTBalance.generated.h"

USTRUCT(BlueprintType)
struct FSFTBalance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "balance"))
	FString Balance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "owner"))
	FAccount Owner;
};
