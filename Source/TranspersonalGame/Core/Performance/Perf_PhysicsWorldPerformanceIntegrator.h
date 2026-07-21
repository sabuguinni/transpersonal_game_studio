#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Perf_PhysicsWorldPerformanceIntegrator.generated.h"

class UCore_PhysicsWorldManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 PhysicsConstraints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 CollisionChecks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsThreadTime = 0.0f;

    FPerf_PhysicsPerformanceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActiveRigidBodies = 0;
        PhysicsConstraints = 0;
        PhysicsMemoryUsage = 0.0f;
        CollisionChecks = 0;
        PhysicsThreadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxActiveRigidBodies = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAsyncPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsSubstepDelta = 0.0166f;

    FPerf_PhysicsOptimizationSettings()
    {
        MaxPhysicsFrameTime = 16.67f;
        MaxActiveRigidBodies = 1000;
        PhysicsLODDistance = 5000.0f;
        bEnablePhysicsLOD = true;
        bEnableAsyncPhysics = true;
        PhysicsSubstepDelta = 0.0166f;
    }
};

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Custom      UMETA(DisplayName = "Custom")
};

/**
 * Performance optimizer specifically designed to integrate with the Core_PhysicsWorldManager
 * Monitors physics performance and applies optimizations to maintain target frame rates
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsWorldPerformanceIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsWorldPerformanceIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceOptimal() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ApplyPhysicsOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void EnablePhysicsLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsLODDistance(float Distance);

    // Integration with physics world manager
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithPhysicsWorldManager();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizeRegionalPhysics(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsPerformanceBudget();

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Debug")
    void ValidatePhysicsPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Debug")
    void LogPhysicsPerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Debug")
    void TestPhysicsOptimizations();

protected:
    // Core performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    EPerf_PhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    bool bIsMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    float LastPerformanceCheckTime;

    // Physics world manager integration
    UPROPERTY()
    UCore_PhysicsWorldManager* PhysicsWorldManager;

    UPROPERTY(BlueprintReadOnly, Category = "Integration State")
    bool bIsIntegratedWithPhysicsWorld;

    // Performance optimization data
    UPROPERTY()
    TArray<AActor*> PhysicsActorsToOptimize;

    UPROPERTY()
    TMap<EBiomeType, FPerf_PhysicsOptimizationSettings> BiomeOptimizationSettings;

private:
    // Internal performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyAutomaticOptimizations();

    // Physics optimization implementation
    void OptimizePhysicsActors();
    void UpdatePhysicsLOD();
    void ManagePhysicsMemory();

    // Integration helpers
    void InitializePhysicsWorldIntegration();
    void SynchronizeWithPhysicsWorld();
    void UpdateRegionalOptimizations();

    // Performance calculation helpers
    float CalculatePhysicsFrameTime() const;
    int32 CountActiveRigidBodies() const;
    float EstimatePhysicsMemoryUsage() const;

    // Timer handles
    FTimerHandle PerformanceMonitoringTimer;
    FTimerHandle OptimizationUpdateTimer;
};