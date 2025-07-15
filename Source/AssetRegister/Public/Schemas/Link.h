// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once
#include "Link.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FOffChainAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AssetId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CreatorCollectionId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CreatorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TokenId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type;
};

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FLink
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAsset Asset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOffChainAsset OffChainAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Path;
};