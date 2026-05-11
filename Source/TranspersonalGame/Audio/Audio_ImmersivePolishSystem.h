#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Audio_ImmersivePolishSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ImmersionLevel : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Immersion"),
    Standard    UMETA(DisplayName = "Standard Immersion"),
    Enhanced    UMETA(DisplayName = "Enhanced Immersion"),
    Maximum     UMETA(DisplayName = "Maximum Immersion")
};

USTRUCT(BlueprintType)
struct FAudio_ImmersiveSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion")
    float AmbientVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion")
    float EffectsVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion")
    float VoiceVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion")
    float ReverbIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion")
    bool bEnableDirectionalAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion")
    bool bEnableHapticFeedback = true;

    FAudio_ImmersiveSettings()
    {
        AmbientVolumeMultiplier = 1.0f;
        EffectsVolumeMultiplier = 1.0f;
        VoiceVolumeMultiplier = 1.0f;
        ReverbIntensity = 0.5f;
        bEnableDirectionalAudio = true;
        bEnableHapticFeedback = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ImmersivePolishSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ImmersivePolishSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* EffectsAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VoiceAudioComponent;

    // Immersion Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion Settings")
    EAudio_ImmersionLevel CurrentImmersionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immersion Settings")
    FAudio_ImmersiveSettings ImmersiveSettings;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* TRexProximityAlert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* MedicalEmergencyAlert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* PrehistoricAmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* FootstepDustSFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* ScreenShakeRumbleSFX;

    // Polish Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Polish Effects")
    bool bEnableScreenShakeAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Polish Effects")
    bool bEnableDamageFlashAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Polish Effects")
    bool bEnableFootstepParticleAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Polish Effects")
    bool bEnableDayNightCycleAudio;

    // Timing and Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float PolishEffectUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float ImmersionTransitionDuration;

    FTimerHandle PolishEffectTimer;
    FTimerHandle ImmersionTransitionTimer;

public:
    // Core Polish Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void SetImmersionLevel(EAudio_ImmersionLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void PlayTRexProximityAlert(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void PlayMedicalEmergencyAlert(float UrgencyLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerScreenShakeAudio(float ShakeIntensity, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerDamageFlashAudio(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void PlayFootstepParticleAudio(FVector FootstepLocation, float ParticleIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void UpdateDayNightCycleAudio(float TimeOfDay);

    // System Control
    UFUNCTION(BlueprintCallable, Category = "System Control")
    void StartPolishEffectsSystem();

    UFUNCTION(BlueprintCallable, Category = "System Control")
    void StopPolishEffectsSystem();

    UFUNCTION(BlueprintCallable, Category = "System Control")
    void ResetAllPolishEffects();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio Polish")
    EAudio_ImmersionLevel GetCurrentImmersionLevel() const { return CurrentImmersionLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio Polish")
    FAudio_ImmersiveSettings GetImmersiveSettings() const { return ImmersiveSettings; }

    UFUNCTION(BlueprintPure, Category = "Audio Polish")
    bool IsPolishEffectsSystemActive() const;

private:
    void UpdatePolishEffects();
    void TransitionImmersionLevel();
    void ApplyImmersiveSettings();
    void UpdateAmbientAudio();
    void UpdateEffectsAudio();
    void UpdateVoiceAudio();

    bool bIsSystemActive;
    float CurrentTransitionTime;
    EAudio_ImmersionLevel TargetImmersionLevel;
    FAudio_ImmersiveSettings PreviousSettings;
};