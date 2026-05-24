#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

/**
 * Environmental Sound Manager - Handles ambient audio, weather sounds, and atmospheric audio layers
 * Manages dynamic audio based on biome, weather, time of day, and player location
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // === ENVIRONMENTAL AUDIO COMPONENTS ===
    
    /** Primary ambient sound component for biome-specific audio */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* BiomeAmbientComponent;

    /** Wind audio component for atmospheric effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* WindAudioComponent;

    /** Weather audio component for rain, thunder, etc */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* WeatherAudioComponent;

    /** Distance-based dinosaur audio component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* DinosaurAmbienceComponent;

    // === SOUND ASSETS ===

    /** Forest ambience sound cue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sounds")
    class USoundCue* ForestAmbienceSound;

    /** Swamp ambience sound cue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sounds")
    class USoundCue* SwampAmbienceSound;

    /** Savanna ambience sound cue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sounds")
    class USoundCue* SavannaAmbienceSound;

    /** Desert ambience sound cue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sounds")
    class USoundCue* DesertAmbienceSound;

    /** Mountain ambience sound cue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Sounds")
    class USoundCue* MountainAmbienceSound;

    /** Wind through trees sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    class USoundCue* WindThroughTreesSound;

    /** Heavy wind sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    class USoundCue* HeavyWindSound;

    /** Rain sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    class USoundCue* RainSound;

    /** Thunder sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    class USoundCue* ThunderSound;

    /** Distant dinosaur roars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    class USoundCue* DistantDinosaurRoars;

    /** T-Rex footsteps earthquake sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Sounds")
    class USoundCue* TRexFootstepsSound;

    // === AUDIO MANAGEMENT ===

    /** Current active biome */
    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EBiomeType CurrentBiome;

    /** Current weather state */
    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EWeatherType CurrentWeather;

    /** Audio fade transition duration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float AudioFadeDuration;

    /** Master environmental volume multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float EnvironmentalVolumeMultiplier;

    /** Distance at which dinosaur sounds start fading */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float DinosaurAudioMaxDistance;

    // === AUDIO CONTROL FUNCTIONS ===

    /** Change biome ambient sound with crossfade */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiomeAmbience(EBiomeType NewBiome);

    /** Update weather audio layers */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetWeatherAudio(EWeatherType NewWeather, float Intensity = 1.0f);

    /** Play distant dinosaur roar at specific location */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayDistantDinosaurRoar(FVector Location, float VolumeMultiplier = 1.0f);

    /** Play T-Rex earthquake footsteps */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayTRexFootsteps(FVector TRexLocation, float Distance);

    /** Update all environmental audio based on player location */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateEnvironmentalAudio(FVector PlayerLocation);

    /** Stop all environmental audio */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopAllEnvironmentalAudio();

    /** Get current biome from world position */
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation);

private:
    // === INTERNAL AUDIO MANAGEMENT ===

    /** Timer handle for audio updates */
    FTimerHandle AudioUpdateTimer;

    /** Last known player location for optimization */
    FVector LastPlayerLocation;

    /** Distance threshold for audio updates */
    float AudioUpdateDistance;

    /** Current audio fade timer */
    float CurrentFadeTime;

    /** Target volume for current fade */
    float TargetVolume;

    /** Is currently fading audio */
    bool bIsFading;

    /** Internal function to handle audio crossfading */
    void HandleAudioFade(float DeltaTime);

    /** Internal function to update audio based on distance */
    void UpdateDistanceBasedAudio();

    /** Internal function to calculate volume based on distance */
    float CalculateVolumeByDistance(float Distance, float MaxDistance);
};