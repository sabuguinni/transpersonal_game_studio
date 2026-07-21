#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "World_BiomeAudioController.generated.h"

/**
 * World_BiomeAudioController - Advanced biome audio management system
 * Handles dynamic audio transitions between biomes with performance optimization
 * Integrates with existing biome systems and provides seamless audio experiences
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeAudioController : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeAudioController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SecondaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    // Biome audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, class USoundCue*> BiomeAmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, class USoundCue*> BiomeCreatureSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, class USoundCue*> BiomeWeatherSounds;

    // Audio transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CrossfadeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition")
    bool bUseDistanceBasedTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float TransitionRadius;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float AudioUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAudioCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float AudioCullingDistance;

    // Dynamic weather integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    bool bEnableWeatherAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TMap<EWorld_WeatherType, class USoundCue*> WeatherSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float WeatherAudioIntensityMultiplier;

    // Time of day audio variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    bool bEnableTimeBasedAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    class USoundCue* DayAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    class USoundCue* NightAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    class USoundCue* DawnDuskSound;

public:
    // Audio control functions
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EWorld_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void TransitionToBiome(EWorld_BiomeType TargetBiome, float CustomTransitionTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherAudio(EWorld_WeatherType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Time Audio")
    void UpdateTimeBasedAudio(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAudioPerformance();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PauseAllBiomeAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void ResumeAllBiomeAudio();

    // Audio query functions
    UFUNCTION(BlueprintPure, Category = "Biome Audio")
    EWorld_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio Status")
    bool IsTransitioning() const { return bIsTransitioning; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    int32 GetActiveSoundCount() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCurrentAudioLoad() const;

protected:
    // Internal audio management
    void UpdateAudioTransition(float DeltaTime);
    void ProcessAudioCulling();
    void UpdateWeatherAudioIntensity();
    void HandleTimeOfDayTransition(float TimeOfDay);
    
    // Performance monitoring
    void MonitorAudioPerformance();
    void CullDistantAudioSources();
    void OptimizeAudioComponents();

private:
    // Current state
    EWorld_BiomeType CurrentBiome;
    EWorld_BiomeType TargetBiome;
    EWorld_WeatherType CurrentWeather;
    
    // Transition state
    bool bIsTransitioning;
    float TransitionProgress;
    float CurrentTransitionDuration;
    
    // Performance tracking
    float LastAudioUpdate;
    int32 ActiveSoundCount;
    float CurrentAudioLoad;
    
    // Audio component management
    TArray<class UAudioComponent*> ManagedAudioComponents;
    TArray<float> AudioComponentDistances;
    
    // Time tracking
    float LastTimeOfDay;
    bool bIsDayTime;
    bool bIsTransitioningTimeAudio;
};