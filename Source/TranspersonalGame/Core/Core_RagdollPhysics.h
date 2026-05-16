#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "SharedTypes.h"
#include "Core_RagdollPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Activating      UMETA(DisplayName = "Activating"),
    Active          UMETA(DisplayName = "Active"),
    Deactivating    UMETA(DisplayName = "Deactivating"),
    Blending        UMETA(DisplayName = "Blending")
};

UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    Manual          UMETA(DisplayName = "Manual"),
    HealthThreshold UMETA(DisplayName = "Health Threshold"),
    ImpactForce     UMETA(DisplayName = "Impact Force"),
    Death           UMETA(DisplayName = "Death"),
    Stun            UMETA(DisplayName = "Stun")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float DeactivationDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bUsePhysicsBlending = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float PhysicsBlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableCollisionWithWorld = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableCollisionWithPawns = false;

    FCore_RagdollSettings()
    {
        ActivationThreshold = 500.0f;
        DeactivationDelay = 3.0f;
        BlendInTime = 0.2f;
        BlendOutTime = 0.5f;
        bUsePhysicsBlending = true;
        PhysicsBlendWeight = 1.0f;
        bEnableCollisionWithWorld = true;
        bEnableCollisionWithPawns = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollBoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone")
    bool bIsKeyBone = false;

    FCore_RagdollBoneData()
    {
        BoneName = NAME_None;
        Mass = 1.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        bIsKeyBone = false;
    }
};

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Ragdoll Control
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(ECore_RagdollTrigger Trigger = ECore_RagdollTrigger::Manual);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void BlendToAnimation(float BlendTime = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const;

    // Impact and Force Application
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactForce(const FVector& Force, const FVector& Location, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyRadialImpulse(const FVector& Origin, float Radius, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBonePhysicsProperties(const FName& BoneName, float Mass, float LinearDamping, float AngularDamping);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    FCore_RagdollSettings GetRagdollSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void AddBoneConfiguration(const FCore_RagdollBoneData& BoneData);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollStateChanged OnRagdollStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollImpact, const FVector&, ImpactLocation, float, ImpactStrength);
    UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
    FOnRagdollImpact OnRagdollImpact;

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_RagdollBoneData> BoneConfigurations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    ECore_RagdollState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    float StateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll", meta = (AllowPrivateAccess = "true"))
    float BlendAlpha;

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Internal Methods
    void InitializeRagdollSystem();
    void UpdateRagdollState(float DeltaTime);
    void SetRagdollState(ECore_RagdollState NewState);
    void ConfigurePhysicsAsset();
    void BlendPhysicsAndAnimation(float DeltaTime);
    bool ShouldActivateRagdoll(ECore_RagdollTrigger Trigger) const;
    void CacheAnimationPose();
    void RestoreAnimationPose();

    // Cached Data
    TMap<FName, FTransform> CachedBoneTransforms;
    TMap<FName, FVector> CachedBoneVelocities;
    float LastImpactTime;
    float LastImpactStrength;
};