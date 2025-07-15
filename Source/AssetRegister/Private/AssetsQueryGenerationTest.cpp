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
			->AddField(&FAsset::CollectionId)
			->AddField(&FAsset::AssetType)
			->AddField(&FAsset::Profiles);
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
	    collectionIds: ["7668:root:17508"],
	    addresses: ["0xFfffFffF000000000000000000000000000012ef"],
	    after: "WyI3NjY4OnJvb3Q6MTc1MDg6NTk5fDB4ZmZmZmZmZmYwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMTJlZiJd",
	    first: 10) {
	    edges {
	      node {
	        tokenId
	        collectionId
	        assetType
	        profiles
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
		[this, &bHttpRequestCompleted](const FLoadAssetsResult& Result)
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
			
			const FString AssetProfileKey =TEXT("asset-profile");
			if (Asset.Profiles.Contains(AssetProfileKey))
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed AssetProfile: %s"), *Asset.Profiles[AssetProfileKey]);
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
					TEXT("0xFfffFffF000000000000000000000000000012ef"));
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
