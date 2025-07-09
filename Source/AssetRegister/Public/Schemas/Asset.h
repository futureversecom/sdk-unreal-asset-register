#pragma once
#include "AssetLink.h"
#include "JsonObjectWrapper.h"
#include "Asset.generated.h"

class UAssetLink;

// Wrapper struct for holding UAssetLink
USTRUCT(BlueprintType)
struct FAssetLinkWrapper
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UAssetLink* Links;
};

USTRUCT(BlueprintType)
struct FCollection
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "chainId"))
	FString ChainId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "chainType"))
	FString ChainType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "location"))
	FString Location;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "name"))
	FString Name;
};

USTRUCT(BlueprintType)
struct FRawAttributes
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Trait_type;
};

USTRUCT(BlueprintType)
struct FAssetMetadata
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "uri"))
	FString Uri;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "properties"))
	TMap<FString, FString> Properties;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "attributes"))
	TMap<FString, FString> Attributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "rawAttributes"))
	TArray<FRawAttributes> RawAttributes;
};

USTRUCT(BlueprintType, Blueprintable, meta=(QueryName = "asset"))
struct FAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "collectionId"))
	FString CollectionId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "tokenId"))
	FString TokenId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "collection"))
	FCollection Collection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAssetLinkWrapper LinkWrapper;

	UPROPERTY(meta=(QueryName = "links"))
	FAssetLinkData Links;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "profiles"))
	TMap<FString, FString> Profiles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "metadata"))
	FAssetMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FJsonObjectWrapper OriginalJsonData;
};

