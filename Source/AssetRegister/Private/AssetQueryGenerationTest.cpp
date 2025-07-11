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
	const auto TokenId = TEXT("2227");
	const auto CollectionId = TEXT("7668:root:17508");
	auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));
	
	AssetQuery->AddField(&FAsset::AssetType)->AddField(&FAsset::Profiles);
	AssetQuery->OnMember(&FAsset::Metadata)
		->AddField(&FAssetMetadata::RawAttributes);
	AssetQuery->OnMember(&FAsset::Ownership)->OnUnion<FNFTAssetOwnershipData>()
		->OnMember(&FNFTAssetOwnershipData::Owner)
				->AddField(&FAccount::Address);
	
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<FNFTAssetLinkData>()
		->OnArray(&FNFTAssetLinkData::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);
	
	FString ExpectedQueryString = R"(
	query {
	  asset(tokenId:"2227",collectionId:"7668:root:17508") {
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
	})";
	
	UE_LOG(LogTemp, Log, TEXT("Actual QueryString: %s"), *AssetQuery->GetQueryString());
	
	TestEqual(TEXT("Full query string should include everything"),
		QueryTestUtil::RemoveAllWhitespace(AssetQuery->GetQueryString()),
		QueryTestUtil::RemoveAllWhitespace(ExpectedQueryString));

	bool bHttpRequestCompleted = false;
	UAssetRegisterQueryingLibrary::MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([this, &bHttpRequestCompleted]
		(const FLoadAssetResult& Result)
	{
		TestTrue("Result should succeed", Result.bSuccess);
		const FAsset Asset = Result.Value;

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
		
		if (const auto Ownership = Cast<UNFTAssetOwnership>(Asset.OwnershipWrapper.Ownership))
		{
			UE_LOG(LogTemp, Log, TEXT("Owner Address: %s"), *Ownership->Data.Owner.Address);
			TestEqual("Owner Address should match", Ownership->Data.Owner.Address,
				TEXT("0xffffffff00000000000000000000000000000f59"));
		}
		else
		{
			AddError(TEXT("Failed to cast to UNFTAssetLink!"));
		}
		
		if (auto Links = Cast<UNFTAssetLink>(Asset.LinkWrapper.Links))
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
