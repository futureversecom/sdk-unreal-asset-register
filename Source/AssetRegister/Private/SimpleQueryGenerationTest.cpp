#include "AssetRegisterQueryingLibrary.h"
#include "QueryBuilder.h"
#include "Misc/AutomationTest.h"
#include "Schemas/Asset.h"
#include "Schemas/AssetLink.h"
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
	//TSharedPtr<FQuery<FAsset>> Query = MakeShareable(new FQuery<FAsset>());
	TSharedPtr<FQueryBuilder<FAsset>> Query = MakeShareable(FQueryBuilder<FAsset>::Create());

	TestEqual(TEXT("Model Name should match with type of QueryRegistry"), Query->GetModelString(),
		TEXT("asset"));

	const auto TokenId = TEXT("400");
	const auto CollectionId = TEXT("7672:root:303204");
	const auto Argument = FAssetInput(TokenId, CollectionId);
	Query->AddArgument(Argument);

	TestEqual(TEXT("Arguments should be split up by ,"), Query->GetArgumentsString(),
		TEXT("tokenId:\"400\",collectionId:\"7672:root:303204\""));
	
	Query->AddField(&FAsset::Id);
	Query->AddField(&FAsset::Profiles);
	Query->OnMember(&FAsset::Metadata).AddField(&FAssetMetadata::RawAttributes);
	Query->OnUnion<FNFTAssetLink>(&FAsset::Links).AddField(&FNFTAssetLink::ChildLinks);
	
	FString ExpectedFieldString = R"(
	{
	  id
	  profiles
	  metadata {
	    rawAttributes
	}
	})";
	UE_LOG(LogTemp, Log, TEXT("Actual FieldString: %s"), *Query->GetFieldString());
	TestEqual(TEXT("Fields should only display if set"), RemoveAllWhitespace(Query->GetFieldString()), RemoveAllWhitespace(ExpectedFieldString));
	
	FString ExpectedQueryString = R"(
	query {
	  asset(tokenId: "400", collectionId: "7672:root:303204")
		{
		  id
		  profiles
		  metadata {
		    rawAttributes
		}
		}
	})";
	
	UE_LOG(LogTemp, Log, TEXT("Actual QueryString: %s"), *RemoveAllWhitespace(Query->GetQueryString()));
	UE_LOG(LogTemp, Log, TEXT("Expected QueryString %s"), *RemoveAllWhitespace(ExpectedQueryString));
	
	TestEqual(TEXT("Full query string should include everything"), RemoveAllWhitespace(Query->GetQueryString()), RemoveAllWhitespace(ExpectedQueryString));
	
	bool bHttpRequestCompleted = false;
	UAssetRegisterQueryingLibrary::SendRequest(TEXT("https://ar-api.futureverse.cloud/graphql"), ExpectedQueryString).Next(
		[this, Query, &bHttpRequestCompleted](const FString& OutJson)
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
