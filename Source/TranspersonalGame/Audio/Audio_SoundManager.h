#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/AmbientSound.h"
#include "SharedTypes.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    FAudio_BiomeSettings()
    {
        BaseVolume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatStinger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float IntensityMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float HeartbeatVolume = 0.8f;

    FAudio_ThreatSettings()
    {
        IntensityMultiplier = 1.5f;
        HeartbeatVolume = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TransitionToBiome(EAudio_BiomeType TargetBiome, float TransitionTime = 3.0f);

    // Threat level management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerThreatStinger(EAudio_ThreatLevel ThreatType);

    // Dynamic music system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartDynamicMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopDynamicMusic(float FadeOutTime = 2.0f);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalSound(const FString& SoundName, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetWeatherAudio(bool bIsRaining, bool bIsStormy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDayAudio(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    // Footstep system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(ESurfaceType SurfaceType, bool bIsRunning = false);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    // Narration system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayNarration(const FString& NarrationKey, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopCurrentNarration();

protected:
    // Biome settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_BiomeType, FAudio_BiomeSettings> BiomeSettings;

    // Threat settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_ThreatLevel, FAudio_ThreatSettings> ThreatSettings;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> BiomeAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> ThreatAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> NarrationAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> WeatherAudioComponent;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsDynamicMusicActive;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

private:
    void InitializeAudioComponents();
    void LoadDefaultSounds();
    void UpdateDynamicMusic();
    void CrossfadeBiomeAudio(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome, float Duration);

    FTimerHandle MusicUpdateTimer;
    FTimerHandle BiomeTransitionTimer;
};