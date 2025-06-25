// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegisterQueryingLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetJsonCompleted, bool, bSuccess, FString, Json);

/**
 * 
 */
UCLASS()
class ASSETREGISTER_API UAssetRegisterQueryingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void GetAssetProfile(const FString& TokenId, const FString& CollectionId, const FGetJsonCompleted& OnCompleted);

	UFUNCTION(BlueprintCallable)
	static void GetAssetLinks(const FString& TokenId, const FString& CollectionId, const FGetJsonCompleted& OnCompleted);
	
	UFUNCTION(BlueprintCallable)
	static void GetAssets(const TArray<FString>& Addresses, const TArray<FString>& Collections, const FGetJsonCompleted& OnCompleted);
	
	static TFuture<FString> SendRequest(const FString& URL, const FString& Content);
};
