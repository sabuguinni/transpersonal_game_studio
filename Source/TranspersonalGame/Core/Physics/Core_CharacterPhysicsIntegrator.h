#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Core_CharacterPhysicsIntegrator.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Ragdoll     UMETA(DisplayName = "Ragdoll"),
    Stunned     UMETA(DisplayName = "Stunned"),
    Falling     UMETA(DisplayName = "Falling"),
    Swimming    UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsImpact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCausesRagdoll = false;
};

/**
 * Core Character Physics Integrator
 * Integrates character movement with physics simulation, ragdoll system, and environmental interactions
 * Handles transition between normal movement and physics-driven states
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_CharacterPhysicsIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CharacterPhysicsIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CORE PHYSICS PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics State")
    ECore_PhysicsState CurrentPhysicsState = ECore_PhysicsState::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float RagdollActivationThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float RagdollRecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float PhysicsBlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsIntegration = true;

    // === RAGDOLL SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bIsRagdollActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FVector RagdollImpulse = FVector::ZeroVector;

    // === PHYSICS CONSTRAINTS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    float MaxVelocityMagnitude = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    float MaxAngularVelocity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    bool bLimitPhysicsVelocity = true;

    // === COMPONENT REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCapsuleComponent* CapsuleComp;

    UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComp;

public:
    // === PHYSICS STATE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Physics State")
    void SetPhysicsState(ECore_PhysicsState NewState);

    UFUNCTION(BlueprintPure, Category = "Physics State")
    ECore_PhysicsState GetPhysicsState() const { return CurrentPhysicsState; }

    UFUNCTION(BlueprintPure, Category = "Physics State")
    bool IsInPhysicsMode() const;

    // === RAGDOLL CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(const FVector& ImpulseDirection = FVector::ZeroVector, float ImpulseMagnitude = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return bIsRagdollActive; }

    // === IMPACT SYSTEM ===

    UFUNCTION(BlueprintCallable, Category = "Impact")
    void ProcessPhysicsImpact(const FCore_PhysicsImpact& Impact);

    UFUNCTION(BlueprintCallable, Category = "Impact")
    void ApplyImpulseToCharacter(const FVector& Impulse, const FVector& Location = FVector::ZeroVector);

    // === PHYSICS VALIDATION ===

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void LogPhysicsState();

    // === ENVIRONMENTAL INTERACTION ===

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void HandleGroundImpact(float ImpactVelocity);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void HandleWaterEntry();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void HandleWaterExit();

protected:
    // === INTERNAL PHYSICS METHODS ===

    void UpdatePhysicsState(float DeltaTime);
    void UpdateRagdollState(float DeltaTime);
    void BlendToPhysicsMode();
    void BlendToNormalMode();
    void LimitPhysicsVelocity();
    void InitializeComponentReferences();
    void ValidateComponentReferences();

    // === PHYSICS TRANSITION HELPERS ===

    void TransitionToNormal();
    void TransitionToRagdoll();
    void TransitionToStunned();
    void TransitionToFalling();
    void TransitionToSwimming();

    // === RAGDOLL HELPERS ===

    void SetupRagdollPhysics();
    void CleanupRagdollPhysics();
    void UpdateRagdollRecovery(float DeltaTime);
    bool CanRecoverFromRagdoll() const;

    // === PHYSICS CONSTRAINT HELPERS ===

    void ApplyPhysicsConstraints();
    void UpdateVelocityLimits();
    void ValidatePhysicsConstraints();

private:
    // === INTERNAL STATE ===

    float StateTransitionTimer = 0.0f;
    bool bIsTransitioning = false;
    ECore_PhysicsState PreviousPhysicsState = ECore_PhysicsState::Normal;
    
    // === PHYSICS CACHE ===

    FVector LastValidPosition = FVector::ZeroVector;
    FRotator LastValidRotation = FRotator::ZeroRotator;
    bool bHasValidPhysicsState = false;
};