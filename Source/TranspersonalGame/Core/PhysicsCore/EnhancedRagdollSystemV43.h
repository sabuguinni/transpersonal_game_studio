#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "EnhancedRagdollSystemV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRagdollV43, Log, All);

// Ragdoll activation reasons
UENUM(BlueprintType)
enum class ERagdollActivationReason : uint8
{
    Death = 0,              // Character death
    Unconsciousness = 1,    // Knocked unconscious
    Stunned = 2,            // Temporarily stunned
    PhysicsImpact = 3,      // Hit by large force
    Falling = 4,            // Falling from height
    DinosaurAttack = 5,     // Attacked by dinosaur
    Environmental = 6       // Environmental hazard
};

// Ragdoll state
UENUM(BlueprintType)
enum class ERagdollState : uint8
{
    Inactive = 0,           // Normal animation
    Activating = 1,         // Transitioning to ragdoll
    Active = 2,             // Full ragdoll physics
    Recovering = 3,         // Transitioning back to animation
    Blending = 4            // Blending between ragdoll and animation
};

// Body part types for targeted ragdoll effects
UENUM(BlueprintType)
enum class EBodyPartType : uint8
{
    Head = 0,
    Torso = 1,
    LeftArm = 2,
    RightArm = 3,
    LeftLeg = 4,
    RightLeg = 5,
    Spine = 6,
    Pelvis = 7
};

// Delegate for ragdoll events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRagdollStateChanged, ERagdollState, NewState, 
    ERagdollState, PreviousState, ERagdollActivationReason, Reason);

// Forward declarations
class UPhysicalAnimationComponent;
class UPoseableMeshComponent;

/**
 * Enhanced Ragdoll System V43
 * Provides realistic character physics for death, unconsciousness, and impact reactions
 * Features: Smooth transitions, partial ragdoll, recovery animations, dinosaur-specific reactions
 * Optimized for large-scale character simulation with performance LOD
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnhancedRagdollSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnhancedRagdollSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Initialize ragdoll system for this character */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void InitializeRagdollSystem();

    /** Activate ragdoll physics */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void ActivateRagdoll(ERagdollActivationReason Reason, float BlendTime = 0.2f);

    /** Deactivate ragdoll and return to animation */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void DeactivateRagdoll(float BlendTime = 0.5f);

    /** Activate partial ragdoll for specific body parts */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void ActivatePartialRagdoll(const TArray<EBodyPartType>& BodyParts, float Strength = 1.0f);

    /** Apply impulse to ragdoll body part */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void ApplyRagdollImpulse(EBodyPartType BodyPart, FVector Impulse, bool bVelChange = false);

    /** Apply force to entire ragdoll */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void ApplyRagdollForce(FVector Force, FVector Location);

    /** Set ragdoll physics properties */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void SetRagdollPhysicsProperties(float LinearDamping = 0.1f, float AngularDamping = 0.1f);

    /** Enable/disable ragdoll collision with environment */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void SetRagdollCollisionEnabled(bool bEnabled);

    /** Get current ragdoll state */
    UFUNCTION(BlueprintPure, Category = \"Ragdoll System\")
    ERagdollState GetRagdollState() const { return CurrentRagdollState; }

    /** Check if ragdoll is active */
    UFUNCTION(BlueprintPure, Category = \"Ragdoll System\")
    bool IsRagdollActive() const { return CurrentRagdollState == ERagdollState::Active; }

    /** Get ragdoll activation reason */
    UFUNCTION(BlueprintPure, Category = \"Ragdoll System\")
    ERagdollActivationReason GetActivationReason() const { return LastActivationReason; }

    /** Set ragdoll enabled/disabled */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void SetRagdollEnabled(bool bEnabled);

    /** Get bone name for body part */
    UFUNCTION(BlueprintPure, Category = \"Ragdoll System\")
    FName GetBoneNameForBodyPart(EBodyPartType BodyPart) const;

    /** Get velocity of specific body part */
    UFUNCTION(BlueprintPure, Category = \"Ragdoll System\")
    FVector GetBodyPartVelocity(EBodyPartType BodyPart) const;

    /** Check if character is on ground (for recovery) */
    UFUNCTION(BlueprintPure, Category = \"Ragdoll System\")
    bool IsOnGround() const;

    /** Force immediate recovery from ragdoll */
    UFUNCTION(BlueprintCallable, Category = \"Ragdoll System\")
    void ForceRecovery();

    /** Ragdoll state change event */
    UPROPERTY(BlueprintAssignable, Category = \"Ragdoll Events\")
    FOnRagdollStateChanged OnRagdollStateChanged;

protected:
    /** Target skeletal mesh component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Ragdoll\")
    TObjectPtr<USkeletalMeshComponent> TargetMeshComponent;

    /** Physical animation component for blending */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Ragdoll\")
    TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;

    /** Current ragdoll state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Ragdoll State\")
    ERagdollState CurrentRagdollState = ERagdollState::Inactive;

    /** Last activation reason */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Ragdoll State\")
    ERagdollActivationReason LastActivationReason = ERagdollActivationReason::Death;

    /** Whether ragdoll system is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    bool bRagdollEnabled = true;

    /** Auto-recovery from ragdoll after time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    bool bAutoRecovery = true;

    /** Time before auto-recovery (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\", meta = (ClampMin = \"0.0\"))
    float AutoRecoveryTime = 5.0f;

    /** Minimum velocity for ragdoll activation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    float MinActivationVelocity = 500.0f; // 5 m/s

    /** Blend time for ragdoll activation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    float DefaultBlendTime = 0.2f;

    /** Recovery blend time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    float RecoveryBlendTime = 0.5f;

    /** Bone name mappings for body parts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    TMap<EBodyPartType, FName> BodyPartBoneNames;

    /** Physics properties for different body parts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Ragdoll Settings\")
    TMap<EBodyPartType, float> BodyPartMasses;

    /** Linear damping for ragdoll physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Physics Settings\")
    float LinearDamping = 0.1f;

    /** Angular damping for ragdoll physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Physics Settings\")
    float AngularDamping = 0.1f;

    /** Maximum ragdoll distance from player for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    float MaxRagdollDistance = 5000.0f; // 50m

    /** Use distance LOD for ragdoll quality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    bool bUseDistanceLOD = true;

    /** Maximum simultaneous ragdolls */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    int32 MaxSimultaneousRagdolls = 10;

private:
    /** Initialize bone name mappings */
    void InitializeBoneNameMappings();

    /** Initialize body part masses */
    void InitializeBodyPartMasses();

    /** Update ragdoll state machine */
    void UpdateRagdollStateMachine(float DeltaTime);

    /** Handle ragdoll activation transition */
    void HandleRagdollActivation(float DeltaTime);

    /** Handle ragdoll recovery transition */
    void HandleRagdollRecovery(float DeltaTime);

    /** Apply physics settings to ragdoll */
    void ApplyPhysicsSettings();

    /** Check if should use ragdoll based on distance */
    bool ShouldUseRagdoll() const;

    /** Get player distance for LOD calculations */
    float GetPlayerDistance() const;

    /** Setup physical animation profiles */
    void SetupPhysicalAnimationProfiles();

    /** Cleanup ragdoll state */
    void CleanupRagdollState();

    /** Get body instance for body part */
    FBodyInstance* GetBodyInstanceForBodyPart(EBodyPartType BodyPart) const;

    // Internal state
    float CurrentBlendTime = 0.0f;
    float TargetBlendTime = 0.0f;
    float BlendAlpha = 0.0f;
    float TimeInCurrentState = 0.0f;
    
    // Auto-recovery timer
    FTimerHandle AutoRecoveryTimer;
    
    // Performance tracking
    static int32 ActiveRagdollCount;
    bool bIsActiveRagdoll = false;
    
    // Cached animation data for recovery
    FTransform CachedRootTransform;
    TArray<FTransform> CachedBoneTransforms;
};