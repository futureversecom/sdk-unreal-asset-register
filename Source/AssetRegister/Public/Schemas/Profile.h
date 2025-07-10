#pragma once
#include "Avatar.h"
#include "ProfileProperties.h"

#include "Profile.generated.h"

USTRUCT(BlueprintType)
struct FProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "additionalProperties"))
	TMap<FString, FString> AdditionalProperties;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "avatar"))
	FAvatar Avatar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "displayName"))
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "owner"))
	FString Owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "profileId"))
	FString ProfileId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "profileProperties"))
	FProfileProperties ProfileProperties;
};
