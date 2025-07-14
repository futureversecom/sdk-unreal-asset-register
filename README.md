# Asset Register SDK
This Unreal Engine plugin provides an API for creating and sending GraphQL requests to the **Asset Register (AR)** server.

>**Tip**
>
>See https://ar-docs.futureverse.app/ for details on the Asset Register.

---

## ✨ Features
- Blueprint Library containing common Asset Register GraphQL queries in both Blueprint and C++ accessible format
- Includes custom strongly-typed query builder to detect errors in compile time and auto-completion
- Handles deserializing polymorphic objects
- Supports array fields, union types, and nested field selection
  
---

## 🔧 Getting Started
Add the `AssetRegisterPlugin` to your project's plugins folder and enable it in the Plugins tab in Unreal Engine.

To set the target Asset Registry end point, head to your Project Settings and navigate to your `Plugins/Futureverse Asset Register`.

Select the URL you want to target:
- Production -- `https://ar-api.futureverse.app/graphql`
- Staging -- `https://ar-api.futureverse.cloud/graphql`
- Dev -- `https://ar-api.futureverse.dev/graphql`

![image](https://github.com/user-attachments/assets/e37e6858-0c0a-4998-8b88-10f19f6d53d1)

---

## 🔍 Querying Assets using Asset Register Querying Library
### Blueprint Example
<img width="1045" height="646" alt="image" src="https://github.com/user-attachments/assets/ecd6fdd8-0f55-4a9d-b2e0-60810a6fb38c" />

### C++ Example
```cpp
FAssetConnection AssetConnectionInput;
AssetConnectionInput.Addresses = {OwnerAddress};
AssetConnectionInput.First = 1000;

UAssetRegisterQueryingLibrary::GetAssets(AssetConnectionInput).Next([](const FLoadAssetsResult& Result)
{
	if (Result.bSuccess)
	{
		const FAssets Assets = Result.Value;
		for (const FAssetEdge& AssetEdge : Assets.Edges)
		{
			const FAsset Asset = AssetEdge.Node;
			// Process Asset
		}
	}
});
```
Note: this assets query has pre-configured fields. If you want to configure which fields to be included in the query, see [Making Custom Assets Query](#making-custom-assets-query) section below

---

## 🔍 Querying Asset Profile URI using Asset Register Querying Library
### Blueprint Example
![image](https://github.com/user-attachments/assets/f74c16eb-f223-449c-ad0a-cda13a8a2257)

### C++ Example
```cpp
const FString TokenId = TEXT("2227");
const FString CollectionId = TEXT("7668:root:17508");

UAssetRegisterQueryingLibrary::GetAssetProfile(TokenId, CollectionId).Next([](const FLoadJsonResult& Result)
{
	if (Result.bSuccess)
	{
		const FString ProfileURI = Result.Value;
		// Process Profile URI
	}
});
```

---

## 🔍 Querying AssetLinks using Asset Register Querying Library
### Blueprint Example
![image](https://github.com/user-attachments/assets/fcfb4ef5-172e-4598-80ab-8fe6c02feb22)

### C++ Example
```cpp
const FString TokenId = TEXT("2227");
const FString CollectionId = TEXT("7668:root:17508");

UAssetRegisterQueryingLibrary::GetAssetLinks(TokenId, CollectionId).Next([](const FLoadAssetResult& Result)
{
	const auto Asset = Result.Value;
	if (const UNFTAssetLink* NFTAssetLink = Cast<UNFTAssetLink>(Asset.LinkWrapper.Links))
	{
		for (const FLink& ChildLink : NFTAssetLink->ChildLinks)
		{
			// Process Link
		}
	}
});
```
Note: `FAssetLinkWrapper` contains the actual UObject with data.

---
## 🔧 Building and sending a Custom Query Step by Step

1. Use `FAssetRegisterQueryBuilder` to create either a Asset or Assets query.
 	- This returns either `FQueryNode<FAsset>` or `FQueryNode<FAssets>`
	- Each query requires a different input. E.g. Asset query requires `FAssetInput` whereas Assets query requires `FAssetConnection`
3. Populate your input
4. Populate your query with fields
	- Use `AddField` to add field from your initial Schema Object e.g. `AssetNode->AddField(&FAsset::TokenId)`
	- Use `OnMember` to add the member field of the Schema Object, then use `AddField` after to add its fields e.g. `AssetNode->OnMember(&FAsset::Metadata)->AddField(&FAssetMetadata::Properties)`
	- Use `OnUnion` to specify the polymorphic member field of the Schema Object e.g. `AssetNode->OnMember(&FAsset::Links)->OnUnion<FNFTAssetLinkData>()`
	- Use `OnArray` to add the member field of array type of the Schema Object e.g. `AssetQuery->OnMember(&FAsset::Links)->OnUnion<FNFTAssetLinkData>()->OnArray(&FNFTAssetLinkData::ChildLinks)`
	- Use `AddArgument` to add argument to one of the fields e.g. `AssetQuery->OnMember(&FAsset::Links)->OnUnion<FSFTAssetLinkData>()->AddArgument(TEXT("addresses"), addresses)`
5. Get your query json string from the root query node e.g. `AssetQuery->GetQueryJsonString()` and send it to the `UAssetRegisterQueryingLibrary`.
	- Highly recommend using `UAssetRegisterQueryingLibrary::MakeAssetQuery` or `UAssetRegisterQueryingLibrary::MakeAssetsQuery` so that you don't have to deal with deserializing polymorphic objects yourself. 
	- You can still handle the response yourself by sending the http request yourself or use `UAssetRegisterQueryingLibrary::SendRequest`

See below sections for examples

## 🔍 Making Custom Asset Query
### C++ Example
```cpp
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

UAssetRegisterQueryingLibrary::MakeAssetQuery(AssetQuery->GetQueryJsonString()).Next([](const FLoadAssetResult& Result)
{
	if (Result.bSuccess)
	{
		// Process Asset here
	  	const FAsset Asset = Result.Value;
	}
}
```

## 🔍 Making Custom Assets Query
### C++ Example
```cpp
const auto CollectionId = TEXT("7668:root:17508");
const auto Address = TEXT("0xFfffFffF000000000000000000000000000012ef");

auto AssetConnectionInput = FAssetConnection();
AssetConnectionInput.Addresses = {Address};
AssetConnectionInput.CollectionIds = {CollectionId};
AssetConnectionInput.First = 2;

// create Assets Query
auto AssetsQuery = FAssetRegisterQueryBuilder::AddAssetsQuery(AssetConnectionInput);
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
	
MakeAssetQuery(AssetsQuery->GetQueryJsonString()).Next([](const FLoadAssetsResult& Result)
{
	if (Result.bSuccess)
	{
		const FAssets Assets = Result.Value;
		for (const FAssetEdge& AssetEdge : Assets.Edges)
		{
			const FAsset Asset = AssetEdge.Node;
		}
	}
});
```
