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
    Partial     UMETA(DisplayName = "Partial"),
    Full        UMETA(DisplayName = "Full"),
    Recovering  UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    FCore_RagdollSettings()
    {
        ActivationForce = 1000.0f;
        RecoveryTime = 3.0f;
        BlendOutTime = 1.0f;
        bAutoRecover = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll Control Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(float Force = 0.0f, FVector ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollState(ECore_RagdollState NewState);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Full || CurrentState == ECore_RagdollState::Partial; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    FCore_RagdollSettings GetRagdollSettings() const { return Settings; }

    // Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactForce(FVector Force, FVector Location, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBonePhysicsBlendWeight(FName BoneName, float BlendWeight);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollStateChanged OnRagdollStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollActivated);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollActivated OnRagdollActivated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollDeactivated);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollDeactivated OnRagdollDeactivated;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollSettings Settings;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Recovery System
    UPROPERTY()
    float RecoveryTimer;

    UPROPERTY()
    bool bIsRecovering;

    // Blend Weights for Partial Ragdoll
    UPROPERTY()
    TMap<FName, float> BoneBlendWeights;

private:
    void InitializeRagdollSystem();
    void UpdateRecoverySystem(float DeltaTime);
    void BlendToRagdoll(float BlendWeight);
    void RestoreAnimation();
    void SetPhysicsBlendWeight(float BlendWeight);
    void BroadcastStateChange(ECore_RagdollState NewState);
};