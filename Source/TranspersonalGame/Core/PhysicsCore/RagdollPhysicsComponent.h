#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "SharedTypes.h"
#include "RagdollPhysicsComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Transitioning   UMETA(DisplayName = "Transitioning"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct FCore_RagdollTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float TransitionDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bPreserveVelocity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FVector ImpactForce = FVector::ZeroVector;

    FCore_RagdollTransition()
    {
        TransitionDuration = 0.5f;
        BlendWeight = 1.0f;
        bPreserveVelocity = true;
        ImpactForce = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MassScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float MinVelocityThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bAutoRecover = true;

    FCore_RagdollSettings()
    {
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        MassScale = 1.0f;
        RecoveryTime = 3.0f;
        MinVelocityThreshold = 50.0f;
        bAutoRecover = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollImpact, FVector, ImpactLocation, float, ImpactForce);

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URagdollPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URagdollPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Ragdoll Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FCore_RagdollTransition& Transition = FCore_RagdollTransition());

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    float GetStateTime() const { return StateTimer; }

    // Advanced Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBonePhysicsSettings(const FName& BoneName, float LinearDamping, float AngularDamping);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    FVector GetBoneVelocity(const FName& BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void FreezeRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void UnfreezeRagdoll();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRagdollStateChanged OnRagdollStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRagdollImpact OnRagdollImpact;

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECore_RagdollState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float StateTimer;

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UPhysicsAsset* CachedPhysicsAsset;

    // Transition Data
    FCore_RagdollTransition CurrentTransition;
    float TransitionTimer;
    TMap<FName, FTransform> PreRagdollBoneTransforms;

private:
    // Internal Functions
    void UpdateRagdollState(float DeltaTime);
    void TransitionToRagdoll();
    void TransitionFromRagdoll();
    void UpdatePhysicsSettings();
    void CacheBoneTransforms();
    void RestoreBoneTransforms();
    bool ShouldAutoRecover() const;
    float CalculateRagdollVelocity() const;

    // Cached Data
    bool bWasSimulatingPhysics;
    TArray<FName> PhysicsBones;
};