// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.


#include "AssetRegisterQueryingLibrary.h"

#include "AssetRegisterLog.h"
#include "AssetRegisterSettings.h"
#include "HttpModule.h"
#include "QueryBuilder.h"
#include "Interfaces/IHttpResponse.h"
#include "Schemas/Asset.h"
#include "Schemas/AssetLink.h"
#include "Schemas/Inputs/AssetInput.h"

void UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId, const FString& CollectionId,
	const FGetJsonCompleted& OnCompleted)
{
	auto AssetQuery = FAssetRegister::AddAssetQuery(FAssetInput(TokenId, CollectionId));

	AssetQuery->AddField(&FAsset::Id)
		->AddField<FAsset>(&FAsset::Profiles);
	
	SendRequest(AssetQuery->GetQueryString()).Next([OnCompleted, TokenId, CollectionId](const FString& OutJson)
	{
		if (OutJson.IsEmpty())
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			OnCompleted.ExecuteIfBound(false, TEXT(""));
			return;
		}
		
		FAsset OutAsset;
		if (!QueryStringUtil::TryGetModel(OutJson, OutAsset))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to get Asset Object from Json: %s!"), *OutJson);
			OnCompleted.ExecuteIfBound(false, TEXT(""));
		}
		
		OnCompleted.ExecuteIfBound(OutAsset.Profiles.Contains(TEXT("asset-profile")),
				OutAsset.Profiles.FindOrAdd(TEXT("asset-profile")));
	});
}

void UAssetRegisterQueryingLibrary::GetAssetLinks(const FString& TokenId, const FString& CollectionId,
	const FGetAssetCompleted& OnCompleted)
{
	auto AssetQuery = FAssetRegister::AddAssetQuery(FAssetInput(TokenId, CollectionId));
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<UNFTAssetLink, UAssetLink>()
		->OnArray(&UNFTAssetLink::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);

	FString ModelName = AssetQuery->GetModelString();
	SendRequest(AssetQuery->GetQueryString()).Next([OnCompleted, TokenId, CollectionId, ModelName](const FString& OutJson)
	{
		if (OutJson.IsEmpty())
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			OnCompleted.ExecuteIfBound(false, FAsset());
			return;
		}
		
		FAsset OutAsset;
		const bool bGotModel = QueryStringUtil::TryGetModel(OutJson, OutAsset);
		if (!bGotModel)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to get Asset Object from Json: %s!"), *OutJson);
			OnCompleted.ExecuteIfBound(false, OutAsset);
			return;
		}
		
		FNFTAssetLinkData NFTAssetLinkData;
		const bool bGotLinks = QueryStringUtil::TryGetModelField<FAsset, FNFTAssetLinkData>(OutJson, TEXT("links"), NFTAssetLinkData);
		if (!bGotLinks)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("[GetAssetLinks] Failed to get NFTAssetLink Data!"));
			OnCompleted.ExecuteIfBound(false, OutAsset);
		}
		
		UNFTAssetLink* NFTAssetLink = NewObject<UNFTAssetLink>();
		NFTAssetLink->ChildLinks.Append(NFTAssetLinkData.ChildLinks);
		
		for (auto ChildLink : NFTAssetLink->ChildLinks)
		{
			UE_LOG(LogAssetRegister, Verbose, TEXT("Parsed ChildLink Path: %s TokenId: %s CollectionId: %s"), *ChildLink.Path, *ChildLink.Asset.TokenId, *ChildLink.Asset.CollectionId);
		}
		
		OnCompleted.ExecuteIfBound(true, OutAsset);
	});
}

void UAssetRegisterQueryingLibrary::GetAssets(const TArray<FString>& Addresses, const TArray<FString>& Collections,
	const FGetJsonCompleted& OnCompleted)
{
	//TSharedPtr<FQuery<FAsset>> QueryBuilder = MakeShareable(new FQuery<FAsset>());
	// QueryBuilder->RegisterField<FAsset>(&FAsset::TokenId);
	// QueryBuilder->RegisterField<FAsset>(&FAsset::CollectionId);
	// QueryBuilder->RegisterField<FAsset>(&FAsset::Metadata, &FAssetMetadata::Id);
}

TFuture<FString> UAssetRegisterQueryingLibrary::SendRequest(const FString& RawContent)
{
	TSharedPtr<TPromise<FString>> Promise = MakeShareable(new TPromise<FString>());
	TFuture<FString> Future = Promise->GetFuture();

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
		Promise->SetValue(TEXT(""));
	}
	
	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("content-type", "application/json");

	TSharedRef<FJsonObject> JsonBody = MakeShared<FJsonObject>();
	JsonBody->SetStringField("query", RawContent);

	FString Content;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonBody, Writer);

	UE_LOG(LogAssetRegister, Verbose, TEXT("UAssetRegisterQueryingLibrary::Sending Request. URL: %s Content: %s"), *URL, *Content);
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
