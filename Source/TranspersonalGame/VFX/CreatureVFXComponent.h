// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "VFXManager.h"
#include "Engine/Engine.h"
#include "CreatureVFXComponent.generated.h"

class UNiagaraComponent;
class USkeletalMeshComponent;

/**
 * Creature VFX States
 * Different states that trigger different visual effects
 */
UENUM(BlueprintType)
enum class ECreatureVFXState : uint8
{
    Idle        UMETA(DisplayName = "Idle - Breathing, subtle ambient"),
    Moving      UMETA(DisplayName = "Moving - Footsteps, dust, movement"),
    Aggressive  UMETA(DisplayName = "Aggressive - Threat display, intimidation"),
    Feeding     UMETA(DisplayName = "Feeding - Eating, foraging effects"),
    Injured     UMETA(DisplayName = "Injured - Blood, pain indicators"),
    Death       UMETA(DisplayName = "Death - Death effects, body dissolution"),
    Sleeping    UMETA(DisplayName = "Sleeping - Peaceful, slow breathing"),
    Mating      UMETA(DisplayName = "Mating - Courtship display effects"),
    Territorial UMETA(DisplayName = "Territorial - Marking, dominance display")
};

/**
 * Creature Size Categories for VFX scaling
 */
UENUM(BlueprintType)
enum class ECreatureSizeCategory : uint8
{
    Tiny        UMETA(DisplayName = "Tiny - Small birds, insects"),
    Small       UMETA(DisplayName = "Small - Compsognathus, small mammals"),
    Medium      UMETA(DisplayName = "Medium - Human-sized creatures"),
    Large       UMETA(DisplayName = "Large - Triceratops, large predators"),
    Massive     UMETA(DisplayName = "Massive - T-Rex, Brontosaurus"),
    Colossal    UMETA(DisplayName = "Colossal - Largest dinosaurs")
};

/**
 * Creature VFX Configuration
 * Defines VFX behavior for a specific creature type
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCreatureVFXConfig
{
    GENERATED_BODY()

    // Creature identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CreatureTypeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString DisplayName;

    // Size category for automatic scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    ECreatureSizeCategory SizeCategory = ECreatureSizeCategory::Medium;

    // Base scale multiplier for all effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
    float BaseScaleMultiplier = 1.0f;

    // VFX effects for different states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Effects")
    TMap<ECreatureVFXState, FString> StateEffects;

    // Socket-based effects (breathing from nostrils, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Effects")
    TMap<FName, FString> SocketEffects;

    // Footstep effects for different surface types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    TMap<FString, FString> FootstepEffects; // Surface type -> Effect ID

    // Blood and injury effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FString BloodEffectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FString DeathEffectID;

    // Environmental interaction effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FString WaterSplashEffectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FString DustCloudEffectID;

    // Special ability effects (roar, breath attacks, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
    TMap<FString, FString> AbilityEffects; // Ability name -> Effect ID

    FCreatureVFXConfig()
    {
        CreatureTypeID = TEXT("");
        DisplayName = TEXT("Unknown Creature");
        SizeCategory = ECreatureSizeCategory::Medium;
        BaseScaleMultiplier = 1.0f;
        BloodEffectID = TEXT("Blood_Generic");
        DeathEffectID = TEXT("Death_Generic");
        WaterSplashEffectID = TEXT("WaterSplash_Generic");
        DustCloudEffectID = TEXT("DustCloud_Generic");
    }
};

/**
 * Active Creature VFX Instance
 * Tracks an active VFX effect on a creature
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FActiveCreatureVFX
{
    GENERATED_BODY()

    // VFX Manager instance ID
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    int32 VFXInstanceID = -1;

    // Effect ID being played
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    FString EffectID;

    // State this effect is associated with
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    ECreatureVFXState AssociatedState = ECreatureVFXState::Idle;

    // Socket this effect is attached to (if any)
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    FName AttachedSocket = NAME_None;

    // Is this effect looping?
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    bool bIsLooping = false;

    // Time when this effect was started
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    float StartTime = 0.0f;

    FActiveCreatureVFX()
    {
        VFXInstanceID = -1;
        EffectID = TEXT("");
        AssociatedState = ECreatureVFXState::Idle;
        AttachedSocket = NAME_None;
        bIsLooping = false;
        StartTime = 0.0f;
    }
};

/**
 * Creature VFX Component
 * Manages visual effects for creatures based on their state and actions
 * Integrates with the VFX Manager for optimized performance
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCreatureVFXComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCreatureVFXComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void SetCreatureVFXConfig(const FCreatureVFXConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    FCreatureVFXConfig GetCreatureVFXConfig() const;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void SetCreatureState(ECreatureVFXState NewState);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    ECreatureVFXState GetCurrentCreatureState() const;

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void TransitionToState(ECreatureVFXState NewState, float TransitionDuration = 0.5f);

    // Effect Triggering
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    int32 PlayStateEffect(ECreatureVFXState State, bool bOverrideExisting = false);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    int32 PlaySocketEffect(FName SocketName, const FString& EffectID = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    int32 PlayFootstepEffect(const FString& SurfaceType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    int32 PlayAbilityEffect(const FString& AbilityName);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    int32 PlayBloodEffect(const FVector& HitLocation, const FVector& HitNormal);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void PlayDeathEffect();

    // Effect Control
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void StopStateEffect(ECreatureVFXState State);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void StopSocketEffect(FName SocketName);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void StopAllEffects();

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    bool IsStateEffectActive(ECreatureVFXState State) const;

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    bool IsSocketEffectActive(FName SocketName) const;

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    TArray<int32> GetActiveVFXInstances() const;

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void OnEnterWater();

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void OnExitWater();

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void OnMovementSpeedChanged(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Creature VFX")
    void OnHealthChanged(float CurrentHealth, float MaxHealth);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreatureVFXStateChanged, ECreatureVFXState, OldState, ECreatureVFXState, NewState);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreatureVFXTriggered, const FString&, EffectID, ECreatureVFXState, State, int32, InstanceID);

    UPROPERTY(BlueprintAssignable, Category = "Creature VFX Events")
    FOnCreatureVFXStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Creature VFX Events")
    FOnCreatureVFXTriggered OnVFXTriggered;

protected:
    // Internal Methods
    void InitializeVFXSystem();
    void CleanupActiveEffects();
    void UpdateStateEffects();
    void UpdateSocketEffects();
    
    FVector GetSocketWorldLocation(FName SocketName) const;
    FRotator GetSocketWorldRotation(FName SocketName) const;
    FVector CalculateEffectScale() const;
    
    USkeletalMeshComponent* GetOwnerSkeletalMesh() const;
    UVFXManager* GetVFXManager() const;

    // Internal effect management
    int32 SpawnVFXAtLocation(const FString& EffectID, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    int32 SpawnVFXAtSocket(const FString& EffectID, FName SocketName);
    void RegisterActiveEffect(int32 VFXInstanceID, const FString& EffectID, ECreatureVFXState State, FName Socket = NAME_None);
    void UnregisterActiveEffect(int32 VFXInstanceID);

private:
    // Configuration
    UPROPERTY(EditAnywhere, Category = "Configuration")
    FCreatureVFXConfig VFXConfig;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECreatureVFXState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ECreatureVFXState PreviousState;

    // Active effects tracking
    UPROPERTY()
    TArray<FActiveCreatureVFX> ActiveEffects;

    // State effect instances (for looping effects)
    UPROPERTY()
    TMap<ECreatureVFXState, int32> StateEffectInstances;

    // Socket effect instances
    UPROPERTY()
    TMap<FName, int32> SocketEffectInstances;

    // Transition state
    bool bInTransition;
    float TransitionStartTime;
    float TransitionDuration;
    ECreatureVFXState TransitionTargetState;

    // Environmental state
    bool bInWater;
    float LastMovementSpeed;
    float LastHealthPercentage;

    // Performance tracking
    float LastEffectCleanupTime;
    static const float EFFECT_CLEANUP_INTERVAL;
};