#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Handles all physics simulation, collision detection, and material properties
 * for the prehistoric survival game. Manages realistic physics for dinosaurs,
 * environmental objects, and player interactions.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS CONFIGURATION ===
    
    /** Global gravity multiplier for the game world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GravityMultiplier = 1.0f;
    
    /** Maximum physics simulation distance from player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxPhysicsDistance = 5000.0f;
    
    /** Enable/disable physics simulation globally */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsSimulation = true;
    
    /** Physics update frequency (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "30", ClampMax = "120"))
    int32 PhysicsUpdateFrequency = 60;

    // === COLLISION DETECTION ===
    
    /** Enable advanced collision detection for dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableAdvancedCollision = true;
    
    /** Collision detection accuracy level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionAccuracy CollisionAccuracy = ECore_CollisionAccuracy::High;
    
    /** Maximum number of collision checks per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    int32 MaxCollisionChecksPerFrame = 500;

    // === PHYSICS MATERIALS ===
    
    /** Rock physics material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TObjectPtr<UPhysicalMaterial> RockPhysicsMaterial;
    
    /** Wood physics material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TObjectPtr<UPhysicalMaterial> WoodPhysicsMaterial;
    
    /** Flesh physics material (for dinosaurs) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TObjectPtr<UPhysicalMaterial> FleshPhysicsMaterial;
    
    /** Ground/terrain physics material */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TObjectPtr<UPhysicalMaterial> GroundPhysicsMaterial;

    // === RAGDOLL PHYSICS ===
    
    /** Enable ragdoll physics for dead dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdollPhysics = true;
    
    /** Ragdoll simulation time before cleanup */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollLifetime = 30.0f;
    
    /** Maximum number of active ragdolls */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    int32 MaxActiveRagdolls = 10;

    // === DESTRUCTION PHYSICS ===
    
    /** Enable destructible environments */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bEnableDestruction = true;
    
    /** Destruction force threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 1000.0f;

    // === PHYSICS FUNCTIONS ===
    
    /** Initialize physics system */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();
    
    /** Update physics simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsSimulation(float DeltaTime);
    
    /** Apply physics material to component */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsMaterial(UPrimitiveComponent* Component, ECore_PhysicsMaterialType MaterialType);
    
    /** Enable ragdoll physics on skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableRagdollPhysics(class USkeletalMeshComponent* SkeletalMesh);
    
    /** Disable ragdoll physics */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void DisableRagdollPhysics(class USkeletalMeshComponent* SkeletalMesh);
    
    /** Create physics constraint between two components */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    class UPhysicsConstraintComponent* CreatePhysicsConstraint(
        UPrimitiveComponent* ComponentA, 
        UPrimitiveComponent* ComponentB,
        FVector ConstraintLocation
    );
    
    /** Apply impulse to physics object */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyImpulse(UPrimitiveComponent* Component, FVector Impulse, FVector Location = FVector::ZeroVector);
    
    /** Check if physics simulation is active for component */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    bool IsPhysicsSimulationActive(UPrimitiveComponent* Component) const;
    
    /** Get physics material type for component */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    ECore_PhysicsMaterialType GetPhysicsMaterialType(UPrimitiveComponent* Component) const;

    // === PERFORMANCE MONITORING ===
    
    /** Get current physics performance metrics */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    FCore_PhysicsPerformanceData GetPhysicsPerformanceData() const;
    
    /** Get number of active physics bodies */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    int32 GetActivePhysicsBodiesCount() const;

private:
    // === INTERNAL STATE ===
    
    /** Active ragdoll components */
    UPROPERTY()
    TArray<TObjectPtr<class USkeletalMeshComponent>> ActiveRagdolls;
    
    /** Physics performance tracking */
    FCore_PhysicsPerformanceData PerformanceData;
    
    /** Last physics update time */
    float LastPhysicsUpdateTime = 0.0f;
    
    /** Physics bodies within simulation range */
    UPROPERTY()
    TArray<TObjectPtr<UPrimitiveComponent>> ActivePhysicsBodies;

    // === INTERNAL FUNCTIONS ===
    
    /** Update active physics bodies list */
    void UpdateActivePhysicsBodies();
    
    /** Cleanup old ragdolls */
    void CleanupRagdolls();
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Create default physics materials */
    void CreateDefaultPhysicsMaterials();
    
    /** Validate physics configuration */
    bool ValidatePhysicsConfiguration() const;
};