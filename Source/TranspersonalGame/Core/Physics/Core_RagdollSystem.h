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
    Disabled    UMETA(DisplayName = "Disabled"),
    Transitioning UMETA(DisplayName = "Transitioning"),
    Active      UMETA(DisplayName = "Active"),
    Recovering  UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float TransitionTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MinImpulseThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    FCore_RagdollSettings()
    {
        TransitionTime = 0.2f;
        RecoveryTime = 1.0f;
        MinImpulseThreshold = 500.0f;
        MaxRagdollDuration = 10.0f;
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
    // Ragdoll Control
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdoll(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdoll(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ToggleRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    // Impact Response
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactForce(const FVector& ImpactLocation, const FVector& ImpactForce, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ProcessDamageImpact(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection);

    // Recovery System
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ForceRecovery();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    FCore_RagdollSettings GetRagdollSettings() const { return RagdollSettings; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    ECore_RagdollState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    float StateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    float RagdollActiveTime;

    // Component References
    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

    UPROPERTY()
    TWeakObjectPtr<UAnimInstance> CachedAnimInstance;

    // Transition Data
    UPROPERTY()
    FTransform PreRagdollTransform;

    UPROPERTY()
    FVector PreRagdollVelocity;

    UPROPERTY()
    TArray<FTransform> BoneTransforms;

private:
    // Internal Methods
    void UpdateRagdollState(float DeltaTime);
    void TransitionToRagdoll();
    void TransitionFromRagdoll();
    void CachePreRagdollState();
    void RestorePreRagdollState();
    bool ShouldAutoRecover() const;
    void UpdateBoneTransforms();
    void BlendToRecoveryPose(float BlendAlpha);

    // Validation
    bool IsValidForRagdoll() const;
    USkeletalMeshComponent* GetSkeletalMeshComponent() const;

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollStateChanged OnRagdollStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollImpact, FVector, ImpactLocation, float, ImpactForce);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollImpact OnRagdollImpact;
};