#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "PrehistoricAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> DangerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeDistance = 5000.0f;

    FAudio_BiomeAudioData()
    {
        BaseVolume = 0.7f;
        FadeDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SurvivalAudioCues
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundBase> LowHealthSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundBase> HighFearSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundBase> HungerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundBase> ThirstSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float SurvivalAudioVolume = 0.5f;

    FAudio_SurvivalAudioCues()
    {
        SurvivalAudioVolume = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricAudioManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MainAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AudioTriggerSphere;

    // Biome audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    // Survival audio cues
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    FAudio_SurvivalAudioCues SurvivalAudioCues;

    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentDangerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsPlayingDangerAudio;

    // Audio timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Timing")
    float AudioUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Timing")
    float BiomeTransitionTime = 3.0f;

    float LastAudioUpdateTime;

public:
    // Audio management functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateBiomeAudio(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlaySurvivalAudioCue(ESurvivalStat StatType, float StatValue);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetDangerLevel(float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayDinosaurProximityAudio(EDinosaurSpecies Species, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void StopAllAudio();

    // Weather and time-based audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(EWeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateTimeOfDayAudio(float TimeOfDay);

protected:
    // Internal audio management
    void InitializeBiomeAudioData();
    void UpdateAudioBasedOnPlayerState();
    float CalculateAudioVolume(float Distance, float MaxDistance);
    void CrossfadeToNewAudio(USoundBase* NewSound, float FadeTime);

    // Player reference for proximity calculations
    UPROPERTY()
    APawn* CachedPlayerPawn;

    // Audio fade state
    bool bIsFading;
    float FadeStartTime;
    float FadeDuration;
    USoundBase* FadeTargetSound;
};