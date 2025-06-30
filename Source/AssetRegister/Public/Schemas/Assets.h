#pragma once
#include "Asset.h"
#include "Assets.generated.h"

USTRUCT()
struct FAssetEdge
{
	GENERATED_BODY()

	UPROPERTY(meta=(QueryName = "node"))
	FAsset Node;
};

USTRUCT(meta=(QueryName = "assets"))
struct FAssets
{
	GENERATED_BODY()

	UPROPERTY(meta=(QueryName = "edges"))
	TArray<FAssetEdge> Edges;
};
