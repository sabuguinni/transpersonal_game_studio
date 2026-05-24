#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimInstance.h"
#include "Core_RagdollComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Transitioning   UMETA(DisplayName = "Transitioning"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BlendOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxLinearVelocity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float MaxAngularVelocity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    float RecoveryThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
    bool bAutoRecover = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChanged, ECore_RagdollState, NewState);

/**
 * Advanced ragdoll physics component with smooth transitions and recovery
 * Handles character death, knockdowns, and physics-based animations
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ragdoll control
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ActivateRagdoll(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DeactivateRagdoll(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void SetRagdollSettings(const FCore_RagdollSettings& NewSettings);

    // State queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    ECore_RagdollState GetRagdollState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return CurrentState == ECore_RagdollState::Active; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ragdoll")
    float GetRagdollBlendWeight() const { return CurrentBlendWeight; }

    // Physics manipulation
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddImpulseAtBone(const FString& BoneName, const FVector& Impulse);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddForceAtBone(const FString& BoneName, const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetBoneLinearVelocity(const FString& BoneName, const FVector& Velocity);

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Recovery")
    void StartRecovery();

    UFUNCTION(BlueprintCallable, Category = "Recovery")
    bool CanRecover() const;

    UFUNCTION(BlueprintCallable, Category = "Recovery")
    FVector GetRecoveryPosition() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRagdollStateChanged OnRagdollStateChanged;

    // Editor tools
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void TestRagdollActivation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FCore_RagdollSettings RagdollSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TSoftObjectPtr<USkeletalMeshComponent> TargetMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECore_RagdollState CurrentState = ECore_RagdollState::Inactive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float StateTimer = 0.0f;

private:
    void UpdateRagdollTransition(float DeltaTime);
    void UpdateRecoveryCheck();
    void ApplyRagdollSettings();
    void SetRagdollState(ECore_RagdollState NewState);
    USkeletalMeshComponent* GetTargetMeshComponent() const;

    FVector LastKnownPosition;
    FRotator LastKnownRotation;
    bool bWasGrounded = true;
};