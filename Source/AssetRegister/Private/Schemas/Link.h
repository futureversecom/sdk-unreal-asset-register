// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once
#include "Link.generated.h"

USTRUCT()
struct FOffChainAsset
{
	GENERATED_BODY()

	UPROPERTY()
	FString AssetId;
	
	UPROPERTY()
	FString CreatorCollectionId;
	
	UPROPERTY()
	FString CreatorId;
	
	UPROPERTY()
	FString TokenId;
	
	UPROPERTY()
	FString Type;
};

USTRUCT()
struct FLink
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(QueryName = "asset"))
	FAsset Asset;
	
	UPROPERTY(meta=(QueryName = "offChainAsset"))
	FOffChainAsset OffChainAsset;
	
	UPROPERTY(meta=(QueryName = "path"))
	FString Path;
};