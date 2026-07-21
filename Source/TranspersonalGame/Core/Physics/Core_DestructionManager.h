#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Core_DestructionManager.generated.h"

/**
 * Core_DestructionManager
 * 
 * Manages destruction physics for breakable objects in the prehistoric world.
 * Handles boulder breaking, log splitting, and environmental destruction.
 * 
 * Features:
 * - Impact-based destruction thresholds
 * - Debris spawning and cleanup
 * - Sound and particle effect triggers
 * - Performance-optimized destruction batching
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_DestructionManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_DestructionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === DESTRUCTION CONFIGURATION ===
    
    /** Minimum impact force required to trigger destruction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction|Thresholds")
    float MinDestructionForce = 500.0f;
    
    /** Maximum number of debris pieces to spawn per destruction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction|Spawning")
    int32 MaxDebrisCount = 8;
    
    /** Time before debris pieces are automatically cleaned up */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction|Cleanup")
    float DebrisLifetime = 30.0f;
    
    /** Scale factor for debris pieces relative to original object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction|Spawning")
    float DebrisScale = 0.3f;

    // === DESTRUCTION METHODS ===
    
    /** Register a destructible object for management */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RegisterDestructible(AActor* DestructibleActor, float DestructionThreshold = 300.0f);
    
    /** Trigger destruction of an object with specified impact force */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(AActor* TargetActor, FVector ImpactLocation, float ImpactForce);
    
    /** Create debris pieces from destroyed object */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SpawnDebris(AActor* OriginalActor, FVector ImpactLocation, int32 DebrisCount);
    
    /** Clean up old debris to maintain performance */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupOldDebris();
    
    /** Check if an object can be destroyed by given force */
    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool CanDestroy(AActor* TargetActor, float ImpactForce) const;

    // === DESTRUCTION EVENTS ===
    
    /** Called when an object is destroyed */
    UFUNCTION(BlueprintImplementableEvent, Category = "Destruction|Events")
    void OnObjectDestroyed(AActor* DestroyedActor, FVector ImpactLocation);
    
    /** Called when debris is spawned */
    UFUNCTION(BlueprintImplementableEvent, Category = "Destruction|Events")
    void OnDebrisSpawned(const TArray<AActor*>& DebrisPieces);

protected:
    // === INTERNAL DATA ===
    
    /** Map of registered destructible objects and their thresholds */
    UPROPERTY()
    TMap<AActor*, float> DestructibleObjects;
    
    /** Array of active debris pieces with spawn times */
    UPROPERTY()
    TArray<AActor*> ActiveDebris;
    
    /** Spawn times for debris cleanup tracking */
    UPROPERTY()
    TArray<float> DebrisSpawnTimes;
    
    /** Current world time for debris aging */
    float CurrentWorldTime = 0.0f;

    // === INTERNAL METHODS ===
    
    /** Handle collision events for registered destructibles */
    UFUNCTION()
    void OnDestructibleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                          UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                          const FHitResult& Hit);
    
    /** Calculate impact force from collision data */
    float CalculateImpactForce(const FVector& NormalImpulse, float ObjectMass) const;
    
    /** Generate random debris piece from original mesh */
    AActor* CreateDebrisPiece(AActor* OriginalActor, FVector SpawnLocation, FVector Velocity);
    
    /** Bind collision events for a destructible object */
    void BindDestructibleEvents(AActor* DestructibleActor);
};