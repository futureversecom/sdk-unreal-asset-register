// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetLink.h"
#include "Schemas/Asset.h"
#include "Schemas/Link.h"

#include "NFTAssetLink.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FNFTAssetLink : public FAssetLink
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAsset ParentLink;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLink> ChildLinks;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ASSETREGISTER_API UNFTAssetLinkObject : public UAssetLinkObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNFTAssetLink Data;
};
