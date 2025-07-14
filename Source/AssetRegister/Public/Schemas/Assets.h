#pragma once
#include "Asset.h"
#include "Assets.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FPageInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "endCursor"))
	FString EndCursor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "hasNextPage"))
	bool HasNextPage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "hasPreviousPage"))
	bool HasPreviousPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "nextPage"))
	FString NextPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "startCursor"))
	FString StartCursor;
};

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "cursor"))
	FString Cursor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "node"))
	FAsset Node;
};

USTRUCT(Blueprintable, meta=(QueryName = "assets"))
struct ASSETREGISTER_API FAssets
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "edges"))
	TArray<FAssetEdge> Edges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "pageInfo"))
	FPageInfo PageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "total"))
	float Total = 0;
};
