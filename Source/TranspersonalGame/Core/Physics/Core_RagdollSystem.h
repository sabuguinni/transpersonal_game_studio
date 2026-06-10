#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Blending        UMETA(DisplayName = "Blending"),
    FullRagdoll     UMETA(DisplayName = "Full Ragdoll"),
    Recovery        UMETA(DisplayName = "Recovery")
};

UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    Manual          UMETA(DisplayName = "Manual"),
    HealthThreshold UMETA(DisplayName = "Health Threshold"),
    ImpactForce     UMETA(DisplayName = "Impact Force"),
    Unconscious     UMETA(DisplayName = "Unconscious")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    // Ragdoll activation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationForceThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float HealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 0.5f;

    // Physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxAngularVelocity = 1000.0f;

    // Recovery settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryDelay = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float GetUpAnimationTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bAutoRecover = true;

    FCore_RagdollConfig()
    {
        ActivationForceThreshold = 500.0f;
        HealthThreshold = 0.2f;
        BlendInTime = 0.2f;
        BlendOutTime = 0.5f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        MaxAngularVelocity = 1000.0f;
        RecoveryDelay = 2.0f;
        GetUpAnimationTime = 1.5f;
        bAutoRecover = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll control functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(ECore_RagdollTrigger Trigger = ECore_RagdollTrigger::Manual);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void BlendToRagdoll(float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void BlendFromRagdoll(float BlendTime = 0.5f);

    // Force application
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactForce(const FVector& Force, const FVector& Location, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyRadialImpulse(const FVector& Origin, float Radius, float Strength, bool bVelChange = false);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::FullRagdoll; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsBlending() const { return CurrentState == ECore_RagdollState::Blending; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    float GetBlendAlpha() const { return BlendAlpha; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    FCore_RagdollConfig GetRagdollConfig() const { return RagdollConfig; }

    // Physics asset management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetPhysicsAsset(UPhysicsAsset* NewPhysicsAsset);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ConfigureBonePhysics(FName BoneName, float LinearDamping, float AngularDamping, float Mass = -1.0f);

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool CanRecover() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollStateChanged OnRagdollStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollImpact, const FVector&, ImpactLocation, float, ImpactForce);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollImpact OnRagdollImpact;

protected:
    // Core components
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    FCore_RagdollConfig RagdollConfig;

    // State management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    ECore_RagdollState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    float BlendAlpha;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    float StateTimer;

    // Blend data
    UPROPERTY()
    float BlendDuration;

    UPROPERTY()
    bool bBlendingIn;

    // Recovery data
    UPROPERTY()
    FVector RecoveryLocation;

    UPROPERTY()
    FRotator RecoveryRotation;

    UPROPERTY()
    float RecoveryTimer;

    // Impact tracking
    UPROPERTY()
    float LastImpactForce;

    UPROPERTY()
    FVector LastImpactLocation;

    UPROPERTY()
    float ImpactCooldown;

private:
    // Internal functions
    void UpdateBlending(float DeltaTime);
    void UpdateRecovery(float DeltaTime);
    void SetRagdollState(ECore_RagdollState NewState);
    void ConfigurePhysicsProperties();
    void StoreAnimationPose();
    void RestoreAnimationPose();
    bool IsGrounded() const;
    FVector GetCenterOfMass() const;
    void HandleImpactEvent(const FVector& ImpactLocation, float ImpactForce);

    // Cached data
    TArray<FTransform> StoredBoneTransforms;
    bool bHasStoredPose;
    float GroundCheckDistance;
};