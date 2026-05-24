#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Core_RagdollPhysics.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UPhysicsAsset;
class UAnimInstance;

/**
 * Enum defining ragdoll activation triggers
 */
UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    Death           UMETA(DisplayName = "Death"),
    HighImpact      UMETA(DisplayName = "High Impact"),
    Unconscious     UMETA(DisplayName = "Unconscious"),
    Manual          UMETA(DisplayName = "Manual"),
    Falling         UMETA(DisplayName = "Falling"),
    Explosion       UMETA(DisplayName = "Explosion")
};

/**
 * Enum defining ragdoll states
 */
UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Activating      UMETA(DisplayName = "Activating"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering"),
    Blending        UMETA(DisplayName = "Blending")
};

/**
 * Structure containing ragdoll configuration parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    // Activation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    float FallHeightThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
    float HealthThreshold = 0.0f;

    // Physics parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MassScale = 1.0f;

    // Recovery parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float BlendTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bAutoRecover = true;

    // Bone-specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones")
    TMap<FName, float> BoneMassOverrides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones")
    TArray<FName> ProtectedBones;

    FCore_RagdollConfig()
    {
        ImpactThreshold = 500.0f;
        FallHeightThreshold = 300.0f;
        HealthThreshold = 0.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        MassScale = 1.0f;
        RecoveryTime = 3.0f;
        BlendTime = 1.0f;
        bAutoRecover = true;
    }
};

/**
 * Structure containing ragdoll runtime data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECore_RagdollState CurrentState = ECore_RagdollState::Inactive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECore_RagdollTrigger LastTrigger = ECore_RagdollTrigger::None;

    UPROPERTY(BlueprintReadOnly, Category = "Timing")
    float ActivationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Timing")
    float StateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    FVector LastImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Physics")
    FVector LastImpactVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    FTransform RecoveryTransform = FTransform::Identity;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    bool bCanRecover = true;

    FCore_RagdollData()
    {
        CurrentState = ECore_RagdollState::Inactive;
        LastTrigger = ECore_RagdollTrigger::None;
        ActivationTime = 0.0f;
        StateTime = 0.0f;
        LastImpactLocation = FVector::ZeroVector;
        LastImpactVelocity = FVector::ZeroVector;
        RecoveryTransform = FTransform::Identity;
        bCanRecover = true;
    }
};

/**
 * Delegate for ragdoll state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCore_OnRagdollStateChanged, ECore_RagdollState, OldState, ECore_RagdollState, NewState, ECore_RagdollTrigger, Trigger);

/**
 * Delegate for ragdoll activation
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnRagdollActivated, ECore_RagdollTrigger, Trigger, FVector, ImpactLocation);

/**
 * Delegate for ragdoll recovery
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCore_OnRagdollRecovered, float, RecoveryDuration);

/**
 * Advanced ragdoll physics system for realistic character death and impact responses
 * Handles automatic ragdoll activation, physics simulation, and recovery blending
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Config")
    FCore_RagdollConfig RagdollConfig;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    FCore_RagdollData RagdollData;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<ACharacter> OwnerCharacter;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnRagdollStateChanged OnRagdollStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnRagdollActivated OnRagdollActivated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnRagdollRecovered OnRagdollRecovered;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void ActivateRagdoll(ECore_RagdollTrigger Trigger, const FVector& ImpactLocation = FVector::ZeroVector, const FVector& ImpactVelocity = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void DeactivateRagdoll(bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    bool CanActivateRagdoll() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    bool IsRecovering() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Config")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Config")
    void SetBoneMassOverride(FName BoneName, float Mass);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Config")
    void AddProtectedBone(FName BoneName);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Config")
    void RemoveProtectedBone(FName BoneName);

    // Physics control
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpulseToBody(FName BoneName, const FVector& Impulse, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyForceToBody(FName BoneName, const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetBodyLinearDamping(FName BoneName, float Damping);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetBodyAngularDamping(FName BoneName, float Damping);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    ECore_RagdollState GetRagdollState() const { return RagdollData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    ECore_RagdollTrigger GetLastTrigger() const { return RagdollData.LastTrigger; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    float GetStateTime() const { return RagdollData.StateTime; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    FVector GetLastImpactLocation() const { return RagdollData.LastImpactLocation; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll State")
    FVector GetLastImpactVelocity() const { return RagdollData.LastImpactVelocity; }

    // Bone utilities
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utilities")
    TArray<FName> GetAllBoneNames() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utilities")
    FVector GetBoneLocation(FName BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utilities")
    FVector GetBoneVelocity(FName BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utilities")
    float GetBoneMass(FName BoneName) const;

protected:
    // Internal state management
    void SetRagdollState(ECore_RagdollState NewState, ECore_RagdollTrigger Trigger = ECore_RagdollTrigger::None);
    void UpdateRagdollState(float DeltaTime);
    
    // Activation/deactivation
    void PerformRagdollActivation(ECore_RagdollTrigger Trigger, const FVector& ImpactLocation, const FVector& ImpactVelocity);
    void PerformRagdollDeactivation();
    
    // Recovery system
    void UpdateRecovery(float DeltaTime);
    void CalculateRecoveryTransform();
    bool IsValidRecoveryPosition(const FVector& Position) const;
    
    // Physics setup
    void ConfigureRagdollPhysics();
    void ApplyBoneMassOverrides();
    void SetupPhysicsConstraints();
    
    // Impact detection
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    
    void CheckForImpactActivation(const FVector& ImpactVelocity, const FVector& ImpactLocation);
    void CheckForFallActivation();
    void CheckForHealthActivation();
    
    // Component initialization
    void InitializeComponents();
    void BindCollisionEvents();

private:
    // Internal state
    float LastFallHeight;
    bool bWasFalling;
    FVector LastValidPosition;
    
    // Cached animation data for recovery
    TObjectPtr<UAnimInstance> CachedAnimInstance;
    FTransform PreRagdollTransform;
    
    // Performance optimization
    float LastUpdateTime;
    static constexpr float UpdateFrequency = 0.016f; // ~60 FPS
};