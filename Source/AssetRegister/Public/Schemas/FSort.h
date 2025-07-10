#pragma once
#include "Enums/SortOrder.h"

#include "FSort.generated.h"

USTRUCT(BlueprintType)
struct FSort
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "name"))
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "order"))
	ESortOrder Order;
};
