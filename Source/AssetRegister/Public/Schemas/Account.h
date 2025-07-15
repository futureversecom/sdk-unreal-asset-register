#pragma once
#include "Profile.h"

#include "Account.generated.h"

USTRUCT(BlueprintType)
struct ASSETREGISTER_API FAccount
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> AdditionalProperties;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Address;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Eoa;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Futurepass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LastUsedProfileByExperience;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FProfile> Profiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Rns;
};
