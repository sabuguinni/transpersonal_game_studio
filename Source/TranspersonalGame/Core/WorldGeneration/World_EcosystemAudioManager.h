#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_EcosystemAudioManager.generated.h"

// Forward declarations
class UWorld_BiomeAudioController;
class UWorld_EnvironmentalAudio;
class APlayerController;

/**
 * Ecosystem Audio Manager - Orchestrates dynamic audio across biomes and weather systems
 * Manages real-time audio transitions, environmental soundscapes, and weather-responsive audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_EcosystemAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_EcosystemAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Core")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Core")
    class UAudioComponent* BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Core")
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Core")
    class UAudioComponent* AmbientAudioComponent;

    // Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float AudioTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float MaxAudioDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float BiomeBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float WeatherIntensityMultiplier;

    // Biome Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, TSoftObjectPtr<USoundCue>> BiomeAudioCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, float> BiomeVolumeMultipliers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float CurrentBiomeVolume;

    // Weather Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TMap<EWorld_WeatherType, TSoftObjectPtr<USoundCue>> WeatherAudioCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TMap<EWorld_WeatherType, float> WeatherVolumeMultipliers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float CurrentWeatherIntensity;

    // Audio State Management
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWorld_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWorld_BiomeType TargetBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWorld_WeatherType CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float BiomeTransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsTransitioning;

    // Audio Zone Management
    UPROPERTY(BlueprintReadOnly, Category = "Audio Zones")
    TArray<class AWorld_AudioZoneManager*> ActiveAudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float AudioZoneUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zones")
    float LastAudioZoneUpdate;

public:
    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Ecosystem Audio")
    void InitializeEcosystemAudio();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Audio")
    void UpdateEcosystemAudio(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem Audio")
    void ShutdownEcosystemAudio();

    // Biome Audio Control
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EWorld_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void TransitionToBiome(EWorld_BiomeType TargetBiomeType, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateBiomeAudio(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Biome Audio")
    float GetBiomeAudioVolume(EWorld_BiomeType BiomeType) const;

    // Weather Audio Control
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetCurrentWeather(EWorld_WeatherType NewWeather, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void UpdateWeatherAudio(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Weather Audio")
    float GetWeatherAudioVolume(EWorld_WeatherType WeatherType) const;

    // Audio Zone Management
    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void RegisterAudioZone(class AWorld_AudioZoneManager* AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void UnregisterAudioZone(class AWorld_AudioZoneManager* AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void UpdateAudioZones();

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayEnvironmentalSound(USoundCue* SoundCue, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopEnvironmentalSound(USoundCue* SoundCue);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAmbientAudio(float DeltaTime);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAudioPerformance();

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetActiveAudioSourceCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantAudioSources();

private:
    // Internal audio management
    void LoadBiomeAudioCues();
    void LoadWeatherAudioCues();
    void UpdateAudioComponentSettings();
    float CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation) const;
    void ProcessBiomeTransition(float DeltaTime);
    void ValidateAudioComponents();

    // Audio transition helpers
    float TransitionDuration;
    float TransitionTimer;
    FVector LastPlayerLocation;
    float AudioUpdateTimer;
    
    // Performance tracking
    int32 MaxActiveAudioSources;
    float AudioCullDistance;
    TArray<UAudioComponent*> ActiveAudioComponents;
};