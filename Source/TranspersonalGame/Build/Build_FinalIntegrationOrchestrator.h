#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString LastValidationTime;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsOperational = false;
        ActorCount = 0;
        PerformanceScore = 0.0f;
        LastValidationTime = TEXT("Never");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float OverallHealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bBuildStable;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString BuildTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CriticalIssues;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        OverallHealthScore = 0.0f;
        bBuildStable = false;
        BuildTimestamp = TEXT("Unknown");
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

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateSystemReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void PerformIntegrationHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetCriticalSystemErrors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float CalculateSystemPerformanceScore(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterSystemForMonitoring(const FString& SystemName, AActor* SystemActor);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsSystemOperational(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void TriggerEmergencySystemRestart(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogIntegrationEvent(const FString& EventMessage, bool bIsError = false);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, FBuild_SystemStatus> MonitoredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> SystemValidationLog;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastHealthCheckTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIntegrationSystemActive;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxAllowedActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float MinimumPerformanceThreshold;

private:
    void InitializeSystemMonitoring();
    void ValidateWorldState();
    void CheckSystemDependencies();
    void UpdateSystemMetrics();
    void ProcessSystemAlerts();
    bool ValidateActorIntegrity(AActor* Actor);
    void CleanupOrphanedActors();
    void OptimizeSystemPerformance();
    void GeneratePerformanceReport();
    void HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationMonitorActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationMonitorActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration Monitor")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Monitor")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Monitor")
    bool bAutoRestartFailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Monitor")
    FBuild_IntegrationReport LastReport;

private:
    float TimeSinceLastCheck;
    UBuild_FinalIntegrationOrchestrator* IntegrationOrchestrator;

    void PerformScheduledHealthCheck();
    void UpdateMonitoringDisplay();
    void HandleSystemAlerts();
};