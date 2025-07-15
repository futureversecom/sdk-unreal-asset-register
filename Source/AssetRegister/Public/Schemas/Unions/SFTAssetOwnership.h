// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetOwnership.h"
#include "Schemas/SFTBalance.h"

#include "SFTAssetOwnership.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FSFTAssetOwnership : public FAssetOwnership
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSFTBalance BalanceOf;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSFTBalance> BalancesOf;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETREGISTER_API USFTAssetOwnershipObject : public UAssetOwnershipObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSFTAssetOwnership OwnershipData;
};
