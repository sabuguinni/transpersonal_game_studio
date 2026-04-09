// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsOptimizer.generated.h"

/** Physics optimization levels */
UENUM(BlueprintType)
enum class EPhysicsOptimizationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Low             UMETA(DisplayName = "Low Optimization"),
    Medium          UMETA(DisplayName = "Medium Optimization"),
    High            UMETA(DisplayName = "High Optimization"),
    Aggressive      UMETA(DisplayName = "Aggressive Optimization")
};

/** Physics LOD settings */
USTRUCT(BlueprintType)
struct FPhysicsLODSettings
{
    GENERATED_BODY()

    /** Distance at which this LOD level activates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float Distance = 1000.0f;

    /** Physics simulation frequency multiplier (1.0 = full rate, 0.5 = half rate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float SimulationFrequency = 1.0f;

    /** Collision complexity level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    /** Whether to use simplified collision shapes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseSimplifiedCollision = false;

    /** Maximum number of physics bodies at this LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxPhysicsBodies = 1000;
};

/**
 * @brief Physics Optimization System for large-scale dinosaur simulation
 * 
 * Manages physics performance through dynamic LOD, culling, and optimization
 * techniques to maintain stable framerates with hundreds of physics objects.
 * 
 * Key Features:
 * - Distance-based physics LOD
 * - Dynamic physics body culling
 * - Adaptive simulation frequency
 * - Performance monitoring and auto-adjustment
 * - Memory pool management for physics objects
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS()
class TRANSPERSONALGAME_API UPhysicsOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    /** Get the physics optimizer instance */
    UFUNCTION(BlueprintPure, Category = "Physics Optimization")
    static UPhysicsOptimizer* Get(const UObject* WorldContext);

    /** Set global physics optimization level */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(EPhysicsOptimizationLevel Level);

    /** Configure physics LOD settings */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ConfigurePhysicsLOD(const TArray<FPhysicsLODSettings>& LODSettings);

    /** Register an actor for physics optimization */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void RegisterPhysicsActor(AActor* Actor, int32 Priority = 0);

    /** Unregister an actor from physics optimization */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UnregisterPhysicsActor(AActor* Actor);

    /** Update optimization based on current performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void UpdateOptimization(FVector ViewerLocation);

    /** Get current physics performance metrics */
    UFUNCTION(BlueprintPure, Category = "Physics Optimization")
    void GetPerformanceMetrics(float& FrameTime, int32& ActiveBodies, float& PhysicsTime) const;

    /** Enable/disable automatic optimization */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetAutoOptimizationEnabled(bool bEnabled);

    /** Force optimization update for all registered actors */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ForceOptimizationUpdate();

protected:
    /** Current optimization level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization")
    EPhysicsOptimizationLevel CurrentOptimizationLevel = EPhysicsOptimizationLevel::Medium;

    /** Physics LOD settings array */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD")
    TArray<FPhysicsLODSettings> PhysicsLODSettings;

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "16.0", ClampMax = "33.0"))
    float TargetFrameTime = 16.67f; // 60 FPS

    /** Maximum physics simulation time per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float MaxPhysicsTime = 5.0f;

    /** Enable automatic optimization based on performance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization")
    bool bAutoOptimizationEnabled = true;

    /** Optimization update frequency in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float OptimizationUpdateFrequency = 0.5f;

private:
    /** Registered physics actors with their priorities */
    UPROPERTY()
    TMap<AActor*, int32> RegisteredActors;

    /** Current viewer location for distance calculations */
    FVector CurrentViewerLocation = FVector::ZeroVector;

    /** Performance tracking */
    float LastFrameTime = 0.0f;
    float LastPhysicsTime = 0.0f;
    int32 LastActiveBodies = 0;

    /** Timer handles */
    FTimerHandle OptimizationUpdateTimer;
    FTimerHandle PerformanceMonitorTimer;

    /** Initialize default LOD settings */
    void InitializeDefaultLODSettings();

    /** Apply optimization to a specific actor */
    void ApplyOptimizationToActor(AActor* Actor, const FPhysicsLODSettings& LODSettings);

    /** Calculate LOD level for an actor based on distance */
    int32 CalculateLODLevel(AActor* Actor, FVector ViewerLocation) const;

    /** Update performance metrics */
    void UpdatePerformanceMetrics();

    /** Auto-adjust optimization based on performance */
    void AutoAdjustOptimization();

    /** Get distance from viewer to actor */
    float GetDistanceToViewer(AActor* Actor) const;

    /** Apply LOD settings to actor's physics components */
    void ApplyLODToPhysicsComponents(AActor* Actor, const FPhysicsLODSettings& LODSettings);
};