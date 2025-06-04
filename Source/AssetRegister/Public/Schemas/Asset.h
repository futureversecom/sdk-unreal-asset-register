#pragma once

USTRUCT(BlueprintType)
struct FAssetMetadata
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "uri"))
	FString Uri;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "attributes"))
	TMap<FString, FString> Attributes;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "rawAttributes"))
	TMap<FString, FString> RawAttributes;
};

USTRUCT(BlueprintType, meta=(QueryName = "asset"))
struct FAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(QueryName = "id"))
	FString Id;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "collectionId"))
	FString CollectionId;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "tokenId"))
	FString TokenId;
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "metadata"))
	FAssetMetadata Metadata;
};

