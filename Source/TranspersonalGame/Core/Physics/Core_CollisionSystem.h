#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "Core/SharedTypes.h"
#include "Core_CollisionSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCollisionDetected, AActor*, Actor1, AActor*, Actor2, const FHitResult&, HitResult);

/**
 * Core Collision System - Advanced collision detection and response
 * 
 * Features:
 * - Multi-layered collision detection (precise, approximate, bounds)
 * - Dynamic collision response based on material properties
 * - Performance optimization with spatial partitioning
 * - Integration with physics and destruction systems
 * 
 * Design Philosophy:
 * - Collision should feel realistic and responsive
 * - Performance scales with scene complexity
 * - Supports both gameplay and cinematic needs
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Collision Detection Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    bool bEnablePreciseCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    bool bEnableApproximateCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    bool bEnableBoundsCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    float CollisionCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    float CollisionUpdateFrequency;

    // Collision Response Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Response")
    float DefaultBounciness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Response")
    float DefaultFriction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Response")
    float DefaultDamping;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionChecksPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseSpatialPartitioning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SpatialPartitionSize;

    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    // Collision Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCollisionDetected OnCollisionDetected;

    // Internal State
    UPROPERTY()
    TArray<AActor*> TrackedActors;

    UPROPERTY()
    TMap<AActor*, FVector> ActorVelocities;

    UPROPERTY()
    TMap<AActor*, float> LastCollisionTimes;

    float CollisionTimer;
    int32 CollisionChecksThisFrame;

public:
    // Core Collision Functions
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void InitializeCollisionSystem();

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void UpdateCollisionSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool CheckCollision(AActor* Actor1, AActor* Actor2, FHitResult& OutHitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ProcessCollisionResponse(AActor* Actor1, AActor* Actor2, const FHitResult& HitResult);

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ClearAllActors();

    // Collision Detection Methods
    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    bool PreciseCollisionCheck(AActor* Actor1, AActor* Actor2, FHitResult& OutHitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    bool ApproximateCollisionCheck(AActor* Actor1, AActor* Actor2);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    bool BoundsCollisionCheck(AActor* Actor1, AActor* Actor2);

    // Collision Response Methods
    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void ApplyCollisionForces(AActor* Actor1, AActor* Actor2, const FVector& CollisionNormal, float ImpactStrength);

    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void HandleMaterialInteraction(AActor* Actor1, AActor* Actor2, const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void ProcessDamageFromCollision(AActor* Actor1, AActor* Actor2, float ImpactStrength);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateSpatialPartitioning();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<AActor*> GetNearbyActors(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    ECore_LODLevel GetCollisionLOD(AActor* Actor) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Collision Utility")
    float CalculateImpactStrength(const FVector& Velocity1, const FVector& Velocity2, float Mass1, float Mass2);

    UFUNCTION(BlueprintCallable, Category = "Collision Utility")
    FVector CalculateCollisionNormal(const FVector& Location1, const FVector& Location2);

    UFUNCTION(BlueprintCallable, Category = "Collision Utility")
    bool ShouldProcessCollision(AActor* Actor1, AActor* Actor2);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawCollisionBounds();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCollisionStats();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleCollisionVisualization();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Collision")
    int32 GetTrackedActorCount() const { return TrackedActors.Num(); }

    UFUNCTION(BlueprintPure, Category = "Collision")
    bool IsCollisionSystemEnabled() const { return bEnablePreciseCollision || bEnableApproximateCollision || bEnableBoundsCollision; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetCollisionChecksThisFrame() const { return CollisionChecksThisFrame; }

protected:
    // Internal Helper Functions
    void UpdateActorVelocities(float DeltaTime);
    void ProcessCollisionQueue();
    void OptimizeCollisionChecks();
    void UpdateCollisionLOD();
    bool IsWithinCollisionRange(AActor* Actor1, AActor* Actor2) const;
    void BroadcastCollisionEvent(AActor* Actor1, AActor* Actor2, const FHitResult& HitResult);

    // Spatial Partitioning
    TMap<FIntVector, TArray<AActor*>> SpatialGrid;
    void UpdateSpatialGrid();
    FIntVector GetGridCoordinate(const FVector& Location) const;
    void AddActorToGrid(AActor* Actor, const FIntVector& GridCoord);
    void RemoveActorFromGrid(AActor* Actor, const FIntVector& GridCoord);

    // Performance Tracking
    float LastPerformanceCheck;
    int32 TotalCollisionChecks;
    float AverageCollisionTime;
};