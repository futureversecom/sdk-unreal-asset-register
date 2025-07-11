// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.


#include "AssetRegisterQueryingLibrary.h"

#include "AssetRegisterLog.h"
#include "AssetRegisterSettings.h"
#include "HttpModule.h"
#include "AssetRegisterQueryBuilder.h"
#include "Interfaces/IHttpResponse.h"
#include "Schemas/Asset.h"
#include "Schemas/Unions/AssetLink.h"
#include "Schemas/Unions/NFTAssetLink.h"
#include "Schemas/Inputs/AssetInput.h"
#include "Schemas/Unions/NFTAssetOwnership.h"

void UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId, const FString& CollectionId,
	const FGetJsonCompleted& OnCompleted)
{
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));

	AssetQuery->AddField(&FAsset::Id)
		->AddField<FAsset>(&FAsset::Profiles);

	MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([OnCompleted, TokenId, CollectionId]
		(const FLoadAssetResult& Result)
	{
		FAsset Asset = Result.Value;

		if (!Result.bSuccess)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
		}

		const FString AssetProfileKey = TEXT("asset-profile");
		const FString AssetProfileURI = Asset.Profiles.Contains(AssetProfileKey) ? Asset.Profiles[AssetProfileKey] : TEXT("");
		OnCompleted.ExecuteIfBound(Asset.Profiles.Contains(AssetProfileKey),AssetProfileURI);
	});
}

TFuture<FLoadJsonResult> UAssetRegisterQueryingLibrary::GetAssetProfile(const FString& TokenId,
	const FString& CollectionId)
{
	TSharedPtr<TPromise<FLoadJsonResult>> Promise = MakeShareable(new TPromise<FLoadJsonResult>());
	
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));

	AssetQuery->AddField<FAsset>(&FAsset::Profiles);
	
	MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([Promise, TokenId, CollectionId]
	(const FLoadAssetResult& Result)
	{
		FAsset Asset = Result.Value;
		
		auto OutResult = FLoadJsonResult();
		
		if (!Result.bSuccess)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			OutResult.SetFailure();
			Promise->SetValue(OutResult);
			return;
		}
		
		const FString AssetProfileKey = TEXT("asset-profile");
		if (Asset.Profiles.Contains(AssetProfileKey))
		{
			OutResult.SetResult(Asset.Profiles[AssetProfileKey]);
		}
		else
		{
			OutResult.SetFailure();
		}

		Promise->SetValue(OutResult);
	});

	return Promise->GetFuture();
}

void UAssetRegisterQueryingLibrary::GetAssetLinks(const FString& TokenId, const FString& CollectionId,
	const FGetAssetCompleted& OnCompleted)
{
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<FNFTAssetLinkData>()
		->OnArray(&FNFTAssetLinkData::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);
	
	MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([OnCompleted, TokenId, CollectionId]
	(const FLoadAssetResult& Result)
	{
		if (!Result.bSuccess)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			OnCompleted.ExecuteIfBound(false, FAsset());
			return;
		}
		
		FAsset OutAsset = Result.Value;
		auto Links = Cast<UNFTAssetLink>(OutAsset.LinkWrapper.Links);
		if (!Links)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("[GetAssetLinks] Failed to get NFTAssetLink Data!"));
			OnCompleted.ExecuteIfBound(false, OutAsset);
			return;
		}
		
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
	->OnUnion<FNFTAssetLinkData>()
		->OnArray(&FNFTAssetLinkData::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);

	MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([Promise, TokenId, CollectionId]
	(const FLoadAssetResult& Result)
	{
		auto OutResult = FLoadAssetResult();
		if (!Result.bSuccess)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("[UGetAssetProfile] failed to load remote AssetProfile for %s:%s"), *CollectionId, *TokenId);
			OutResult.SetFailure();
			Promise->SetValue(OutResult);
			return;
		}
		
		FAsset OutAsset = Result.Value;
		auto Links = Cast<UNFTAssetLink>(OutAsset.LinkWrapper.Links);
		if (!Links)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("[GetAssetLinks] Failed to get NFTAssetLink Data!"));
			OutResult.SetFailure();
			Promise->SetValue(OutResult);
			return;
		}
		
		OutResult.SetResult(OutAsset);
		Promise->SetValue(OutResult);
	});

	return Future;
}

void UAssetRegisterQueryingLibrary::GetAssets(const FAssetConnection& AssetsInput, const FGetAssetsCompleted& OnCompleted)
{
	auto AssetsQuery = FAssetRegisterQueryBuilder::AddAssetsQuery(AssetsInput);
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)
			->AddField(&FAsset::CollectionId)
			->AddField(&FAsset::AssetType)
			->AddField(&FAsset::Profiles);
	
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::Properties)
			->AddField(&FAssetMetadata::Attributes)
			->AddField(&FAssetMetadata::RawAttributes);

	AssetNode->OnMember(&FAsset::Ownership)
			->OnUnion<FNFTAssetOwnershipData>()
				->OnMember(&FNFTAssetOwnershipData::Owner)
				->AddField(&FAccount::Address);
	
	AssetNode->OnMember(&FAsset::Collection)
			->AddField(&FCollection::ChainId)
			->AddField(&FCollection::ChainType)
			->AddField(&FCollection::Location)
			->AddField(&FCollection::Name);
	
	MakeAssetsQuery(AssetsQuery->GetQueryJsonString()).Next([OnCompleted]
	(const FLoadAssetsResult& Result)
	{
		if (!Result.bSuccess)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("UAssetRegisterQueryingLibrary::GetAssets failed to get assets"));
			OnCompleted.ExecuteIfBound(false, FAssets());
			return;
		}
		
		FAssets Assets = Result.Value;
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
			->AddField(&FAsset::AssetType)
			->AddField(&FAsset::Profiles);
	
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::Properties)
			->AddField(&FAssetMetadata::Attributes)
			->AddField(&FAssetMetadata::RawAttributes);

	AssetNode->OnMember(&FAsset::Ownership)
			->OnUnion<FNFTAssetOwnershipData>()
				->OnMember(&FNFTAssetOwnershipData::Owner)
				->AddField(&FAccount::Address);
	
	AssetNode->OnMember(&FAsset::Collection)
			->AddField(&FCollection::ChainId)
			->AddField(&FCollection::ChainType)
			->AddField(&FCollection::Location)
			->AddField(&FCollection::Name);
	
	MakeAssetsQuery(AssetsQuery->GetQueryJsonString()).Next([Promise]
	(const FLoadAssetsResult& Result)
	{
		if (!Result.bSuccess)
		{
			UE_LOG(LogAssetRegister, Warning, TEXT("UAssetRegisterQueryingLibrary::GetAssets failed to get assets"));
			auto OutResult = FLoadAssetsResult();
			OutResult.SetFailure();
			Promise->SetValue(OutResult);
			return;
		}
		Promise->SetValue(Result);
	});

	return Promise->GetFuture();
}

TFuture<FLoadAssetsResult> UAssetRegisterQueryingLibrary::MakeAssetsQuery(const FString& QueryContent)
{
	TSharedPtr<TPromise<FLoadAssetsResult>> Promise = MakeShared<TPromise<FLoadAssetsResult>>();
	
	FString URL;
	const UAssetRegisterSettings* Settings = GetDefault<UAssetRegisterSettings>();
	check(Settings);
	
	if (Settings)
	{
		URL = Settings->AssetRegisterURL;
	}
	else
	{
		UE_LOG(LogAssetRegister, Warning, TEXT("UAssetRegisterQueryingLibrary::MakeAssetsQuery UAssetRegisterSettings was null, returning empty string"));
		auto Result = FLoadAssetsResult();
		Result.SetFailure();
		Promise->SetValue(Result);
	}

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("content-type", "application/json");
	
	UE_LOG(LogAssetRegister, Verbose, TEXT("UAssetRegisterQueryingLibrary::MakeAssetsQuery Sending Request. URL: %s Content: %s"), *URL, *QueryContent);
	Request->SetContentAsString(QueryContent);
	Request->SetTimeout(60);

	auto RequestCallback = [Promise]
	(FHttpRequestPtr Request, const FHttpResponsePtr& Response, bool bWasSuccessful) mutable
	{
		auto Result = FLoadAssetsResult();
		if (Response == nullptr)
		{
			Result.SetFailure();
			Promise->SetValue(Result);
			return;
		}
		if (!bWasSuccessful || !Response.IsValid())
		{
			Result.SetFailure();
			Promise->SetValue(Result);
			return;
		}
		if (Response->GetContentAsString().IsEmpty())
		{
			Result.SetFailure();
			Promise->SetValue(Result);
			return;
		}
	
		HandleAssetsResponse(Response->GetContentAsString()).Next([Promise](const FLoadAssetsResult& LoadResult)
		{
			Promise->SetValue(LoadResult);
		});
	};
	
	Request->OnProcessRequestComplete().BindLambda(RequestCallback);
	Request->ProcessRequest();

	return Promise->GetFuture();
}

TFuture<FLoadAssetResult> UAssetRegisterQueryingLibrary::MakeAssetQuery(const FString& QueryContent)
{
	TSharedPtr<TPromise<FLoadAssetResult>> Promise = MakeShared<TPromise<FLoadAssetResult>>();
	
	FString URL;
	const UAssetRegisterSettings* Settings = GetDefault<UAssetRegisterSettings>();
	check(Settings);
	
	if (Settings)
	{
		URL = Settings->AssetRegisterURL;
	}
	else
	{
		UE_LOG(LogAssetRegister, Warning, TEXT("UAssetRegisterQueryingLibrary::MakeAssetQuery UAssetRegisterSettings was null, returning empty string"));
		auto Result = FLoadAssetResult();
		Result.SetFailure();
		Promise->SetValue(Result);
	}

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("content-type", "application/json");
	
	UE_LOG(LogAssetRegister, Verbose, TEXT("UAssetRegisterQueryingLibrary::MakeAssetQuery Sending Request. URL: %s Content: %s"), *URL, *QueryContent);
	Request->SetContentAsString(QueryContent);
	Request->SetTimeout(60);

	auto RequestCallback = [Promise]
	(FHttpRequestPtr Request, const FHttpResponsePtr& Response, bool bWasSuccessful) mutable
	{
		auto Result = FLoadAssetResult();
		if (Response == nullptr)
		{
			Result.SetFailure();
			Promise->SetValue(Result);
			return;
		}
		if (!bWasSuccessful || !Response.IsValid())
		{
			Result.SetFailure();
			Promise->SetValue(Result);
			return;
		}
		if (Response->GetContentAsString().IsEmpty())
		{
			Result.SetFailure();
			Promise->SetValue(Result);
			return;
		}
	
		HandleAssetResponse(Response->GetContentAsString()).Next([Promise](const FLoadAssetResult& LoadResult)
		{
			Promise->SetValue(LoadResult);
		});
	};
	
	Request->OnProcessRequestComplete().BindLambda(RequestCallback);
	Request->ProcessRequest();

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

TFuture<FLoadAssetsResult> UAssetRegisterQueryingLibrary::HandleAssetsResponse(const FString& ResponseJson)
{
	UE_LOG(LogAssetRegister, Verbose, TEXT("UAssetRegisterQueryingLibrary::HandleAssetsResponse Attempting to handle Response: %s"), *ResponseJson);
	
	TSharedPtr<TPromise<FLoadAssetsResult>> Promise = MakeShared<TPromise<FLoadAssetsResult>>();

	FAssets OutAssets = FAssets();

	FAssets Assets;
	if (!QueryStringUtil::TryGetModel(ResponseJson, Assets))
	{
		UE_LOG(LogAssetRegister, Error, TEXT("Failed to get Assets Object from Json: %s!"), *ResponseJson);
		auto OutResult = FLoadAssetsResult();
		OutResult.SetFailure();
		Promise->SetValue(OutResult);
		return Promise->GetFuture();
	}
	OutAssets.PageInfo = Assets.PageInfo;
	OutAssets.Total = Assets.Total;
	
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJson);
	TSharedPtr<FJsonObject> RootObject;
	FJsonSerializer::Deserialize(Reader, RootObject);

	TArray<TSharedPtr<FJsonValue>> AssetNodes;
	QueryStringUtil::FindAllFieldsRecursively(RootObject, TEXT("node"), AssetNodes);
	
	TArray<TFuture<FLoadAssetResult>> LoadAssetFutures;
	TSharedPtr<TArray<FAsset>> LoadedAssets = MakeShared<TArray<FAsset>>();
	
	for (const auto& AssetNode : AssetNodes)
	{
		auto AssetNodeObject = AssetNode->AsObject();

		// wrap json under "asset" field
		TSharedRef<FJsonObject> AssetBody = MakeShared<FJsonObject>();
		AssetBody->SetObjectField("asset", AssetNodeObject);
		
		FString AssetJsonString;
		FJsonObjectWrapper ObjectWrapper;
		ObjectWrapper.JsonObject = AssetBody;
		ObjectWrapper.JsonObjectToString(AssetJsonString);
		
		TFuture<FLoadAssetResult> LoadAssetFuture = HandleAssetResponse(AssetJsonString).Next(
		[LoadedAssets, AssetNodeObject](const FLoadAssetResult& AssetResult)
		{
			if (AssetResult.bSuccess)
			{
				auto Asset = AssetResult.Value;
				Asset.OriginalJsonData.JsonObject = AssetNodeObject;
				LoadedAssets->Add(Asset);
			}

			return AssetResult;
		});
		LoadAssetFutures.Add(MoveTemp(LoadAssetFuture));
	}
	
	WhenAll(LoadAssetFutures).Next([Promise, &OutAssets, LoadedAssets](const TArray<FLoadAssetResult>& Results)
	{
		for (const FAsset& Asset : *LoadedAssets)
		{
			FAssetEdge Edge;
			Edge.Node = Asset;
			OutAssets.Edges.Add(Edge);
		}
		auto OutResult = FLoadAssetsResult();
		OutResult.SetResult(OutAssets);
		Promise->SetValue(OutResult);
	});
	
	return Promise->GetFuture();
}

TFuture<FLoadAssetResult> UAssetRegisterQueryingLibrary::HandleAssetResponse(const FString& ResponseJson)
{
	UE_LOG(LogAssetRegister, Verbose, TEXT("UAssetRegisterQueryingLibrary::HandleAssetResponse Attempting to handle Response: %s"), *ResponseJson);
	
	TSharedPtr<TPromise<FLoadAssetResult>> Promise = MakeShared<TPromise<FLoadAssetResult>>();
	auto Result = FLoadAssetResult();

	FAsset OutAsset;
	if (!QueryStringUtil::TryGetModel(ResponseJson, OutAsset))
	{
		UE_LOG(LogAssetRegister, Error, TEXT("Failed to get Asset Object from Json: %s!"), *ResponseJson);
		Result.SetFailure();
		Promise->SetValue(Result);
		return Promise->GetFuture();
	}

	// manually try to get FNFTAssetOwnershipData
	FNFTAssetOwnershipData NFTOwnershipData;
	if (QueryStringUtil::TryGetModelField<FAsset>(ResponseJson, TEXT("ownership"), NFTOwnershipData))
	{
		UNFTAssetOwnership* NFTOwnership = NewObject<UNFTAssetOwnership>();
		NFTOwnership->Data = NFTOwnershipData;
		
		OutAsset.OwnershipWrapper.Ownership = NFTOwnership;
	}
	
	// manually try to get FNFTAssetLinkData
	FNFTAssetLinkData NFTAssetLinkData;
	if (QueryStringUtil::TryGetModelField<FAsset>(ResponseJson, TEXT("links"), NFTAssetLinkData))
	{
		UNFTAssetLink* NFTAssetLink = NewObject<UNFTAssetLink>();
		NFTAssetLink->Data = NFTAssetLinkData;
				
		for (FLink& ChildLink : NFTAssetLink->Data.ChildLinks)
		{
			FString Path = ChildLink.Path;
			int32 Index = 0;
			if (ChildLink.Path.FindChar('#', Index))
			{
				Path = ChildLink.Path.Mid(Index + 1);
				Path = Path.Replace(TEXT("_accessory"), TEXT(""));
			}
					
			ChildLink.Path = Path;
		}
				
		OutAsset.LinkWrapper.Links = NFTAssetLink;
	}

	Result.SetResult(OutAsset);
	Promise->SetValue(Result);
	
	return Promise->GetFuture();
}