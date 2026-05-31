#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Animation/AnimInstance.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Activating  UMETA(DisplayName = "Activating"),
    Active      UMETA(DisplayName = "Active"),
    Blending    UMETA(DisplayName = "Blending Back")
};

USTRUCT(BlueprintType)
struct FCore_RagdollBone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float Mass = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableGravity = true;

    FCore_RagdollBone()
    {
        BoneName = NAME_None;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FString ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TArray<FCore_RagdollBone> Bones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollTime = 10.0f;

    FCore_RagdollProfile()
    {
        ProfileName = TEXT("Default");
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core ragdoll functionality
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FString& ProfileName = TEXT("Default"));

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void BlendToAnimation(float BlendTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void AddRagdollProfile(const FCore_RagdollProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetSkeletalMeshComponent(USkeletalMeshComponent* InMeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBonePhysicsProperties(const FName& BoneName, float Mass, float LinearDamping, float AngularDamping);

    // Impact and force application
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseAtBone(const FName& BoneName, const FVector& Impulse);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyRadialImpulse(const FVector& Origin, float Radius, float Strength, bool bVelChange = false);

    // Constraint management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetConstraintLimits(const FName& BoneName, bool bEnableSwing1, bool bEnableSwing2, bool bEnableTwist);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ModifyConstraintStrength(const FName& BoneName, float Strength);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TArray<FCore_RagdollProfile> RagdollProfiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FString ActiveProfileName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollActiveTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float BlendTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoBlendBack = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AutoBlendBackDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bPreserveMomentum = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MomentumScale = 1.0f;

    // Cached animation data for blending back
    UPROPERTY()
    TArray<FTransform> CachedBoneTransforms;

    UPROPERTY()
    TArray<FName> RagdollBoneNames;

private:
    void InitializeRagdollProfiles();
    void CacheBoneTransforms();
    void ApplyRagdollProfile(const FCore_RagdollProfile& Profile);
    void UpdateBlending(float DeltaTime);
    void UpdateRagdollState(float DeltaTime);
    FCore_RagdollProfile* FindProfile(const FString& ProfileName);
    void SetupDefaultProfile();
};