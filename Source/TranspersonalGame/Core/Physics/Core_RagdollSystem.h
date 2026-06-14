#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive UMETA(DisplayName = "Inactive"),
    Transitioning UMETA(DisplayName = "Transitioning"),
    Active UMETA(DisplayName = "Active"),
    Recovering UMETA(DisplayName = "Recovering")
};

UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    Death UMETA(DisplayName = "Death"),
    Impact UMETA(DisplayName = "Impact"),
    Explosion UMETA(DisplayName = "Explosion"),
    Manual UMETA(DisplayName = "Manual")
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
    float MinImpactForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bBlendPhysics = true;

    FCore_RagdollSettings()
    {
        TransitionTime = 0.2f;
        RecoveryTime = 1.0f;
        MinImpactForce = 500.0f;
        MaxRagdollTime = 10.0f;
        bAutoRecover = true;
        bBlendPhysics = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCore_OnRagdollStateChanged, ECore_RagdollState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnRagdollTriggered, ECore_RagdollTrigger, Trigger, float, ImpactForce);

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    ECore_RagdollState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Components")
    USkeletalMeshComponent* TargetMesh;

    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FCore_OnRagdollStateChanged OnRagdollStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FCore_OnRagdollTriggered OnRagdollTriggered;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void ActivateRagdoll(ECore_RagdollTrigger Trigger = ECore_RagdollTrigger::Manual, float ImpactForce = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    float GetRagdollActiveTime() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpactForce(const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetPhysicsBlendWeight(float BlendWeight);

protected:
    UPROPERTY()
    float StateTimer;

    UPROPERTY()
    float RagdollActiveTime;

    UPROPERTY()
    bool bWasSimulatingPhysics;

    UPROPERTY()
    UAnimInstance* CachedAnimInstance;

    void UpdateRagdollState(float DeltaTime);
    void TransitionToState(ECore_RagdollState NewState);
    void HandleStateTransition(float DeltaTime);
    void HandleActiveRagdoll(float DeltaTime);
    void HandleRecovery(float DeltaTime);
    void BlendPhysicsWeight(float TargetWeight, float DeltaTime);

private:
    float CurrentPhysicsBlend;
    float TargetPhysicsBlend;
};