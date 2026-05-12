#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Audio_SoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance = 5000.0f;

    FAudio_SoundConfig()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
        bLoop = false;
        MaxDistance = 5000.0f;
    }
};

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    None = 0,
    Ambient,
    Music,
    SFX,
    Voice,
    UI,
    Footsteps,
    Combat,
    Environment,
    Dinosaur
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core sound playback functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAtLocation(const FAudio_SoundConfig& SoundConfig, const FVector& Location, EAudio_SoundCategory Category = EAudio_SoundCategory::SFX);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySound2D(const FAudio_SoundConfig& SoundConfig, EAudio_SoundCategory Category = EAudio_SoundCategory::SFX);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSoundsInCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Ambient audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbientLoop(const FAudio_SoundConfig& SoundConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientLoop();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CrossfadeAmbient(const FAudio_SoundConfig& NewSoundConfig, float CrossfadeTime = 2.0f);

    // Dinosaur-specific audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurFootstep(EDinosaurSpecies Species, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurVocalization(EDinosaurSpecies Species, const FVector& Location, ECreatureEmotionalState EmotionalState = ECreatureEmotionalState::Neutral);

    // Screen shake audio integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayScreenShakeAudio(const FVector& SourceLocation, float Intensity, float Duration);

    // Audio occlusion and environmental effects
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioOcclusion();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetEnvironmentalReverb(float ReverbAmount, float DecayTime);

protected:
    // Category volume controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Active audio components by category
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_SoundCategory, TArray<UAudioComponent*>> ActiveAudioComponents;

    // Current ambient loop
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* CurrentAmbientLoop;

    // Dinosaur sound libraries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EDinosaurSpecies, TArray<FAudio_SoundConfig>> DinosaurFootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EDinosaurSpecies, TArray<FAudio_SoundConfig>> DinosaurVocalizationSounds;

    // Environmental audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    float GlobalReverbAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    float GlobalDecayTime;

private:
    void InitializeDefaultVolumes();
    void CleanupFinishedAudioComponents();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundConfig& SoundConfig, const FVector& Location, bool bIs2D = false);
    void ApplyCategoryVolume(UAudioComponent* AudioComponent, EAudio_SoundCategory Category);

    // Timer for cleanup
    FTimerHandle CleanupTimerHandle;
};