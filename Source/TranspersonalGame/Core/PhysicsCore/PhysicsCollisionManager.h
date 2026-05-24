#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "PhysicsCollisionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCollisionDetected, AActor*, Actor1, AActor*, Actor2, const FHitResult&, HitResult);

/**
 * @brief Advanced collision detection and response system
 * 
 * Manages collision detection for prehistoric creatures, environmental objects,
 * and interactive elements. Implements spatial partitioning and LOD for performance.
 * 
 * Key Features:
 * - Multi-layered collision detection (creatures, environment, projectiles)
 * - Spatial partitioning for large open world
 * - Dynamic LOD based on distance and importance
 * - Predictive collision for fast-moving objects
 * 
 * @author Core Systems Programmer #03
 * @version 1.0
 * @date 2024
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsCollisionManager : public UObject
{
    GENERATED_BODY()

public:
    UPhysicsCollisionManager();

    /**
     * Initialize collision system with world context
     * @param InWorld The world to manage collisions for
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void InitializeCollisionSystem(UWorld* InWorld);

    /**
     * Shutdown collision system and clean up resources
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ShutdownCollisionSystem();

    /**
     * Update collision system each frame
     * @param DeltaTime Time since last update
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void UpdateCollisionSystem(float DeltaTime);

    /**
     * Register an actor for collision tracking
     * @param Actor Actor to track
     * @param CollisionLayer Collision layer (0=Environment, 1=Creatures, 2=Projectiles, 3=Vehicles)
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void RegisterCollisionActor(AActor* Actor, int32 CollisionLayer = 1);

    /**
     * Unregister an actor from collision tracking
     * @param Actor Actor to stop tracking
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void UnregisterCollisionActor(AActor* Actor);

    /**
     * Perform sphere collision check
     * @param Center Sphere center
     * @param Radius Sphere radius
     * @param CollisionLayer Layer to check against
     * @return Array of overlapping actors
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<AActor*> SphereCollisionCheck(FVector Center, float Radius, int32 CollisionLayer = -1);

    /**
     * Perform line trace collision check
     * @param Start Trace start point
     * @param End Trace end point
     * @param CollisionLayer Layer to check against
     * @param OutHitResult Hit result if collision found
     * @return True if collision detected
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool LineTraceCollisionCheck(FVector Start, FVector End, int32 CollisionLayer, FHitResult& OutHitResult);

    /**
     * Set collision LOD level (0=highest quality, 3=lowest)
     * @param LODLevel New LOD level
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetLODLevel(int32 LODLevel);

    /**
     * Enable/disable collision system
     * @param bEnabled New enabled state
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetEnabled(bool bEnabled);

    /**
     * Get performance report for this subsystem
     * @return Performance metrics string
     */
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    FString GetPerformanceReport() const;

    /**
     * Event fired when collision is detected
     */
    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnCollisionDetected OnCollisionDetected;

protected:
    /** World context for collision queries */
    UPROPERTY()
    TWeakObjectPtr<UWorld> WorldContext;

    /** Actors tracked by collision layer */
    UPROPERTY()
    TMap<int32, TArray<TWeakObjectPtr<AActor>>> TrackedActorsByLayer;

    /** Current LOD level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentLODLevel;

    /** System enabled flag */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision System")
    bool bSystemEnabled;

    /** Maximum collision checks per frame by LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<int32> MaxCollisionChecksPerFrame;

    /** Collision check distance by LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> CollisionCheckDistance;

    /** Current frame collision check count */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 CurrentFrameCollisionChecks;

    /** Total collision checks this frame */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TotalCollisionChecks;

    /** Average collision checks per frame */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float AverageCollisionChecks;

    /** Collision check samples for averaging */
    TArray<int32> CollisionCheckSamples;

    /** Maximum samples for averaging */
    static constexpr int32 MaxCollisionSamples = 60;

private:
    /**
     * Initialize default LOD settings
     */
    void InitializeLODSettings();

    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics();

    /**
     * Clean up invalid actor references
     */
    void CleanupInvalidActors();

    /**
     * Check if collision check is within LOD limits
     * @param Distance Distance to collision check
     * @return True if check should be performed
     */
    bool ShouldPerformCollisionCheck(float Distance) const;

    /**
     * Process collision between two actors
     * @param Actor1 First actor in collision
     * @param Actor2 Second actor in collision
     * @param HitResult Collision hit result
     */
    void ProcessCollision(AActor* Actor1, AActor* Actor2, const FHitResult& HitResult);

    /** Frame counter for cleanup operations */
    int32 FrameCounter;

    /** Cleanup frequency (every N frames) */
    static constexpr int32 CleanupFrequency = 60;
};