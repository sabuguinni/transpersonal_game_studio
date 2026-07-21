#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_PerformanceManager.generated.h"

class UCrowd_LODComponent;
class UCrowd_StreamingComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveCrowdAgents = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 VisibleCrowdAgents = 0;

    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CrowdSystemCPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 LODLevel0Count = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 LODLevel1Count = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 LODLevel2Count = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CulledAgentsCount = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxCrowdAgents = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance0 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance1 = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistance2 = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableFrustumCulling = true;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    FCrowd_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void SetPerformanceSettings(const FCrowd_PerformanceSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    FCrowd_PerformanceSettings GetPerformanceSettings() const { return Settings; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void OptimizeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    bool ShouldSpawnNewAgent() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    int32 GetOptimalLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void RegisterCrowdAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void UnregisterCrowdAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void EnableEmergencyOptimization();

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void DisableEmergencyOptimization();

private:
    UPROPERTY()
    FCrowd_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    FCrowd_PerformanceSettings Settings;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredAgents;

    UPROPERTY()
    bool bEmergencyOptimizationActive = false;

    UPROPERTY()
    float LastOptimizationTime = 0.0f;

    UPROPERTY()
    float OptimizationInterval = 1.0f;

    void UpdateLODLevels();
    void UpdateCulling();
    void PerformEmergencyOptimization();
    bool IsAgentVisible(AActor* Agent) const;
    float GetDistanceToPlayer(AActor* Agent) const;
};