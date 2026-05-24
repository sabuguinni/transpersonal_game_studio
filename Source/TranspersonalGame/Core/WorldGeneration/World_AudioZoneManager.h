#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/AudioVolume.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "World_AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeAudioType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EWorld_BiomeAudioType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector ZoneScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioFadeDistance;

    FWorld_BiomeAudioData()
    {
        BiomeType = EWorld_BiomeAudioType::Forest;
        ZoneLocation = FVector::ZeroVector;
        ZoneScale = FVector(20.0f, 20.0f, 10.0f);
        AmbientVolume = 0.5f;
        AudioFadeDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_AudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_AudioZoneManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Audio zone management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeBiomeAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CreateAudioZone(const FWorld_BiomeAudioData& AudioData);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdatePlayerAudioZone(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EWorld_BiomeAudioType GetCurrentBiomeAudio() const;

    // Environmental audio control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetWeatherAudio(bool bIsRaining, bool bIsStormy);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDayAudio(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayBiomeTransitionSound(EWorld_BiomeAudioType FromBiome, EWorld_BiomeAudioType ToBiome);

protected:
    // Audio zone data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FWorld_BiomeAudioData> BiomeAudioZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zones")
    TArray<AAudioVolume*> SpawnedAudioVolumes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zones")
    TArray<AAmbientSound*> SpawnedAmbientSounds;

    // Current audio state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    EWorld_BiomeAudioType CurrentBiomeAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    float CurrentWeatherIntensity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* TimeOfDayAudioComponent;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float WeatherAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float TimeOfDayAudioVolume;

private:
    // Internal audio management
    void UpdateAudioZoneVolumes();
    void CrossfadeBiomeAudio(EWorld_BiomeAudioType NewBiome);
    float CalculateDistanceToZone(const FVector& PlayerLocation, const FWorld_BiomeAudioData& ZoneData) const;
    void InitializeDefaultBiomeZones();
};

#include "World_AudioZoneManager.generated.h"