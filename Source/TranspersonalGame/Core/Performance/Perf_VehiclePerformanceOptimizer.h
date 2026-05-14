#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_PerformanceMetrics.h"
#include "Perf_VehiclePerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehiclePerformanceConfig
{
    GENERATED_BODY()

    // LOD distances for vehicle components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle LOD")
    float HighDetailDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle LOD")
    float MediumDetailDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle LOD")
    float LowDetailDistance = 3000.0f;

    // Physics optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxPhysicsSubsteps = 4;

    // Rendering optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float CullingDistance = 5000.0f;

    // Performance budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float VehiclePhysicsBudgetMs = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float VehicleRenderBudgetMs = 3.0f;

    FPerf_VehiclePerformanceConfig()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1500.0f;
        LowDetailDistance = 3000.0f;
        bEnablePhysicsOptimization = true;
        PhysicsUpdateRate = 60.0f;
        MaxPhysicsSubsteps = 4;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = true;
        CullingDistance = 5000.0f;
        VehiclePhysicsBudgetMs = 2.0f;
        VehicleRenderBudgetMs = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehicleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalVehicles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 ActiveVehicles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 PhysicsEnabledVehicles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AveragePhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageRenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameImpactPercent = 0.0f;

    FPerf_VehicleMetrics()
    {
        TotalVehicles = 0;
        ActiveVehicles = 0;
        PhysicsEnabledVehicles = 0;
        AveragePhysicsTime = 0.0f;
        AverageRenderTime = 0.0f;
        MemoryUsageMB = 0.0f;
        FrameImpactPercent = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_VehiclePerformanceOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_VehiclePerformanceOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void OptimizeVehiclePerformance();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void UpdateVehicleLOD(AActor* VehicleActor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void OptimizeVehiclePhysics(AActor* VehicleActor, bool bIsNearPlayer);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void SetVehiclePerformanceConfig(const FPerf_VehiclePerformanceConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Vehicle Performance")
    FPerf_VehiclePerformanceConfig GetVehiclePerformanceConfig() const { return PerformanceConfig; }

    UFUNCTION(BlueprintPure, Category = "Vehicle Performance")
    FPerf_VehicleMetrics GetVehicleMetrics() const { return CurrentMetrics; }

    // Monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void UpdatePerformanceMetrics();

    // Vehicle registration
    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void RegisterVehicle(AActor* VehicleActor);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void UnregisterVehicle(AActor* VehicleActor);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance", CallInEditor = true)
    void RunVehiclePerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance", CallInEditor = true)
    void GeneratePerformanceReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FPerf_VehiclePerformanceConfig PerformanceConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_VehicleMetrics CurrentMetrics;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredVehicles;

    UPROPERTY()
    bool bIsMonitoring = false;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    float UpdateInterval = 0.1f; // Update every 100ms

private:
    void UpdateVehicleList();
    void CalculatePerformanceMetrics();
    float CalculateDistanceToPlayer(AActor* VehicleActor);
    void ApplyLODSettings(AActor* VehicleActor, int32 LODLevel);
    void OptimizeVehicleComponents(AActor* VehicleActor);
    void UpdatePhysicsSettings(AActor* VehicleActor, bool bHighDetail);

    // Performance tracking
    TMap<TWeakObjectPtr<AActor>, float> VehiclePhysicsTimes;
    TMap<TWeakObjectPtr<AActor>, float> VehicleRenderTimes;
    
    float TotalPhysicsTime = 0.0f;
    float TotalRenderTime = 0.0f;
    int32 FrameCounter = 0;
};