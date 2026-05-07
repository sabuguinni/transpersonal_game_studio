#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/CapsuleComponent.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Transitioning   UMETA(DisplayName = "Transitioning"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering")
};

UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    Death           UMETA(DisplayName = "Death"),
    Unconscious     UMETA(DisplayName = "Unconscious"),
    Impact          UMETA(DisplayName = "Impact"),
    Explosion       UMETA(DisplayName = "Explosion"),
    Manual          UMETA(DisplayName = "Manual")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float TransitionTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bBlendPhysics = true;

    FCore_RagdollConfig()
    {
        TransitionTime = 0.2f;
        RecoveryTime = 1.0f;
        ImpactThreshold = 500.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bAutoRecover = true;
        bBlendPhysics = true;
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
    // Core ragdoll functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdoll(ECore_RagdollTrigger Trigger = ECore_RagdollTrigger::Manual);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollState(ECore_RagdollState NewState);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactForce(FVector Force, FVector Location, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    // Physics blending
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetPhysicsBlendWeight(float BlendWeight);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void BlendToPhysics(float BlendTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void BlendToAnimation(float BlendTime = 1.0f);

    // Bone manipulation
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBonePhysicsEnabled(FName BoneName, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetAllBonesPhysicsEnabled(bool bEnabled);

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool CanRecover() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollConfig RagdollConfig;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCapsuleComponent* CapsuleComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Transition variables
    float TransitionTimer;
    float RecoveryTimer;
    float PhysicsBlendWeight;
    bool bIsTransitioning;
    bool bIsRecovering;

    // Cached values
    FVector LastValidLocation;
    FRotator LastValidRotation;
    TArray<FTransform> BoneTransforms;

    // Internal functions
    void InitializeComponents();
    void UpdateRagdollTransition(float DeltaTime);
    void UpdateRecovery(float DeltaTime);
    void CacheCurrentPose();
    void RestoreCachedPose();
    void SetupPhysicsProperties();
    void HandleStateTransition(ECore_RagdollState NewState);

    // Event handlers
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};

UCLASS()
class TRANSPERSONALGAME_API ACore_RagdollTestActor : public APawn
{
    GENERATED_BODY()

public:
    ACore_RagdollTestActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCapsuleComponent* CapsuleCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCore_RagdollSystem* RagdollSystem;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void TestRagdollActivation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void TestRagdollDeactivation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void TestImpactForce();
};