#pragma once
#include "Asset.h"
#include "Assets.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FPageInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EndCursor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasNextPage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasPreviousPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NextPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StartCursor;
};

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAssetEdge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Cursor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAsset Node;
};

USTRUCT(Blueprintable)
struct ASSETREGISTER_API FAssets
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAssetEdge> Edges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPageInfo PageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Total = 0;
};
