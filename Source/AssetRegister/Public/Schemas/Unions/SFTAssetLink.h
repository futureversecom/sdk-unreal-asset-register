// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetLink.h"
#include "Schemas/Asset.h"
#include "SFTAssetLink.generated.h"

// data struct for USFTAssetLink so it can deserialized
USTRUCT(BlueprintType, meta=(QueryName = "SFTAssetLink"))
struct ASSETREGISTER_API FSFTAssetLinkData : public FAssetLinkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "parentLinks"))
	TArray<FAsset> ParentLinks;
};

/**
 * 
 */
UCLASS(BlueprintType, meta=(QueryName = "SFTAssetLink"))
class ASSETREGISTER_API USFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSFTAssetLinkData Data;
};
