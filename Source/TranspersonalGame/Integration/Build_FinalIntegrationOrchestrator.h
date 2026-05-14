#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationPhase : uint8
{
    PreValidation,
    SystemDiscovery,
    CrossSystemValidation,
    PerformanceValidation,
    FinalIntegration,
    PostIntegrationTest,
    Complete
};

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown,
    Initializing,
    Operational,
    Warning,
    Error,
    Disabled
};

USTRUCT(BlueprintType)
struct FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly)
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly)
    bool bCriticalSystem;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        ValidationMessage = TEXT("");
        PerformanceScore = 0.0f;
        ActorCount = 0;
        bCriticalSystem = false;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EBuild_IntegrationPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly)
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly)
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly)
    FString BuildTimestamp;

    UPROPERTY(BlueprintReadOnly)
    bool bBuildStable;

    FBuild_IntegrationReport()
    {
        CurrentPhase = EBuild_IntegrationPhase::PreValidation;
        OverallHealthScore = 0.0f;
        TotalActorCount = 0;
        AverageFrameTime = 0.0f;
        BuildTimestamp = TEXT("");
        bBuildStable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GetCurrentIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsSystemOperational(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ForceSystemRevalidation(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetSystemPerformanceScore(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UpdatePerformanceMetrics();

    // Cross-System Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetCriticalSystemDependencies() const;

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void MarkBuildStable();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void MarkBuildUnstable(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsBuildStable() const { return CurrentReport.bBuildStable; }

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrationPhaseChanged, EBuild_IntegrationPhase, NewPhase);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemStatusChanged, FBuild_SystemValidationResult, SystemResult);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildStabilityChanged, bool, bIsStable);

    UPROPERTY(BlueprintAssignable)
    FOnIntegrationPhaseChanged OnIntegrationPhaseChanged;

    UPROPERTY(BlueprintAssignable)
    FOnSystemStatusChanged OnSystemStatusChanged;

    UPROPERTY(BlueprintAssignable)
    FOnBuildStabilityChanged OnBuildStabilityChanged;

protected:
    // Internal Validation Functions
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateAISystems();
    void ValidateVFXSystems();
    void ValidateQASystems();

    // Performance Analysis
    void AnalyzeFrameTime();
    void AnalyzeMemoryUsage();
    void AnalyzeActorCounts();

    // System Discovery
    void DiscoverAvailableSystems();
    void RegisterSystemValidators();

    // Integration Phases
    void ExecutePreValidation();
    void ExecuteSystemDiscovery();
    void ExecuteCrossSystemValidation();
    void ExecutePerformanceValidation();
    void ExecuteFinalIntegration();
    void ExecutePostIntegrationTest();

private:
    UPROPERTY()
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY()
    TMap<FString, FBuild_SystemValidationResult> SystemValidationCache;

    UPROPERTY()
    TArray<FString> CriticalSystems;

    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    bool bIntegrationInProgress;

    UPROPERTY()
    EBuild_IntegrationPhase CurrentPhase;

    // Performance Tracking
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    float PerformanceValidationThreshold;

    // System Dependencies
    UPROPERTY()
    TMap<FString, TArray<FString>> SystemDependencies;

    // Validation Timers
    FTimerHandle ValidationTimerHandle;
    FTimerHandle PerformanceTimerHandle;

    // Constants
    static constexpr float VALIDATION_INTERVAL = 5.0f;
    static constexpr float PERFORMANCE_THRESHOLD = 60.0f;
    static constexpr int32 MAX_FRAME_HISTORY = 100;
};