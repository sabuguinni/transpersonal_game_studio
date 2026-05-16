#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Core_RagdollPhysics.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float BlendOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RecoveryDelay = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float AngularDamping = 0.1f;

    FCore_RagdollConfiguration()
    {
        BlendInTime = 0.2f;
        BlendOutTime = 0.5f;
        ImpactThreshold = 500.0f;
        RecoveryDelay = 2.0f;
        bAutoRecover = true;
        MaxRagdollTime = 10.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    bool bIsRagdollActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    float RagdollTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    float BlendAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    FVector LastImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    float LastImpactMagnitude = 0.0f;

    FCore_RagdollState()
    {
        bIsRagdollActive = false;
        RagdollTime = 0.0f;
        BlendAlpha = 0.0f;
        LastImpactLocation = FVector::ZeroVector;
        LastImpactMagnitude = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RagdollPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_RagdollPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core ragdoll functionality
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ActivateRagdoll(const FVector& ImpactLocation = FVector::ZeroVector, float ImpactMagnitude = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DeactivateRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetRagdollConfiguration(const FCore_RagdollConfiguration& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Ragdoll Physics")
    bool IsRagdollActive() const { return RagdollState.bIsRagdollActive; }

    UFUNCTION(BlueprintPure, Category = "Ragdoll Physics")
    FCore_RagdollState GetRagdollState() const { return RagdollState; }

    // Impact detection
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void OnImpactReceived(const FVector& ImpactLocation, float ImpactMagnitude, AActor* Instigator = nullptr);

    // Biome-specific ragdoll behavior
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyBiomeRagdollModifiers(EBiomeType BiomeType);

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ForceRecovery();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool CanRecover() const;

    // Physics asset management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetPhysicsAsset(UPhysicsAsset* NewPhysicsAsset);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ConfigurePhysicsBodies();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Configuration")
    FCore_RagdollConfiguration RagdollConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll State")
    FCore_RagdollState RagdollState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    UPhysicsAsset* DefaultPhysicsAsset;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Timers and state tracking
    float RecoveryTimer = 0.0f;
    bool bRecoveryPending = false;
    FVector PreRagdollLocation;
    FRotator PreRagdollRotation;

    // Internal methods
    void UpdateRagdollBlending(float DeltaTime);
    void CheckAutoRecovery(float DeltaTime);
    void BlendToRagdoll(float DeltaTime);
    void BlendFromRagdoll(float DeltaTime);
    void InitializeSkeletalMeshReference();
    void ApplyPhysicsSettings();
    void RestoreAnimationState();
    void SavePreRagdollTransform();

    // Biome-specific modifiers
    void ApplySavannaRagdollSettings();
    void ApplySwampRagdollSettings();
    void ApplyForestRagdollSettings();
    void ApplyDesertRagdollSettings();
    void ApplyMountainRagdollSettings();

    // Event callbacks
    UFUNCTION()
    void OnSkeletalMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};