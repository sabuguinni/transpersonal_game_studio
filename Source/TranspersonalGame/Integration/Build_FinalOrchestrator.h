#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationPhase : uint8
{
    PreValidation,
    SystemLoading,
    CrossSystemValidation,
    PerformanceValidation,
    FinalIntegration,
    Complete,
    Failed
};

USTRUCT(BlueprintType)
struct FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationErrors;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsValidated = false;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallIntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime IntegrationTimestamp;

    FBuild_IntegrationReport()
    {
        CurrentPhase = EBuild_IntegrationPhase::PreValidation;
        OverallIntegrationScore = 0.0f;
        TotalActorsInLevel = 0;
        MemoryUsagePercent = 0.0f;
        bBuildSuccessful = false;
        IntegrationTimestamp = FDateTime::Now();
    }
};

/**
 * Final Build Orchestrator - Integration Agent #19
 * Coordinates all systems, validates cross-system integration, and ensures build stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformCrossSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetCurrentIntegrationReport() const { return CurrentReport; }

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterSystem(const FString& SystemName);

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegrity(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateLevelState();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetSystemPerformanceScore(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallPerformanceScore();

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool RestoreBuildSnapshot(int32 SnapshotIndex);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupFailedIntegration();

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationPhaseChanged, EBuild_IntegrationPhase, NewPhase);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemValidated, FString, SystemName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationComplete, bool, bSuccess);

    UPROPERTY(BlueprintAssignable, Category = "Integration Events")
    FOnIntegrationPhaseChanged OnIntegrationPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Integration Events")
    FOnSystemValidated OnSystemValidated;

    UPROPERTY(BlueprintAssignable, Category = "Integration Events")
    FOnIntegrationComplete OnIntegrationComplete;

protected:
    // Core Integration Data
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_IntegrationReport> BuildSnapshots;

    // Integration Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    float MaxMemoryUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    float MinPerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    int32 MaxBuildSnapshots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bAutoCleanupOnFailure;

    // Internal Functions
    void UpdateIntegrationPhase(EBuild_IntegrationPhase NewPhase);
    void ValidateSystemDependencies();
    void CheckSystemCompatibility();
    void MonitorSystemPerformance();
    void LogIntegrationStatus();

    // System-specific validation
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateAISystems();
    bool ValidateVFXSystems();
    bool ValidateQASystems();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckMemoryUsage();
    void ValidateFrameRate();

    // Error handling
    void HandleIntegrationError(const FString& ErrorMessage);
    void RecordValidationError(const FString& SystemName, const FString& Error);
};