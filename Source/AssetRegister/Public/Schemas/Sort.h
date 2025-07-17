#pragma once
#include "Enums/SortOrder.h"

#include "Sort.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FSort
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESortOrder Order = ESortOrder::ASC;
};
