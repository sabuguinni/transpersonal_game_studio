#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Blending    UMETA(DisplayName = "Blending"),
    Active      UMETA(DisplayName = "Active"),
    Recovering  UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MinImpulseThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bPreserveVelocity = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollImpact, FVector, ImpactLocation, float, ImpactForce);

/**
 * Core_RagdollSystem - Manages realistic ragdoll physics for prehistoric creatures
 * Handles death animations, impact responses, and recovery mechanics
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll Control
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(bool bForceActivation = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void RecoverFromRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FVector& Location, FName BoneName = NAME_None);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    float GetRagdollBlendWeight() const { return CurrentBlendWeight; }

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    FCore_RagdollSettings GetRagdollSettings() const { return RagdollSettings; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollStateChanged OnRagdollStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollImpact OnRagdollImpact;

protected:
    // Core Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float CurrentBlendWeight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float StateTimer;

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Internal State
    FVector LastKnownVelocity;
    FRotator LastKnownRotation;
    TArray<FTransform> BoneTransforms;
    bool bWasRagdollActive;

private:
    void UpdateBlendWeight(float DeltaTime);
    void HandleStateTransition(ECore_RagdollState NewState);
    void CacheAnimationState();
    void RestoreAnimationState();
    void ProcessRagdollPhysics();
    bool ShouldActivateRagdoll(float ImpulseStrength) const;
    void SetupPhysicsAsset();
    void CleanupRagdoll();
};