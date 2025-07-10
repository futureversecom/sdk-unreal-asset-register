#pragma once
#include "Asset.h"
#include "Assets.generated.h"

USTRUCT(BlueprintType)
struct FPageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "startCursor"))
	FString StartCursor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "endCursor"))
	FString EndCursor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "hasNextPage"))
	bool HasNextPage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "hasPreviousPage"))
	FString HasPreviousPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "nextPage"))
	FString NextPage;
};

USTRUCT(BlueprintType)
struct FAssetEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "cursor"))
	FString Cursor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "node"))
	FAsset Node;
};

USTRUCT(Blueprintable, meta=(QueryName = "assets"))
struct FAssets
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "edges"))
	TArray<FAssetEdge> Edges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "pageInfo"))
	FPageInfo PageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "total"))
	float Total = 0;
};
