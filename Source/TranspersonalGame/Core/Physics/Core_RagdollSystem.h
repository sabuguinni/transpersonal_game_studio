#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Core_RagdollSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCore_RagdollSystem, Log, All);

/**
 * Ragdoll Configuration Structure
 * Defines how ragdoll physics should behave for different body types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    // Mass distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float TotalMass = 70.0f;

    // Bone-specific mass multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float HeadMassMultiplier = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float TorsoMassMultiplier = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LimbMassMultiplier = 0.1f;

    // Constraint settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    float JointStiffness = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    float JointDamping = 50.0f;

    // Collision settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableSelfCollision = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionRadius = 5.0f;

    // Recovery settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bAutoRecover = true;

    FCore_RagdollConfig()
    {
        TotalMass = 70.0f;
        HeadMassMultiplier = 0.15f;
        TorsoMassMultiplier = 0.45f;
        LimbMassMultiplier = 0.1f;
        JointStiffness = 1000.0f;
        JointDamping = 50.0f;
        bEnableSelfCollision = false;
        CollisionRadius = 5.0f;
        RecoveryTime = 3.0f;
        bAutoRecover = true;
    }
};

/**
 * Ragdoll State Enumeration
 */
UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Activating      UMETA(DisplayName = "Activating"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering"),
    Failed          UMETA(DisplayName = "Failed")
};

/**
 * Core Ragdoll System Component
 * 
 * Manages realistic ragdoll physics for characters and creatures in the prehistoric world.
 * Handles death animations, knockdown effects, and physics-based character responses.
 * 
 * Key Features:
 * - Realistic mass distribution based on creature type
 * - Dynamic constraint adjustment for different impact forces
 * - Recovery system for non-fatal knockdowns
 * - Integration with survival and combat systems
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Config")
    FCore_RagdollConfig RagdollConfig;

    // Current ragdoll state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    ECore_RagdollState CurrentState;

    // Target skeletal mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* TargetMesh;

    // Recovery timer
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    float RecoveryTimer;

    // Last impact force that triggered ragdoll
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    FVector LastImpactForce;

    // Original animation blueprint class (for recovery)
    UPROPERTY()
    TSubclassOf<UAnimInstance> OriginalAnimBP;

    // Bone mass overrides
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    TMap<FName, float> BoneMassOverrides;

    // Constraint components for joints
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UPhysicsConstraintComponent*> JointConstraints;

public:
    // Primary ragdoll control functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpactForce = FVector::ZeroVector, const FVector& ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ForceRecovery();

    // Configuration functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBoneMass(const FName& BoneName, float Mass);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetJointConstraints(float Stiffness, float Damping);

    // State query functions
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRecovering() const { return CurrentState == ECore_RagdollState::Recovering; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    float GetRecoveryProgress() const;

    // Impact response functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactToRagdoll(const FVector& Force, const FVector& Location, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SimulateExplosionImpact(const FVector& ExplosionCenter, float ExplosionForce, float ExplosionRadius);

    // Dinosaur-specific functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ConfigureForDinosaur(float DinosaurMass, bool bIsLargeDinosaur = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ConfigureForHuman();

protected:
    // Internal implementation functions
    void InitializeRagdollPhysics();
    void SetupBoneMasses();
    void SetupJointConstraints();
    void UpdateRecoveryProcess(float DeltaTime);
    void BlendToRecoveryPose(float BlendAlpha);
    
    // Bone utility functions
    void GetAllBoneNames(TArray<FName>& OutBoneNames) const;
    float CalculateBoneMass(const FName& BoneName) const;
    bool IsCriticalBone(const FName& BoneName) const;
    
    // Physics state management
    void SaveOriginalAnimationState();
    void RestoreOriginalAnimationState();
    void SetPhysicsBlendWeight(float BlendWeight);

    // Event handlers
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
               FVector NormalImpulse, const FHitResult& Hit);

public:
    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Ragdoll Events")
    void OnRagdollActivated(const FVector& ImpactForce);

    UFUNCTION(BlueprintImplementableEvent, Category = "Ragdoll Events")
    void OnRagdollDeactivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Ragdoll Events")
    void OnRecoveryStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Ragdoll Events")
    void OnRecoveryCompleted();

    // Debug functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawRagdollInfo();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void TestRagdollActivation();
};