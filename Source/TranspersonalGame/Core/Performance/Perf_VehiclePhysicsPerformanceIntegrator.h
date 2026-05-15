#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Perf_VehiclePhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_VehiclePerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehiclePhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveVehicleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionComplexity;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FPerf_VehiclePhysicsMetrics()
        : PhysicsUpdateTime(0.0f)
        , ActiveVehicleCount(0)
        , AverageVelocity(0.0f)
        , CollisionComplexity(0.0f)
        , MemoryUsageMB(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehicleOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActiveVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableComplexCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODSystem;

    FPerf_VehicleOptimizationSettings()
        : MaxActiveVehicles(20)
        , PhysicsUpdateRate(60.0f)
        , bEnableComplexCollision(true)
        , CullingDistance(10000.0f)
        , bEnableLODSystem(true)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_VehiclePhysicsPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_VehiclePhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_VehiclePerformanceLevel PerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_VehicleOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_VehiclePhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAutomaticOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFrameRate;

private:
    float LastPerformanceUpdate;
    TArray<AActor*> TrackedVehicles;
    float AccumulatedPhysicsTime;
    int32 PhysicsUpdateCount;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_VehiclePerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_VehiclePhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterVehicle(AActor* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterVehicle(AActor* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableVehicleLOD(AActor* Vehicle, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVehiclePhysicsComplexity(AActor* Vehicle, bool bUseComplexCollision);

protected:
    void AnalyzeVehiclePerformance();
    void ApplyPerformanceLevelSettings();
    void CullDistantVehicles();
    void OptimizePhysicsSettings();
    float CalculateAverageVelocity() const;
    float CalculateCollisionComplexity() const;
    float EstimateMemoryUsage() const;
    void AdjustPhysicsUpdateRate();
    void ManageActiveVehicleCount();
};