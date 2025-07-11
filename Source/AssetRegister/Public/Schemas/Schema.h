#pragma once

#include "Schema.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FSchema
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "name"))
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "namespace"))
	FString NameSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "schema"))
	FString Schema;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "version"))
	int32 Version;
};
