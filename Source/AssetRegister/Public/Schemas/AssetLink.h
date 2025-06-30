#pragma once
#include "Asset.h"
#include "Link.h"
#include "AssetLink.generated.h"

UCLASS(meta=(QueryName = "SFTAssetLink"))
class USFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(QueryName = "parentLinks"))
	TArray<FAsset> ParentLinks;
};

UCLASS(meta=(QueryName = "NFTAssetLink"))
class UNFTAssetLink : public UAssetLink
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FAsset ParentLink;
	
	UPROPERTY(meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};

USTRUCT(meta=(QueryName = "NFTAssetLink"))
struct FNFTAssetLinkData
{
	GENERATED_BODY()

	UPROPERTY()
	FAsset ParentLink;
	
	UPROPERTY(meta=(QueryName = "childLinks"))
	TArray<FLink> ChildLinks;
};

USTRUCT(meta=(QueryName = "SFTAssetLink"))
struct FSFTAssetLinkData
{
	GENERATED_BODY()

	UPROPERTY(meta=(QueryName = "parentLinks"))
	TArray<FAsset> ParentLinks;
};

