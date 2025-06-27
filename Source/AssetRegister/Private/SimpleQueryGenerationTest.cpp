#include "AssetRegisterQueryingLibrary.h"
#include "QueryBuilder.h"
#include "Misc/AutomationTest.h"
#include "Schemas/Asset.h"
#include "Schemas/AssetLink.h"
#include "Schemas/Inputs/AssetConnection.h"
#include "Schemas/Inputs/AssetInput.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(SimpleQueryGenerationTest,
	"UBFEmergenceDemo.sdk_unreal_asset_register.Source.AssetRegister.Private.SimpleQueryGenerationTest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

FString RemoveAllWhitespace(const FString& Input)
{
	FString Result;
	
	for (TCHAR Char : Input)
	{
		if (!FChar::IsWhitespace(Char))
		{
			Result.AppendChar(Char);
		}
	}

	return Result;
}

bool SimpleQueryGenerationTest::RunTest(const FString& Parameters)
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
		->OnUnion<FNFTAssetLink, FAssetLink>()
			->OnArray(&FNFTAssetLink::ChildLinks)->AddField(&FLink::Path)
			->OnMember(&FLink::Asset)->AddField(&FAsset::Id);
	
	AssetQuery->OnMember(&FAsset::Links)->OnUnion<FSFTAssetLink, FAssetLink>()
			->OnArray(&FSFTAssetLink::ParentLinks)->AddArgument(Input)->AddField(&FAsset::Id);
	
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
	           id
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
	
	TestEqual(TEXT("Full query string should include everything"), RemoveAllWhitespace(AssetQuery->GetQueryString()), RemoveAllWhitespace(ExpectedQueryString));
	
	bool bHttpRequestCompleted = false;
	UAssetRegisterQueryingLibrary::SendRequest(ExpectedQueryString).Next(
		[this, &bHttpRequestCompleted](const FString& OutJson)
	{
		FAsset OutAsset;
		if (QueryStringUtil::TryGetModel(OutJson, OutAsset))
		{
			TestEqual(TEXT("Parsed Asset should contain asset-profile key in profiles map"),
		OutAsset.Profiles.Contains(TEXT("asset-profile")), true);
			
			UE_LOG(LogTemp, Log, TEXT("Parsed AssetProfile: %s"), *OutAsset.Profiles.FindOrAdd(TEXT("asset-profile")));
			for (auto RawAttribute : OutAsset.Metadata.RawAttributes)
			{
				UE_LOG(LogTemp, Log, TEXT("Parsed Metadata.RawAttributes trait_type: %s value: %s"), *RawAttribute.Trait_type, *RawAttribute.Value);
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
