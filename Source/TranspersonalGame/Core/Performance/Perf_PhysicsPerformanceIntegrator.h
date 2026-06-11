#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_PhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsQualityLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationMode : uint8
{
    Performance     UMETA(DisplayName = "Performance Focused"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Quality         UMETA(DisplayName = "Quality Focused"),
    Adaptive        UMETA(DisplayName = "Adaptive")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActiveRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsSimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    bool bIsPerformanceTarget;

    FPerf_PhysicsPerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        ActivePhysicsActors = 0;
        ActiveRagdolls = 0;
        PhysicsMemoryUsageMB = 0.0f;
        PhysicsSimulationTime = 0.0f;
        bIsPerformanceTarget = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    EPerf_PhysicsQualityLevel QualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    EPerf_PhysicsOptimizationMode OptimizationMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float MaxPhysicsDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    int32 MaxActiveRagdolls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float RagdollCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    float PhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableAdaptiveQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Optimization")
    bool bEnableRagdollPooling;

    FPerf_PhysicsOptimizationSettings()
    {
        QualityLevel = EPerf_PhysicsQualityLevel::High;
        OptimizationMode = EPerf_PhysicsOptimizationMode::Balanced;
        MaxPhysicsDistance = 5000.0f;
        MaxActiveRagdolls = 10;
        RagdollCullingDistance = 3000.0f;
        PhysicsUpdateRate = 60.0f;
        bEnableAdaptiveQuality = true;
        bEnablePhysicsLOD = true;
        bEnableRagdollPooling = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics GetPhysicsPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPerformanceTargetMet() const;

    // Physics Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel NewQualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationMode(EPerf_PhysicsOptimizationMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeRagdollSystems();

    // Adaptive Quality Control
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void UpdateAdaptiveQuality();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void AdjustQualityBasedOnPerformance();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UpdatePhysicsLOD();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetPhysicsLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    int32 GetPhysicsLODLevel(float Distance) const;

    // Ragdoll Management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void ManageRagdollPool();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    void CullDistantRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Management")
    int32 GetActiveRagdollCount() const;

    // Memory Optimization
    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void OptimizePhysicsMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    float GetPhysicsMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Optimization")
    void CleanupUnusedPhysicsAssets();

    // Debug and Profiling
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DisplayPerformanceHUD(bool bShow);

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    FPerf_PhysicsOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    FPerf_PhysicsPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxPhysicsMemoryMB;

    // Optimization State
    UPROPERTY(BlueprintReadOnly, Category = "Optimization State")
    bool bIsOptimizing;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization State")
    float LastOptimizationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization State")
    int32 OptimizationCycles;

    // Performance Tracking
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    TArray<int32> PhysicsActorCountHistory;

    UPROPERTY()
    float PerformanceUpdateInterval;

    UPROPERTY()
    float LastPerformanceUpdate;

    // Physics Actor Management
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ActiveRagdolls;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, int32> ActorLODLevels;

private:
    // Internal Methods
    void InitializePerformanceTracking();
    void UpdateFrameTimeHistory(float DeltaTime);
    void ScanForPhysicsActors();
    void ApplyQualitySettings();
    void ProcessAdaptiveQuality();
    float CalculateDistanceToPlayer(AActor* Actor) const;
    void ApplyPhysicsLOD(AActor* Actor, int32 LODLevel);
    void OptimizeActorPhysics(AActor* Actor);
    void CleanupWeakReferences();
};