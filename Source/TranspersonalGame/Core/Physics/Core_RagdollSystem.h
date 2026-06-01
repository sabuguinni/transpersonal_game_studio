#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Core_RagdollSystem.generated.h"

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

    FCore_RagdollBone()
    {
        BoneName = NAME_None;
        Mass = 1.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
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
    float LinearLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float Strength;

    FCore_RagdollConstraint()
    {
        ParentBone = NAME_None;
        ChildBone = NAME_None;
        LinearLimit = 100.0f;
        AngularLimit = 45.0f;
        Strength = 1000.0f;
    }
};

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive UMETA(DisplayName = "Inactive"),
    Activating UMETA(DisplayName = "Activating"),
    Active UMETA(DisplayName = "Active"),
    Deactivating UMETA(DisplayName = "Deactivating")
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
    void ActivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollImpulse(const FVector& Impulse, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetupRagdollConstraints();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ConfigureBonePhysics(const FName& BoneName, float Mass, float LinearDamping, float AngularDamping);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TObjectPtr<USkeletalMeshComponent> TargetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TArray<FCore_RagdollBone> RagdollBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TArray<FCore_RagdollConstraint> RagdollConstraints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float DeactivationBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoSetupConstraints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float GlobalMassScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float GlobalDampingScale;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    float StateTransitionTime;

private:
    void InitializeRagdollBones();
    void UpdateStateTransition(float DeltaTime);
    void BlendToRagdoll(float Alpha);
    void BlendFromRagdoll(float Alpha);
    
    float TransitionTimer;
    TArray<FTransform> PreRagdollBoneTransforms;
    bool bRagdollInitialized;
};