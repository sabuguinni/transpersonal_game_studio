#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "World_EnvironmentalAudio.generated.h"

/**
 * Environmental Audio Zone Actor
 * Manages biome-specific ambient sounds and audio transitions
 * Integrates with weather system for dynamic audio mixing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_EnvironmentalAudio : public AActor
{
    GENERATED_BODY()

public:
    AWorld_EnvironmentalAudio();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* WildlifeAudioComponent;

    // Trigger Volume
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    class USphereComponent* AudioTriggerSphere;

public:
    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AudioZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FadeDistance;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* WeatherSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* WildlifeSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float WeatherVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float WildlifeVolumeMultiplier;

    // Dynamic Audio Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    bool bUseTimeOfDayVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float DayVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float NightVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    bool bUseWeatherIntegration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float RainVolumeIncrease;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float StormVolumeIncrease;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConcurrentSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AudioLODDistance;

    // Audio Zone Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetBiomeType(EWorld_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void UpdateAudioForWeather(EWorld_WeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void UpdateAudioForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetAudioZoneActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float CalculatePlayerDistance() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float CalculateVolumeForDistance(float Distance) const;

    // Trigger Events
    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void StopAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void CrossfadeToNewBiome(AWorld_EnvironmentalAudio* NewBiomeAudio, float FadeTime);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateAudioMix();

private:
    // Internal State
    bool bPlayerInZone;
    float CurrentVolume;
    float TargetVolume;
    float VolumeTransitionSpeed;
    
    // Audio Fade System
    float FadeTimer;
    float FadeDuration;
    bool bIsFading;
    
    // Performance Tracking
    float LastUpdateTime;
    float UpdateFrequency;
    
    // Helper Functions
    void InitializeAudioComponents();
    void ConfigureBiomeAudio();
    void UpdateVolumeSmooth(float DeltaTime);
    void ApplyPerformanceLOD();
    bool ShouldPlayAudio() const;
    float GetBiomeSpecificVolume() const;
};