// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegisterSettings.generated.h"

/**
 * 
 */
UCLASS()
class ASSETREGISTER_API UAssetRegisterSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(GetOptions="GetURLOptions"))
	FString ProfileURL = "https://ar-api.futureverse.app/graphql";

	UFUNCTION()
	TArray<FString> GetURLOptions() const
	{
		return
		{
			"https://ar-api.futureverse.app/graphql", // prod
			"https://ar-api.futureverse.cloud/graphql", // staging
			"https://ar-api.futureverse.dev/graphql" // dev
		};
	}
};
