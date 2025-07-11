#pragma once
#include "Asset.h"
#include "Avatar.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAvatar
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "asset"))
	FAsset Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "customImage"))
	FString CustomImage;
};
