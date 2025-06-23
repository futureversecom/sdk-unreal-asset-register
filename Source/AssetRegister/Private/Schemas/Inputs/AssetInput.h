#pragma once
#include "AssetInput.generated.h"

USTRUCT(BlueprintType)
struct FAssetInput
{
	FAssetInput(){}
	FAssetInput(const FString& TokenId, const FString& CollectionId)
		: TokenId(TokenId), CollectionId(CollectionId){}

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(ParameterType = "String"), meta=(QueryName = "tokenId"))
	FString TokenId;
	
	UPROPERTY(EditAnywhere, meta=(ParameterType = "CollectionId"), meta=(QueryName = "collectionId"))
	FString CollectionId;
};
