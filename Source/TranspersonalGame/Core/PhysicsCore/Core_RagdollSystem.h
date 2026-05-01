#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "../SharedTypes.h"
#include "Core_RagdollSystem.generated.h"

// Ragdoll activation reasons for different gameplay scenarios
UENUM(BlueprintType)
enum class ECore_RagdollTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    Death           UMETA(DisplayName = "Death"),
    Unconscious     UMETA(DisplayName = "Unconscious"),
    Impact          UMETA(DisplayName = "Impact"),
    Explosion       UMETA(DisplayName = "Explosion"),
    DinosaurAttack  UMETA(DisplayName = "Dinosaur Attack"),
    Fall            UMETA(DisplayName = "Fall Damage"),
    Manual          UMETA(DisplayName = "Manual Trigger")
};

// Ragdoll state for tracking current status
UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Activating      UMETA(DisplayName = "Activating"),
    Active          UMETA(DisplayName = "Active"),
    Recovering      UMETA(DisplayName = "Recovering"),
    GetUp           UMETA(DisplayName = "Getting Up")
};

// Configuration for ragdoll physics properties
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollConfig
{
    GENERATED_BODY()

    // Physics properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float MassScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float MaxAngularVelocity = 1000.0f;

    // Recovery settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Recovery")
    float RecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Recovery")
    float GetUpTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Recovery")
    bool bCanRecoverFromRagdoll = true;

    // Activation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Activation")
    float ImpactThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Activation")
    float VelocityThreshold = 1000.0f;

    FCore_RagdollConfig()
    {
        // Default values already set above
    }
};

// Ragdoll performance metrics for monitoring
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    int32 ActiveRagdolls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    float AverageRecoveryTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    int32 RagdollsThisFrame = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll Metrics")
    float PhysicsUpdateTime = 0.0f;
};

/**
 * Core Ragdoll System
 * Manages realistic ragdoll physics for characters and dinosaurs
 * Handles activation, recovery, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_RagdollSystem : public UObject
{
    GENERATED_BODY()

public:
    UCore_RagdollSystem();

    // System initialization
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void InitializeRagdollSystem();

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void ShutdownRagdollSystem();

    // Ragdoll activation and deactivation
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    bool ActivateRagdoll(ACharacter* Character, ECore_RagdollTrigger Trigger, const FVector& ImpactPoint = FVector::ZeroVector, float ImpactForce = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    bool DeactivateRagdoll(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    bool IsRagdollActive(ACharacter* Character) const;

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    ECore_RagdollState GetRagdollState(ACharacter* Character) const;

    // Ragdoll configuration
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void SetRagdollConfig(ACharacter* Character, const FCore_RagdollConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    FCore_RagdollConfig GetRagdollConfig(ACharacter* Character) const;

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void StartRagdollRecovery(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    bool CanCharacterRecover(ACharacter* Character) const;

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void ForceGetUp(ACharacter* Character);

    // Impact and force application
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void ApplyImpactToRagdoll(ACharacter* Character, const FVector& ImpactPoint, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void ApplyExplosionToRagdoll(ACharacter* Character, const FVector& ExplosionCenter, float ExplosionForce, float ExplosionRadius);

    // Physics properties management
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void SetRagdollPhysicsProperties(ACharacter* Character, float MassScale, float LinearDamping, float AngularDamping);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void SetBonePhysicsProperties(ACharacter* Character, const FName& BoneName, float Mass, float LinearDamping, float AngularDamping);

    // Constraint management
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void ModifyJointConstraints(ACharacter* Character, float ConstraintScale);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void SetJointDriveStrength(ACharacter* Character, const FName& BoneName, float DriveStrength);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void OptimizeRagdollPerformance();

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void SetRagdollLOD(ACharacter* Character, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void CullDistantRagdolls(const FVector& ViewerLocation, float CullDistance);

    // Metrics and monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    FCore_RagdollMetrics GetRagdollMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll")
    void UpdateMetrics();

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll", CallInEditor)
    void DebugDrawRagdollConstraints(ACharacter* Character, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Core Ragdoll", CallInEditor)
    void LogRagdollStatus(ACharacter* Character);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRagdollActivated, ACharacter*, Character, ECore_RagdollTrigger, Trigger);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollDeactivated, ACharacter*, Character);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollRecoveryStarted, ACharacter*, Character);

    UPROPERTY(BlueprintAssignable, Category = "Core Ragdoll Events")
    FOnRagdollActivated OnRagdollActivated;

    UPROPERTY(BlueprintAssignable, Category = "Core Ragdoll Events")
    FOnRagdollDeactivated OnRagdollDeactivated;

    UPROPERTY(BlueprintAssignable, Category = "Core Ragdoll Events")
    FOnRagdollRecoveryStarted OnRagdollRecoveryStarted;

protected:
    // Internal ragdoll data tracking
    USTRUCT()
    struct FRagdollData
    {
        GENERATED_BODY()

        ECore_RagdollState State = ECore_RagdollState::Inactive;
        ECore_RagdollTrigger Trigger = ECore_RagdollTrigger::None;
        FCore_RagdollConfig Config;
        float ActivationTime = 0.0f;
        float RecoveryStartTime = 0.0f;
        FVector LastKnownPosition = FVector::ZeroVector;
        FRotator LastKnownRotation = FRotator::ZeroRotator;
        bool bWasSimulatingPhysics = false;
    };

    // Ragdoll tracking
    UPROPERTY()
    TMap<TWeakObjectPtr<ACharacter>, FRagdollData> ActiveRagdolls;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_RagdollMetrics CurrentMetrics;

    // Default configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Settings")
    FCore_RagdollConfig DefaultRagdollConfig;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RagdollCullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval = 1.0f;

private:
    // Internal helper functions
    void SetupRagdollPhysics(ACharacter* Character, const FCore_RagdollConfig& Config);
    void CleanupRagdollPhysics(ACharacter* Character);
    void UpdateRagdollState(ACharacter* Character, FRagdollData& RagdollData);
    bool ValidateCharacterForRagdoll(ACharacter* Character) const;
    void LimitActiveRagdolls();
    void ProcessRecoveryLogic(ACharacter* Character, FRagdollData& RagdollData);
    
    // System state
    bool bIsSystemInitialized = false;
    float LastMetricsUpdate = 0.0f;
};