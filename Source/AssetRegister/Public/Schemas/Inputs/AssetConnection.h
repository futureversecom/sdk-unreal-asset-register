#pragma once
#include "AssetConnection.generated.h"

USTRUCT()
struct FAssetConnection
{
	GENERATED_BODY()
	
	FAssetConnection(){}
	
	UPROPERTY(EditAnywhere, meta=(QueryName = "addresses"))
	TArray<FString> Addresses;
};
