#pragma once
#include "AssetInput.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetInput
{
	GENERATED_BODY()

	FAssetInput(){}
	FAssetInput(const FString& TokenId, const FString& CollectionId)
		: TokenId(TokenId), CollectionId(CollectionId){}
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TokenId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CollectionId;
};
