// Copyright (c) 2025, Futureverse Corporation Limited. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetOwnership.generated.h"

USTRUCT()
struct ASSETREGISTER_API FAssetOwnership
{
	GENERATED_BODY()
};

UCLASS()
class ASSETREGISTER_API UAssetOwnershipObject : public UObject
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)                                                                        
struct ASSETREGISTER_API FAssetOwnershipWrapper                                                    
{                                                                                             
	GENERATED_BODY()                                                                          

	UPROPERTY(EditAnywhere, BlueprintReadWrite)                                        
	UAssetOwnershipObject* Ownership ;                                                        
}; 