#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Core/SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

/**
 * Advanced ragdoll physics system for realistic character death and impact responses.
 * Handles transition from animated to physics-driven movement with proper bone constraints.
 * Essential for dinosaur combat and character death sequences in prehistoric survival.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Target skeletal mesh component for ragdoll physics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    USkeletalMeshComponent* TargetMesh;

    /** Physics asset used for ragdoll simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    UPhysicsAsset* RagdollPhysicsAsset;

    /** Current ragdoll state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState RagdollState;

    /** Time since ragdoll activation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollTime;

    /** Maximum time before ragdoll stabilization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float MaxRagdollTime;

    /** Force multiplier for impact-based ragdoll activation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float ImpactForceMultiplier;

    /** Minimum impact force required to trigger ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float MinImpactForce;

    /** Bone-specific force multipliers for realistic impact distribution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TMap<FName, float> BoneForceMultipliers;

    /** Linear damping for ragdoll physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float LinearDamping;

    /** Angular damping for ragdoll physics bodies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float AngularDamping;

    /** Blend weight for animation-to-ragdoll transition */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollBlendWeight;

    /** Speed of transition from animation to ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float TransitionSpeed;

    /** Whether to use partial ragdoll (only specific bones) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bUsePartialRagdoll;

    /** Bones to exclude from ragdoll simulation in partial mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (EditCondition = "bUsePartialRagdoll"))
    TArray<FName> ExcludedBones;

    /** Recovery settings for getting up from ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bCanRecoverFromRagdoll;

    /** Time required to be stable before recovery can begin */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float StabilityTimeRequired;

    /** Maximum velocity for ragdoll to be considered stable */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float MaxStableVelocity;

public:
    /** Activate ragdoll physics with optional impact force and location */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpactForce = FVector::ZeroVector, const FVector& ImpactLocation = FVector::ZeroVector, const FName& ImpactBone = NAME_None);

    /** Deactivate ragdoll and return to animation */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    /** Apply force to specific bone in ragdoll */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyForceToRagdoll(const FVector& Force, const FName& BoneName, bool bAccelChange = false);

    /** Apply impulse to specific bone in ragdoll */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName, bool bVelChange = false);

    /** Check if ragdoll is currently active */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const;

    /** Check if ragdoll is stable (low velocity) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    bool IsRagdollStable() const;

    /** Get current ragdoll state */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const;

    /** Set target skeletal mesh component */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetTargetMesh(USkeletalMeshComponent* NewTargetMesh);

    /** Configure bone-specific force multipliers */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBoneForceMultiplier(const FName& BoneName, float Multiplier);

    /** Get velocity of specific bone in ragdoll */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    FVector GetBoneVelocity(const FName& BoneName) const;

    /** Get center of mass velocity for entire ragdoll */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    FVector GetRagdollCenterOfMassVelocity() const;

    /** Force immediate recovery from ragdoll state */
    UFUNCTION(BlueprintCallable, Category = "Recovery")
    void ForceRecovery();

    /** Check if recovery is possible based on current conditions */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Recovery")
    bool CanRecover() const;

protected:
    /** Initialize ragdoll physics settings */
    void InitializeRagdollPhysics();

    /** Update ragdoll blend weight during transition */
    void UpdateRagdollBlending(float DeltaTime);

    /** Check stability conditions for recovery */
    void CheckStabilityForRecovery(float DeltaTime);

    /** Apply physics settings to all ragdoll bodies */
    void ApplyPhysicsSettings();

    /** Handle partial ragdoll setup */
    void SetupPartialRagdoll();

    /** Stability tracking timer */
    float StabilityTimer;

    /** Original animation instance for restoration */
    UAnimInstance* OriginalAnimInstance;

    /** Cached bone transforms for recovery positioning */
    TMap<FName, FTransform> CachedBoneTransforms;

public:
    /** Event called when ragdoll is activated */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_RagdollActivated OnRagdollActivated;

    /** Event called when ragdoll is deactivated */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_RagdollDeactivated OnRagdollDeactivated;

    /** Event called when ragdoll becomes stable */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_RagdollStabilized OnRagdollStabilized;

    /** Event called when recovery begins */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_RagdollRecoveryStarted OnRecoveryStarted;
};