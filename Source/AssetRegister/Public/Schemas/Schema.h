#pragma once

#include "Schema.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FSchema
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NameSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Schema;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;
};
