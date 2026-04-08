// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Performance/PerformanceTargets.h"
#include "Core/PhysicsCore/PhysicsSystemManager.h"
#include "PhysicsPerformanceOptimizer.generated.h"

/**
 * @brief Physics Performance Optimizer for Jurassic Survival Game
 * 
 * Monitors and dynamically adjusts physics performance to maintain:
 * - 60fps on PC (16.67ms budget)
 * - 30fps on Console (33.33ms budget)
 * 
 * Optimizes:
 * - Chaos Physics simulation quality
 * - Collision detection complexity
 * - Ragdoll physics LOD
 * - Destruction system detail
 * - Mass creature physics simulation
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
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
     * @brief Initialize physics performance monitoring
     * 
     * Sets up performance tracking for:
     * - Physics thread timing
     * - Collision query counts
     * - Active physics bodies
     * - Destruction simulation load
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void InitializePhysicsMonitoring();

    /**
     * @brief Set performance target for physics optimization
     * 
     * @param Target Performance target (PC 60fps / Console 30fps)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPerformanceTarget(EPerformanceTarget Target);

    /**
     * @brief Dynamically adjust physics quality based on performance
     * 
     * Automatically reduces physics complexity when frame time exceeds budget:
     * - Reduces physics substeps
     * - Simplifies collision detection
     * - Culls distant physics objects
     * - Reduces destruction detail
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsQuality();

    /**
     * @brief Optimize creature physics simulation
     * 
     * Manages physics LOD for dinosaurs and NPCs:
     * - Full physics for nearby creatures
     * - Simplified physics for medium distance
     * - Kinematic simulation for distant creatures
     * 
     * @param MaxFullPhysicsCreatures Maximum creatures with full physics
     * @param MaxSimplifiedPhysicsCreatures Maximum creatures with simplified physics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeCreaturePhysics(int32 MaxFullPhysicsCreatures = 50, int32 MaxSimplifiedPhysicsCreatures = 200);

    /**
     * @brief Optimize destruction system performance
     * 
     * Manages destruction complexity based on performance:
     * - Reduces fracture detail when needed
     * - Limits active destruction chunks
     * - Culls distant destruction effects
     * 
     * @param MaxActiveChunks Maximum destruction chunks active at once
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeDestructionSystem(int32 MaxActiveChunks = 1000);

    /**
     * @brief Get current physics performance metrics
     * 
     * @return Current physics timing and object counts
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Performance")
    FString GetPhysicsPerformanceReport() const;

    /**
     * @brief Force physics performance level
     * 
     * @param Level 0=Minimum, 1=Low, 2=Medium, 3=High, 4=Ultra
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsPerformanceLevel(int32 Level);

protected:
    /** Current performance target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    EPerformanceTarget CurrentTarget = EPerformanceTarget::PC_HighEnd;

    /** Physics performance budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float PhysicsBudgetMs = 2.0f;

    /** Enable automatic physics optimization */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    bool bAutoOptimizePhysics = true;

    /** Physics performance monitoring interval */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float MonitoringInterval = 0.1f; // 100ms

    /** Maximum physics substeps per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Quality")
    int32 MaxPhysicsSubsteps = 6;

    /** Minimum physics substeps per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Quality")
    int32 MinPhysicsSubsteps = 1;

    /** Maximum active physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Quality")
    int32 MaxActivePhysicsBodies = 5000;

    /** Physics culling distance multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Quality")
    float PhysicsCullingMultiplier = 1.0f;

    /** Enable physics LOD for distant objects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Quality")
    bool bEnablePhysicsLOD = true;

private:
    /** Performance monitoring data */
    USTRUCT()
    struct FPhysicsPerformanceData
    {
        GENERATED_BODY()

        float PhysicsThreadTime = 0.0f;
        float CollisionQueryTime = 0.0f;
        int32 ActivePhysicsBodies = 0;
        int32 ActiveDestructionChunks = 0;
        int32 CollisionQueriesPerFrame = 0;
        int32 PhysicsSubstepsUsed = 0;
        float AverageFrameTime = 0.0f;
        
        FPhysicsPerformanceData()
        {
            PhysicsThreadTime = 0.0f;
            CollisionQueryTime = 0.0f;
            ActivePhysicsBodies = 0;
            ActiveDestructionChunks = 0;
            CollisionQueriesPerFrame = 0;
            PhysicsSubstepsUsed = 0;
            AverageFrameTime = 0.0f;
        }
    };

    /** Current performance data */
    FPhysicsPerformanceData CurrentPerformanceData;

    /** Performance history for averaging */
    TArray<float> FrameTimeHistory;

    /** Maximum history samples */
    static constexpr int32 MaxHistorySamples = 60;

    /** Timer for monitoring interval */
    float MonitoringTimer = 0.0f;

    /** Current physics performance level (0-4) */
    int32 CurrentPhysicsLevel = 3;

    /** Reference to physics system manager */
    UPROPERTY()
    UPhysicsSystemManager* PhysicsSystemManager;

    /** Cached world reference */
    UPROPERTY()
    UWorld* CachedWorld;

    /** Physics performance optimization functions */
    void UpdatePerformanceMetrics();
    void AdjustPhysicsSubsteps(float TargetFrameTime);
    void CullDistantPhysicsObjects();
    void OptimizeCollisionComplexity();
    void ManageDestructionChunks();
    void ApplyPhysicsLOD();
    
    /** Get distance-based physics quality level */
    int32 GetPhysicsLODLevel(float Distance) const;
    
    /** Calculate average frame time */
    float CalculateAverageFrameTime() const;
    
    /** Check if physics budget is exceeded */
    bool IsPhysicsBudgetExceeded() const;
    
    /** Apply physics settings for performance level */
    void ApplyPhysicsPerformanceLevel(int32 Level);
    
    /** Get all physics bodies in world */
    TArray<UPrimitiveComponent*> GetAllPhysicsBodies() const;
    
    /** Get all destruction components in world */
    TArray<class UGeometryCollectionComponent*> GetAllDestructionComponents() const;
};