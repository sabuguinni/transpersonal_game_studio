#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_AdvancedPhysicsController.generated.h"

UENUM(BlueprintType)
enum class EAnim_PhysicsState : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Partial         UMETA(DisplayName = "Partial Physics"),
    FullRagdoll     UMETA(DisplayName = "Full Ragdoll"),
    Recovery        UMETA(DisplayName = "Recovery"),
    Blending        UMETA(DisplayName = "Blending")
};

UENUM(BlueprintType)
enum class EAnim_ImpactType : uint8
{
    Light           UMETA(DisplayName = "Light Impact"),
    Medium          UMETA(DisplayName = "Medium Impact"),
    Heavy           UMETA(DisplayName = "Heavy Impact"),
    Extreme         UMETA(DisplayName = "Extreme Impact"),
    Environmental   UMETA(DisplayName = "Environmental")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float RagdollBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float RecoveryBlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MinRagdollTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bUsePhysicsBlending = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float PhysicsBlendWeight = 0.0f;

    FAnim_PhysicsSettings()
    {
        RagdollBlendTime = 0.2f;
        RecoveryBlendTime = 0.5f;
        ImpactThreshold = 500.0f;
        MinRagdollTime = 1.0f;
        MaxRagdollTime = 10.0f;
        bUsePhysicsBlending = true;
        bAutoRecover = true;
        PhysicsBlendWeight = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    EAnim_ImpactType ImpactType = EAnim_ImpactType::Light;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    bool bShouldTriggerRagdoll = false;

    FAnim_ImpactData()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactVelocity = FVector::ZeroVector;
        ImpactForce = 0.0f;
        ImpactType = EAnim_ImpactType::Light;
        BoneName = NAME_None;
        bShouldTriggerRagdoll = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnim_OnPhysicsStateChanged, EAnim_PhysicsState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnim_OnImpactReceived, const FAnim_ImpactData&, ImpactData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAnim_OnRagdollRecovery);

/**
 * Advanced physics-based animation controller for realistic character responses
 * Handles ragdoll physics, impact reactions, and seamless blending between
 * animated and physics-driven states for prehistoric survival scenarios
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_AdvancedPhysicsController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_AdvancedPhysicsController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Controller")
    FAnim_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Controller")
    EAnim_PhysicsState CurrentPhysicsState = EAnim_PhysicsState::Disabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Controller")
    bool bIsRagdollActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Controller")
    float CurrentRagdollTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Controller")
    float PhysicsBlendAlpha = 0.0f;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> GetUpFromBackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> GetUpFromFrontMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> ImpactReactionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<TObjectPtr<UAnimSequence>> RecoveryAnimations;

    // Impact System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    TMap<EAnim_ImpactType, float> ImpactThresholds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    float LastImpactTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    FAnim_ImpactData LastImpactData;

    // Bone Constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Constraints")
    TArray<FName> CriticalBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Constraints")
    TArray<FName> PartialPhysicsBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Constraints")
    bool bConstrainCriticalBones = true;

public:
    // Physics State Management
    UFUNCTION(BlueprintCallable, Category = "Physics Controller")
    void EnableRagdoll(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Controller")
    void DisableRagdoll(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Controller")
    void SetPhysicsState(EAnim_PhysicsState NewState);

    UFUNCTION(BlueprintCallable, Category = "Physics Controller")
    void BlendToPhysics(float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Physics Controller")
    void BlendToAnimation(float BlendTime = 0.5f);

    // Impact System
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ProcessImpact(const FAnim_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ApplyImpactForce(FVector Force, FVector Location, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    bool ShouldTriggerRagdoll(const FAnim_ImpactData& ImpactData) const;

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    EAnim_ImpactType CalculateImpactType(float ImpactForce) const;

    // Recovery System
    UFUNCTION(BlueprintCallable, Category = "Recovery System")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Recovery System")
    bool CanRecover() const;

    UFUNCTION(BlueprintCallable, Category = "Recovery System")
    UAnimMontage* GetRecoveryMontage() const;

    UFUNCTION(BlueprintCallable, Category = "Recovery System")
    bool IsCharacterFacingUp() const;

    // Bone Management
    UFUNCTION(BlueprintCallable, Category = "Bone Management")
    void SetBonePhysicsState(FName BoneName, bool bSimulatePhysics);

    UFUNCTION(BlueprintCallable, Category = "Bone Management")
    void SetPartialPhysics(const TArray<FName>& BoneNames);

    UFUNCTION(BlueprintCallable, Category = "Bone Management")
    void ConstrainBone(FName BoneName, bool bConstrain);

    // Utility Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Controller")
    bool IsInRagdoll() const { return bIsRagdollActive; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Controller")
    EAnim_PhysicsState GetPhysicsState() const { return CurrentPhysicsState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Controller")
    float GetPhysicsBlendAlpha() const { return PhysicsBlendAlpha; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Controller")
    float GetRagdollTime() const { return CurrentRagdollTime; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FAnim_OnPhysicsStateChanged OnPhysicsStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FAnim_OnImpactReceived OnImpactReceived;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FAnim_OnRagdollRecovery OnRagdollRecovery;

private:
    // Internal State
    float BlendTimer = 0.0f;
    float TargetBlendAlpha = 0.0f;
    float BlendSpeed = 1.0f;
    bool bIsBlending = false;
    
    // Internal Methods
    void UpdatePhysicsBlending(float DeltaTime);
    void UpdateRagdollTimer(float DeltaTime);
    void InitializePhysicsSettings();
    void CacheComponentReferences();
    void SetupBoneConstraints();
    FVector GetCharacterUpVector() const;
    float CalculateGroundDistance() const;
};