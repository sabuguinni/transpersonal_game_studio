#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

/**
 * Core Ragdoll Physics System
 * Handles realistic dinosaur death animations, impact responses, and procedural ragdoll physics
 * Designed for prehistoric survival scenarios with large creatures
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === RAGDOLL ACTIVATION ===
    
    /** Activate ragdoll physics on skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ActivateRagdoll(USkeletalMeshComponent* SkeletalMesh, bool bPreserveVelocity = true);

    /** Deactivate ragdoll and return to animation */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DeactivateRagdoll(USkeletalMeshComponent* SkeletalMesh);

    /** Check if ragdoll is currently active */
    UFUNCTION(BlueprintPure, Category = "Ragdoll Physics")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const;

    // === IMPACT RESPONSE ===

    /** Apply impact force to specific bone */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpactToBone(USkeletalMeshComponent* SkeletalMesh, const FString& BoneName, const FVector& ImpactForce, const FVector& ImpactLocation);

    /** Apply radial impulse around impact point */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyRadialImpulse(USkeletalMeshComponent* SkeletalMesh, const FVector& Origin, float Radius, float Strength);

    /** Simulate death impact with realistic physics */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SimulateDeathImpact(USkeletalMeshComponent* SkeletalMesh, const FVector& DeathDirection, float ImpactStrength = 1000.0f);

    // === PHYSICS CONFIGURATION ===

    /** Configure bone physics properties */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ConfigureBonePhysics(USkeletalMeshComponent* SkeletalMesh, const FString& BoneName, float Mass, float LinearDamping, float AngularDamping);

    /** Set global ragdoll physics multipliers */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetPhysicsMultipliers(float MassMultiplier, float ForceMultiplier, float DampingMultiplier);

    /** Configure constraint limits for realistic movement */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetConstraintLimits(USkeletalMeshComponent* SkeletalMesh, bool bEnableProjection, float ProjectionLinearTolerance, float ProjectionAngularTolerance);

    // === PROCEDURAL EFFECTS ===

    /** Create procedural twitching during death sequence */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void StartDeathTwitching(USkeletalMeshComponent* SkeletalMesh, float Duration = 3.0f, float Intensity = 0.5f);

    /** Stop all procedural effects */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void StopProceduralEffects();

    // === RECOVERY SYSTEM ===

    /** Attempt to recover from ragdoll to standing position */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void AttemptRecovery(USkeletalMeshComponent* SkeletalMesh, float RecoveryTime = 2.0f);

    /** Check if recovery is possible from current position */
    UFUNCTION(BlueprintPure, Category = "Ragdoll Physics")
    bool CanRecover(USkeletalMeshComponent* SkeletalMesh) const;

protected:
    // === PROPERTIES ===

    /** Global physics multipliers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalMassMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalForceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float GlobalDampingMultiplier = 1.0f;

    /** Constraint projection settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableConstraintProjection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float ConstraintProjectionLinearTolerance = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float ConstraintProjectionAngularTolerance = 10.0f;

    /** Death twitching settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Effects")
    float DeathTwitchingDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Effects")
    float DeathTwitchingIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Effects")
    float TwitchingFrequency = 2.0f;

    /** Recovery settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryBlendTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float MaxRecoveryAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float MinRecoveryVelocity = 50.0f;

    // === INTERNAL STATE ===

    /** Currently active ragdoll meshes */
    UPROPERTY()
    TArray<USkeletalMeshComponent*> ActiveRagdolls;

    /** Meshes currently in recovery process */
    UPROPERTY()
    TArray<USkeletalMeshComponent*> RecoveringMeshes;

    /** Twitching timer */
    float TwitchingTimer = 0.0f;

    /** Twitching phase */
    float TwitchingPhase = 0.0f;

    /** Current twitching mesh */
    UPROPERTY()
    USkeletalMeshComponent* TwitchingMesh = nullptr;

    // === INTERNAL METHODS ===

    /** Update death twitching effects */
    void UpdateDeathTwitching(float DeltaTime);

    /** Update recovery blending */
    void UpdateRecovery(float DeltaTime);

    /** Apply random impulse for twitching effect */
    void ApplyTwitchImpulse(USkeletalMeshComponent* SkeletalMesh);

    /** Get bone mass based on creature size */
    float GetBoneMassForCreature(USkeletalMeshComponent* SkeletalMesh, const FString& BoneName) const;

    /** Calculate optimal recovery position */
    FVector CalculateRecoveryPosition(USkeletalMeshComponent* SkeletalMesh) const;
};