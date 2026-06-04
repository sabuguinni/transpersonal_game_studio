#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "WorldGenerationTypes.h"
#include "World_BiomeAudioController.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeAudioType : uint8
{
    Forest_Ambient UMETA(DisplayName = "Forest Ambient"),
    Plains_Wind UMETA(DisplayName = "Plains Wind"),
    River_Flow UMETA(DisplayName = "River Flow"),
    Cave_Echo UMETA(DisplayName = "Cave Echo"),
    Volcanic_Rumble UMETA(DisplayName = "Volcanic Rumble"),
    Swamp_Atmosphere UMETA(DisplayName = "Swamp Atmosphere"),
    Desert_Wind UMETA(DisplayName = "Desert Wind"),
    Mountain_Wind UMETA(DisplayName = "Mountain Wind")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    FWorld_BiomeAudioSettings()
    {
        Volume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
        MaxDistance = 5000.0f;
        bLooping = true;
    }
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EWorld_BiomeAudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FWorld_BiomeAudioSettings AudioSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    FWorld_BiomeAudioZone()
    {
        Center = FVector::ZeroVector;
        Radius = 2000.0f;
        AudioType = EWorld_BiomeAudioType::Forest_Ambient;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeAudioController : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeAudioController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void InitializeBiomeAudio();

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdatePlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void RegisterAudioZone(const FWorld_BiomeAudioZone& AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UnregisterAudioZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetGlobalAudioVolume(float Volume);

    // Weather integration
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void OnWeatherChanged(const FString& WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void PlayWeatherAudio(const FString& WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StopWeatherAudio();

    // Time of day integration
    UFUNCTION(BlueprintCallable, Category = "Time Audio")
    void OnTimeOfDayChanged(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Time Audio")
    void UpdateNightAmbience(bool bIsNight);

protected:
    // Audio zone management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FWorld_BiomeAudioZone> AudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float AudioTransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float AudioUpdateFrequency = 0.5f;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UAudioComponent*> BiomeAudioComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* NightAmbienceComponent;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentActiveZone = -1;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsNightTime = false;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EWorld_BiomeAudioType, FWorld_BiomeAudioSettings> DefaultAudioSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TMap<FString, TSoftObjectPtr<USoundCue>> WeatherSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Audio")
    TSoftObjectPtr<USoundCue> NightAmbienceSound;

private:
    // Internal methods
    void UpdateAudioZones();
    int32 FindNearestAudioZone(const FVector& Location);
    void TransitionToAudioZone(int32 ZoneIndex);
    void FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime);
    void LoadDefaultAudioSettings();

    // Timing
    float LastAudioUpdate = 0.0f;
};