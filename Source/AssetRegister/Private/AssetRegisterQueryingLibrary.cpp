// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.


#include "AssetRegisterQueryingLibrary.h"

#include "AssetRegisterLog.h"
#include "AssetRegisterSettings.h"
#include "HttpModule.h"
#include "QueryRegistry.h"
#include "Interfaces/IHttpResponse.h"
#include "Schemas/Asset.h"
#include "Schemas/Inputs/AssetInput.h"

void UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId, const FString& CollectionId,
	const FGetJsonCompleted& OnCompleted)
{
	TSharedPtr<FQueryRegistry<FAsset>> QueryRegistry = MakeShareable(new FQueryRegistry<FAsset>());
	const auto Argument = FAssetInput(TokenId, CollectionId);
	QueryRegistry->RegisterArgument(Argument);
	
	auto AssetProfileInput = FAsset();
	AssetProfileInput.Profiles.Add(TEXT("1"), TEXT("1"));
	QueryRegistry->RegisterField(AssetProfileInput);
	
	FString URL;
	const UAssetRegisterSettings* Settings = GetDefault<UAssetRegisterSettings>();
	check(Settings);
	
	if (Settings)
	{
		URL = Settings->ProfileURL;
	}
	else
	{
		UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] UAssetRegisterSettings was null, returning empty string"));
		OnCompleted.ExecuteIfBound(false, "");
		return;
	}
	
	SendRequest(URL, QueryRegistry->GetQueryString()).Next([OnCompleted, TokenId, CollectionId, QueryRegistry](const FString& OutJson)
	{
		if (OutJson.IsEmpty())
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			OnCompleted.ExecuteIfBound(false, TEXT(""));
			return;
		}
		
		FAsset OutAsset;
		if (QueryRegistry->TryGetModel(OutJson, OutAsset))
		{
			OnCompleted.ExecuteIfBound(OutAsset.Profiles.Contains(TEXT("asset-profile")),
				OutAsset.Profiles.FindOrAdd(TEXT("asset-profile")));
			return;
		}
		
		OnCompleted.ExecuteIfBound(false, TEXT(""));
	});
}

void UAssetRegisterQueryingLibrary::GetAssets(const TArray<FString>& Addresses, const TArray<FString>& Collections,
	const FGetJsonCompleted& OnCompleted)
{
	// TSharedPtr<FQueryRegistry<FAsset>> QueryBuilder = MakeShareable(new FQueryRegistry<FAsset>());
	// QueryBuilder->RegisterField<FAsset>(&FAsset::TokenId);
	// QueryBuilder->RegisterField<FAsset>(&FAsset::CollectionId);
	// QueryBuilder->RegisterField<FAsset>(&FAsset::Metadata, &FAssetMetadata::Id);
}

TFuture<FString> UAssetRegisterQueryingLibrary::SendRequest(const FString& URL, const FString& Content)
{
	TSharedPtr<TPromise<FString>> Promise = MakeShareable(new TPromise<FString>());
	TFuture<FString> Future = Promise->GetFuture();

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("content-type", "application/json");
	Request->SetContentAsString(Content);
	Request->SetTimeout(60);
	
	auto RequestCallback = [Promise]
	(FHttpRequestPtr Request, const FHttpResponsePtr& Response, bool bWasSuccessful) mutable
	{
		if (Response == nullptr)
		{
			Promise->SetValue(TEXT(""));
			return;
		}
		if (bWasSuccessful && Response.IsValid())
		{
			UE_LOG(LogAssetRegister, Verbose, TEXT("UAssetRegisterQueryingLibrary::SendRequest Response: %s"), *Response->GetContentAsString());
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
