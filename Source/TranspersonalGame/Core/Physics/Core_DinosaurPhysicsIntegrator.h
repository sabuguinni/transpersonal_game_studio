#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "GameFramework/Character.h"
#include "Core_DinosaurPhysicsIntegrator.generated.h"

/**
 * Core_DinosaurPhysicsIntegrator
 * 
 * Integrates physics systems specifically for DinosaurBase and derived classes.
 * Handles realistic dinosaur movement physics, collision responses, ragdoll transitions,
 * and impact systems for authentic prehistoric creature behavior.
 * 
 * Features:
 * - Mass-based movement calculations for different dinosaur sizes
 * - Realistic collision responses (knockback, stumbling, falling)
 * - Smooth ragdoll transitions for death/unconsciousness
 * - Impact force distribution for attacks and environmental collisions
 * - Terrain adaptation physics (foot placement, slope handling)
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DinosaurPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DinosaurPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === DINOSAUR PHYSICS CONFIGURATION ===
    
    /** Base mass multiplier for physics calculations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float BaseMassMultiplier = 1.0f;
    
    /** Movement force multiplier for different dinosaur sizes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MovementForceMultiplier = 1.0f;
    
    /** Collision response strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float CollisionResponseStrength = 1.0f;
    
    /** Enable realistic physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    bool bEnableRealisticPhysics = true;
    
    /** Enable ragdoll physics on death */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    bool bEnableRagdollPhysics = true;

    // === MOVEMENT PHYSICS ===
    
    /** Apply movement forces based on dinosaur mass and size */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyMovementPhysics(FVector MovementDirection, float MovementSpeed);
    
    /** Calculate realistic turning forces for large dinosaurs */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyTurningPhysics(float TurnInput, float DeltaTime);
    
    /** Handle slope movement physics */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void HandleSlopePhysics(FVector SlopeNormal, float SlopeAngle);

    // === COLLISION PHYSICS ===
    
    /** Handle collision with other dinosaurs or objects */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void HandleCollisionPhysics(AActor* OtherActor, FVector ImpactPoint, FVector ImpactNormal, float ImpactForce);
    
    /** Apply knockback force from impacts */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyKnockbackForce(FVector ForceDirection, float ForceStrength);
    
    /** Handle environmental collision responses */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void HandleEnvironmentalCollision(FVector ImpactPoint, float ImpactMagnitude);

    // === RAGDOLL PHYSICS ===
    
    /** Activate ragdoll physics (death, unconsciousness) */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ActivateRagdoll();
    
    /** Deactivate ragdoll physics (recovery) */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void DeactivateRagdoll();
    
    /** Check if ragdoll is currently active */
    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    bool IsRagdollActive() const { return bRagdollActive; }

    // === IMPACT SYSTEMS ===
    
    /** Apply impact force from attacks */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyAttackImpact(FVector ImpactLocation, FVector ImpactDirection, float ImpactForce);
    
    /** Handle fall damage physics */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void HandleFallImpact(float FallHeight, FVector ImpactVelocity);
    
    /** Calculate impact damage based on physics */
    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    float CalculateImpactDamage(float ImpactForce, float Mass) const;

    // === TERRAIN ADAPTATION ===
    
    /** Adapt physics to terrain type */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void AdaptToTerrain(FVector GroundNormal, float TerrainFriction);
    
    /** Handle foot placement physics for uneven terrain */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void HandleFootPlacement(FVector FootLocation, FVector GroundNormal);

protected:
    // === INTERNAL PHYSICS STATE ===
    
    /** Reference to owner's mesh component */
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;
    
    /** Reference to owner's static mesh (if applicable) */
    UPROPERTY()
    UStaticMeshComponent* OwnerStaticMeshComponent;
    
    /** Current ragdoll state */
    bool bRagdollActive = false;
    
    /** Cached mass for physics calculations */
    float CachedMass = 100.0f;
    
    /** Current terrain friction coefficient */
    float CurrentTerrainFriction = 1.0f;
    
    /** Physics constraint components for ragdoll */
    UPROPERTY()
    TArray<UPhysicsConstraintComponent*> RagdollConstraints;

    // === INTERNAL PHYSICS METHODS ===
    
    /** Initialize physics components */
    void InitializePhysicsComponents();
    
    /** Update physics properties based on dinosaur state */
    void UpdatePhysicsProperties(float DeltaTime);
    
    /** Calculate mass-based forces */
    FVector CalculateMassBasedForce(FVector InputForce) const;
    
    /** Setup ragdoll constraints */
    void SetupRagdollConstraints();
    
    /** Cleanup ragdoll constraints */
    void CleanupRagdollConstraints();
    
    /** Validate physics component references */
    bool ValidatePhysicsComponents() const;

    // === PHYSICS DEBUGGING ===
    
    /** Debug draw physics forces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDrawForces = false;
    
    /** Debug draw collision responses */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDrawCollisions = false;
    
    /** Draw debug information */
    void DrawDebugPhysics(float DeltaTime);
};