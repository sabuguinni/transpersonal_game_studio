#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

/**
 * Core Ragdoll System Component
 * Handles realistic ragdoll physics for character death, knockdown, and impact reactions
 * Integrates with dinosaur AI and player character for immersive survival gameplay
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;

    /** Skeletal mesh component to apply ragdoll physics to */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll System")
    TWeakObjectPtr<USkeletalMeshComponent> TargetMesh;

    /** Current ragdoll state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll System")
    ECore_RagdollState RagdollState;

    /** Ragdoll activation threshold (damage required to trigger) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float ActivationThreshold;

    /** Time before ragdoll automatically recovers (0 = manual recovery only) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings", meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float AutoRecoveryTime;

    /** Impulse force multiplier for ragdoll activation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float ImpulseMultiplier;

    /** Bone names that are critical for ragdoll physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    TArray<FName> CriticalBones;

    /** Physics constraints for ragdoll joints */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    TMap<FName, FCore_RagdollConstraint> BoneConstraints;

    /** Current ragdoll timer */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll System")
    float CurrentRagdollTime;

    /** Damage that triggered current ragdoll state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll System")
    float TriggerDamage;

    /** Location where ragdoll was triggered */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll System")
    FVector TriggerLocation;

    /** Timer handle for auto recovery */
    FTimerHandle RecoveryTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Initialize ragdoll system with target skeletal mesh */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void InitializeRagdoll(USkeletalMeshComponent* InTargetMesh);

    /** Activate ragdoll physics with damage and impact force */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    bool ActivateRagdoll(float Damage, const FVector& ImpactPoint, const FVector& ImpactForce);

    /** Deactivate ragdoll and return to normal animation */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void DeactivateRagdoll();

    /** Force immediate ragdoll activation (bypass threshold) */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void ForceRagdollActivation(const FVector& ImpactPoint, const FVector& ImpactForce);

    /** Check if ragdoll is currently active */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    bool IsRagdollActive() const;

    /** Get current ragdoll state */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    ECore_RagdollState GetRagdollState() const;

    /** Apply impulse to specific bone during ragdoll */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void ApplyBoneImpulse(const FName& BoneName, const FVector& Impulse);

    /** Set bone constraint parameters */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void SetBoneConstraint(const FName& BoneName, const FCore_RagdollConstraint& Constraint);

    /** Get bone velocity during ragdoll */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    FVector GetBoneVelocity(const FName& BoneName) const;

    /** Check if bone is moving above threshold */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    bool IsBoneMoving(const FName& BoneName, float VelocityThreshold = 50.0f) const;

    /** Blend from ragdoll back to animation */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void BlendToAnimation(float BlendTime = 1.0f);

    /** Set ragdoll auto recovery time */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll System")
    void SetAutoRecoveryTime(float NewTime);

    /** Get time remaining until auto recovery */
    UFUNCTION(BlueprintPure, Category = "Ragdoll System")
    float GetRecoveryTimeRemaining() const;

    /** Event called when ragdoll is activated */
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollActivated OnRagdollActivated;

    /** Event called when ragdoll is deactivated */
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollDeactivated OnRagdollDeactivated;

    /** Event called when bone impact is detected */
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnBoneImpact OnBoneImpact;

private:
    /** Internal function to setup physics constraints */
    void SetupPhysicsConstraints();

    /** Internal function to apply impulse forces */
    void ApplyImpulseForces(const FVector& ImpactPoint, const FVector& ImpactForce);

    /** Internal function to handle auto recovery timer */
    UFUNCTION()
    void HandleAutoRecovery();

    /** Internal function to validate bone names */
    bool ValidateBoneName(const FName& BoneName) const;

    /** Internal function to calculate optimal impulse distribution */
    TMap<FName, FVector> CalculateImpulseDistribution(const FVector& ImpactPoint, const FVector& ImpactForce) const;
};