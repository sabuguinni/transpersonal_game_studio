#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/AmbientSound.h"
#include "SharedTypes.h"
#include "World_BiomeAudioSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FString> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FString> WeatherSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    bool bIsActive;

    FWorld_BiomeAudioZone()
    {
        BiomeType = EBiomeType::Savanna;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 10000.0f;
        BaseVolume = 0.7f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    EWeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float WeatherIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    bool bIsTransitioning;

    FWorld_WeatherAudioState()
    {
        CurrentWeather = EWeatherType::Clear;
        WeatherIntensity = 0.0f;
        TransitionSpeed = 1.0f;
        bIsTransitioning = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FWorld_BiomeAudioZone> BiomeAudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<AAmbientSound*> BiomeAudioActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<AAmbientSound*> WeatherAudioActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    FWorld_WeatherAudioState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float AudioUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MaxAudioDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    bool bEnableWeatherAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    bool bEnableBiomeTransitions;

private:
    float LastAudioUpdateTime;
    FVector LastPlayerLocation;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void InitializeBiomeAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdatePlayerAudioZone(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    EBiomeType GetCurrentBiome(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void UpdateWeatherAudio(EWeatherType NewWeather, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetBiomeAudioVolume(EBiomeType BiomeType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void EnableBiomeAudio(EBiomeType BiomeType, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float CalculateAudioFalloff(FVector ListenerLocation, FVector AudioLocation, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CreateBiomeAudioActor(FWorld_BiomeAudioZone AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CleanupAudioActors();

protected:
    void UpdateAudioSystem(float DeltaTime);
    void ProcessBiomeTransitions(FVector PlayerLocation);
    void UpdateWeatherAudioEffects(float DeltaTime);
    bool IsLocationInBiome(FVector Location, FWorld_BiomeAudioZone BiomeZone);
    float GetBiomeInfluence(FVector Location, FWorld_BiomeAudioZone BiomeZone);
};