// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnhancedRagdollSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnhancedRagdoll, Log, All);

/**
 * Ragdoll activation modes
 */
UENUM(BlueprintType)
enum class ERagdollActivationMode : uint8
{
    Immediate = 0,      // Activate ragdoll immediately
    Blended = 1,        // Blend from animation to ragdoll
    Partial = 2,        // Only activate specific body parts
    Timed = 3          // Activate after a delay
};

/**
 * Ragdoll body regions for partial activation
 */
UENUM(BlueprintType)
enum class ERagdollBodyRegion : uint8
{
    FullBody = 0,       // Entire body
    UpperBody = 1,      // Torso, arms, head
    LowerBody = 2,      // Pelvis, legs
    LeftArm = 3,        // Left arm only
    RightArm = 4,       // Right arm only
    LeftLeg = 5,        // Left leg only
    RightLeg = 6,       // Right leg only
    Head = 7,           // Head only
    Spine = 8          // Spine only
};

/**
 * Ragdoll recovery modes
 */
UENUM(BlueprintType)
enum class ERagdollRecoveryMode : uint8
{
    Instant = 0,        // Instant recovery to standing
    Animated = 1,       // Play get-up animation
    Blended = 2,        // Blend from ragdoll to animation
    Manual = 3         // Manual recovery control
};

/**
 * Ragdoll event data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRagdollEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ACharacter* Character = nullptr;

    UPROPERTY(BlueprintReadOnly)
    ERagdollActivationMode ActivationMode = ERagdollActivationMode::Immediate;

    UPROPERTY(BlueprintReadOnly)
    ERagdollBodyRegion BodyRegion = ERagdollBodyRegion::FullBody;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float ImpactForce = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsRecovering = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollEvent, const FRagdollEventData&, EventData);

/**
 * Ragdoll configuration settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRagdollConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERagdollActivationMode DefaultActivationMode = ERagdollActivationMode::Blended;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ERagdollRecoveryMode DefaultRecoveryMode = ERagdollRecoveryMode::Animated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinActivationForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxRagdollTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoRecover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoRecoveryDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePartialRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePhysicsBlending = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PhysicsBlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableCollisionDuringRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableGroundAdaptation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroundAdaptationStrength = 0.5f;
};

/**
 * Enhanced Ragdoll System - Advanced ragdoll physics with blending and partial activation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnhancedRagdollSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnhancedRagdollSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core ragdoll management
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void InitializeRagdollSystem();

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ShutdownRagdollSystem();

    // Character registration
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void RegisterCharacter(ACharacter* Character, const FRagdollConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void UnregisterCharacter(ACharacter* Character);

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    bool IsCharacterRegistered(ACharacter* Character) const;

    // Ragdoll activation
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ActivateRagdoll(ACharacter* Character, ERagdollActivationMode Mode = ERagdollActivationMode::Blended, ERagdollBodyRegion Region = ERagdollBodyRegion::FullBody);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ActivateRagdollWithImpact(ACharacter* Character, const FVector& ImpactLocation, const FVector& ImpactDirection, float ImpactForce, ERagdollActivationMode Mode = ERagdollActivationMode::Blended);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void DeactivateRagdoll(ACharacter* Character, ERagdollRecoveryMode RecoveryMode = ERagdollRecoveryMode::Animated);

    // Ragdoll state queries
    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    bool IsRagdollActive(ACharacter* Character) const;

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    bool IsRagdollBlending(ACharacter* Character) const;

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    float GetRagdollBlendWeight(ACharacter* Character) const;

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    ERagdollBodyRegion GetActiveRagdollRegion(ACharacter* Character) const;

    // Partial ragdoll control
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ActivatePartialRagdoll(ACharacter* Character, ERagdollBodyRegion Region, float BlendWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void SetPartialRagdollWeight(ACharacter* Character, ERagdollBodyRegion Region, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void BlendPartialRagdoll(ACharacter* Character, ERagdollBodyRegion FromRegion, ERagdollBodyRegion ToRegion, float BlendTime);

    // Physics manipulation
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ApplyForceToRagdoll(ACharacter* Character, const FVector& Force, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ApplyImpulseToRagdoll(ACharacter* Character, const FVector& Impulse, const FName& BoneName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void SetRagdollPhysicsProperties(ACharacter* Character, float LinearDamping, float AngularDamping, float Mass = -1.0f);

    // Ground adaptation
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void EnableGroundAdaptation(ACharacter* Character, bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void SetGroundAdaptationStrength(ACharacter* Character, float Strength);

    // Recovery system
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void StartRagdollRecovery(ACharacter* Character, ERagdollRecoveryMode Mode = ERagdollRecoveryMode::Animated);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void SetRecoveryAnimation(ACharacter* Character, UAnimMontage* RecoveryMontage);

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    bool IsRecovering(ACharacter* Character) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void SetRagdollConfiguration(ACharacter* Character, const FRagdollConfiguration& Config);

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll")
    FRagdollConfiguration GetRagdollConfiguration(ACharacter* Character) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Enhanced Ragdoll")
    FOnRagdollEvent OnRagdollActivated;

    UPROPERTY(BlueprintAssignable, Category = "Enhanced Ragdoll")
    FOnRagdollEvent OnRagdollDeactivated;

    UPROPERTY(BlueprintAssignable, Category = "Enhanced Ragdoll")
    FOnRagdollEvent OnRagdollRecoveryStarted;

    UPROPERTY(BlueprintAssignable, Category = "Enhanced Ragdoll")
    FOnRagdollEvent OnRagdollRecoveryCompleted;

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void EnableRagdollDebugVisualization(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    void ValidateRagdollConfiguration(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll")
    bool RunRagdollTest(ACharacter* Character);

protected:
    // Default configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll Settings")
    FRagdollConfiguration DefaultConfiguration;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    int32 MaxActiveRagdolls = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float UpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float MaxRagdollDistance = 2000.0f;

    // Runtime data
    UPROPERTY()
    TMap<TWeakObjectPtr<ACharacter>, FRagdollConfiguration> RegisteredCharacters;

private:
    // Internal ragdoll state tracking
    struct FRagdollState
    {
        bool bIsActive = false;
        bool bIsBlending = false;
        bool bIsRecovering = false;
        ERagdollActivationMode ActivationMode = ERagdollActivationMode::Immediate;
        ERagdollBodyRegion ActiveRegion = ERagdollBodyRegion::FullBody;
        float BlendWeight = 0.0f;
        float BlendTimer = 0.0f;
        float ActivationTime = 0.0f;
        TWeakObjectPtr<UAnimMontage> RecoveryMontage = nullptr;
        FTimerHandle RecoveryTimer;
        FTimerHandle AutoRecoveryTimer;
    };

    TMap<TWeakObjectPtr<ACharacter>, FRagdollState> RagdollStates;

    // Internal ragdoll logic
    void ExecuteRagdollActivation(ACharacter* Character, const FRagdollConfiguration& Config, ERagdollActivationMode Mode, ERagdollBodyRegion Region);
    void ExecuteRagdollDeactivation(ACharacter* Character, const FRagdollConfiguration& Config, ERagdollRecoveryMode RecoveryMode);
    void UpdateRagdollBlending(ACharacter* Character, float DeltaTime);
    void UpdateGroundAdaptation(ACharacter* Character, const FRagdollConfiguration& Config);
    
    // Physics asset manipulation
    void ConfigurePhysicsAssetForRagdoll(USkeletalMeshComponent* MeshComponent, ERagdollBodyRegion Region, bool bActivate);
    void SetBodyRegionPhysics(USkeletalMeshComponent* MeshComponent, ERagdollBodyRegion Region, bool bSimulate);
    TArray<FName> GetBoneNamesForRegion(USkeletalMeshComponent* MeshComponent, ERagdollBodyRegion Region);
    
    // Blending system
    void StartRagdollBlend(ACharacter* Character, bool bBlendIn, float BlendTime);
    void UpdateRagdollBlendWeight(ACharacter* Character, float DeltaTime);
    void ApplyRagdollBlendWeight(ACharacter* Character, float BlendWeight);
    
    // Recovery system
    void StartRecoveryProcess(ACharacter* Character, ERagdollRecoveryMode Mode);
    void CompleteRecoveryProcess(ACharacter* Character);
    void PlayRecoveryAnimation(ACharacter* Character, UAnimMontage* RecoveryMontage);
    
    // Auto recovery
    void SetupAutoRecovery(ACharacter* Character, float Delay);
    void TriggerAutoRecovery(ACharacter* Character);
    
    // Performance optimization
    void OptimizeRagdollPerformance();
    void CullDistantRagdolls();
    void CleanupInvalidReferences();
    
    // Event handling
    void BroadcastRagdollEvent(const FRagdollEventData& EventData, FOnRagdollEvent& EventDelegate);
    void HandleCharacterDestroyed(ACharacter* Character);
    
    // Timers and updates
    FTimerHandle SystemUpdateTimer;
    FTimerHandle CleanupTimer;
    FTimerHandle PerformanceOptimizationTimer;
    
    void SystemUpdate();
    
    // Debug visualization
    bool bDebugVisualizationEnabled = false;
    void DrawRagdollDebugInfo();
    
    // System state
    bool bSystemInitialized = false;
};

/**
 * Enhanced Ragdoll Component - Attach to characters for advanced ragdoll functionality
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Ragdoll), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnhancedRagdollComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnhancedRagdollComponent();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Ragdoll control
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll Component")
    void ActivateRagdoll(ERagdollActivationMode Mode = ERagdollActivationMode::Blended, ERagdollBodyRegion Region = ERagdollBodyRegion::FullBody);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll Component")
    void DeactivateRagdoll(ERagdollRecoveryMode RecoveryMode = ERagdollRecoveryMode::Animated);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll Component")
    void ActivateRagdollWithImpact(const FVector& ImpactLocation, const FVector& ImpactDirection, float ImpactForce);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Enhanced Ragdoll Component")
    void SetConfiguration(const FRagdollConfiguration& Config);

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll Component")
    FRagdollConfiguration GetConfiguration() const { return Configuration; }

    // State queries
    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll Component")
    bool IsRagdollActive() const;

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll Component")
    bool IsRecovering() const;

    UFUNCTION(BlueprintPure, Category = "Enhanced Ragdoll Component")
    float GetBlendWeight() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Enhanced Ragdoll Component")
    FOnRagdollEvent OnComponentRagdollEvent;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll Settings")
    FRagdollConfiguration Configuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll Settings")
    bool bAutoRegisterWithSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recovery Settings")
    UAnimMontage* DefaultRecoveryMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recovery Settings")
    TMap<ERagdollBodyRegion, UAnimMontage*> RegionSpecificRecoveryMontages;

private:
    UPROPERTY()
    UEnhancedRagdollSystem* RagdollSystem = nullptr;

    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    void RegisterWithRagdollSystem();
    void UnregisterFromRagdollSystem();
    void InitializeComponent();

    // Event handling
    UFUNCTION()
    void OnOwnerTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnOwnerHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
};