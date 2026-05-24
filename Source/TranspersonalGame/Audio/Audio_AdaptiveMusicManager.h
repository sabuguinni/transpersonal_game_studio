#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveMusicManager.generated.h"

// Audio system enums specific to Audio Agent
UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient         UMETA(DisplayName = "Ambient"),
    Tension         UMETA(DisplayName = "Tension"),
    Combat          UMETA(DisplayName = "Combat"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Music           UMETA(DisplayName = "Music"),
    SFX             UMETA(DisplayName = "SFX"),
    Ambience        UMETA(DisplayName = "Ambience"),
    Voice           UMETA(DisplayName = "Voice"),
    UI              UMETA(DisplayName = "UI")
};

UENUM(BlueprintType)
enum class EAudio_ProximityLevel : uint8
{
    VeryFar         UMETA(DisplayName = "Very Far (1000m+)"),
    Far             UMETA(DisplayName = "Far (500-1000m)"),
    Medium          UMETA(DisplayName = "Medium (200-500m)"),
    Close           UMETA(DisplayName = "Close (50-200m)"),
    VeryClose       UMETA(DisplayName = "Very Close (0-50m)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> MusicTheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundWave>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime;

    FAudio_BiomeAudioData()
    {
        BiomeType = EEng_BiomeType::Forest;
        BaseVolume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EEng_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<TSoftObjectPtr<USoundWave>> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<TSoftObjectPtr<USoundWave>> MovementSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<TSoftObjectPtr<USoundWave>> AggressiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxHearingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeMultiplier;

    FAudio_DinosaurAudioData()
    {
        Species = EEng_DinosaurSpecies::Raptor;
        MaxHearingDistance = 2000.0f;
        VolumeMultiplier = 1.0f;
    }
};

/**
 * Adaptive Music Manager - Handles dynamic music and ambient audio based on biome, threat level, and player state
 * Provides seamless audio transitions and 3D positional audio for dinosaurs and environmental sounds
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateCurrentBiome(EEng_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateThreatLevel(EEng_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateTimeOfDay(EEng_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateWeather(EEng_WeatherType NewWeather);

    // Music layer control
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicLayer(EAudio_MusicLayer Layer, float Volume = 1.0f, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void FadeMusicLayer(EAudio_MusicLayer Layer, float TargetVolume, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void StopAllMusic(float FadeTime = 3.0f);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(EEng_DinosaurSpecies Species, EEng_DinosaurBehavior Behavior, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void RegisterDinosaurActor(class AActor* DinosaurActor, EEng_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UnregisterDinosaurActor(class AActor* DinosaurActor);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayEnvironmentalSound(FVector Location, TSoftObjectPtr<USoundWave> SoundWave, float Volume = 1.0f, float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAmbientAudio();

    // Proximity system
    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    EAudio_ProximityLevel CalculateProximityLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void TriggerProximityAlert(EEng_DinosaurSpecies Species, float Distance);

    // Master volume controls
    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Volume Control")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

protected:
    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEng_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEng_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEng_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEng_WeatherType CurrentWeather;

    // Audio components for different layers
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudio_MusicLayer, class UAudioComponent*> MusicLayers;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<class UAudioComponent*> EnvironmentalAudioPool;

    // Audio data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TArray<FAudio_BiomeAudioData> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TArray<FAudio_DinosaurAudioData> DinosaurAudioData;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Registered dinosaur actors for proximity tracking
    UPROPERTY(BlueprintReadOnly, Category = "Tracking")
    TMap<class AActor*, EEng_DinosaurSpecies> RegisteredDinosaurs;

    // Timer handles for audio updates
    FTimerHandle AmbientUpdateTimer;
    FTimerHandle ProximityCheckTimer;

private:
    // Internal helper functions
    void InitializeAudioComponents();
    void InitializeDefaultAudioData();
    void UpdateMusicBasedOnState();
    void CheckDinosaurProximity();
    FAudio_BiomeAudioData* GetBiomeAudioData(EEng_BiomeType BiomeType);
    FAudio_DinosaurAudioData* GetDinosaurAudioData(EEng_DinosaurSpecies Species);
    class UAudioComponent* GetAvailableAudioComponent();
    void ReturnAudioComponent(class UAudioComponent* Component);
};