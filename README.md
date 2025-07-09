# Asset Register SDK
This Unreal Engine plugin provides an API for creating and sending GraphQL requests to the **Asset Register (AR)** server.

>**Tip**
>
>See https://ar-docs.futureverse.app/ for details on the Asset Register.

## Features
- Blueprint Library containing common Asset Register GraphQL queries in both Blueprint and C++ accessible format
- Includes custom strongly-typed query builder to detect errors in compile time and auto-completion
- Supports array fields, union types, and nested field selection

## Installation
Add the `AssetRegisterPlugin` to your project's plugins folder and enable it in the Plugins tab in unreal.

## Getting Started
First, set the endpoint URL you want to send your requests.

The endpoint URL can be changed in `Plugins/Futureverse Asset Register`

![image](https://github.com/user-attachments/assets/e37e6858-0c0a-4998-8b88-10f19f6d53d1)

## Querying Assets using Asset Register Querying Library
### Blueprint Example
![image](https://github.com/user-attachments/assets/44fb43ae-28fd-4bea-94f7-d5afc4181cd4)

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

## Querying Asset Profile URI using Asset Register Querying Library
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
    // Process Response
  }
});
```

## Querying AssetLinks using Asset Register Querying Library
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
      // Process response
    }
  }
});
```
Note: `FAssetLinkWrapper` contains the actual UObject with data.

## Making Custom Asset Query
### C++ Example
```cpp
const auto TokenId = TEXT("2227");
const auto CollectionId = TEXT("7668:root:17508");

// create Asset Query
auto AssetQuery = FAssetRegisterQueryBuilder::AddAssetQuery(FAssetInput(TokenId, CollectionId));

// Add custom fields here
AssetQuery->AddField(&FAsset::Id)
  ->AddField<FAsset>(&FAsset::Profiles)
  ->OnMember(&FAsset::Metadata)
    ->AddField(&FAssetMetadata::RawAttributes);
	
FAssetConnection Input = FAssetConnection();
Input.Addresses = {TEXT("0xFFfffffF00000000000000000000000000000f59")};
	
AssetQuery->OnMember(&FAsset::Links)
  ->OnUnion<FNFTAssetLinkData, FAssetLinkData>()
    ->OnArray(&FNFTAssetLinkData::ChildLinks)
      ->AddField(&FLink::Path)
      ->OnMember(&FLink::Asset)
        ->AddField(&FAsset::CollectionId)
        ->AddField(&FAsset::TokenId);
	
AssetQuery->OnMember(&FAsset::Links)
  ->OnUnion<FSFTAssetLinkData, FAssetLinkData>()
    ->OnArray(&FSFTAssetLinkData::ParentLinks)->AddArgument(Input)
      ->AddField(&FAsset::Id);

UAssetRegisterQueryingLibrary::SendRequest(AssetsQuery->GetQueryString()).Next([](const FString& OutJson)
{
  FAsset OutAsset;
  if (QueryStringUtil::TryGetModel(OutJson, OutAsset))
  {
    
  }
}
```
Note: Use `QueryStringUtil::TryGetModel<FAsset>` to convert raw json to `FAsset`

## Making Custom Assets Query
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

// Add custom fields here
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
  ->OnUnion<FNFTAssetLinkData, FAssetLinkData>()
    ->OnArray(&FNFTAssetLinkData::ChildLinks)
      ->AddField(&FLink::Path)
        ->OnMember(&FLink::Asset)
          ->AddField(&FAsset::CollectionId)
          ->AddField(&FAsset::TokenId);

UAssetRegisterQueryingLibrary::SendRequest(AssetsQuery->GetQueryString()).Next([](const FString& OutJson)
{
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OutJson);
  TSharedPtr<FJsonObject> RootObject;
  FJsonSerializer::Deserialize(Reader, RootObject);

  TArray<TSharedPtr<FJsonValue>> AssetNodes;
  QueryStringUtil::FindAllFieldsRecursively(RootObject, TEXT("node"), AssetNodes);
			
  for (const auto& AssetNode : AssetNodes)
  {
    auto AssetNodeObject = AssetNode->AsObject();
    FAsset Asset;

    if (FJsonObjectConverter::JsonObjectToUStruct(AssetNodeObject.ToSharedRef(), &Asset))
    {

    }
  }
}
```
Note: Use `QueryStringUtil::FindAllFieldsRecursively` and `FJsonObjectConverter::JsonObjectToUStruct` to fully deserialize the response properly. 
Especially for polymorphic properties like `AssetLink`. See `UAssetRegisterQueryingLibrary` for more details.
