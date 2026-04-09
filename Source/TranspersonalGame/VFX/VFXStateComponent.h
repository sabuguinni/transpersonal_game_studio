// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTags.h"
#include "VFXSubsystem.h"
#include "VFXStateComponent.generated.h"

class UVFXSubsystem;
class UNiagaraComponent;
class UDecalComponent;

USTRUCT(BlueprintType)
struct FVFXStateBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    FGameplayTag TriggerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    FVFXEffectData EffectData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    bool bAutoTrigger = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    float TriggerDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    float CooldownTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    int32 MaxTriggers = -1; // -1 = unlimited

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    bool bRequireLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Binding")
    float MaxTriggerDistance = 5000.0f;

    FVFXStateBinding()
    {
        bAutoTrigger = true;
        TriggerDelay = 0.0f;
        CooldownTime = 0.0f;
        MaxTriggers = -1;
        bRequireLineOfSight = false;
        MaxTriggerDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FVFXHealthBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health VFX")
    float HealthThreshold = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health VFX")
    FVFXEffectData LowHealthEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health VFX")
    FVFXEffectData CriticalHealthEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health VFX")
    FVFXEffectData DeathEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health VFX")
    bool bScaleWithHealthLoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health VFX")
    bool bPersistentLowHealthEffect = true;

    FVFXHealthBinding()
    {
        HealthThreshold = 0.5f;
        bScaleWithHealthLoss = true;
        bPersistentLowHealthEffect = true;
    }
};

USTRUCT(BlueprintType)
struct FVFXMovementBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    float MinVelocityThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    float MaxVelocityThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    FVFXEffectData WalkingEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    FVFXEffectData RunningEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    FVFXEffectData SprintingEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    FVFXEffectData JumpingEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    FVFXEffectData LandingEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    bool bScaleWithVelocity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    bool bAdaptToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement VFX")
    TMap<FString, FVFXEffectData> SurfaceSpecificEffects;

    FVFXMovementBinding()
    {
        MinVelocityThreshold = 100.0f;
        MaxVelocityThreshold = 1000.0f;
        bScaleWithVelocity = true;
        bAdaptToSurface = true;
    }
};

USTRUCT(BlueprintType)
struct FVFXEmotionalBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    FString EmotionType; // "Fear", "Wonder", "Aggression", "Calm", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    float EmotionIntensity = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    FVFXEffectData EmotionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    bool bAffectsPostProcess = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    bool bAffectsParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional VFX")
    FLinearColor EmotionColor = FLinearColor::White;

    FVFXEmotionalBinding()
    {
        EmotionType = TEXT("");
        EmotionIntensity = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
        bAffectsPostProcess = true;
        bAffectsParticles = true;
        EmotionColor = FLinearColor::White;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVFXTriggered, const FGameplayTag&, TriggerTag, const FString&, EffectID, const FVFXEffectData&, EffectData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVFXStateChanged, const FString&, StateName, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVFXHealthChanged, float, OldHealth, float, NewHealth, const FString&, EffectID);

/**
 * VFX State Component - Manages VFX effects for individual actors
 * 
 * This component handles:
 * - State-based VFX triggering (health, movement, emotions)
 * - Automatic effect management based on actor conditions
 * - Integration with gameplay tags and events
 * - Performance optimization for actor-specific effects
 * - Creature-specific visual feedback
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXStateComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "VFX State")
    void TriggerVFXByTag(const FGameplayTag& TriggerTag, bool bForceRetrigger = false);

    UFUNCTION(BlueprintCallable, Category = "VFX State")
    void SetVFXState(const FString& StateName, bool bActive, float TransitionTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX State")
    bool GetVFXState(const FString& StateName) const;

    UFUNCTION(BlueprintCallable, Category = "VFX State")
    void StopAllVFXEffects(bool bFadeOut = true);

    // Health-based VFX
    UFUNCTION(BlueprintCallable, Category = "VFX State|Health")
    void UpdateHealthVFX(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Health")
    void SetHealthBinding(const FVFXHealthBinding& HealthBinding);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Health")
    void TriggerDamageVFX(float DamageAmount, const FVector& DamageLocation, const FString& DamageType);

    // Movement-based VFX
    UFUNCTION(BlueprintCallable, Category = "VFX State|Movement")
    void UpdateMovementVFX(const FVector& Velocity, bool bIsGrounded, const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Movement")
    void SetMovementBinding(const FVFXMovementBinding& MovementBinding);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Movement")
    void TriggerFootstepVFX(const FVector& FootLocation, const FString& SurfaceType, float ImpactForce);

    // Emotional VFX
    UFUNCTION(BlueprintCallable, Category = "VFX State|Emotion")
    void SetEmotionalState(const FString& EmotionType, float Intensity, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Emotion")
    void AddEmotionalBinding(const FVFXEmotionalBinding& EmotionalBinding);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Emotion")
    void ClearEmotionalState(float FadeOutTime = 3.0f);

    // Creature-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX State|Creature")
    void SetCreatureType(const FString& CreatureType);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Creature")
    void TriggerCreatureRoar(float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Creature")
    void TriggerCreatureBreath(float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Creature")
    void SetCreatureSize(float SizeMultiplier);

    // Environmental interaction
    UFUNCTION(BlueprintCallable, Category = "VFX State|Environment")
    void OnEnvironmentChanged(const FVFXEnvironmentalState& NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Environment")
    void SetEnvironmentalSensitivity(bool bSensitive, float SensitivityMultiplier = 1.0f);

    // Transpersonal VFX
    UFUNCTION(BlueprintCallable, Category = "VFX State|Transpersonal")
    void SetConsciousnessLevel(const FString& Level, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Transpersonal")
    void TriggerTranspersonalExperience(const FString& ExperienceType, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Transpersonal")
    void ShowAura(const FLinearColor& AuraColor, float Radius, float Duration = -1.0f);

    // Performance
    UFUNCTION(BlueprintCallable, Category = "VFX State|Performance")
    void SetVFXLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Performance")
    void SetMaxConcurrentEffects(int32 MaxEffects);

    UFUNCTION(BlueprintCallable, Category = "VFX State|Performance")
    int32 GetActiveEffectCount() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "VFX State|Events")
    FOnVFXTriggered OnVFXTriggered;

    UPROPERTY(BlueprintAssignable, Category = "VFX State|Events")
    FOnVFXStateChanged OnVFXStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "VFX State|Events")
    FOnVFXHealthChanged OnVFXHealthChanged;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    TArray<FVFXStateBinding> StateBindings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    FVFXHealthBinding HealthBinding;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    FVFXMovementBinding MovementBinding;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    TArray<FVFXEmotionalBinding> EmotionalBindings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    FString CreatureType = TEXT("Generic");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    float CreatureSizeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    bool bEnvironmentallySensitive = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    float EnvironmentalSensitivityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    int32 MaxConcurrentEffects = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    int32 VFXLODLevel = 2; // 0=Low, 1=Medium, 2=High, 3=Ultra

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    bool bAutoManageEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX State|Config")
    float EffectCullingDistance = 5000.0f;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    TMap<FString, bool> ActiveStates;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    TMap<FString, FString> ActiveEffectIDs;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    TMap<FGameplayTag, float> LastTriggerTimes;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    TMap<FGameplayTag, int32> TriggerCounts;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    float CurrentHealth = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    float MaxHealth = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    FVector LastVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    FString CurrentSurfaceType = TEXT("Default");

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    FString CurrentEmotionalState = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    float CurrentEmotionalIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State|Runtime")
    FString CurrentConsciousnessLevel = TEXT("");

    // System references
    UPROPERTY()
    UVFXSubsystem* VFXSubsystem;

private:
    // Internal methods
    void InitializeVFXState();
    void UpdateVFXLOD();
    void CleanupExpiredEffects();
    bool CanTriggerEffect(const FVFXStateBinding& Binding) const;
    void ProcessHealthThresholds(float HealthPercentage);
    void ProcessMovementThresholds(float VelocityMagnitude);
    FString GetSurfaceTypeAtLocation(const FVector& Location) const;
    void UpdateEmotionalEffects(float DeltaTime);
    void ApplyLODSettings();

    // Timers
    FTimerHandle LODUpdateTimer;
    FTimerHandle EffectCleanupTimer;
    FTimerHandle StateUpdateTimer;
};