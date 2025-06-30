#include "AssetRegisterQueryingLibrary.h"
#include "QueryBuilder.h"
#include "QueryTestUtil.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AssetsQueryGenerationTest,
								"UBFEmergenceDemo.sdk_unreal_asset_register.Source.AssetRegister.Private.AssetsQueryGenerationTest",
								EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AssetsQueryGenerationTest::RunTest(const FString& Parameters)
{
	const auto CollectionId = TEXT("7668:root:17508");
	const auto Address = TEXT("0xFfffFffF000000000000000000000000000012ef");
	
	auto AssetsQuery =
		FAssetRegister::AddAssetsQuery(FAssetConnection({Address}, {CollectionId}));
	AssetsQuery->AddArgument(TEXT("first"), 1000);
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)->AddField(&FAsset::CollectionId);
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::Properties)
			->AddField(&FAssetMetadata::Attributes)
			->AddField(&FAssetMetadata::RawAttributes);
	
	AssetNode->OnMember(&FAsset::Collection)
			->AddField(&FCollection::ChainId)
			->AddField(&FCollection::ChainType)
			->AddField(&FCollection::Location)
			->AddField(&FCollection::Name);

	const auto ExpectedQueryString = R"(query {
		assets(addresses: ["0xFfffFffF000000000000000000000000000012ef"],collectionIds: ["7668:root:17508"], first: 1000) {
			edges {
				node {
					tokenId
					collectionId
					metadata {
						properties
						attributes
						rawAttributes
					  }
					collection {
						chainId
						chainType
						location
						name
					  }
				}
			}
		}
	})";
	
	UE_LOG(LogTemp, Log, TEXT("Actual QueryString: %s"), *AssetsQuery->GetQueryString());
	
	TestEqual(TEXT("Full query string should include everything"),
		QueryTestUtil::RemoveAllWhitespace(AssetsQuery->GetQueryString()),
		QueryTestUtil::RemoveAllWhitespace(ExpectedQueryString));
	
	bool bHttpRequestCompleted = false;
	UAssetRegisterQueryingLibrary::SendRequest(AssetsQuery->GetQueryString()).Next(
		[this, &bHttpRequestCompleted](const FString& OutJson)
	{
		FAssets OutAssets;
		if (QueryStringUtil::TryGetModel<FAssets>(OutJson, OutAssets))
		{
			for (const auto& FAssetEdge : OutAssets.Edges)
			{
				const auto Asset = FAssetEdge.Node;
				for (const auto& RawAttribute : Asset.Metadata.RawAttributes)
				{
					UE_LOG(LogTemp, Log, TEXT("Parsed Metadata.RawAttributes trait_type: %s value: %s"), *RawAttribute.Trait_type, *RawAttribute.Value);
				}

				for (auto Property : Asset.Metadata.Properties)
				{
					UE_LOG(LogTemp, Log, TEXT("Parsed Metadata.Properties Key: %s value: %s"), *Property.Key, *Property.Value);
				}

				for (auto Attribute : Asset.Metadata.Attributes)
				{
					UE_LOG(LogTemp, Log, TEXT("Parsed Metadata.Attribute Key: %s value: %s"), *Attribute.Key, *Attribute.Value);
				}

				TestEqual(TEXT("Collection Chain Id should match"), Asset.Collection.ChainId, TEXT("7668"));
				TestEqual(TEXT("Collection Chain Type should match"), Asset.Collection.ChainType, TEXT("root"));
				TestEqual(TEXT("Collection Chain location should match"), Asset.Collection.Location, TEXT("17508"));
				TestEqual(TEXT("Collection Chain name should match"), Asset.Collection.Name, TEXT("Party Bear Unleashed"));
				TestEqual(TEXT("Collection Id should match"), Asset.CollectionId, TEXT("7668:root:17508"));
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
