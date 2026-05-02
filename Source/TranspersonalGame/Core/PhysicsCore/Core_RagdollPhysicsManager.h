#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "../SharedTypes.h"
#include "Core_RagdollPhysicsManager.generated.h"

/**
 * Ragdoll state configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bIsRagdollActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FVector ImpactForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float TimeActivated = 0.0f;

    FCore_RagdollState()
    {
        bIsRagdollActive = false;
        RagdollBlendTime = 0.2f;
        RagdollLifetime = 10.0f;
        ImpactForce = FVector::ZeroVector;
        ImpactLocation = FVector::ZeroVector;
        TimeActivated = 0.0f;
    }
};

/**
 * Ragdoll physics configuration per bone
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_BonePhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Physics")
    float Restitution = 0.1f;

    FCore_BonePhysicsConfig()
    {
        BoneName = NAME_None;
        Mass = 1.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        Friction = 0.7f;
        Restitution = 0.1f;
    }
};

/**
 * Ragdoll impact data for realistic physics responses
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollImpact
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactMagnitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FName HitBoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    ECore_DamageType DamageType = ECore_DamageType::Blunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float TimeStamp = 0.0f;

    FCore_RagdollImpact()
    {
        ImpactPoint = FVector::ZeroVector;
        ImpactDirection = FVector::ZeroVector;
        ImpactMagnitude = 0.0f;
        HitBoneName = NAME_None;
        DamageType = ECore_DamageType::Blunt;
        TimeStamp = 0.0f;
    }
};

/**
 * World Subsystem that manages ragdoll physics for characters and creatures
 * Handles realistic death animations, impact responses, and physics transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollPhysicsManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_RagdollPhysicsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Ragdoll Activation
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ActivateRagdoll(USkeletalMeshComponent* SkeletalMesh, const FCore_RagdollImpact& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void DeactivateRagdoll(USkeletalMeshComponent* SkeletalMesh, float BlendTime = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const;

    // Ragdoll Configuration
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetRagdollPhysicsProperties(USkeletalMeshComponent* SkeletalMesh, const TArray<FCore_BonePhysicsConfig>& BoneConfigs);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void SetGlobalRagdollSettings(float GlobalMass, float GlobalDamping, float GlobalFriction);

    // Impact System
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyImpactToRagdoll(USkeletalMeshComponent* SkeletalMesh, const FCore_RagdollImpact& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ApplyForceToRagdollBone(USkeletalMeshComponent* SkeletalMesh, FName BoneName, const FVector& Force, bool bAcceleration = false);

    // Ragdoll State Management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    FCore_RagdollState GetRagdollState(USkeletalMeshComponent* SkeletalMesh) const;

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void UpdateRagdollState(USkeletalMeshComponent* SkeletalMesh, const FCore_RagdollState& NewState);

    // Physics Asset Management
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void OptimizePhysicsAsset(UPhysicsAsset* PhysicsAsset);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void ValidatePhysicsAsset(UPhysicsAsset* PhysicsAsset);

    // Cleanup and Performance
    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    void CleanupInactiveRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Ragdoll Physics")
    int32 GetActiveRagdollCount() const;

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugDrawRagdollPhysics();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateAllRagdolls();

protected:
    // Ragdoll tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TMap<TWeakObjectPtr<USkeletalMeshComponent>, FCore_RagdollState> ActiveRagdolls;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float GlobalMassMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float GlobalDampingMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float GlobalFrictionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float MaxRagdollLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    int32 MaxActiveRagdolls = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    bool bAutoCleanupRagdolls = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float CleanupCheckInterval = 5.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float RagdollCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnableLODBasedPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float PhysicsUpdateRate = 60.0f;

private:
    // Internal state
    FTimerHandle CleanupTimerHandle;
    FTimerHandle PhysicsUpdateHandle;

    // Internal helper functions
    void TickRagdollPhysics();
    void CleanupExpiredRagdolls();
    
    bool ValidateSkeletalMeshComponent(USkeletalMeshComponent* SkeletalMesh) const;
    void SetupRagdollPhysics(USkeletalMeshComponent* SkeletalMesh);
    void ApplyBoneConfiguration(USkeletalMeshComponent* SkeletalMesh, const FCore_BonePhysicsConfig& BoneConfig);
    
    FVector CalculateImpactResponse(const FCore_RagdollImpact& ImpactData, float BoneMass) const;
    void DistributeImpactForce(USkeletalMeshComponent* SkeletalMesh, const FCore_RagdollImpact& ImpactData);
    
    // Default bone configurations for different creature types
    void InitializeDefaultBoneConfigs();
    TMap<FString, TArray<FCore_BonePhysicsConfig>> DefaultBoneConfigs;
};