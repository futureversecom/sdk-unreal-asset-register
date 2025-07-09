// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Asset.h"
#include "AssetLink.h"
#include "SFTAssetLink.generated.h"

// data struct for USFTAssetLink so it can deserialized
USTRUCT(meta=(QueryName = "SFTAssetLink"))
struct FSFTAssetLinkData : public FAssetLinkData
{
	GENERATED_BODY()

	UPROPERTY(meta=(QueryName = "parentLinks"))
	TArray<FAsset> ParentLinks;
};

/**
 * 
 */
UCLASS(meta=(QueryName = "SFTAssetLink"))
class ASSETREGISTER_API USFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FAsset> ParentLinks;
};
