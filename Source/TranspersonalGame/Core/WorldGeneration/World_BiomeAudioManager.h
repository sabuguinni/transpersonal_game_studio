#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Engine/AudioVolume.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "World_BiomeAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping;

    FWorld_BiomeAudioSettings()
    {
        AmbientSound = nullptr;
        Volume = 0.7f;
        Pitch = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
        bLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    EWeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    class USoundBase* WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float MaxDistance;

    FWorld_WeatherAudioData()
    {
        WeatherType = EWeatherType::Clear;
        WeatherSound = nullptr;
        IntensityMultiplier = 1.0f;
        MaxDistance = 5000.0f;
    }
};

/**
 * Manages environmental audio for different biomes and weather conditions
 * Handles smooth transitions between biome audio zones and weather audio layers
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* TransitionAudioComponent;

    // Biome Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EBiomeType, FWorld_BiomeAudioSettings> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<FWorld_WeatherAudioData> WeatherAudioData;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBiomeType TargetBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EWeatherType CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeTransitionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float WeatherAudioIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableDistanceAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableBiomeTransitions;

public:
    // Biome Audio Management
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EBiomeType NewBiome, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void StartBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome, float TransitionDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void StopBiomeTransition();

    UFUNCTION(BlueprintPure, Category = "Biome Audio")
    EBiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Biome Audio")
    bool IsTransitioning() const { return bIsTransitioning; }

    // Weather Audio Management
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherAudio(EWeatherType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StopWeatherAudio(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void UpdateWeatherIntensity(float NewIntensity);

    // Audio Volume Management
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void RegisterAudioVolume(class AAudioVolume* AudioVolume, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UnregisterAudioVolume(class AAudioVolume* AudioVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdatePlayerPosition(const FVector& PlayerLocation);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void LoadBiomeAudioConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SaveBiomeAudioConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBiomeAudioSettings(EBiomeType BiomeType, const FWorld_BiomeAudioSettings& Settings);

    // Debug and Utilities
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintAudioState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TestBiomeAudio(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TestWeatherAudio(EWeatherType WeatherType);

private:
    // Internal Methods
    void UpdateBiomeTransition(float DeltaTime);
    void UpdateWeatherAudio(float DeltaTime);
    void UpdateAudioComponents();
    
    void PlayBiomeAudio(EBiomeType BiomeType, float VolumeMultiplier = 1.0f);
    void StopBiomeAudio(float FadeTime = 1.0f);
    
    void PlayWeatherAudio(EWeatherType WeatherType, float Intensity);
    void UpdateAudioAttenuation(const FVector& PlayerLocation);
    
    FWorld_BiomeAudioSettings* GetBiomeAudioSettings(EBiomeType BiomeType);
    FWorld_WeatherAudioData* GetWeatherAudioData(EWeatherType WeatherType);

    // Internal State
    float TransitionDuration;
    float TransitionTimer;
    FVector LastPlayerLocation;
    TMap<class AAudioVolume*, EBiomeType> RegisteredAudioVolumes;
};