// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetOwnership.h"
#include "Schemas/Account.h"

#include "NFTAssetOwnership.generated.h"


USTRUCT(BlueprintType)
struct ASSETREGISTER_API FNFTAssetOwnership : public FAssetOwnership
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAccount Owner;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETREGISTER_API UNFTAssetOwnershipObject : public UAssetOwnershipObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNFTAssetOwnership Data;
};
