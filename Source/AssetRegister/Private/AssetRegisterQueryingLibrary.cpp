// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.


#include "AssetRegisterQueryingLibrary.h"

#include "AssetRegisterLog.h"
#include "AssetRegisterSettings.h"
#include "HttpModule.h"
#include "AssetRegisterQueryBuilder.h"
#include "Interfaces/IHttpResponse.h"
#include "Schemas/Asset.h"
#include "Schemas/AssetLink.h"
#include "Schemas/NFTAssetLink.h"
#include "Schemas/Inputs/AssetInput.h"

void UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId, const FString& CollectionId,
	const FGetJsonCompleted& OnCompleted)
{
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));

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

TFuture<FLoadJsonResult> UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId,
	const FString& CollectionId)
{
	TSharedPtr<TPromise<FLoadJsonResult>> Promise = MakeShareable(new TPromise<FLoadJsonResult>());
	TFuture<FLoadJsonResult> Future = Promise->GetFuture();
	
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));

	AssetQuery->AddField<FAsset>(&FAsset::Profiles);
	
	SendRequest(AssetQuery->GetQueryString()).Next([Promise, TokenId, CollectionId](const FString& OutJson)
	{
		auto Result = FLoadJsonResult();
		if (OutJson.IsEmpty())
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			Result.SetFailure();
			Promise->SetValue(Result);
		}
		
		FAsset OutAsset;
		if (!QueryStringUtil::TryGetModel(OutJson, OutAsset))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to get Asset Object from Json: %s!"), *OutJson);
			Result.SetFailure();
			Promise->SetValue(Result);
		}
		
		if (OutAsset.Profiles.Contains(TEXT("asset-profile")))
		{
			Result.SetResult(OutAsset.Profiles.FindOrAdd(TEXT("asset-profile")));
		}
		else
		{
			Result.SetFailure();
		}

		Promise->SetValue(Result);
	});

	return Future;
}

void UAssetRegisterQueryingLibrary::GetAssetLinks(const FString& TokenId, const FString& CollectionId,
	const FGetAssetCompleted& OnCompleted)
{
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<FNFTAssetLinkData, FAssetLinkData>()
		->OnArray(&FNFTAssetLinkData::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);
	
	SendRequest(AssetQuery->GetQueryString()).Next([OnCompleted, TokenId, CollectionId](const FString& OutJson)
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
				
		for (FLink& ChildLink : NFTAssetLinkData.ChildLinks)
		{
			FString Path = ChildLink.Path;
			int32 Index = 0;
			if (ChildLink.Path.FindChar('#', Index))
			{
				Path = ChildLink.Path.Mid(Index + 1);
				Path = Path.Replace(TEXT("_accessory"), TEXT(""));
			}
			
			ChildLink.Path = Path;
			NFTAssetLink->ChildLinks.Add(ChildLink);
		}
		
		OutAsset.LinkWrapper.Links = NFTAssetLink;
		
		OnCompleted.ExecuteIfBound(true, OutAsset);
	});
}

TFuture<FLoadAssetResult> UAssetRegisterQueryingLibrary::GetAssetLinks(const FString& TokenId,
	const FString& CollectionId)
{
	TSharedPtr<TPromise<FLoadAssetResult>> Promise = MakeShareable(new TPromise<FLoadAssetResult>());
	TFuture<FLoadAssetResult> Future = Promise->GetFuture();
	
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<FNFTAssetLinkData, FAssetLinkData>()
		->OnArray(&FNFTAssetLinkData::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);
	
	SendRequest(AssetQuery->GetQueryString()).Next([Promise, TokenId, CollectionId](const FString& OutJson)
	{
		auto Result = FLoadAssetResult();
		if (OutJson.IsEmpty())
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			Result.SetFailure();
			Promise->SetValue(Result);
		}
		
		FAsset OutAsset;
		const bool bGotModel = QueryStringUtil::TryGetModel(OutJson, OutAsset);
		if (!bGotModel)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to get Asset Object from Json: %s!"), *OutJson);
			Result.SetFailure();
			Promise->SetValue(Result);
		}
		
		FNFTAssetLinkData NFTAssetLinkData;
		const bool bGotLinks = QueryStringUtil::TryGetModelField<FAsset, FNFTAssetLinkData>(OutJson, TEXT("links"), NFTAssetLinkData);
		if (!bGotLinks)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("[GetAssetLinks] Failed to get NFTAssetLink Data!"));
			Result.SetFailure();
			Promise->SetValue(Result);
		}
		
		UNFTAssetLink* NFTAssetLink = NewObject<UNFTAssetLink>();
				
		for (FLink& ChildLink : NFTAssetLinkData.ChildLinks)
		{
			FString Path = ChildLink.Path;
			int32 Index = 0;
			if (ChildLink.Path.FindChar('#', Index))
			{
				Path = ChildLink.Path.Mid(Index + 1);
				Path = Path.Replace(TEXT("_accessory"), TEXT(""));
			}
			
			ChildLink.Path = Path;
			NFTAssetLink->ChildLinks.Add(ChildLink);
		}
		
		OutAsset.LinkWrapper.Links = NFTAssetLink;
		
		Result.SetResult(OutAsset);
		Promise->SetValue(Result);
	});

	return Future;
}

void UAssetRegisterQueryingLibrary::GetAssets(const FAssetConnection& AssetsInput, const FGetAssetsCompleted& OnCompleted)
{
	auto AssetsQuery = FAssetRegisterQueryBuilder::AddAssetsQuery(AssetsInput);
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)
			->AddField(&FAsset::CollectionId)
			->AddField(&FAsset::Profiles);
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::Properties)
			->AddField(&FAssetMetadata::Attributes)
			->AddField(&FAssetMetadata::RawAttributes);
	
	AssetNode->OnMember(&FAsset::Collection)
			->AddField(&FCollection::ChainId)
			->AddField(&FCollection::ChainType)
			->AddField(&FCollection::Location)
			->AddField(&FCollection::Name);
	
	SendRequest(AssetsQuery->GetQueryString()).Next([OnCompleted](const FString& OutJson)
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OutJson);
		TSharedPtr<FJsonObject> RootObject;
		FJsonSerializer::Deserialize(Reader, RootObject);

		TArray<TSharedPtr<FJsonValue>> AssetNodes;
		QueryStringUtil::FindAllFieldsRecursively(RootObject, TEXT("node"), AssetNodes);

		FAssets Assets;
		for (const auto& AssetNode : AssetNodes)
		{
			auto AssetNodeObject = AssetNode->AsObject();
			FAsset Asset;

			if (!FJsonObjectConverter::JsonObjectToUStruct(AssetNodeObject.ToSharedRef(), &Asset))
			{
				UE_LOG(LogAssetRegister, Warning, TEXT("UAssetRegisterQueryingLibrary::GetAssets Failed to convert Json to Asset!"));
				LogJsonString(AssetNodeObject);
				continue;
			}
			
			Asset.OriginalJsonData.JsonObject = AssetNodeObject;
			Assets.Edges.Add(FAssetEdge(Asset));
		}
		OnCompleted.ExecuteIfBound(true, Assets);
	});
}

TFuture<FLoadAssetsResult> UAssetRegisterQueryingLibrary::GetAssets(const FAssetConnection& AssetsInput)
{
	TSharedPtr<TPromise<FLoadAssetsResult>> Promise = MakeShared<TPromise<FLoadAssetsResult>>();
	
	auto AssetsQuery = FAssetRegisterQueryBuilder::AddAssetsQuery(AssetsInput);
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)
			->AddField(&FAsset::CollectionId)
			->AddField(&FAsset::Profiles);
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::Properties)
			->AddField(&FAssetMetadata::Attributes)
			->AddField(&FAssetMetadata::RawAttributes);
	
	AssetNode->OnMember(&FAsset::Collection)
			->AddField(&FCollection::ChainId)
			->AddField(&FCollection::ChainType)
			->AddField(&FCollection::Location)
			->AddField(&FCollection::Name);
	
	SendRequest(AssetsQuery->GetQueryString()).Next([Promise](const FString& OutJson)
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OutJson);
		TSharedPtr<FJsonObject> RootObject;
		FJsonSerializer::Deserialize(Reader, RootObject);

		TArray<TSharedPtr<FJsonValue>> AssetNodes;
		QueryStringUtil::FindAllFieldsRecursively(RootObject, TEXT("node"), AssetNodes);

		FAssets Assets;
		for (const auto& AssetNode : AssetNodes)
		{
			auto AssetNodeObject = AssetNode->AsObject();
			FAsset Asset;

			if (!FJsonObjectConverter::JsonObjectToUStruct(AssetNodeObject.ToSharedRef(), &Asset))
			{
				UE_LOG(LogAssetRegister, Warning, TEXT("UAssetRegisterQueryingLibrary::GetAssets Failed to convert Json to Asset!"));
				LogJsonString(AssetNodeObject);
				continue;
			}
				
			Asset.OriginalJsonData.JsonObject = AssetNodeObject;
			Assets.Edges.Add(FAssetEdge(Asset));
		}
		auto OutResult = FLoadAssetsResult();
		OutResult.SetResult(Assets);
		Promise->SetValue(OutResult);
	});

	return Promise->GetFuture();
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
		URL = Settings->AssetRegisterURL;
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

void UAssetRegisterQueryingLibrary::LogJsonString(const TSharedPtr<FJsonObject>& JsonObject)
{
	FString JsonString;

	FJsonObjectWrapper ObjectWrapper;
	ObjectWrapper.JsonObject = JsonObject;
	ObjectWrapper.JsonObjectToString(JsonString);
	
	UE_LOG(LogAssetRegister, Warning, TEXT("Used JsonString: %s"), *JsonString);
}
