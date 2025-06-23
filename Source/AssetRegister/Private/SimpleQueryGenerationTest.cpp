#include "AssetRegisterQueryingLibrary.h"
#include "QueryRegistry.h"
#include "Misc/AutomationTest.h"
#include "Schemas/Asset.h"
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
	TSharedPtr<FQueryRegistry<FAsset>> QueryRegistry = MakeShareable(new FQueryRegistry<FAsset>());
	
	TestEqual(TEXT("Model Name should match with type of QueryRegistry"), QueryRegistry->GetModelString(),
		TEXT("asset"));
	
	const auto Argument = FAssetInput(TEXT("400"), TEXT("7672:root:303204"));
	QueryRegistry->RegisterArgument(Argument);
	QueryRegistry->RegisterArgument("float", 0);

	TestEqual(TEXT("Arguments should be split up by ,"), QueryRegistry->GetArgumentsString(),
		TEXT("tokenId:\"400\",collectionId:\"7672:root:303204\",float:0"));
	
	auto Asset = FAsset();
	Asset.Id = TEXT("123");
	Asset.Profiles.Add(TEXT("123"), TEXT("123"));
	Asset.Metadata.RawAttributes = TArray{FRawAttributes()};
	QueryRegistry->RegisterField(Asset);
	FString ExpectedFieldString = R"(
	{
	  id
	  profiles
	  metadata {
	    rawAttributes
	}
	})";
	UE_LOG(LogTemp, Log, TEXT("Actual FieldString: %s"), *RemoveAllWhitespace(QueryRegistry->GetFieldString()));
	UE_LOG(LogTemp, Log, TEXT("Expected FieldString: %s"), *RemoveAllWhitespace(ExpectedFieldString));
	TestEqual(TEXT("Fields should only display if set"), RemoveAllWhitespace(QueryRegistry->GetFieldString()), RemoveAllWhitespace(ExpectedFieldString));
	
	FString ExpectedQueryString = R"(
	query {
	  asset(tokenId: "400", collectionId: "7672:root:303204", float:0)
		{
		  id
		  profiles
		  metadata {
		    rawAttributes
		}
		}
	})";
	
	UE_LOG(LogTemp, Log, TEXT("Actual QueryString: %s"), *RemoveAllWhitespace(QueryRegistry->GetQueryString()));
	UE_LOG(LogTemp, Log, TEXT("Expected QueryString %s"), *RemoveAllWhitespace(ExpectedQueryString));
	
	TestEqual(TEXT("Full query string should include everything"), RemoveAllWhitespace(QueryRegistry->GetQueryString()), RemoveAllWhitespace(ExpectedQueryString));
	
	UAssetRegisterQueryingLibrary::SendRequest(TEXT("https://ar-api.futureverse.cloud/graphql"), QueryRegistry->GetQueryString()).Next([]
	{
			
	});
	
	FString TestAssetResponse = R"(
	{
		"data": {
			"asset": {
				"profiles": {
					"asset-profile": "https://fv-ubf-assets-dev.s3.us-west-2.amazonaws.com/Genesis/Profiles/303204/profile.json"
				  }
			}
		}
	})";
	FAsset OutAsset;
	QueryRegistry->TryGetModel(TestAssetResponse, OutAsset);
	TestEqual(TEXT("Parsed Asset should contain asset-profile key in profiles map"),
		OutAsset.Profiles.Contains(TEXT("asset-profile")), true);
	
	UE_LOG(LogTemp, Log, TEXT("Parsed AssetProfile: %s"), *OutAsset.Profiles.FindOrAdd(TEXT("asset-profile")));
	return true;
}
