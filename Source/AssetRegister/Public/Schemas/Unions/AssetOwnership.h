// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetOwnership.generated.h"

USTRUCT()
struct ASSETREGISTER_API FAssetOwnershipData
{
	GENERATED_BODY()
};

UCLASS()
class ASSETREGISTER_API UAssetOwnership : public UObject
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)                                                                        
struct ASSETREGISTER_API FAssetOwnershipWrapper                                                    
{                                                                                             
	GENERATED_BODY()                                                                          

	UPROPERTY(EditAnywhere, BlueprintReadWrite)                                        
	UAssetOwnership* Ownership ;                                                        
}; 