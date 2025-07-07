// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Schemas/Asset.h"
#include "Schemas/Assets.h"
#include "Schemas/Inputs/AssetConnection.h"
#include "AssetRegisterQueryingLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetJsonCompleted, bool, bSuccess, const FString&, Json);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetAssetCompleted, bool, bSuccess, const FAsset&, Asset);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetAssetsCompleted, bool, bSuccess, const FAssets&, Assets);

template<typename T>
struct TLoadResult
{
	bool bSuccess = false;
	T Value;

	void SetResult(const T& InValue)
	{
		bSuccess = true;
		Value = InValue;
	}

	void SetFailure()
	{
		bSuccess = false;
	}
};

struct FLoadJsonResult final : TLoadResult<FString> {};
struct FLoadAssetResult final : TLoadResult<FAsset> {};
struct FLoadAssetsResult final : TLoadResult<FAssets> {};

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

	static TFuture<FLoadJsonResult> GetAssetProfile(const FString& TokenId, const FString& CollectionId);

	UFUNCTION(BlueprintCallable)
	static void GetAssetLinks(const FString& TokenId, const FString& CollectionId, const FGetAssetCompleted& OnCompleted);

	static TFuture<FLoadAssetResult> GetAssetLinks(const FString& TokenId, const FString& CollectionId);
	
	UFUNCTION(BlueprintCallable)
	static void GetAssets(const FAssetConnection& AssetsInput, const FGetAssetsCompleted& OnCompleted);
	
	static TFuture<FString> SendRequest(const FString& Content);

private:
	static void LogJsonString(const TSharedPtr<FJsonObject>& JsonObject); 
};
