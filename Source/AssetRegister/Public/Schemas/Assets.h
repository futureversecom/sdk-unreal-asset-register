#pragma once
#include "Asset.h"
#include "Assets.generated.h"

USTRUCT(BlueprintType)
struct FAssetEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "node"))
	FAsset Node;
};

USTRUCT(Blueprintable, meta=(QueryName = "assets"))
struct FAssets
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "edges"))
	TArray<FAssetEdge> Edges;
};
