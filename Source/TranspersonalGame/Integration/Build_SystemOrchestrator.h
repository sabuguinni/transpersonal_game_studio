#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Build_SystemOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationPhase : uint8
{
    Initialization,
    CoreSystemsValidation,
    CrossSystemIntegration,
    PerformanceOptimization,
    FinalValidation,
    BuildComplete
};

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    NotLoaded,
    Loading,
    Loaded,
    Validated,
    Integrated,
    Failed
};

USTRUCT(BlueprintType)
struct FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float LoadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bIsPerformant;

    FBuild_SystemMetrics()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::NotLoaded;
        LoadTime = 0.0f;
        ActorCount = 0;
        MemoryUsage = 0.0f;
        bIsPerformant = true;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    EBuild_IntegrationPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FBuild_SystemMetrics> SystemMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float TotalMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    bool bBuildReady;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString LastError;

    FBuild_IntegrationReport()
    {
        CurrentPhase = EBuild_IntegrationPhase::Initialization;
        TotalActors = 0;
        TotalMemoryUsage = 0.0f;
        AverageFrameTime = 0.0f;
        bBuildReady = false;
        LastError = TEXT("");
    }
};

/**
 * Build System Orchestrator - Manages integration of all game systems
 * Coordinates between VFX, QA, World Generation, Character, and Core systems
 * Provides comprehensive build validation and performance monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_SystemOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_SystemOrchestrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void InitializeSystemOrchestration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void PerformCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void OptimizeSystemPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void GenerateFinalBuildReport();

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCoreSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateQASystem();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSystem();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float MeasureSystemPerformance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorFrameTime();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsSystemPerformant(const FString& SystemName);

    // Integration Management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateVFXWithQA();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWorldWithCharacter();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateAllSystems();

    // Build Management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build")
    void PrepareBuildEnvironment();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build")
    void ExecuteBuildValidation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build")
    void FinalizeBuild();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void ExportBuildMetrics();

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Integration State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBuild_IntegrationPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FBuild_IntegrationReport IntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> CriticalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, EBuild_SystemStatus> SystemStatusMap;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FrameTimeHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MemoryThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoOptimize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bVerboseLogging;

private:
    // Internal Functions
    void InitializeCriticalSystems();
    void UpdateSystemStatus(const FString& SystemName, EBuild_SystemStatus Status);
    void CalculatePerformanceMetrics();
    void ValidateSystemDependencies();
    void OptimizeSystemResources();
    void GenerateSystemReport(const FString& SystemName);
    bool CheckSystemCompatibility(const FString& System1, const FString& System2);
    void CleanupUnusedResources();
    void UpdateIntegrationPhase(EBuild_IntegrationPhase NewPhase);

    // Timing
    float LastValidationTime;
    float ValidationInterval;
    bool bIsValidating;
};