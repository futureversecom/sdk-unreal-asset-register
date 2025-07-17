#pragma once
#include "AssetLink.generated.h"

USTRUCT()
struct ASSETREGISTER_API FAssetLink
{
	GENERATED_BODY()
};

UCLASS()
class ASSETREGISTER_API UAssetLinkObject : public UObject
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)                                                                        
struct ASSETREGISTER_API FAssetLinkWrapper                                                    
{                                                                                             
	GENERATED_BODY()                                                                          

	UPROPERTY(EditAnywhere, BlueprintReadWrite)                                        
	UAssetLinkObject* Links = nullptr;                                                        
};  

