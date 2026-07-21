#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "Kismet/GameplayStatics.h"
#include "World_PrehistoricSoundscape.generated.h"

UENUM(BlueprintType)
enum class EWorld_PrehistoricSoundType : uint8
{
    ForestAmbient,
    PlainWinds,
    RiverFlow,
    VolcanicRumble,
    CaveEcho,
    SwampBubbles,
    DesertWind,
    OceanWaves
};

USTRUCT(BlueprintType)
struct FWorld_PrehistoricSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundCue* SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float AttenuationRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool bSpatial = true;

    FWorld_PrehistoricSoundData()
    {
        SoundCue = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FWorld_SoundscapeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ZoneRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EWorld_PrehistoricSoundType PrimarySoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<EWorld_PrehistoricSoundType> SecondarySounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bTimeOfDayVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bWeatherVariation = true;

    FWorld_SoundscapeZone()
    {
        ZoneCenter = FVector::ZeroVector;
        PrimarySoundType = EWorld_PrehistoricSoundType::ForestAmbient;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_PrehistoricSoundscape : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_PrehistoricSoundscape();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Sound configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TMap<EWorld_PrehistoricSoundType, FWorld_PrehistoricSoundData> SoundDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TArray<FWorld_SoundscapeZone> SoundscapeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float UpdateFrequency = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float MaxSimultaneousSounds = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    bool bEnableTimeOfDayVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    bool bEnableWeatherVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    bool bEnablePlayerProximityFade = true;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FVector LastPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float TimeSinceLastUpdate = 0.0f;

    // Core functionality
    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void InitializeSoundscape();

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void UpdateSoundscape(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void AddSoundscapeZone(const FWorld_SoundscapeZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void RemoveSoundscapeZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void SetGlobalVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void EnableTimeOfDayVariation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void EnableWeatherVariation(bool bEnable);

    // Sound management
    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    UAudioComponent* PlayPrehistoricSound(EWorld_PrehistoricSoundType SoundType, const FVector& Location, float VolumeOverride = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void StopPrehistoricSound(EWorld_PrehistoricSoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void FadeInSound(UAudioComponent* AudioComp, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    void FadeOutSound(UAudioComponent* AudioComp, float FadeTime = 2.0f);

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    FWorld_SoundscapeZone GetNearestSoundscapeZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    TArray<FWorld_SoundscapeZone> GetActiveSoundscapeZones(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    float CalculateZoneInfluence(const FWorld_SoundscapeZone& Zone, const FVector& Location) const;

    // Environmental factors
    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    float GetTimeOfDayMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    float GetWeatherMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Soundscape")
    float GetDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance) const;

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawSoundscapeZones() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogActiveSounds() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogSoundscapeStatus() const;

private:
    void CleanupInactiveSounds();
    void UpdateSoundParameters();
    void ProcessZoneTransitions(const FVector& PlayerLocation);
    UAudioComponent* CreateAudioComponent(EWorld_PrehistoricSoundType SoundType, const FVector& Location);
    void LoadDefaultSounds();
    float CalculateVolumeForLocation(const FVector& SoundLocation, const FVector& PlayerLocation, float BaseVolume) const;
};