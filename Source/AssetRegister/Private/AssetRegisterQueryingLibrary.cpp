// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.


#include "AssetRegisterQueryingLibrary.h"

#include "AssetRegisterLog.h"
#include "AssetRegisterSettings.h"
#include "HttpModule.h"
#include "QueryRegistry.h"
#include "Interfaces/IHttpResponse.h"
#include "Schemas/Asset.h"

void UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId, const FString& CollectionId,
													const FGetJsonCompleted& OnCompleted)
{
	GetAssetProfileJson(TokenId, CollectionId).Next([OnCompleted](const FString& OutJson)
	{
		OnCompleted.ExecuteIfBound(!OutJson.IsEmpty(), OutJson);
	});
}

void UAssetRegisterQueryingLibrary::GetAsset(const FString& Addresses, const FString& CollectionId,
	const FGetJsonCompleted& OnCompleted)
{
	TSharedPtr<FQueryRegistry> QueryBuilder = MakeShareable(new FQueryRegistry());
	QueryBuilder->RegisterField<FAsset>(&FAsset::TokenId);
	QueryBuilder->RegisterField<FAsset>(&FAsset::CollectionId);
	QueryBuilder->RegisterField<FAsset>(&FAsset::Metadata, &FAssetMetadata::Id);
}

TFuture<FString> UAssetRegisterQueryingLibrary::GetAssetProfileJson(const FString& TokenId, const FString& CollectionId)
{
	TSharedPtr<TPromise<FString>> Promise = MakeShareable(new TPromise<FString>());
	TFuture<FString> Future = Promise->GetFuture();

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	FString URL;
	const UAssetRegisterSettings* Settings = GetDefault<UAssetRegisterSettings>();
	check(Settings);
	
	if (Settings)
	{
		Settings->ProfileURL;
	}
	else
	{
		UE_LOG(LogAssetRegister, Warning, TEXT("UGetAssetProfile::GetProfileJson UAssetRegisterSettings was null, returning empty string"));
		Promise->SetValue(TEXT(""));
	}
	
	const FString Query = R"(
		query Assets($assetIds: [AssetInput!]) {
		  assetsByIds(assetIds: $assetIds) {
		    profiles
		  }
		}
	)";

	const FString Content = FString::Printf(
	TEXT("{\"%s\",\"variables\":{\"assetIds\":[{\"tokenId\":\"%s\",\"collectionId\":\"%s\"}"),
		*Query.Replace(TEXT("\n"), TEXT("\\n")).Replace(TEXT("\""), TEXT("\\\"")),
		*TokenId,
		*CollectionId
	);
	
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("content-type", "application/json");
	Request->SetContentAsString(Content);
	Request->SetTimeout(60);
	
	auto RequestCallback = [Promise, TokenId, CollectionId]
	(FHttpRequestPtr Request, const FHttpResponsePtr& Response, bool bWasSuccessful) mutable
	{
		if (Response == nullptr)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("UGetAssetProfile::GetProfileJson failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			Promise->SetValue(TEXT(""));
			return;
		}
		if (bWasSuccessful && Response.IsValid())
		{
			UE_LOG(LogAssetRegister, Verbose, TEXT("UGetAssetProfile::GetProfileJson Response: %s"), *Response->GetContentAsString());
			Promise->SetValue(Response->GetContentAsString());
		}
		else
		{
			Promise->SetValue(TEXT(""));
		}
	};
	
	Request->OnProcessRequestComplete().BindLambda(RequestCallback);
	Request->ProcessRequest();

	return Future;
}
