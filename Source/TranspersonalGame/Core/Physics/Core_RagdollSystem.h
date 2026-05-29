#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Core_RagdollSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRagdollSystem, Log, All);

/**
 * Ragdoll State for tracking physics simulation state
 */
UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Partial     UMETA(DisplayName = "Partial"),
    Full        UMETA(DisplayName = "Full"),
    Blending    UMETA(DisplayName = "Blending")
};

/**
 * Ragdoll Configuration for different scenarios
 */
USTRUCT(BlueprintType)
struct FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float PhysicsBlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    FCore_RagdollConfig()
    {
        BlendTime = 0.5f;
        PhysicsBlendWeight = 1.0f;
        bEnableGravity = true;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
    }
};

/**
 * Core Ragdoll System Component
 * 
 * Manages physics-based ragdoll simulation for characters and dinosaurs.
 * Handles smooth transitions between animated and physics states.
 * 
 * Features:
 * - Smooth blending between animation and physics
 * - Configurable ragdoll parameters per body part
 * - Death ragdoll activation
 * - Impact-based partial ragdoll
 * - Recovery from ragdoll state
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

public:
    // Core Ragdoll Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(bool bFullBody = true);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivatePartialRagdoll(const TArray<FName>& BoneNames);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName = NAME_None);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState != ECore_RagdollState::Disabled; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    float GetPhysicsBlendWeight() const { return CurrentBlendWeight; }

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollConfig DefaultConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollConfig DeathConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoActivateOnDeath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MinImpulseForPartialRagdoll = 500.0f;

    // Runtime State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float CurrentBlendWeight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float BlendTimer;

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UPhysicsAsset* PhysicsAsset;

    // Active Configuration
    FCore_RagdollConfig ActiveConfig;
    TArray<FName> PartialRagdollBones;
    bool bBlendingIn;

private:
    void InitializeRagdollSystem();
    void UpdateBlending(float DeltaTime);
    void ApplyRagdollConfiguration();
    void SetPhysicsBlendWeight(float Weight);
    void EnablePhysicsOnBones(const TArray<FName>& BoneNames);
    void DisablePhysicsOnAllBones();

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollStateChanged OnRagdollStateChanged;
};