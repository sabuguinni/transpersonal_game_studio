#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "AudioFeedbackManager.generated.h"

class UCameraShakeBase;
class USoundCue;
class UAudioComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Range = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bFadeOut = true;

    FAudio_ScreenShakeData()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Range = 1000.0f;
        bFadeOut = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.8f;

    FAudio_DamageFlashData()
    {
        FlashColor = FLinearColor::Red;
        FlashDuration = 0.3f;
        FlashIntensity = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioFeedbackManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Screen shake effects
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerScreenShake(const FVector& EpicenterLocation, const FAudio_ScreenShakeData& ShakeData);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerTRexProximityShake(const FVector& TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerFootstepShake(const FVector& FootstepLocation, float CreatureSize = 1.0f);

    // Damage feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerDamageFlash(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerPainVoiceover(float PainIntensity);

    // Audio cue management
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void PlayProximityWarningSound(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void PlayDamageAudioCue(EDamageType DamageType);

    // Biome-specific audio
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void UpdateBiomeAmbientAudio(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void SetAmbientAudioVolume(float Volume);

protected:
    // Screen shake presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Presets")
    FAudio_ScreenShakeData TRexShakeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Presets")
    FAudio_ScreenShakeData FootstepShakeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Presets")
    FAudio_ScreenShakeData DamageShakeData;

    // Damage flash settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FAudio_DamageFlashData DefaultDamageFlash;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* ProximityWarningComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* DamageAudioComponent;

    // Audio assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* TRexProximitySoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* DamageSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EBiomeType, USoundCue*> BiomeAmbientSounds;

private:
    // Internal state
    float CurrentAmbientVolume = 1.0f;
    EBiomeType CurrentBiome = EBiomeType::Savana;
    bool bDamageFlashActive = false;

    // Helper functions
    void InitializeAudioComponents();
    void SetupBiomeAudioAssets();
    class APlayerController* GetPlayerController() const;
    class UCameraShakeBase* CreateScreenShake(const FAudio_ScreenShakeData& ShakeData);
};