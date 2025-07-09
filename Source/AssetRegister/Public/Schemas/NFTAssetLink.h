// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Asset.h"
#include "AssetLink.h"
#include "Link.h"
#include "NFTAssetLink.generated.h"

// data struct for UNFTAssetLink so it can deserialized
USTRUCT(BlueprintType, meta=(QueryName = "NFTAssetLink"))
struct FNFTAssetLinkData : public FAssetLinkData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "parentLink"))
	FAsset ParentLink;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};

/**
 * 
 */
UCLASS(Blueprintable, meta=(QueryName = "NFTAssetLink"))
class ASSETREGISTER_API UNFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAsset ParentLink;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLink> ChildLinks;
};
