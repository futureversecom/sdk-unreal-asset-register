// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetOwnership.h"
#include "Schemas/Account.h"

#include "NFTAssetOwnership.generated.h"


USTRUCT(BlueprintType, meta=(QueryName = "NFTAssetOwnership"))
struct ASSETREGISTER_API FNFTAssetOwnershipData : public FAssetOwnershipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "owner"))
	FAccount Owner;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETREGISTER_API UNFTAssetOwnership : public UAssetOwnership
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNFTAssetOwnershipData Data;
};
