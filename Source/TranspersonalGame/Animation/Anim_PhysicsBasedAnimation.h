#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Anim_PhysicsBasedAnimation.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsAnimation, Log, All);

/**
 * Enum for different physics animation modes
 */
UENUM(BlueprintType)
enum class EAnim_PhysicsMode : uint8
{
    None            UMETA(DisplayName = "None"),
    Ragdoll         UMETA(DisplayName = "Ragdoll"),
    PartialRagdoll  UMETA(DisplayName = "Partial Ragdoll"),
    PhysicsBlend    UMETA(DisplayName = "Physics Blend"),
    ImpactReaction  UMETA(DisplayName = "Impact Reaction"),
    ClothSimulation UMETA(DisplayName = "Cloth Simulation")
};

/**
 * Struct for physics bone configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PhysicsBoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    bool bUsePhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    float PhysicsBlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    float Damping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    float Stiffness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    bool bLimitRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bone")
    FVector RotationLimits;

    FAnim_PhysicsBoneConfig()
    {
        BoneName = NAME_None;
        bUsePhysics = false;
        PhysicsBlendWeight = 0.0f;
        Damping = 0.1f;
        Stiffness = 1.0f;
        bLimitRotation = true;
        RotationLimits = FVector(45.0f, 45.0f, 45.0f);
    }
};

/**
 * Struct for impact reaction configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_ImpactReactionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    float ImpactThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    float ReactionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    float ReactionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    bool bUseDirectionalReaction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    TArray<FName> ReactionBones;

    FAnim_ImpactReactionConfig()
    {
        ImpactThreshold = 100.0f;
        ReactionDuration = 1.0f;
        ReactionStrength = 1.0f;
        bUseDirectionalReaction = true;
        ReactionBones = {"spine_01", "spine_02", "spine_03", "head"};
    }
};

/**
 * Delegate for physics animation events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAnim_OnPhysicsStateChanged, EAnim_PhysicsMode, NewMode, EAnim_PhysicsMode, OldMode, float, BlendWeight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnim_OnImpactReaction, FVector, ImpactLocation, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnim_OnRagdollActivated, bool, bActivated);

/**
 * Physics-based animation system for realistic character movement and reactions
 * Handles ragdoll physics, impact reactions, cloth simulation, and physics blending
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PhysicsBasedAnimation : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PhysicsBasedAnimation();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE PHYSICS ANIMATION ===
    
    /**
     * Set the physics animation mode
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Animation")
    void SetPhysicsMode(EAnim_PhysicsMode NewMode, float BlendTime = 0.5f);
    
    /**
     * Get the current physics mode
     */
    UFUNCTION(BlueprintPure, Category = "Physics Animation")
    EAnim_PhysicsMode GetCurrentPhysicsMode() const { return CurrentPhysicsMode; }
    
    /**
     * Enable or disable physics for specific bones
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Animation")
    void SetBonePhysicsEnabled(const FName& BoneName, bool bEnabled, float BlendWeight = 1.0f);
    
    /**
     * Set physics blend weight for all bones
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Animation")
    void SetGlobalPhysicsBlendWeight(float BlendWeight);
    
    // === RAGDOLL SYSTEM ===
    
    /**
     * Activate full ragdoll physics
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(bool bImmediate = false);
    
    /**
     * Deactivate ragdoll and return to animation
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll(float BlendTime = 1.0f);
    
    /**
     * Check if ragdoll is currently active
     */
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return bIsRagdollActive; }
    
    /**
     * Set ragdoll physics properties
     */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollProperties(float Damping, float Stiffness, bool bUseGravity = true);
    
    // === IMPACT REACTION SYSTEM ===
    
    /**
     * Trigger an impact reaction at a specific location
     */
    UFUNCTION(BlueprintCallable, Category = "Impact Reaction")
    void TriggerImpactReaction(const FVector& ImpactLocation, const FVector& ImpactForce, float Duration = 1.0f);
    
    /**
     * Set impact reaction configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Impact Reaction")
    void SetImpactReactionConfig(const FAnim_ImpactReactionConfig& NewConfig);
    
    /**
     * Enable or disable automatic impact reactions
     */
    UFUNCTION(BlueprintCallable, Category = "Impact Reaction")
    void SetAutoImpactReaction(bool bEnabled) { bUseAutoImpactReaction = bEnabled; }
    
    // === PHYSICS BLENDING ===
    
    /**
     * Blend between animation and physics for specific bones
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Blending")
    void BlendPhysicsForBones(const TArray<FName>& BoneNames, float PhysicsWeight, float BlendTime = 0.5f);
    
    /**
     * Set physics bone configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Blending")
    void SetPhysicsBoneConfig(const FName& BoneName, const FAnim_PhysicsBoneConfig& Config);
    
    /**
     * Get physics bone configuration
     */
    UFUNCTION(BlueprintPure, Category = "Physics Blending")
    FAnim_PhysicsBoneConfig GetPhysicsBoneConfig(const FName& BoneName) const;
    
    // === CLOTH SIMULATION ===
    
    /**
     * Enable cloth simulation for specific bones
     */
    UFUNCTION(BlueprintCallable, Category = "Cloth Simulation")
    void EnableClothSimulation(const TArray<FName>& ClothBones, bool bEnabled);
    
    /**
     * Set cloth simulation parameters
     */
    UFUNCTION(BlueprintCallable, Category = "Cloth Simulation")
    void SetClothParameters(float Damping, float Stiffness, float WindStrength = 0.0f);
    
    // === UTILITY FUNCTIONS ===
    
    /**
     * Get the skeletal mesh component
     */
    UFUNCTION(BlueprintPure, Category = "Physics Animation")
    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComp; }
    
    /**
     * Check if a bone exists in the skeleton
     */
    UFUNCTION(BlueprintPure, Category = "Physics Animation")
    bool DoesBoneExist(const FName& BoneName) const;
    
    /**
     * Get all physics-enabled bones
     */
    UFUNCTION(BlueprintPure, Category = "Physics Animation")
    TArray<FName> GetPhysicsEnabledBones() const;
    
    // === EVENTS ===
    
    UPROPERTY(BlueprintAssignable, Category = "Physics Animation Events")
    FAnim_OnPhysicsStateChanged OnPhysicsStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Physics Animation Events")
    FAnim_OnImpactReaction OnImpactReaction;
    
    UPROPERTY(BlueprintAssignable, Category = "Physics Animation Events")
    FAnim_OnRagdollActivated OnRagdollActivated;

protected:
    // === CORE PROPERTIES ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Animation")
    EAnim_PhysicsMode CurrentPhysicsMode;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Animation")
    EAnim_PhysicsMode PreviousPhysicsMode;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Animation")
    float GlobalPhysicsBlendWeight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Animation")
    float PhysicsBlendSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Animation")
    bool bUsePhysicsAnimation;
    
    // === COMPONENT REFERENCES ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class ACharacter* OwnerCharacter;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAnimInstance* AnimInstance;
    
    // === RAGDOLL PROPERTIES ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    bool bIsRagdollActive;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollDamping;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollStiffness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bUseRagdollGravity;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollActivationTime;
    
    // === IMPACT REACTION PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    FAnim_ImpactReactionConfig ImpactReactionConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Reaction")
    bool bUseAutoImpactReaction;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Reaction")
    bool bIsReactingToImpact;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Reaction")
    float ImpactReactionTimer;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Reaction")
    FVector LastImpactLocation;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Reaction")
    FVector LastImpactForce;
    
    // === PHYSICS BONE CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bones")
    TMap<FName, FAnim_PhysicsBoneConfig> PhysicsBoneConfigs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Bones")
    TArray<FName> DefaultPhysicsBones;
    
    // === CLOTH SIMULATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Simulation")
    TArray<FName> ClothSimulationBones;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Simulation")
    float ClothDamping;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Simulation")
    float ClothStiffness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Simulation")
    float ClothWindStrength;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Simulation")
    bool bUseClothSimulation;
    
    // === PERFORMANCE SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDetailedPhysics;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateFrequency;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastPhysicsUpdate;
    
    // === INTERNAL STATE ===
    
    UPROPERTY()
    float CurrentBlendWeight;
    
    UPROPERTY()
    float TargetBlendWeight;
    
    UPROPERTY()
    bool bIsTransitioning;
    
    UPROPERTY()
    float TransitionTimer;
    
    UPROPERTY()
    float TransitionDuration;
    
    // === TIMERS ===
    
    FTimerHandle PhysicsUpdateTimer;
    FTimerHandle RagdollBlendTimer;
    FTimerHandle ImpactReactionTimer;

private:
    // === INTERNAL METHODS ===
    
    void InitializePhysicsAnimation();
    void UpdatePhysicsBlending(float DeltaTime);
    void UpdateRagdollState(float DeltaTime);
    void UpdateImpactReaction(float DeltaTime);
    void UpdateClothSimulation(float DeltaTime);
    
    void ApplyPhysicsToSkeleton();
    void BlendPhysicsWithAnimation(float DeltaTime);
    void SetupDefaultPhysicsBones();
    
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, 
               FVector NormalImpulse, const FHitResult& Hit);
    
    void TransitionToPhysicsMode(EAnim_PhysicsMode NewMode, float BlendTime);
    void FinishPhysicsModeTransition();
    
    bool ValidateBoneName(const FName& BoneName) const;
    int32 GetBoneIndex(const FName& BoneName) const;
    
    void SetBonePhysicsBlendWeight(const FName& BoneName, float BlendWeight);
    float GetBonePhysicsBlendWeight(const FName& BoneName) const;
    
    void ApplyImpactForceToBody(const FName& BoneName, const FVector& Force, const FVector& Location);
    void CalculateImpactReactionBones(const FVector& ImpactLocation, TArray<FName>& OutBones, TArray<float>& OutWeights);
    
    void EnablePhysicsForBone(const FName& BoneName, bool bEnabled);
    void SetPhysicsConstraintsForBone(const FName& BoneName, const FAnim_PhysicsBoneConfig& Config);
};