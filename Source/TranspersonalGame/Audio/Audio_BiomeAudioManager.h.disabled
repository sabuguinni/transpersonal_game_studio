#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "SharedTypes.h"
#include "Audio_BiomeAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> WeatherSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> CreatureSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    FAudio_BiomeAudioData()
    {
        BaseVolume = 0.7f;
        FadeInTime = 3.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DynamicAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EBiomeType CurrentBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    int32 NearbyDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsInCombat = false;

    FAudio_DynamicAudioState()
    {
        CurrentBiome = EBiomeType::Savanna;
        TimeOfDay = 12.0f;
        CurrentWeather = EWeatherType::Clear;
        ThreatLevel = 0.0f;
        NearbyDinosaurCount = 0;
        bIsInCombat = false;
    }
};

/**
 * Biome-adaptive audio manager that dynamically adjusts environmental soundscapes
 * based on player location, time of day, weather, and threat level.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_BiomeAudioManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudio_BiomeAudioManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TransitionToBiome(EBiomeType NewBiome, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioState(const FAudio_DynamicAudioState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetWeatherType(EWeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(float ThreatLevel);

    // Audio component management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayBiomeAmbient(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopBiomeAmbient(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeOutAllAmbient(float FadeTime = 2.0f);

    // Audio configuration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCreatureVolume(float Volume);

    // Audio queries
    UFUNCTION(BlueprintPure, Category = "Audio")
    EBiomeType GetCurrentBiome() const { return CurrentAudioState.CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    float GetCurrentThreatLevel() const { return CurrentAudioState.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsAudioTransitioning() const { return bIsTransitioning; }

protected:
    // Audio data configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float CreatureVolume = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float MaxTransitionTime = 5.0f;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_DynamicAudioState CurrentAudioState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_DynamicAudioState TargetAudioState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TransitionDuration = 3.0f;

    // Audio components
    UPROPERTY()
    TMap<EBiomeType, UAudioComponent*> BiomeAudioComponents;

    UPROPERTY()
    UAudioComponent* WeatherAudioComponent;

    UPROPERTY()
    UAudioComponent* CreatureAudioComponent;

    UPROPERTY()
    UAudioComponent* ThreatAudioComponent;

private:
    // Internal audio management
    void InitializeBiomeAudioData();
    void CreateAudioComponents();
    void UpdateAudioTransition(float DeltaTime);
    void ApplyAudioState(const FAudio_DynamicAudioState& AudioState, float BlendWeight);
    void UpdateVolumeBasedOnState();
    
    // Audio loading
    void LoadBiomeAudioAssets();
    USoundCue* LoadSoundCueAsset(const FString& AssetPath);
    
    // Transition helpers
    void StartAudioTransition(const FAudio_DynamicAudioState& NewState, float Duration);
    void CompleteAudioTransition();
    float CalculateTransitionCurve(float Progress) const;
};