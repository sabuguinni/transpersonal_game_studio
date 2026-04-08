#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/CollisionProfile.h"
#include "RagdollSystem.generated.h"

/**
 * Ragdoll System Component
 * Handles realistic character physics for death and unconsciousness states
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URagdollSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URagdollSystemComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /** Activate ragdoll physics */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpulseLocation = FVector::ZeroVector, float ImpulseStrength = 0.0f);

    /** Deactivate ragdoll and return to normal physics */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    /** Check if ragdoll is currently active */
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return bRagdollActive; }

    /** Configure ragdoll parameters */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ConfigureRagdoll(float Mass, float LinearDamping, float AngularDamping);

protected:
    /** Is ragdoll currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    bool bRagdollActive = false;

    /** Ragdoll mass multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollMass = 75.0f;

    /** Linear damping for ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    /** Angular damping for ragdoll */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    /** Auto-deactivate ragdoll after time */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float AutoDeactivateTime = 10.0f;

private:
    void RegisterWithPhysicsSystem();
    void UnregisterFromPhysicsSystem();
    void AutoDeactivateRagdoll();

    FTimerHandle AutoDeactivateTimer;
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
    
    // Store original collision settings for restoration
    TEnumAsByte<ECollisionEnabled::Type> OriginalCollisionEnabled;
    TEnumAsByte<ECollisionChannel> OriginalCollisionObjectType;
};