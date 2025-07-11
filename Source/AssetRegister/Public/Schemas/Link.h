// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once
#include "Link.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FOffChainAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "assetId"))
	FString AssetId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "creatorCollectionId"))
	FString CreatorCollectionId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "creatorId"))
	FString CreatorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "tokenId"))
	FString TokenId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "type"))
	FString Type;
};

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FLink
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "asset"))
	FAsset Asset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "offChainAsset"))
	FOffChainAsset OffChainAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "path"))
	FString Path;
};