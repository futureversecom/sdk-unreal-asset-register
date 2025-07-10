#pragma once
#include "AssetLink.generated.h"

USTRUCT()
struct FAssetLinkData
{
	GENERATED_BODY()
};

UCLASS()
class ASSETREGISTER_API UAssetLink : public UObject
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)                                                                        
struct ASSETREGISTER_API FAssetLinkWrapper                                                    
{                                                                                             
	GENERATED_BODY()                                                                          

	UPROPERTY(EditAnywhere, BlueprintReadWrite)                                        
	UAssetLink* Links;                                                        
};  

