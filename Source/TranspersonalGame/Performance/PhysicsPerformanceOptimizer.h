// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PerformanceTargets.h"
#include "../Core/PhysicsCore/PhysicsSystemManager.h"
#include "../Core/PhysicsCore/CollisionSystem.h"
#include "PhysicsPerformanceOptimizer.generated.h"

/**
 * @brief Physics Performance Optimizer for Jurassic Survival Game
 * 
 * Specialized performance optimization system that ensures physics systems
 * maintain 60fps on PC and 30fps on console while handling:
 * - Massive dinosaur ecosystems (up to 50,000 agents)
 * - Complex environmental destruction
 * - Ragdoll physics for creature deaths
 * - Multi-layered collision detection
 * 
 * Key Optimization Strategies:
 * - Dynamic LOD for physics complexity based on distance and importance
 * - Adaptive physics tick rates for different object categories
 * - Intelligent culling of physics bodies outside player influence
 * - Memory pool management for destruction chunks
 * - Hierarchical collision optimization
 * 
 * Performance Targets:
 * - Physics Budget: 2ms on PC (60fps), 4ms on Console (30fps)
 * - Max Physics Bodies: 10,000 active simultaneously
 * - Max Destruction Chunks: 5,000 active simultaneously
 * - Collision Tests: <500 complex tests per frame
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize physics performance optimization system
     * 
     * Sets up performance monitoring, LOD systems, and adaptive optimization
     * based on current hardware performance target
     * 
     * @param PerformanceTarget Target platform performance level
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void InitializePhysicsOptimization(EPerformanceTarget PerformanceTarget);

    /**
     * @brief Optimize physics systems for current frame conditions
     * 
     * Dynamically adjusts physics complexity, tick rates, and culling
     * based on current performance metrics and frame budget
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsForFrame();

    /**
     * @brief Set physics LOD for an actor based on importance and distance
     * 
     * @param Actor Actor to optimize
     * @param ViewerLocation Current viewer position (usually player)
     * @param ImportanceMultiplier Importance factor (1.0 = normal, 2.0 = high priority)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsLOD(AActor* Actor, FVector ViewerLocation, float ImportanceMultiplier = 1.0f);

    /**
     * @brief Optimize destruction system performance
     * 
     * Manages destruction chunk pools, culls distant fragments,
     * and limits active destruction events per frame
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeDestructionSystem();

    /**
     * @brief Optimize collision detection performance
     * 
     * Implements hierarchical collision culling, reduces collision complexity
     * for distant objects, and manages collision test budgets
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeCollisionDetection();

    /**
     * @brief Optimize ragdoll physics performance
     * 
     * Manages active ragdoll count, applies distance-based LOD,
     * and automatically cleans up old ragdolls
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeRagdollPhysics();

    /**
     * @brief Get current physics performance metrics
     * 
     * @return Current frame physics performance data
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPhysicsPerformanceMetrics GetCurrentPhysicsMetrics() const;

    /**
     * @brief Force immediate physics optimization pass
     * 
     * Emergency optimization when frame time exceeds budget.
     * Aggressively culls physics objects to maintain framerate.
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ForceEmergencyOptimization();

protected:
    /** Physics LOD levels */
    UENUM(BlueprintType)
    enum class EPhysicsLODLevel : uint8
    {
        Disabled    UMETA(DisplayName = "Disabled"),        // No physics simulation
        Minimal     UMETA(DisplayName = "Minimal"),         // Basic collision only
        Reduced     UMETA(DisplayName = "Reduced"),         // Simplified physics
        Standard    UMETA(DisplayName = "Standard"),        // Normal physics
        High        UMETA(DisplayName = "High"),            // Full physics detail
        Maximum     UMETA(DisplayName = "Maximum")          // Highest quality
    };

    /** Physics performance metrics structure */
    USTRUCT(BlueprintType)
    struct FPhysicsPerformanceMetrics
    {
        GENERATED_BODY()

        /** Current physics frame time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float PhysicsFrameTime = 0.0f;

        /** Number of active physics bodies */
        UPROPERTY(BlueprintReadOnly)
        int32 ActivePhysicsBodies = 0;

        /** Number of active destruction chunks */
        UPROPERTY(BlueprintReadOnly)
        int32 ActiveDestructionChunks = 0;

        /** Number of active ragdolls */
        UPROPERTY(BlueprintReadOnly)
        int32 ActiveRagdolls = 0;

        /** Number of collision tests this frame */
        UPROPERTY(BlueprintReadOnly)
        int32 CollisionTestsThisFrame = 0;

        /** Physics memory usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float PhysicsMemoryUsageMB = 0.0f;

        /** Whether physics is within performance budget */
        UPROPERTY(BlueprintReadOnly)
        bool bWithinBudget = true;

        /** Performance efficiency (0.0 = worst, 1.0 = perfect) */
        UPROPERTY(BlueprintReadOnly)
        float PerformanceEfficiency = 1.0f;
    };

    /** Current performance target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    EPerformanceTarget CurrentPerformanceTarget = EPerformanceTarget::PC_HighEnd;

    /** Performance budget for physics systems */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    FPerformanceBudget PhysicsBudget;

    /** Physics LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings")
    TArray<float> PhysicsLODDistances = {1000.0f, 3000.0f, 8000.0f, 20000.0f, 50000.0f};

    /** Maximum active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits", meta = (ClampMin = "100", ClampMax = "50000"))
    int32 MaxActivePhysicsBodies = 10000;

    /** Maximum active destruction chunks */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits", meta = (ClampMin = "100", ClampMax = "10000"))
    int32 MaxActiveDestructionChunks = 5000;

    /** Maximum active ragdolls */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits", meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxActiveRagdolls = 100;

    /** Maximum collision tests per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits", meta = (ClampMin = "50", ClampMax = "2000"))
    int32 MaxCollisionTestsPerFrame = 500;

    /** Enable adaptive physics tick rate */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings")
    bool bEnableAdaptiveTickRate = true;

    /** Enable distance-based physics culling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings")
    bool bEnableDistanceCulling = true;

    /** Enable importance-based optimization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings")
    bool bEnableImportanceOptimization = true;

    /** Emergency optimization threshold (frame time multiplier) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Optimization Settings", meta = (ClampMin = "1.1", ClampMax = "3.0"))
    float EmergencyOptimizationThreshold = 1.5f;

private:
    /** Initialize performance monitoring */
    void InitializePerformanceMonitoring();

    /** Update physics performance metrics */
    void UpdatePhysicsMetrics();

    /** Get optimal physics LOD for distance and importance */
    EPhysicsLODLevel GetOptimalPhysicsLOD(float Distance, float Importance) const;

    /** Apply physics LOD to actor */
    void ApplyPhysicsLODToActor(AActor* Actor, EPhysicsLODLevel LODLevel);

    /** Cull distant physics objects */
    void CullDistantPhysicsObjects(FVector ViewerLocation);

    /** Manage destruction chunk lifecycle */
    void ManageDestructionChunks();

    /** Clean up old ragdolls */
    void CleanupOldRagdolls();

    /** Optimize physics tick rates based on importance */
    void OptimizePhysicsTickRates();

    /** Calculate physics importance for an actor */
    float CalculatePhysicsImportance(AActor* Actor, FVector ViewerLocation) const;

    /** Cached references */
    UPROPERTY()
    UPhysicsSystemManager* PhysicsSystemManager;

    UPROPERTY()
    UCollisionSystem* CollisionSystem;

    UPROPERTY()
    UWorld* CachedWorld;

    /** Performance tracking */
    FPhysicsPerformanceMetrics CurrentMetrics;
    float FrameTimeHistory[60]; // 1 second history at 60fps
    int32 FrameHistoryIndex = 0;

    /** Tracked physics objects for optimization */
    UPROPERTY()
    TArray<AActor*> TrackedPhysicsActors;

    /** Active destruction chunks */
    UPROPERTY()
    TArray<UPrimitiveComponent*> ActiveDestructionChunks;

    /** Active ragdolls with timestamps */
    UPROPERTY()
    TMap<USkeletalMeshComponent*, float> ActiveRagdolls;

    /** Physics LOD cache to avoid recalculation */
    TMap<AActor*, EPhysicsLODLevel> PhysicsLODCache;

    /** Performance optimization flags */
    bool bEmergencyOptimizationActive = false;
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 0.1f; // Optimize every 100ms

    /** Physics budget tracking */
    float PhysicsBudgetUsed = 0.0f;
    float PhysicsBudgetAvailable = 2.0f; // 2ms default
};