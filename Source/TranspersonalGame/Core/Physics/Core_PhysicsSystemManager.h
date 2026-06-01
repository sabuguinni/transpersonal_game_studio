#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Manages all physics systems: collision detection, ragdoll physics, destruction, and environmental physics
 * Handles performance optimization and physics simulation settings
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ShutdownPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSettings();

    // Collision System
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Collision")
    void EnableCollisionForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Collision")
    void DisableCollisionForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Collision")
    bool CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB);

    // Ragdoll Physics
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void EnableRagdollForCharacter(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void DisableRagdollForCharacter(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Ragdoll")
    void SetRagdollStrength(class ACharacter* Character, float Strength);

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Destruction")
    void EnableDestructionForMesh(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Destruction")
    void TriggerDestruction(UStaticMeshComponent* MeshComponent, FVector ImpactPoint, float Force);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Destruction")
    void CleanupDestructionDebris();

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Environment")
    void SetGlobalGravity(float NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Environment")
    void SetWindForce(FVector WindDirection, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Environment")
    void ApplyEnvironmentalForces();

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Core Physics|Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Performance")
    void SetPhysicsLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Core Physics|Performance")
    int32 GetActivePhysicsActorCount();

protected:
    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float DefaultAngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsOptimization;

    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float CollisionTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    int32 MaxCollisionIterations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bUseComplexCollisionAsSimple;

    // Ragdoll Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float RagdollBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float RagdollLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    bool bAutoCleanupRagdolls;

    // Destruction Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float DestructionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    int32 MaxDebrisCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float DebrisLifetime;

    // Environmental Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Settings")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Settings")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Settings")
    bool bEnableEnvironmentalEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 MaxActivePhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float PhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bUseLODSystem;

private:
    // Internal state tracking
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;
    TArray<TWeakObjectPtr<UStaticMeshComponent>> DestructibleMeshes;
    TArray<TWeakObjectPtr<class ACharacter>> RagdollCharacters;

    // Helper functions
    void UpdateTrackedActors();
    void CleanupInvalidReferences();
    bool IsActorValidForPhysics(AActor* Actor);
    void ApplyPerformanceOptimizations();
};