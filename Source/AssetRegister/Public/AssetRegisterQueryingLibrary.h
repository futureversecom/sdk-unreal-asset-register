// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "QueryNode.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Schemas/Asset.h"
#include "Schemas/Assets.h"
#include "Schemas/Inputs/AssetConnection.h"
#include "AssetRegisterQueryingLibrary.generated.h"

/**
 * Delegate used for receiving a JSON string result.
 *
 * @param bSuccess Whether the operation succeeded.
 * @param Json The resulting JSON string (if successful).
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetJsonCompleted, bool, bSuccess, const FString&, Json);

/**
 * Delegate used for receiving a single asset result.
 *
 * @param bSuccess Whether the operation succeeded.
 * @param Asset The loaded asset (if successful).
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGetAssetCompleted, bool, bSuccess, const FAsset&, Asset);

/**
 * Delegate used for receiving a set of assets.
 *
 * @param bSuccess Whether the operation succeeded.
 * @param Assets The result containing multiple assets (if successful).
 */
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
 * Utility library containing common Asset Register GraphQL queries.
 * 
 * Exposes both Blueprint-accessible and C++-friendly async versions.
 */
UCLASS()
class ASSETREGISTER_API UAssetRegisterQueryingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Retrieves the asset profile URI for a specific asset.
	 *
	 * @param TokenId The token ID of the asset.
	 * @param CollectionId The ID of the asset's collection.
	 * @param OnCompleted Callback invoked when the request finishes.
	 */
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "OnCompleted"))
	static void GetAssetProfile(const FString& TokenId, const FString& CollectionId, const FGetJsonCompleted& OnCompleted);

	/**
	 * C++ version of GetAssetProfile that returns a future containing JSON result.
	 */
	static TFuture<FLoadJsonResult> GetAssetProfile(const FString& TokenId, const FString& CollectionId);

	/**
	 * Retrieves asset links associated with a specific asset.
	 *
	 * @param TokenId The token ID of the asset.
	 * @param CollectionId The ID of the asset's collection.
	 * @param OnCompleted Callback invoked when the request finishes.
	 */
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "OnCompleted"))
	static void GetAssetLinks(const FString& TokenId, const FString& CollectionId, const FGetAssetCompleted& OnCompleted);

	/**
	 * C++ version of GetAssetLinks that returns a future with the resolved asset.
	 */
	static TFuture<FLoadAssetResult> GetAssetLinks(const FString& TokenId, const FString& CollectionId);

	/**
	 * Retrieves a list of assets using a FAssetConnection input.
	 *
	 * @param AssetsInput The connection input used to filter, paginate, or search assets.
	 * @param OnCompleted Callback invoked when the request finishes.
	 */
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "OnCompleted"))
	static void GetAssets(const FAssetConnection& AssetsInput, const FGetAssetsCompleted& OnCompleted);

	/**
	 * C++ version of GetAssets that returns a future with a list of assets.
	 */
	static TFuture<FLoadAssetsResult> GetAssets(const FAssetConnection& AssetsInput);
	
	/**
	* Makes the Assets query using the provided raw query string.
	*/
	static TFuture<FLoadAssetsResult> MakeAssetsQuery(const FString& QueryContent);
	
	/**
	* Makes the Asset query using the provided raw query string.
	*/
	static TFuture<FLoadAssetResult> MakeAssetQuery(const FString& QueryContent);
	
	/**
	 * Sends a raw GraphQL request and returns the result as a string.
	 *
	 * @param Content The raw GraphQL query string to send.
	 * @return A future resolving to the raw response string.
	 */
	static TFuture<FString> SendRequest(const FString& Content);

private:
	/**
	* Handles deserializing the response from Assets query.
	*/
	static TFuture<FLoadAssetsResult> HandleAssetsResponse(const FString& ResponseJson);
	
	/**
	* Handles deserializing the response from Asset query.
	*/
	static TFuture<FLoadAssetResult> HandleAssetResponse(const FString& ResponseJson);
	
	static TSharedPtr<FQueryNode<FAssets>> GetAssetsQueryNode(const FAssetConnection& AssetsInput);
	
	template<typename T>
	static TFuture<TArray<T>> WhenAll(TArray<TFuture<T>>& Futures)
	{
		TSharedPtr<TPromise<TArray<T>>> Promise = MakeShared<TPromise<TArray<T>>>();
		TSharedRef<TArray<T>> Results = MakeShared<TArray<T>>();
		TSharedRef<FThreadSafeCounter> Counter = MakeShared<FThreadSafeCounter>(Futures.Num());

		Results->SetNum(Futures.Num());

		if (Futures.IsEmpty())
		{
			Promise->SetValue(*Results);
			return Promise->GetFuture();
		}

		for (int32 Index = 0; Index < Futures.Num(); ++Index)
		{
			TFuture<T>& Future = Futures[Index]; // non-const ref
			Future.Next([Index, Results, Counter, Promise, Total = Futures.Num()](const T& Result)
			{
				(*Results)[Index] = Result;
				if (Counter->Decrement() == 0)
				{
					Promise->SetValue(*Results);
				}
			});
		}

		return Promise->GetFuture();
	}
};
