#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

class USkeletalMeshComponent;
class UPhysicsConstraintComponent;
class UAnimInstance;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollBone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float Mass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableGravity;

    FCore_RagdollBone()
    {
        BoneName = NAME_None;
        Mass = 1.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConstraint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FName ParentBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FName ChildBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularSwing1Limit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularSwing2Limit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularTwistLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearBreakForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularBreakTorque;

    FCore_RagdollConstraint()
    {
        ParentBone = NAME_None;
        ChildBone = NAME_None;
        AngularSwing1Limit = 45.0f;
        AngularSwing2Limit = 45.0f;
        AngularTwistLimit = 15.0f;
        LinearBreakForce = 100000.0f;
        AngularBreakTorque = 100000.0f;
    }
};

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Transitioning   UMETA(DisplayName = "Transitioning"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Ragdoll Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    TArray<FCore_RagdollBone> RagdollBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    TArray<FCore_RagdollConstraint> RagdollConstraints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    float TransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    float RecoveryTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    bool bAutoRecover;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    float MinVelocityForRagdoll;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    float MaxRagdollTime;

    // Ragdoll State
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    ECore_RagdollState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    float StateTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    bool bIsRagdollActive;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TArray<UPhysicsConstraintComponent*> PhysicsConstraints;

    // Ragdoll Control Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void ActivateRagdoll(float Force = 0.0f, FVector ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    void SetRagdollState(ECore_RagdollState NewState);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Control")
    bool IsRagdollActive() const;

    // Bone Configuration Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Configuration")
    void ConfigureBonePhysics(FName BoneName, float Mass, float LinearDamping, float AngularDamping);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Configuration")
    void SetupDefaultRagdollBones();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Configuration")
    void CreatePhysicsConstraints();

    // Impact and Force Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpactForce(FVector Force, FVector Location, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyRadialImpulse(FVector Origin, float Radius, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    FVector GetBoneVelocity(FName BoneName) const;

    // Recovery Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Recovery")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Recovery")
    bool CanRecover() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Recovery")
    void BlendToAnimation(float BlendTime = 1.0f);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utility")
    void SaveCurrentPose();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utility")
    void RestoreSavedPose();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Utility")
    float GetRagdollIntensity() const;

protected:
    // Internal Functions
    void UpdateRagdollState(float DeltaTime);
    void ProcessTransition(float DeltaTime);
    void ProcessRecovery(float DeltaTime);
    void InitializeRagdollBones();
    void CleanupPhysicsConstraints();
    bool ValidateSkeletalMesh() const;

    // Saved Animation Data
    TMap<FName, FTransform> SavedBoneTransforms;
    UAnimInstance* CachedAnimInstance;
    float RagdollTimer;
    FVector LastImpactLocation;
    float LastImpactForce;
};