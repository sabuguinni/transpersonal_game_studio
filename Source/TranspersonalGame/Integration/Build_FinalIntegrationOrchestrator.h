#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ValidationMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ValidSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalIntegrationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        ValidSystemCount = 0;
        TotalIntegrationTime = 0.0f;
        OverallStatus = EBuild_IntegrationStatus::Pending;
    }
};

/**
 * Final Integration Orchestrator - Coordinates all game systems and validates build integrity
 * Responsible for ensuring all agent outputs work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetLastIntegrationReport() const { return LastIntegrationReport; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateLightingAndAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePhysicsAndCollision();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateQuestAndNarrative();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastIntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationInProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bVerboseLogging;

private:
    void ValidateSystem(const FString& SystemName, TFunction<bool()> ValidationFunction);
    void LogIntegrationResult(const FString& Message, bool bSuccess = true);
    void ResetIntegrationReport();
};