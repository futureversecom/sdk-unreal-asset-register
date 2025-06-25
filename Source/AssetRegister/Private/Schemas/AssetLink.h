#pragma once
#include "Asset.h"
#include "Link.h"
#include "AssetLink.generated.h"

USTRUCT(meta=(QueryName = "asset"))
struct FSFTAssetLink : public FAssetLink
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FAsset> ParentLinks;
};

USTRUCT()
struct FNFTAssetLink : public FAssetLink
{
	GENERATED_BODY()
	
	UPROPERTY()
	FAsset ParentLink;
	
	UPROPERTY(meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};

