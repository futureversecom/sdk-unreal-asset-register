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
	const FString CollectionId = TEXT("7668:root:17508");
	const FString Address = TEXT("0xFfffFffF000000000000000000000000000012ef");
	const FString NextPageCursor = TEXT("WyI3NjY4OnJvb3Q6MTc1MDg6NTk5fDB4ZmZmZmZmZmYwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMTJlZiJd");
	const FString ProfileKey = TEXT("staging");
	const int NumberOfItems = 10;
	auto AssetConnectionInput = FAssetConnection();
	AssetConnectionInput.Addresses = {Address};
	AssetConnectionInput.CollectionIds = {CollectionId};
	AssetConnectionInput.First = NumberOfItems;
	AssetConnectionInput.After = NextPageCursor;
	
	auto AssetsQuery = FAssetRegisterQueryBuilder::AddAssetsQuery(AssetConnectionInput);
	
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)
			->AddField(&FAsset::CollectionId)
			->AddField(&FAsset::AssetType);
	AssetNode->OnMember(&FAsset::Profiles)->AddArgument(TEXT("key"), ProfileKey);
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::RawAttributes);
	
	AssetNode->OnMember(&FAsset::Ownership)
				->OnUnion<FNFTAssetOwnership>()
					->OnMember(&FNFTAssetOwnership::Owner)
					->AddField(&FAccount::Address);
	
	AssetNode->OnMember(&FAsset::Links)
		->OnUnion<FNFTAssetLink>()
			->OnArray(&FNFTAssetLink::ChildLinks)
				->AddField(&FLink::Path)
				->OnMember(&FLink::Asset)
					->AddField(&FAsset::CollectionId)
					->AddField(&FAsset::TokenId);
	
	AssetsQuery->OnMember(&FAssets::PageInfo)
				->AddField(&FPageInfo::NextPage);
	
	const auto ExpectedQueryString = R"(query {
	  assets(
	    collectionIds: [")" + CollectionId  + R"("],
	    addresses: [")" + Address  + R"("],
	    after: ")" + NextPageCursor  + R"(",
	    first: )" + FString::FromInt(NumberOfItems)  + R"() {
	    edges {
	      node {
	        tokenId
	        collectionId
	        assetType
	        profiles(key:")" + ProfileKey  + R"(")
	        metadata {
	          rawAttributes
	        }
	        ownership {
	          ... on NFTAssetOwnership {
	            owner {
	              address
	            }
	          }
	        }
	        links {
	          ... on NFTAssetLink {
	            childLinks {
	              path
	              asset {
	                collectionId
	                tokenId
	              }
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
	UAssetRegisterQueryingLibrary::MakeAssetsQuery(AssetsQuery->GetQueryJsonString()).Next(
		[this, ProfileKey, Address, &bHttpRequestCompleted](const FLoadAssetsResult& Result)
	{
		TestTrue("Result should succeed", Result.bSuccess);
		const FAssets Assets = Result.Value;
			
		for (const auto& AssetEdge : Assets.Edges)
		{
			const FAsset Asset = AssetEdge.Node;
			UE_LOG(LogTemp, Log, TEXT("Asset TokenId: %s"), *Asset.TokenId); 
			UE_LOG(LogTemp, Log, TEXT("Asset CollectionId: %s"), *Asset.CollectionId); 
			AddErrorIfFalse(!Asset.TokenId.IsEmpty(), TEXT("TokenId shouldn't be empty!"));
			AddErrorIfFalse(!Asset.CollectionId.IsEmpty(), TEXT("CollectionId shouldn't be empty!"));
			
			if (Asset.Profiles.Contains(ProfileKey))
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed AssetProfile: %s"), *Asset.Profiles[ProfileKey]);
			}
			else
			{
				AddError(FString::Printf(TEXT("Failed to get to AssetProfile using Key: %s!"), *ProfileKey));
			}
			
			FString MetadataJson;
			auto MetadataJsonObject = FJsonObjectConverter::UStructToJsonObject(Asset.Metadata);
			const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetadataJson);
			FJsonSerializer::Serialize(MetadataJsonObject.ToSharedRef(), Writer);
			UE_LOG(LogTemp, Log, TEXT("Parsed Metadata: %s"), *MetadataJson);

			if (const auto Ownership = Cast<UNFTAssetOwnershipObject>(Asset.OwnershipWrapper.Ownership))
			{
				UE_LOG(LogTemp, Log, TEXT("Owner Address: %s"), *Ownership->Data.Owner.Address);
				TestEqual("Owner Address should match", Ownership->Data.Owner.Address,
					Address);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to get UNFTAssetOwnership from Asset: %s:%s"), *Asset.CollectionId, *Asset.TokenId);
			}

			if (auto Links = Cast<UNFTAssetLinkObject>(Asset.LinkWrapper.Links))
			{
				for (auto ChildLink : Links->Data.ChildLinks)
				{
					UE_LOG(LogTemp, Log, TEXT("ChildLink Path: %s TokenId: %s CollectionId: %s"), *ChildLink.Path, *ChildLink.Asset.TokenId, *ChildLink.Asset.CollectionId);
					AddErrorIfFalse(!ChildLink.Path.StartsWith(TEXT("http")),
							FString::Printf(TEXT("Parsed ChildLink Path should have correct format. Actual: %s"), *ChildLink.Path));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to get UNFTAssetLink from Asset: %s:%s"), *Asset.CollectionId, *Asset.TokenId);
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
