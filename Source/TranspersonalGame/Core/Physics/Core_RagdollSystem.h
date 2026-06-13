#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Ragdoll")
    bool bIsRagdollActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "Ragdoll")
    float RagdollDuration = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Ragdoll")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Ragdoll")
    FVector ImpactForce = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Ragdoll")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryBlendTime = 2.0f;

    FCore_RagdollState()
    {
        bIsRagdollActive = false;
        RagdollDuration = 0.0f;
        MaxRagdollTime = 10.0f;
        ImpactForce = FVector::ZeroVector;
        ImpactLocation = FVector::ZeroVector;
        RecoveryBlendTime = 2.0f;
    }
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

    // Ragdoll activation/deactivation
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpactForce, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ForceRecovery();

    // State queries
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return RagdollState.bIsRagdollActive; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    float GetRagdollDuration() const { return RagdollState.RagdollDuration; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    FCore_RagdollState GetRagdollState() const { return RagdollState; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetMaxRagdollTime(float NewMaxTime) { RagdollState.MaxRagdollTime = NewMaxTime; }

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRecoveryBlendTime(float NewBlendTime) { RagdollState.RecoveryBlendTime = NewBlendTime; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    FCore_RagdollState RagdollState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MinImpactForceForRagdoll = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollPhysicsBlend = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecovery = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    TArray<FName> CriticalBones;

private:
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    void InitializeRagdollSystem();
    void UpdateRagdollPhysics(float DeltaTime);
    void BlendToRecovery();
    bool ShouldAutoRecover() const;
    void ApplyImpactForce();
};