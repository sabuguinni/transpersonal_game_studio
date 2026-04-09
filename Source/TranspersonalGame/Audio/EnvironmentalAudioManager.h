// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EnvironmentalAudioManager.generated.h"

/**
 * Environmental Audio Manager
 * 
 * Creates the living soundscape of the prehistoric world. Every rustle, every distant call,
 * every whisper of wind through ancient trees. This system makes the world feel alive
 * even when no creatures are visible.
 * 
 * Philosophy: The world has its own voice. Our job is to let players hear it.
 * Sound design inspired by the layered environmental audio of RDR2 and the
 * procedural soundscapes of No Man's Sky.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentalAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentalAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === ENVIRONMENTAL ZONE SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    EEnvironmentalZone CurrentZone = EEnvironmentalZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TMap<EEnvironmentalZone, FEnvironmentalSoundscape> ZoneSoundscapes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float ZoneTransitionTime = 8.0f;

    // === WEATHER AND TIME SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<USoundBase*> RainSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<USoundBase*> WindSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<USoundBase*> ThunderSounds;

    // === AMBIENT LAYER SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Layers")
    TArray<UAudioComponent*> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Layers")
    TArray<UAudioComponent*> WeatherLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Layers")
    TArray<UAudioComponent*> RandomElementLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Layers")
    float MasterAmbientVolume = 0.8f;

    // === PROCEDURAL SOUND GENERATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Audio")
    bool bEnableProceduralSounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Audio")
    float RandomSoundFrequency = 0.1f; // Chance per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Audio")
    FVector2D RandomSoundDistanceRange = FVector2D(500.0f, 2000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Audio")
    TArray<USoundBase*> DistantCreatureCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Audio")
    TArray<USoundBase*> VegetationSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Audio")
    TArray<USoundBase*> WaterSounds;

    // === SPATIAL AUDIO SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxAudibleDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bUse3DAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float SpatialAudioFalloffExponent = 2.0f;

public:
    // === ZONE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void TransitionToZone(EEnvironmentalZone NewZone, float TransitionTime = 8.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetZoneInstantly(EEnvironmentalZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdatePlayerLocation(FVector PlayerLocation);

    // === WEATHER SYSTEM ===

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherIntensity(float Intensity, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void TriggerThunder(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void UpdateTimeOfDay(ETimeOfDay NewTimeOfDay);

    // === PROCEDURAL SOUND TRIGGERS ===

    UFUNCTION(BlueprintCallable, Category = "Procedural Audio")
    void TriggerDistantCreatureCall(FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Procedural Audio")
    void TriggerVegetationSound(FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Procedural Audio")
    void TriggerWaterSound(FVector Location, float Volume = 1.0f);

    // === VOLUME CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMasterAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeAmbientAudio(float TargetVolume, float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetProceduralSoundsEnabled(bool bEnabled);

protected:
    // === INTERNAL AUDIO MANAGEMENT ===

    UFUNCTION()
    void UpdateAmbientLayers();

    UFUNCTION()
    void UpdateWeatherLayers();

    UFUNCTION()
    void ProcessProceduralSounds();

    UFUNCTION()
    void UpdateSpatialAudioSettings();

    UFUNCTION()
    void CreateAmbientLayer(USoundBase* SoundAsset, float Volume = 1.0f, bool bLoop = true);

    UFUNCTION()
    void PlayRandomElementSound();

    UFUNCTION()
    FVector GetRandomLocationAroundPlayer(float MinDistance, float MaxDistance) const;

    // === TIMER MANAGEMENT ===

    FTimerHandle ProceduralSoundTimer;
    FTimerHandle RandomElementTimer;
    FTimerHandle AmbientUpdateTimer;

    // === INTERNAL STATE ===

    FVector LastPlayerLocation = FVector::ZeroVector;
    float CurrentAmbientVolume = 0.8f;
    bool bIsTransitioningZones = false;

    // === AUDIO COMPONENT MANAGEMENT ===

    static const int32 MaxAmbientLayers = 8;
    static const int32 MaxWeatherLayers = 4;
    static const int32 MaxRandomElements = 16;

private:
    // === HELPER FUNCTIONS ===

    USoundBase* GetRandomSoundFromArray(const TArray<USoundBase*>& SoundArray) const;
    float CalculateVolumeFromDistance(float Distance) const;
    void CleanupFinishedAudioComponents();
};