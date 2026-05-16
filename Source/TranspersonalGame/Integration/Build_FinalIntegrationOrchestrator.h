#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
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
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float PerformanceMetric;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FDateTime LastChecked;

    FBuild_SystemHealthReport()
    {
        SystemName = TEXT("Unknown");
        Status = EBuild_SystemStatus::Unknown;
        ActorCount = 0;
        PerformanceMetric = 0.0f;
        LastError = TEXT("");
        LastChecked = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CharacterActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 EnvironmentActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 SystemActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsOperational;

    FBuild_IntegrationMetrics()
    {
        TotalActors = 0;
        CharacterActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        SystemActors = 0;
        AverageFrameTime = 0.0f;
        MemoryUsageMB = 0.0f;
        bAllSystemsOperational = false;
    }
};

/**
 * Final Integration Orchestrator - Manages and validates all game systems
 * Ensures proper integration between all 18 agent outputs
 * Provides real-time health monitoring and performance metrics
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
    void PerformFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationMetrics GetCurrentIntegrationMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemHealthReport> GetAllSystemHealthReports();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegrity(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ForceSystemReconciliation();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits();

    // System Health Checks
    UFUNCTION(BlueprintCallable, Category = "Health")
    EBuild_SystemStatus CheckWorldGenerationHealth();

    UFUNCTION(BlueprintCallable, Category = "Health")
    EBuild_SystemStatus CheckCharacterSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "Health")
    EBuild_SystemStatus CheckDinosaurAIHealth();

    UFUNCTION(BlueprintCallable, Category = "Health")
    EBuild_SystemStatus CheckEnvironmentHealth();

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyShutdown(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    bool AttemptSystemRecovery(const FString& SystemName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemHealthReport> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastFullValidation;

private:
    void UpdateSystemHealthReport(const FString& SystemName, EBuild_SystemStatus Status, const FString& ErrorMessage = TEXT(""));
    void CollectPerformanceMetrics();
    void ValidateActorCounts();
    void CheckSystemDependencies();
    
    FTimerHandle ValidationTimerHandle;
    float ValidationInterval;
};