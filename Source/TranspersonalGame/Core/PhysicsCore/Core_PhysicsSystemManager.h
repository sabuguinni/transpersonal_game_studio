#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Handles all physics simulation, collision detection, ragdoll physics, and destruction systems
 * Optimized for prehistoric survival gameplay with realistic physics interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics simulation control
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetGlobalPhysicsSettings(float GravityZ = -980.0f, bool bEnableSubstepping = true);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSimulation(float DeltaTime);

    // Collision detection and response
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool CheckCollisionBetweenActors(AActor* ActorA, AActor* ActorB);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupCollisionChannels();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnablePhysicsOnActor(AActor* Actor, bool bSimulatePhysics = true);

    // Ragdoll physics system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableRagdollPhysics(APawn* Pawn);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisableRagdollPhysics(APawn* Pawn);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyImpulseToRagdoll(APawn* Pawn, FVector Impulse, FName BoneName = NAME_None);

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void CreateDestructibleFromStaticMesh(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void TriggerDestruction(AActor* Actor, FVector ImpactPoint, float DestructionForce);

    // Biome-specific physics
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyBiomePhysicsModifiers(ECore_BiomeType BiomeType, AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetWaterPhysics(AActor* Actor, bool bIsInWater);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetMudPhysics(AActor* Actor, bool bIsInMud);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsLOD(AActor* Actor, int32 LODLevel);

    // Debug and testing
    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor)
    void CreatePhysicsTestActors();

    UFUNCTION(BlueprintCallable, Category = "Core Physics", CallInEditor)
    void RunPhysicsValidationTests();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DebugDrawPhysicsInfo(bool bShowCollision = true, bool bShowMass = true);

protected:
    // Physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalGravityZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableSubstepping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float PhysicsTimestep;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUsePhysicsLOD;

    // Biome physics modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Physics")
    TMap<ECore_BiomeType, float> BiomeGravityModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Physics")
    TMap<ECore_BiomeType, float> BiomeFrictionModifiers;

    // Active physics actors tracking
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<APawn*> ActiveRagdolls;

    // Internal methods
    void InitializeBiomePhysicsModifiers();
    void UpdateActivePhysicsActors();
    void CleanupInvalidActors();
    bool IsActorValidForPhysics(AActor* Actor) const;
    void ApplyPerformanceOptimizations();
};