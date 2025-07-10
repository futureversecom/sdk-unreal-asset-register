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
	
	AssetQuery->AddField(&FAsset::AssetType);
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
	UAssetRegisterQueryingLibrary::SendRequest(AssetQuery->GetQueryString()).Next(
		[this, &bHttpRequestCompleted](const FString& OutJson)
	{
		FAsset OutAsset;
		if (QueryStringUtil::TryGetModel(OutJson, OutAsset))
		{
			UE_LOG(LogTemp, Log, TEXT("AssetType: %s "), *StaticEnum<EAssetType>()->GetNameStringByValue((int64)OutAsset.AssetType));
			TestEqual("AssetType should match", OutAsset.AssetType, EAssetType::ERC721);
	
			FNFTAssetOwnershipData NFTOwnershipData;
			if (QueryStringUtil::TryGetModelField<FAsset, FNFTAssetOwnershipData>(OutJson, TEXT("ownership"), NFTOwnershipData))
			{
				UNFTAssetOwnership* NFTOwnership = NewObject<UNFTAssetOwnership>();
				NFTOwnership->Data = NFTOwnershipData;
			
				UE_LOG(LogTemp, Log, TEXT("OwnerAddress: %s "), *NFTOwnership->Data.Owner.Address);
				TestEqual(TEXT("Owner Address should match"), NFTOwnership->Data.Owner.Address, TEXT("0xFFfffffF00000000000000000000000000000f59"));
			}
			else
			{
				AddError("Failed to get NFTOwnershipData!");
			}
			
			FNFTAssetLinkData NFTAssetLinkData;
			const bool bGotLinks = QueryStringUtil::TryGetModelField<FAsset, FNFTAssetLinkData>(OutJson, TEXT("links"), NFTAssetLinkData);
			if (!bGotLinks)
			{
				AddError(TEXT("[GetAssetLinks] Failed to get NFTAssetLink Data!"));
			}
			
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
			
			if (auto ParsedLink = Cast<UNFTAssetLink>(OutAsset.LinkWrapper.Links))
			{
				for (auto ChildLink : ParsedLink->Data.ChildLinks)
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
		}
		else
		{
			AddError(FString::Printf(TEXT("Failed to get Asset Object from Json: %s!"), *OutJson));
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
