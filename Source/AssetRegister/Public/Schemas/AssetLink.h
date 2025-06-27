#pragma once
#include "Asset.h"
#include "Link.h"
#include "AssetLink.generated.h"

USTRUCT(meta=(QueryName = "SFTAssetLink"))
struct FSFTAssetLink : public FAssetLink
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(QueryName = "parentLinks"))
	TArray<FAsset> ParentLinks;
};

USTRUCT(meta=(QueryName = "NFTAssetLink"))
struct FNFTAssetLink : public FAssetLink
{
	GENERATED_BODY()
	
	UPROPERTY()
	FAsset ParentLink;
	
	UPROPERTY(meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};

