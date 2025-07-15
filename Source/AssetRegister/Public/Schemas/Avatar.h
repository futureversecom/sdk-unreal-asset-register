#pragma once
#include "Asset.h"
#include "Avatar.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAvatar
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAsset Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomImage;
};
