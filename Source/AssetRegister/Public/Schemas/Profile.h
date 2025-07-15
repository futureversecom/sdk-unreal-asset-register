#pragma once
#include "Avatar.h"
#include "ProfileProperties.h"

#include "Profile.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> AdditionalProperties;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAvatar Avatar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProfileId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FProfileProperties ProfileProperties;
};
