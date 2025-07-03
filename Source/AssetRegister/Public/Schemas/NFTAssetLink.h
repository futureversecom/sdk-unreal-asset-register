// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Asset.h"
#include "AssetLink.h"
#include "Link.h"
#include "NFTAssetLink.generated.h"

USTRUCT(meta=(QueryName = "NFTAssetLink"))
struct FNFTAssetLinkData : public FAssetLinkData
{
	GENERATED_BODY()

	UPROPERTY()
	FAsset ParentLink;
	
	UPROPERTY(meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};

/**
 * 
 */
UCLASS(meta=(QueryName = "NFTAssetLink"))
class ASSETREGISTER_API UNFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FAsset ParentLink;
	
	UPROPERTY(meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};
