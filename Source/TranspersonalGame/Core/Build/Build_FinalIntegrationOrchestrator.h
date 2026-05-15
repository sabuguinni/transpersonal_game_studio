#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Healthy     UMETA(DisplayName = "Healthy"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString StatusMessage;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float PerformanceMetric;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    int32 ActorCount;

    FBuild_SystemHealthReport()
    {
        SystemName = TEXT("Unknown");
        Status = EBuild_SystemStatus::Unknown;
        StatusMessage = TEXT("Not initialized");
        PerformanceMetric = 0.0f;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsHealthy;

    FBuild_IntegrationMetrics()
    {
        TotalActors = 0;
        ActiveSystems = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        bAllSystemsHealthy = false;
    }
};

/**
 * Final Integration Orchestrator - Manages complete system integration and build validation
 * Coordinates all game systems and ensures stable operation
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

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationMetrics GetIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemHealthReport> GetSystemHealthReports();

    // System Health Monitoring
    UFUNCTION(BlueprintCallable, Category = "Health")
    EBuild_SystemStatus GetOverallSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "Health")
    void RunComprehensiveHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool AreAllSystemsOperational();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsage();

    // Build Validation
    UFUNCTION(BlueprintCallable, Category = "Build")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void SaveIntegrationState();

protected:
    // System Health Data
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemHealthReport> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationValidated;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FDateTime LastHealthCheck;

private:
    // Internal validation functions
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateFoliageSystem();
    void ValidatePhysicsSystems();
    void ValidateLightingSystem();
    void ValidateVFXSystems();
    void ValidateAudioSystems();

    // Helper functions
    void UpdateSystemHealth(const FString& SystemName, EBuild_SystemStatus Status, const FString& Message, float Metric = 0.0f);
    void LogIntegrationStatus();
    void CleanupInvalidActors();
};