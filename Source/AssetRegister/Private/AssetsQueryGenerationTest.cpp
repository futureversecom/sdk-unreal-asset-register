#include "AssetRegisterQueryingLibrary.h"
#include "AssetRegisterQueryBuilder.h"
#include "QueryTestUtil.h"
#include "Misc/AutomationTest.h"
#include "Schemas/Unions/NFTAssetLink.h"
#include "Schemas/Unions/NFTAssetOwnership.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AssetsQueryGenerationTest,
								"UBFEmergenceDemo.sdk_unreal_asset_register.Source.AssetRegister.Private.AssetsQueryGenerationTest",
								EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AssetsQueryGenerationTest::RunTest(const FString& Parameters)
{
	const auto CollectionId = TEXT("7668:root:17508");
	const auto Address = TEXT("0xFfffFffF000000000000000000000000000012ef");
	const auto NextPageCursor = TEXT("WyI3NjY4OnJvb3Q6MTc1MDg6NTk5fDB4ZmZmZmZmZmYwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMTJlZiJd");
	auto AssetConnectionInput = FAssetConnection();
	AssetConnectionInput.Addresses = {Address};
	AssetConnectionInput.CollectionIds = {CollectionId};
	AssetConnectionInput.First = 10;
	AssetConnectionInput.After = NextPageCursor;
	
	auto AssetsQuery = FAssetRegisterQueryBuilder::AddAssetsQuery(AssetConnectionInput);
	
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)
			->AddField(&FAsset::CollectionId);
			//->AddField(&FAsset::Profiles);
	
	AssetNode->OnMember(&FAsset::Ownership)
				->OnUnion<FNFTAssetOwnershipData>()
					->OnMember(&FNFTAssetOwnershipData::Owner)
					->AddField(&FAccount::Futurepass)
					->AddField(&FAccount::Address);
	
	// AssetNode->OnMember(&FAsset::Metadata)
	// 		->AddField(&FAssetMetadata::Properties)
	// 		->AddField(&FAssetMetadata::Attributes)
	// 		->AddField(&FAssetMetadata::RawAttributes);
	//
	// AssetNode->OnMember(&FAsset::Collection)
	// 		->AddField(&FCollection::ChainId)
	// 		->AddField(&FCollection::ChainType)
	// 		->AddField(&FCollection::Location)
	// 		->AddField(&FCollection::Name);
	//
	// AssetNode->OnMember(&FAsset::Links)
	// 	->OnUnion<FNFTAssetLinkData>()
	// 		->OnArray(&FNFTAssetLinkData::ChildLinks)
	// 			->AddField(&FLink::Path)
	// 			->OnMember(&FLink::Asset)
	// 				->AddField(&FAsset::CollectionId)
	// 				->AddField(&FAsset::TokenId);
	
	AssetsQuery->OnMember(&FAssets::PageInfo)
				->AddField(&FPageInfo::NextPage);
	
	const auto ExpectedQueryString = R"(query {
	  assets(
	    collectionIds: ["7668:root:17508"],
	    addresses: ["0xFfffFffF000000000000000000000000000012ef"],
		after: "WyI3NjY4OnJvb3Q6MTc1MDg6NTk5fDB4ZmZmZmZmZmYwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMTJlZiJd",
	    first: 10) {
	    edges {
	      node {
	        tokenId
	        collectionId
	        ownership {
	          ... on NFTAssetOwnership {
	            owner {
	              futurepass
	              address
	            }
	          }
	        }
	      }
	    }
	    pageInfo {
	      nextPage
	    }
	  }
	})";
	
	UE_LOG(LogTemp, Log, TEXT("Actual QueryString: %s"), *AssetsQuery->GetQueryString());
	
	TestEqual(TEXT("Full query string should include everything"),
		QueryTestUtil::RemoveAllWhitespace(AssetsQuery->GetQueryString()),
		QueryTestUtil::RemoveAllWhitespace(ExpectedQueryString));
	
	bool bHttpRequestCompleted = false;
	UAssetRegisterQueryingLibrary::SendRequest(AssetsQuery->GetQueryString()).Next(
		[this, Address, &bHttpRequestCompleted](const FString& OutJson)
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OutJson);
		TSharedPtr<FJsonObject> RootObject;
		FJsonSerializer::Deserialize(Reader, RootObject);

		TArray<TSharedPtr<FJsonValue>> AssetNodes;
		QueryStringUtil::FindAllFieldsRecursively(RootObject, TEXT("node"), AssetNodes);
			
		for (const auto& AssetNode : AssetNodes)
		{
			auto AssetNodeObject = AssetNode->AsObject();
			FAsset Asset;

			if (FJsonObjectConverter::JsonObjectToUStruct(AssetNodeObject.ToSharedRef(), &Asset))
			{
				// FString MetadataJson;
				// auto MetadataJsonObject = FJsonObjectConverter::UStructToJsonObject(Asset.Metadata);
				// const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetadataJson);
				// FJsonSerializer::Serialize(MetadataJsonObject.ToSharedRef(), Writer);
				// UE_LOG(LogTemp, Log, TEXT("Parsed Metadata: %s"), *MetadataJson);

				// TestEqual(TEXT("Collection Chain Id should match"), Asset.Collection.ChainId, TEXT("7668"));
				// TestEqual(TEXT("Collection Chain Type should match"), Asset.Collection.ChainType, TEXT("root"));
				// TestEqual(TEXT("Collection Chain location should match"), Asset.Collection.Location, TEXT("17508"));
				// TestEqual(TEXT("Collection Chain name should match"), Asset.Collection.Name, TEXT("Party Bear Unleashed"));
				// TestEqual(TEXT("Collection Id should match"), Asset.CollectionId, TEXT("7668:root:17508"));
				
				// const TSharedPtr<FJsonObject>* LinksObject;
				// if (AssetNodeObject->TryGetObjectField(TEXT("links"), LinksObject))
				// {
				// 	FNFTAssetLinkData NFTAssetLinkData;
				// 	if (!FJsonObjectConverter::JsonObjectToUStruct(LinksObject->ToSharedRef(), &NFTAssetLinkData))
				// 	{
				// 		AddError(TEXT("Failed to get NFTAssetLink Data!"));
				// 		return;
				// 	}
				// 	UNFTAssetLink* NFTAssetLink = NewObject<UNFTAssetLink>();
				// 	NFTAssetLink->Data = NFTAssetLinkData;
				// 	
				// 	for (FLink& ChildLink : NFTAssetLink->Data.ChildLinks)
				// 	{
				// 		FString Path = ChildLink.Path;
				// 		int32 Index = 0;
				// 		if (ChildLink.Path.FindChar('#', Index))
				// 		{
				// 			Path = ChildLink.Path.Mid(Index + 1);
				// 			Path = Path.Replace(TEXT("_accessory"), TEXT(""));
				// 		}
				// 			
				// 		ChildLink.Path = Path;
				// 	}
				// 		
				// 	Asset.LinkWrapper.Links = NFTAssetLink;
				//
				// 	if (auto ParsedLink = Cast<UNFTAssetLink>(Asset.LinkWrapper.Links))
				// 	{
				// 		for (auto ChildLink : ParsedLink->Data.ChildLinks)
				// 		{
				// 			UE_LOG(LogTemp, Log, TEXT("ChildLink Path: %s TokenId: %s CollectionId: %s"), *ChildLink.Path, *ChildLink.Asset.TokenId, *ChildLink.Asset.CollectionId);
				// 			AddErrorIfFalse(!ChildLink.Path.StartsWith(TEXT("http")),
				// 				FString::Printf(TEXT("Parsed ChildLink Path should have correct format. Actual: %s"), *ChildLink.Path));
				// 		}
				// 	}
				// 	else
				// 	{
				// 		AddError(TEXT("Failed to cast to UNFTAssetLink!"));
				// 	}
				// }
				// else
				// {
				// 	AddError(TEXT("Failed to get links field from AssetObject!"));
				// 	return;
				// }

				const TSharedPtr<FJsonObject>* OwnershipObject;
				if (AssetNodeObject->TryGetObjectField(TEXT("ownership"), OwnershipObject))
				{
					FNFTAssetOwnershipData NftAssetOwnershipData;
					if (!FJsonObjectConverter::JsonObjectToUStruct(OwnershipObject->ToSharedRef(), &NftAssetOwnershipData))
					{
						AddError(TEXT("Failed to get NFTAssetOwnership Data!"));
						return;
					}
					UNFTAssetOwnership* NFTAssetOwnership = NewObject<UNFTAssetOwnership>();
					NFTAssetOwnership->Data = NftAssetOwnershipData;

					Asset.OwnershipWrapper.Ownership = NFTAssetOwnership;
					if (auto ParsedOwnership = Cast<UNFTAssetOwnership>(Asset.OwnershipWrapper.Ownership))
					{
						UE_LOG(LogTemp, Log, TEXT("Owner Address: %s"), *ParsedOwnership->Data.Owner.Address);
						TestEqual("Owner Address should match", ParsedOwnership->Data.Owner.Address, Address);
					}
					else
					{
						AddError(TEXT("Failed to cast to UNFTAssetOwnership!"));
					}
				}
				else
				{
					AddError(TEXT("Failed to get ownership field from AssetObject!"));
					return;
				}
			}
			else
			{
				AddError(TEXT("Failed to get AssetObject from AssetEdges.Node!"));
				return;
			}
		}
	
		bHttpRequestCompleted = true;
	});

	const double StartTime = FPlatformTime::Seconds();
	while (!bHttpRequestCompleted && FPlatformTime::Seconds() - StartTime < 5.0)
	{
		FPlatformProcess::Sleep(0.01f); // Sleep to avoid freezing the main thread
	}
	
	return true;
}
