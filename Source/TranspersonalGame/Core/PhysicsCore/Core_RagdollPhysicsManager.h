#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Core_RagdollPhysicsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    bool bIsRagdollActive;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    float RagdollStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    FVector LastKnownVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    float ImpactForce;

    FCore_RagdollState()
    {
        bIsRagdollActive = false;
        RagdollStartTime = 0.0f;
        LastKnownVelocity = FVector::ZeroVector;
        ImpactForce = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float MinActivationForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float MaxRagdollDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    float RecoveryBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    bool bAutoRecover;

    FCore_RagdollSettings()
    {
        MinActivationForce = 500.0f;
        MaxRagdollDuration = 10.0f;
        RecoveryBlendTime = 2.0f;
        bAutoRecover = true;
    }
};

/**
 * Manages ragdoll physics for characters and creatures in the prehistoric world.
 * Handles death animations, impact responses, and physics-based character reactions.
 * Essential for realistic dinosaur and character interactions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollPhysicsManager : public UObject
{
    GENERATED_BODY()

public:
    UCore_RagdollPhysicsManager();

    // Core ragdoll functionality
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ActivateRagdoll(USkeletalMeshComponent* SkeletalMesh, FVector ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DeactivateRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const;

    // Impact and force application
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpactForce(USkeletalMeshComponent* SkeletalMesh, FVector Force, FVector Location, FName BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyRadialForce(USkeletalMeshComponent* SkeletalMesh, FVector Origin, float Radius, float Strength);

    // Ragdoll state management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    FCore_RagdollState GetRagdollState(USkeletalMeshComponent* SkeletalMesh) const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void UpdateRagdollSettings(const FCore_RagdollSettings& NewSettings);

    // Recovery and blending
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void StartRagdollRecovery(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool CanRecoverFromRagdoll(USkeletalMeshComponent* SkeletalMesh) const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetRagdollPhysicsBlend(USkeletalMeshComponent* SkeletalMesh, float BlendWeight);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    FVector GetRagdollCenterOfMass(USkeletalMeshComponent* SkeletalMesh) const;

    // Tick function for auto-recovery
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void TickRagdollManager(float DeltaTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FCore_RagdollSettings RagdollSettings;

    // Internal state tracking
    UPROPERTY()
    TMap<USkeletalMeshComponent*, FCore_RagdollState> ActiveRagdolls;

    // Helper functions
    void InitializeRagdollPhysics(USkeletalMeshComponent* SkeletalMesh);
    void CleanupRagdollPhysics(USkeletalMeshComponent* SkeletalMesh);
    bool ValidateSkeletalMesh(USkeletalMeshComponent* SkeletalMesh) const;
    void UpdateRagdollState(USkeletalMeshComponent* SkeletalMesh, float DeltaTime);
};