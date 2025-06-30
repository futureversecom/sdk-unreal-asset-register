#pragma once
#include "Asset.generated.h"

UCLASS()
class UAssetLink : public UObject
{
	GENERATED_BODY()
};

USTRUCT()
struct FRawAttributes
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FString Value;
	
	UPROPERTY(EditAnywhere)
	FString Trait_type;
};

USTRUCT()
struct FAssetMetadata
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "uri"))
	FString Uri;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "attributes"))
	TArray<FString> Attributes;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "rawAttributes"))
	TArray<FRawAttributes> RawAttributes;
};

USTRUCT(meta=(QueryName = "asset"))
struct FAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "collectionId"))
	FString CollectionId;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "tokenId"))
	FString TokenId;

	UPROPERTY(EditAnywhere, meta=(QueryName = "links"))
	UAssetLink* Links;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "profiles"))
	TMap<FString, FString> Profiles;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "metadata"))
	FAssetMetadata Metadata;
};

