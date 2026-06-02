#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Core_RagdollController.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    None        UMETA(DisplayName = "None"),
    Activating  UMETA(DisplayName = "Activating"),
    Active      UMETA(DisplayName = "Active"),
    Recovering  UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float DamageThreshold = 50.0f;

    FCore_RagdollSettings()
    {
        ActivationForce = 1000.0f;
        RecoveryTime = 3.0f;
        bAutoRecover = true;
        DamageThreshold = 50.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll Control Functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpactForce = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void RecoverFromRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive() const;

    // Damage Integration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void HandleDamageImpact(float Damage, const FVector& ImpactLocation, const FVector& ImpactForce);

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    TArray<FName> CriticalBones;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ECore_RagdollState CurrentState;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    UAnimInstance* OriginalAnimInstance;

    float RecoveryTimer;
    FVector LastImpactForce;
    FVector OriginalLocation;
    FRotator OriginalRotation;

    // Internal Functions
    void InitializeRagdollSystem();
    void UpdateRecoveryProcess(float DeltaTime);
    void ApplyImpactToRagdoll(const FVector& Force, const FVector& Location);
    void RestoreCharacterState();
    bool ShouldActivateRagdoll(float Damage, const FVector& Force) const;

    // Bone Management
    void SetBonePhysicsRecursive(FName BoneName, bool bEnabled);
    void ConfigureRagdollPhysics();
    void RestoreAnimationState();

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollActivated, const FVector&, ImpactForce);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollDeactivated);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollRecovered);

    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollActivated OnRagdollActivated;

    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollDeactivated OnRagdollDeactivated;

    UPROPERTY(BlueprintAssignable, Category = "Ragdoll Events")
    FOnRagdollRecovered OnRagdollRecovered;
};