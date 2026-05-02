#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Core_RagdollSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    None        UMETA(DisplayName = "None"),
    Activating  UMETA(DisplayName = "Activating"),
    Active      UMETA(DisplayName = "Active"),
    Recovering  UMETA(DisplayName = "Recovering"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FCore_RagdollConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float ActivationForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bPreserveVelocity = true;

    FCore_RagdollConfig()
    {
        ActivationForce = 500.0f;
        RecoveryTime = 3.0f;
        BlendInTime = 0.2f;
        BlendOutTime = 1.0f;
        bAutoRecover = true;
        bPreserveVelocity = true;
    }
};

/**
 * Core Ragdoll System Component
 * Handles realistic character physics simulation for death, unconsciousness, and impact reactions
 * Critical for survival gameplay - characters must react realistically to dinosaur attacks
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
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
    void ActivateRagdoll(const FVector& ImpactForce = FVector::ZeroVector, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollConfig(const FCore_RagdollConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpactForce(const FVector& Force, const FVector& Location, const FName& BoneName = NAME_None);

    // Survival-specific ragdoll triggers
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TriggerDeathRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TriggerUnconsciousnessRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TriggerImpactRagdoll(const FVector& ImpactDirection, float ImpactMagnitude);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Config")
    FCore_RagdollConfig RagdollConfig;

    // State tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    ECore_RagdollState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll State")
    float StateTimer;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAnimInstance> AnimInstance;

    // Physics data preservation
    UPROPERTY()
    FTransform PreRagdollTransform;

    UPROPERTY()
    FVector PreRagdollVelocity;

private:
    void UpdateRagdollState(float DeltaTime);
    void InitializeComponents();
    void BlendToRagdoll();
    void BlendFromRagdoll();
    void ApplyPhysicsConfiguration();
    void RestoreAnimationControl();

    // Survival game specific physics tuning
    void ConfigureForSurvivalGameplay();
    void SetPhysicsConstraints();
    void ApplySurvivalPhysicsSettings();

    bool bIsInitialized;
    float BlendAlpha;
    FTimerHandle RecoveryTimerHandle;
};