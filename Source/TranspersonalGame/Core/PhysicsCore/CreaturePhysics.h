// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "CreaturePhysics.generated.h"

/**
 * @brief Physics component specialized for dinosaur and creature movement
 * 
 * Handles physics simulation for organic creatures including:
 * - Realistic weight distribution for different dinosaur sizes
 * - Ground adaptation using inverse kinematics
 * - Death state ragdoll transitions
 * - Collision response for predator/prey interactions
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCreaturePhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCreaturePhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Configure physics for different creature sizes
     * 
     * @param CreatureType Type of creature (Small, Medium, Large, Massive)
     * @param Mass Creature mass in kg
     * @param Height Creature height for ground adaptation
     */
    UFUNCTION(BlueprintCallable, Category = "Creature Physics")
    void ConfigureCreaturePhysics(ECreatureSize CreatureType, float Mass, float Height);

    /**
     * @brief Apply realistic ground adaptation using IK
     * 
     * @param GroundNormal Normal vector of ground surface
     * @param FootPositions Array of foot positions for IK adjustment
     */
    UFUNCTION(BlueprintCallable, Category = "Creature Physics")
    void ApplyGroundAdaptation(FVector GroundNormal, const TArray<FVector>& FootPositions);

    /**
     * @brief Transition creature to death state with ragdoll physics
     * 
     * @param DeathImpulse Impulse vector applied at moment of death
     * @param DeathLocation World location where death occurred
     */
    UFUNCTION(BlueprintCallable, Category = "Creature Physics")
    void TransitionToDeathState(FVector DeathImpulse, FVector DeathLocation);

    /**
     * @brief Apply collision response for creature interactions
     * 
     * @param OtherCreature The other creature involved in collision
     * @param CollisionImpulse Impulse magnitude from collision
     * @param CollisionLocation World location of collision
     */
    UFUNCTION(BlueprintCallable, Category = "Creature Physics")
    void HandleCreatureCollision(AActor* OtherCreature, float CollisionImpulse, FVector CollisionLocation);

    /**
     * @brief Get current creature physics state
     */
    UFUNCTION(BlueprintPure, Category = "Creature Physics")
    ECreaturePhysicsState GetPhysicsState() const { return CurrentPhysicsState; }

    /**
     * @brief Check if creature is currently stable on ground
     */
    UFUNCTION(BlueprintPure, Category = "Creature Physics")
    bool IsGroundStable() const { return bIsGroundStable; }

protected:
    /** Creature size categories for physics scaling */
    UENUM(BlueprintType)
    enum class ECreatureSize : uint8
    {
        Small       UMETA(DisplayName = "Small (< 50kg)"),      // Compsognathus, small birds
        Medium      UMETA(DisplayName = "Medium (50-500kg)"),   // Velociraptor, player character
        Large       UMETA(DisplayName = "Large (500-5000kg)"),  // Triceratops, Stegosaurus
        Massive     UMETA(DisplayName = "Massive (> 5000kg)")   // T-Rex, Brontosaurus
    };

    /** Current physics state of the creature */
    UENUM(BlueprintType)
    enum class ECreaturePhysicsState : uint8
    {
        Alive       UMETA(DisplayName = "Alive"),
        Stunned     UMETA(DisplayName = "Stunned"),
        Dying       UMETA(DisplayName = "Dying"),
        Dead        UMETA(DisplayName = "Dead")
    };

    /** Current creature size category */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature Settings")
    ECreatureSize CreatureSize = ECreatureSize::Medium;

    /** Current physics state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State")
    ECreaturePhysicsState CurrentPhysicsState = ECreaturePhysicsState::Alive;

    /** Creature mass in kilograms */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature Settings", meta = (ClampMin = "1.0", ClampMax = "50000.0"))
    float CreatureMass = 100.0f;

    /** Creature height for ground adaptation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature Settings", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
    float CreatureHeight = 180.0f;

    /** Ground stability check */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics State")
    bool bIsGroundStable = true;

    /** Maximum ground slope angle for stability (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Adaptation", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float MaxStableSlope = 45.0f;

    /** IK adjustment strength for ground adaptation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground Adaptation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKAdjustmentStrength = 0.8f;

    /** Ragdoll transition time in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Physics", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float RagdollTransitionTime = 1.0f;

    /** Collision damage threshold */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float CollisionDamageThreshold = 500.0f;

private:
    /** Initialize physics settings based on creature size */
    void InitializePhysicsForSize();
    
    /** Update ground stability based on current surface */
    void UpdateGroundStability();
    
    /** Apply size-appropriate physics constraints */
    void ApplyPhysicsConstraints();
    
    /** Handle transition between physics states */
    void HandleStateTransition(ECreaturePhysicsState NewState);

    /** Reference to creature's skeletal mesh component */
    UPROPERTY()
    USkeletalMeshComponent* CreatureMesh;

    /** Timer for ragdoll transition */
    FTimerHandle RagdollTransitionTimer;

    /** Ground trace distance based on creature height */
    float GroundTraceDistance;

    /** Physics scaling factors for different creature sizes */
    struct FCreaturePhysicsScaling
    {
        float MassMultiplier;
        float ForceMultiplier;
        float DamageResistance;
        float CollisionRadius;
    };

    /** Physics scaling data for each creature size */
    TMap<ECreatureSize, FCreaturePhysicsScaling> PhysicsScalingData;
};