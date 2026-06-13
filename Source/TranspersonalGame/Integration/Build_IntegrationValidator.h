#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    NotLoaded,
    Loaded,
    Functional,
    Error
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    FBuild_SystemReport()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::NotLoaded;
        ErrorMessage = TEXT("");
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ReadySystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildReady;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        ReadySystems = 0;
        IntegrationScore = 0.0f;
        bBuildReady = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculateIntegrationScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorCaps();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxDinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CoreSystemClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> EssentialActorTypes;

private:
    FBuild_SystemReport ValidateSystem(const FString& SystemName, const FString& ClassName);
    void PruneExcessActors();
    int32 CountActorsByType(const FString& ActorType);
};