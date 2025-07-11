// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetOwnership.h"
#include "Schemas/SFTBalance.h"

#include "SFTAssetOwnership.generated.h"

USTRUCT(BlueprintType, meta=(QueryName = "SFTAssetOwnership"))
struct ASSETREGISTER_API FSFTAssetOwnershipData : public FAssetOwnershipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "balanceOf"))
	FSFTBalance BalanceOf;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "balancesOf"))
	TArray<FSFTBalance> BalancesOf;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETREGISTER_API USFTAssetOwnership : public UAssetOwnership
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSFTAssetOwnershipData OwnershipData;
};
