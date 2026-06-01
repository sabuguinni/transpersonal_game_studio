#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive,
    Activating,
    Active,
    Deactivating
};

USTRUCT(BlueprintType)
struct FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoDeactivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AutoDeactivateTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MinVelocityThreshold = 50.0f;

    FCore_RagdollConfig()
    {
        ActivationForce = 1000.0f;
        BlendTime = 0.5f;
        bAutoDeactivate = true;
        AutoDeactivateTime = 5.0f;
        MinVelocityThreshold = 50.0f;
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
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpactForce = FVector::ZeroVector, const FVector& ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToRagdoll(const FVector& Impulse, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetBoneLinearVelocity(const FName& BoneName, const FVector& Velocity);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    FVector GetBoneVelocity(const FName& BoneName) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FCore_RagdollConfig RagdollConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    ECore_RagdollState CurrentState;

    UPROPERTY()
    USkeletalMeshComponent* TargetMesh;

    UPROPERTY()
    UPhysicsAsset* CachedPhysicsAsset;

    float StateTimer;
    float BlendWeight;
    FTransform PreRagdollTransform;
    TArray<FTransform> PreRagdollBoneTransforms;

    void UpdateRagdollBlending(float DeltaTime);
    void CachePreRagdollState();
    void RestorePreRagdollState();
    bool ShouldAutoDeactivate() const;
    void FindTargetMesh();
};