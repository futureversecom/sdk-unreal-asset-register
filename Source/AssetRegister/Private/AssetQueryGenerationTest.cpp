#include "AssetRegisterQueryingLibrary.h"
#include "QueryBuilder.h"
#include "QueryTestUtil.h"
#include "Misc/AutomationTest.h"
#include "Schemas/Asset.h"
#include "Schemas/AssetLink.h"
#include "Schemas/NFTAssetLink.h"
#include "Schemas/SFTAssetLink.h"
#include "Schemas/Inputs/AssetConnection.h"
#include "Schemas/Inputs/AssetInput.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AssetQueryGenerationTest,
	"UBFEmergenceDemo.sdk_unreal_asset_register.Source.AssetRegister.Private.AssetQueryGenerationTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AssetQueryGenerationTest::RunTest(const FString& Parameters)
{
	const auto TokenId = TEXT("2227");
	const auto CollectionId = TEXT("7668:root:17508");
	auto AssetQuery = FAssetRegister::AddAssetQuery(FAssetInput(TokenId, CollectionId));

	AssetQuery->AddField(&FAsset::Id)
		->AddField<FAsset>(&FAsset::Profiles)
		->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::RawAttributes);
	
	FAssetConnection Input = FAssetConnection();
	Input.Addresses = {TEXT("0xFFfffffF00000000000000000000000000000f59")};
	
	AssetQuery->OnMember(&FAsset::Links)
	->OnUnion<UNFTAssetLink, UAssetLink>()
		->OnArray(&UNFTAssetLink::ChildLinks)
			->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)
				->AddField(&FAsset::CollectionId)
				->AddField(&FAsset::TokenId);
	
	AssetQuery->OnMember(&FAsset::Links)->OnUnion<USFTAssetLink, UAssetLink>()
			->OnArray(&USFTAssetLink::ParentLinks)->AddArgument(Input)->AddField(&FAsset::Id);
	
	FString ExpectedQueryString = R"(
	query {
	 asset(tokenId:"2227",collectionId:"7668:root:17508") {
	   id
	   profiles
	   metadata {
	     rawAttributes
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
	     ... on SFTAssetLink {
	       parentLinks(addresses:["0xFFfffffF00000000000000000000000000000f59"]) {
	         id
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
			TestEqual(TEXT("Parsed Asset should contain asset-profile key in profiles map"),
		OutAsset.Profiles.Contains(TEXT("asset-profile")), true);
			
			UE_LOG(LogTemp, Log, TEXT("Parsed AssetProfile: %s"), *OutAsset.Profiles.FindOrAdd(TEXT("asset-profile")));

			FString MetadataJson;
			auto MetadataJsonObject = FJsonObjectConverter::UStructToJsonObject(OutAsset.Metadata);
			const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetadataJson);
			FJsonSerializer::Serialize(MetadataJsonObject.ToSharedRef(), Writer);
			UE_LOG(LogTemp, Log, TEXT("Parsed Metadata: %s"), *MetadataJson);
			
			FNFTAssetLinkData NFTAssetLinkData;
			if (QueryStringUtil::TryGetModelField<FAsset, FNFTAssetLinkData>(OutJson, TEXT("links"), NFTAssetLinkData))
			{
				UNFTAssetLink* NFTAssetLink = NewObject<UNFTAssetLink>();
				NFTAssetLink->ChildLinks.Append(NFTAssetLinkData.ChildLinks);
				
				for (auto ChildLink : NFTAssetLink->ChildLinks)
				{
					UE_LOG(LogTemp, Log, TEXT("ChildLink Path: %s TokenId: %s CollectionId: %s"), *ChildLink.Path, *ChildLink.Asset.TokenId, *ChildLink.Asset.CollectionId);
				}
			}
			else
			{
				AddError("Failed to get NFTAssetLink Data!");
			}
		}
			else
		{
			AddError(FString::Printf(TEXT("Failed to get Asset Object from Json: %s!"), *OutJson));
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
