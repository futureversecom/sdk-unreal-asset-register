#include "AssetRegisterQueryingLibrary.h"
#include "AssetRegisterQueryBuilder.h"
#include "QueryTestUtil.h"
#include "Misc/AutomationTest.h"
#include "Schemas/Asset.h"
#include "Schemas/Unions/AssetLink.h"
#include "Schemas/Unions/NFTAssetLink.h"
#include "Schemas/Unions/NFTAssetOwnership.h"
#include "Schemas/Inputs/AssetInput.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AssetQueryGenerationTest,
								"UBFEmergenceDemo.sdk_unreal_asset_register.Source.AssetRegister.Private.AssetQueryGenerationTest",
								EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AssetQueryGenerationTest::RunTest(const FString& Parameters)
{
	const FString TokenId = TEXT("105");
	const FString CollectionId = TEXT("7668:root:17508");
	const FString ProfileKey = TEXT("staging");
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));
	AssetQuery->AddField(&FAsset::AssetType);
	AssetQuery->OnMember(&FAsset::Profiles)->AddArgument(TEXT("key"), ProfileKey);
	AssetQuery->OnMember(&FAsset::Metadata)
		->AddField(&FAssetMetadata::RawAttributes)
		->AddField(&FAssetMetadata::Properties);
	AssetQuery->OnMember(&FAsset::Ownership)->OnUnion<FNFTAssetOwnership>()
		->OnMember(&FNFTAssetOwnership::Owner)
				->AddField(&FAccount::Address);
	
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<FNFTAssetLink>()
		->OnArray(&FNFTAssetLink::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);
	
	FString ExpectedQueryString = R"(
	query {
	  asset(tokenId:")" + TokenId  + R"(",collectionId:")" + CollectionId +  R"(") {
	    assetType
		profiles(key:")" + ProfileKey  + R"(")
	    metadata {
	     rawAttributes
		 properties
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
	})";
	
	UE_LOG(LogTemp, Log, TEXT("Actual QueryString: %s"), *AssetQuery->GetQueryString());
	
	TestEqual(TEXT("Full query string should include everything"),
		QueryTestUtil::RemoveAllWhitespace(AssetQuery->GetQueryString()),
		QueryTestUtil::RemoveAllWhitespace(ExpectedQueryString));

	bool bHttpRequestCompleted = false;
	UAssetRegisterQueryingLibrary::MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([this, ProfileKey, &bHttpRequestCompleted]
		(const FLoadAssetResult& Result)
	{
		TestTrue("Result should succeed", Result.bSuccess);
		const FAsset Asset = Result.Value;
		
		if (Asset.Profiles.Contains(ProfileKey))
		{
			UE_LOG(LogTemp, Log, TEXT("Parsed AssetProfile: %s"), *Asset.Profiles[ProfileKey]);
		}
		else
		{
			AddError(FString::Printf(TEXT("Failed to get to AssetProfile using Key: %s!"), *ProfileKey));
		}
		
		FString MetadataJson;
		Asset.Metadata.Properties.JsonObjectToString(MetadataJson);
		UE_LOG(LogTemp, Log, TEXT("Parsed Metadata: %s"), *MetadataJson);
		
		if (const auto Ownership = Cast<UNFTAssetOwnershipObject>(Asset.OwnershipWrapper.Ownership))
		{
			UE_LOG(LogTemp, Log, TEXT("Owner Address: %s"), *Ownership->Data.Owner.Address);
			// TestEqual("Owner Address should match", Ownership->Data.Owner.Address,
			// 	TEXT("0xffffffff00000000000000000000000000000f59"));
		}
		else
		{
			AddError(TEXT("Failed to cast to UNFTAssetLink!"));
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
			AddError(TEXT("Failed to cast to UNFTAssetLink!"));
		}
		bHttpRequestCompleted = true;
	});

	const double StartTime = FPlatformTime::Seconds();
	while (!bHttpRequestCompleted && FPlatformTime::Seconds() - StartTime < 6.0)
	{
		FPlatformProcess::Sleep(0.01f); // Sleep to avoid freezing the main thread
	}
	
	return true;
}
