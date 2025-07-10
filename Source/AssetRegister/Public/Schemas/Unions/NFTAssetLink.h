// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetLink.h"
#include "Schemas/Asset.h"
#include "Schemas/Link.h"

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
UCLASS(BlueprintType)
class ASSETREGISTER_API UNFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNFTAssetLinkData Data;
};
