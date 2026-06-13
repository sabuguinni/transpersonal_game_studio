#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown = 0,
    Initializing = 1,
    Ready = 2,
    Warning = 3,
    Error = 4,
    Critical = 5
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float HealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        HealthScore = 0.0f;
        LastError = TEXT("");
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StaticMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 SkeletalMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LightActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CustomActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    FBuild_PerformanceMetrics()
    {
        TotalActors = 0;
        StaticMeshActors = 0;
        SkeletalMeshActors = 0;
        LightActors = 0;
        CustomActors = 0;
        MemoryUsageMB = 0.0f;
        FrameTimeMS = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemHealth> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadWrite, Category = "Integration")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadWrite, Category = "Integration")
    float HealthCheckInterval;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformFullSystemCheck();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ReportSystemError(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunIntegrationTests();

private:
    UPROPERTY()
    float LastHealthCheckTime;

    UPROPERTY()
    TMap<FString, float> SystemHealthScores;

    void UpdateSystemHealth();
    void CalculatePerformanceMetrics();
    void CheckCriticalSystems();
    void ValidateActorCounts();
    void CleanupExcessActors();
};