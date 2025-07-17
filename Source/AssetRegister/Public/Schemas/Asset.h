#pragma once
#include "JsonObjectWrapper.h"
#include "Schema.h"
#include "Enums/AssetType.h"
#include "Unions/AssetOwnership.h"
#include "Unions/AssetLink.h"
#include "Asset.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FCollection
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Location;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;
};

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FRawAttributes
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Trait_type;
};

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetMetadata
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Uri;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Properties;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Attributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRawAttributes> RawAttributes;
};

USTRUCT(BlueprintType, Blueprintable)
struct ASSETREGISTER_API FAsset
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAssetType AssetType = EAssetType::ERC721;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCollection Collection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CollectionId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAssetLinkWrapper LinkWrapper;

	UPROPERTY()
	FAssetLink Links;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAssetMetadata Metadata;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAssetOwnershipWrapper OwnershipWrapper;

	UPROPERTY()
	FAssetOwnership Ownership;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Profiles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSchema Schema;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TokenId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FJsonObjectWrapper OriginalJsonData;
};

