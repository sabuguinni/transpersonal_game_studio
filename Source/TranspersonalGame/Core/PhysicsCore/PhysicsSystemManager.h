// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsSystemManager.generated.h"

/**
 * @brief Core Physics System Manager for the Jurassic survival game
 * 
 * Manages all physics-related systems including:
 * - Chaos Physics configuration for organic movement
 * - Destruction system for environmental interaction
 * - Ragdoll physics for creature death states
 * - Collision detection optimized for large-scale world
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize physics settings optimized for Jurassic world
     * 
     * Configures Chaos Physics with settings that prioritize:
     * - Organic creature movement (dinosaurs, player)
     * - Environmental destruction (trees, rocks)
     * - Performance for large open world
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializeJurassicPhysics();

    /**
     * @brief Configure collision channels for survival gameplay
     * 
     * Sets up collision detection for:
     * - Player vs Environment
     * - Dinosaur vs Environment  
     * - Projectiles vs All
     * - Destruction vs All
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ConfigureCollisionChannels();

    /**
     * @brief Enable ragdoll physics for creature death
     * 
     * @param SkeletalMeshComp The skeletal mesh to apply ragdoll to
     * @param ImpulseLocation World location where death impulse originated
     * @param ImpulseStrength Strength of death impulse
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnableCreatureRagdoll(USkeletalMeshComponent* SkeletalMeshComp, FVector ImpulseLocation, float ImpulseStrength = 500.0f);

    /**
     * @brief Apply environmental destruction to geometry
     * 
     * @param GeometryCollection The geometry collection to fracture
     * @param ImpactLocation World location of destruction impact
     * @param DestructionRadius Radius of destruction effect
     * @param DestructionForce Force magnitude applied to fragments
     */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TriggerEnvironmentalDestruction(class UGeometryCollectionComponent* GeometryCollection, 
                                       FVector ImpactLocation, 
                                       float DestructionRadius = 200.0f, 
                                       float DestructionForce = 1000.0f);

protected:
    /** Physics settings optimized for survival gameplay */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float WorldGravityZ = -980.0f;

    /** Maximum physics simulation substeps per frame */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    int32 MaxSubsteps = 6;

    /** Physics delta time for consistent simulation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float FixedDeltaTime = 0.016667f; // 60fps

    /** Enable Chaos destruction system */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    bool bEnableDestruction = true;

    /** Default damage threshold for environmental objects */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
    float DefaultDamageThreshold = 100.0f;

    /** Ragdoll physics impulse multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollImpulseMultiplier = 1.5f;

private:
    /** Initialize Chaos Physics solver settings */
    void InitializeChaosSettings();
    
    /** Setup collision object types and responses */
    void SetupCollisionProfiles();
    
    /** Configure physics materials for different surfaces */
    void ConfigurePhysicsMaterials();

    /** Cached reference to world physics scene */
    UPROPERTY()
    class UWorld* CachedWorld;

    /** Physics materials for different terrain types */
    UPROPERTY()
    TMap<FString, class UPhysicalMaterial*> TerrainPhysicsMaterials;
};