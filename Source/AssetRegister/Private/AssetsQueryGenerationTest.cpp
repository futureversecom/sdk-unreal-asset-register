#include "AssetRegisterQueryingLibrary.h"
#include "QueryBuilder.h"
#include "QueryTestUtil.h"
#include "Misc/AutomationTest.h"
#include "Schemas/AssetLink.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(AssetsQueryGenerationTest,
								"UBFEmergenceDemo.sdk_unreal_asset_register.Source.AssetRegister.Private.AssetsQueryGenerationTest",
								EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool AssetsQueryGenerationTest::RunTest(const FString& Parameters)
{
	const auto CollectionId = TEXT("7668:root:17508");
	const auto Address = TEXT("0xFfffFffF000000000000000000000000000012ef");

	auto AssetConnectionInput = FAssetConnection();
	AssetConnectionInput.Addresses = {Address};
	AssetConnectionInput.CollectionIds = {CollectionId};
	AssetConnectionInput.First = 2;
	
	auto AssetsQuery = FAssetRegister::AddAssetsQuery(AssetConnectionInput);
	const auto AssetNode = AssetsQuery->OnArray(&FAssets::Edges)->OnMember(&FAssetEdge::Node);
	AssetNode->AddField(&FAsset::TokenId)
			->AddField(&FAsset::CollectionId)
			->AddField(&FAsset::Profiles);
	AssetNode->OnMember(&FAsset::Metadata)
			->AddField(&FAssetMetadata::Properties)
			->AddField(&FAssetMetadata::Attributes)
			->AddField(&FAssetMetadata::RawAttributes);
	
	AssetNode->OnMember(&FAsset::Collection)
			->AddField(&FCollection::ChainId)
			->AddField(&FCollection::ChainType)
			->AddField(&FCollection::Location)
			->AddField(&FCollection::Name);
	
	AssetNode->OnMember(&FAsset::Links)
		->OnUnion<UNFTAssetLink, UAssetLink>()
			->OnArray(&UNFTAssetLink::ChildLinks)
				->AddField(&FLink::Path)
				->OnMember(&FLink::Asset)
					->AddField(&FAsset::CollectionId)
					->AddField(&FAsset::TokenId);

	const auto ExpectedQueryString = R"(query {
		assets(addresses: ["0xFfffFffF000000000000000000000000000012ef"],collectionIds: ["7668:root:17508"], first: 2) {
			edges {
				node {
					tokenId
					collectionId
					profiles
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
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OutJson);
		TSharedPtr<FJsonObject> RootObject;
		FJsonSerializer::Deserialize(Reader, RootObject);

		TArray<TSharedPtr<FJsonValue>> AllAssetNodes;
		QueryStringUtil::FindAllFieldsRecursively(RootObject, TEXT("node"), AllAssetNodes);
			
		for (const auto& AssetNode : AllAssetNodes)
		{
			auto AssetNodeObject = AssetNode->AsObject();
			FAsset Asset;

			if (FJsonObjectConverter::JsonObjectToUStruct(AssetNodeObject.ToSharedRef(), &Asset))
			{
				FString MetadataJson;
				auto MetadataJsonObject = FJsonObjectConverter::UStructToJsonObject(Asset.Metadata);
				const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetadataJson);
				FJsonSerializer::Serialize(MetadataJsonObject.ToSharedRef(), Writer);
				// UE_LOG(LogTemp, Log, TEXT("Parsed Metadata: %s"), *MetadataJson);

				TestEqual(TEXT("Collection Chain Id should match"), Asset.Collection.ChainId, TEXT("7668"));
				TestEqual(TEXT("Collection Chain Type should match"), Asset.Collection.ChainType, TEXT("root"));
				TestEqual(TEXT("Collection Chain location should match"), Asset.Collection.Location, TEXT("17508"));
				TestEqual(TEXT("Collection Chain name should match"), Asset.Collection.Name, TEXT("Party Bear Unleashed"));
				TestEqual(TEXT("Collection Id should match"), Asset.CollectionId, TEXT("7668:root:17508"));
				
				const TSharedPtr<FJsonObject>* LinksObject;
				if (AssetNodeObject->TryGetObjectField(TEXT("links"), LinksObject))
				{
					FNFTAssetLinkData NFTAssetLinkData;
					if (!FJsonObjectConverter::JsonObjectToUStruct(LinksObject->ToSharedRef(), &NFTAssetLinkData))
					{
						AddError(TEXT("Failed to get NFTAssetLink Data!"));
						return;
					}
					UNFTAssetLink* NFTAssetLink = NewObject<UNFTAssetLink>();
				
					for (FLink& ChildLink : NFTAssetLinkData.ChildLinks)
					{
						FString Path = ChildLink.Path;
						int32 Index = 0;
						if (ChildLink.Path.FindChar('#', Index))
						{
							Path = ChildLink.Path.Mid(Index + 1);
							Path = Path.Replace(TEXT("_accessory"), TEXT(""));
						}
							
						ChildLink.Path = Path;
						NFTAssetLink->ChildLinks.Add(ChildLink);
					}
						
					Asset.Links = NFTAssetLink;

					if (auto ParsedLink = Cast<UNFTAssetLink>(Asset.Links))
					{
						for (auto ChildLink : ParsedLink->ChildLinks)
						{
							AddErrorIfFalse(!ChildLink.Path.StartsWith(TEXT("http")),
								FString::Printf(TEXT("Parsed ChildLink Path should have correct format. Actual: %s"), *ChildLink.Path));
						}
					}
				}
				else
				{
					AddError(TEXT("Failed to get links field from AssetObject!"));
					return;
				}
			}
			else
			{
				AddError(TEXT("Failed to get AssetObject from AssetEdges.Node!"));
				return;
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
