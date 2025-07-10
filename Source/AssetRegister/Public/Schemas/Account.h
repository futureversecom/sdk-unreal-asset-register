#pragma once
#include "Profile.h"

#include "Account.generated.h"

USTRUCT(BlueprintType)
struct FAccount
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "additionalProperties"))
	TMap<FString, FString> AdditionalProperties;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "address"))
	FString Address;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "eoa"))
	FString Eoa;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "futurepass"))
	FString Futurepass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "handle"))
	FString Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "id"))
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "lastUsedProfileByExperience"))
	FString LastUsedProfileByExperience;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "profiles"))
	TArray<FProfile> Profiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(QueryName = "rns"))
	FString Rns;
};
