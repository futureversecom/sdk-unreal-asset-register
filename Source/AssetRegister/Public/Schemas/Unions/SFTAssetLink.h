// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetLink.h"
#include "Schemas/Asset.h"
#include "SFTAssetLink.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FSFTAssetLink : public FAssetLink
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAsset> ParentLinks;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETREGISTER_API USFTAssetLinkObject : public UAssetLinkObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSFTAssetLink Data;
};
