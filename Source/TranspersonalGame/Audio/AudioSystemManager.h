#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Forest UMETA(DisplayName = "Forest"),
    Desert UMETA(DisplayName = "Desert"),
    Swamp UMETA(DisplayName = "Swamp"),
    Mountain UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe UMETA(DisplayName = "Safe"),
    Caution UMETA(DisplayName = "Caution"),
    Danger UMETA(DisplayName = "Danger"),
    Extreme UMETA(DisplayName = "Extreme Danger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> WildlifeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AttenuationRadius = 5000.0f;

    FAudio_BiomeAudioData()
    {
        BaseVolume = 0.5f;
        AttenuationRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TSoftObjectPtr<USoundCue> ThreatMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float FadeInTime = 2.0f;

    FAudio_ThreatAudioData()
    {
        IntensityMultiplier = 1.0f;
        FadeInTime = 2.0f;
    }
};

/**
 * Audio System Manager - Handles dynamic audio based on biome, threat level, and player state
 * Provides spatial audio, adaptive music, and environmental soundscapes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurFootsteps(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurRoar(const FVector& Location, EAudio_ThreatLevel ThreatLevel);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateWeatherAudio(float RainIntensity, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    // Player Audio Feedback
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDamageAudio(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayHeartbeat(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayCraftingSound(const FString& CraftingType);

    // Spatial Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CreateAmbientZone(const FVector& Location, float Radius, EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RemoveAmbientZone(const FVector& Location, float Radius);

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEffectsVolume(float Volume);

protected:
    // Biome Audio Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TMap<EAudio_ThreatLevel, FAudio_ThreatAudioData> ThreatAudioMap;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

    // Audio Components
    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> WeatherAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> HeartbeatAudioComponent;

    // Active Audio Zones
    UPROPERTY()
    TArray<TObjectPtr<UAudioComponent>> ActiveAmbientZones;

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.6f;

private:
    // Internal Methods
    void InitializeBiomeAudioData();
    void InitializeThreatAudioData();
    void UpdateAmbientAudio();
    void UpdateMusicLayer();
    void CrossfadeBiomeAudio(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome);
    
    // Audio Timers
    FTimerHandle AmbientUpdateTimer;
    FTimerHandle ThreatUpdateTimer;
    
    // Audio State
    bool bIsInitialized = false;
    float LastBiomeTransitionTime = 0.0f;
    float LastThreatTransitionTime = 0.0f;
};

#include "AudioSystemManager.generated.h"