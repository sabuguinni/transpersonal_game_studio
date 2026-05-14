#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Initializing    UMETA(DisplayName = "Initializing"),
    Validating      UMETA(DisplayName = "Validating Systems"),
    Integrating     UMETA(DisplayName = "Integrating Components"),
    Testing         UMETA(DisplayName = "Running Tests"),
    Stable          UMETA(DisplayName = "Stable"),
    Failed          UMETA(DisplayName = "Failed"),
    Ready           UMETA(DisplayName = "Ready for Deployment")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsOperational = false;
        ValidationMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 OperationalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 WorldActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CharacterInstances;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float TotalValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus BuildStatus;

    FBuild_IntegrationMetrics()
    {
        TotalSystems = 0;
        LoadedSystems = 0;
        OperationalSystems = 0;
        WorldActorCount = 0;
        CharacterInstances = 0;
        TotalValidationTime = 0.0f;
        BuildStatus = EBuild_IntegrationStatus::Initializing;
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Coordinates and validates the integration of all game systems
 * Ensures build stability and deployment readiness
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void GenerateBuildReport();

    // System validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateSystem(const FString& SystemName, const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateWorldState();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateQASystems();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PrepareBuildPackaging();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void FinalizeIntegration();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Integration")
    FBuild_IntegrationMetrics GetIntegrationMetrics() const { return IntegrationMetrics; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_IntegrationStatus GetBuildStatus() const { return IntegrationMetrics.BuildStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsBuildReady() const { return IntegrationMetrics.BuildStatus == EBuild_IntegrationStatus::Ready; }

protected:
    // Integration state
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalSystemPaths;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationStartTime;

    // Internal functions
    void InitializeCriticalSystems();
    void UpdateIntegrationMetrics();
    void LogIntegrationStatus();
    FBuild_SystemValidationResult ValidateSystemInternal(const FString& SystemName, const FString& ClassPath);

private:
    // Integration timers
    FTimerHandle IntegrationTimerHandle;
    FTimerHandle ValidationTimerHandle;

    // Validation callbacks
    void OnIntegrationComplete();
    void OnValidationComplete();
};