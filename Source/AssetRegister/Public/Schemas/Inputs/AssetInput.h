#pragma once
#include "AssetInput.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetInput
{
	GENERATED_BODY()

	FAssetInput(){}
	FAssetInput(const FString& TokenId, const FString& CollectionId)
		: TokenId(TokenId), CollectionId(CollectionId){}
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "tokenId"))
	FString TokenId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "collectionId"))
	FString CollectionId;
};
