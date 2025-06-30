#pragma once
#include "Asset.generated.h"

UCLASS()
class UAssetLink : public UObject
{
	GENERATED_BODY()
};

USTRUCT()
struct FCollection
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(QueryName = "chainId"))
	FString ChainId;
	
	UPROPERTY(meta=(QueryName = "chainType"))
	FString ChainType;

	UPROPERTY(meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(meta=(QueryName = "location"))
	FString Location;
	
	UPROPERTY(meta=(QueryName = "name"))
	FString Name;
};

USTRUCT()
struct FRawAttributes
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString Value;
	
	UPROPERTY()
	FString Trait_type;
};

USTRUCT()
struct FAssetMetadata
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(meta=(QueryName = "uri"))
	FString Uri;

	UPROPERTY(meta=(QueryName = "properties"))
	TMap<FString, FString> Properties;
	
	UPROPERTY(meta=(QueryName = "attributes"))
	TMap<FString, FString> Attributes;
	
	UPROPERTY(meta=(QueryName = "rawAttributes"))
	TArray<FRawAttributes> RawAttributes;
};

USTRUCT(meta=(QueryName = "asset"))
struct FAsset
{
	GENERATED_BODY()

	UPROPERTY(meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(meta=(QueryName = "collectionId"))
	FString CollectionId;
	
	UPROPERTY(meta=(QueryName = "tokenId"))
	FString TokenId;

	UPROPERTY(meta=(QueryName = "collection"))
	FCollection Collection;

	UPROPERTY(meta=(QueryName = "links"))
	UAssetLink* Links;
	
	UPROPERTY(meta=(QueryName = "profiles"))
	TMap<FString, FString> Profiles;
	
	UPROPERTY(meta=(QueryName = "metadata"))
	FAssetMetadata Metadata;
};

