#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundBase> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundBase> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundBase>> RandomNatureSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundInterval = 15.0f;

    FAudio_BiomeAudioSettings()
    {
        AmbientVolume = 0.7f;
        RandomSoundInterval = 15.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityAudioTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    TSoftObjectPtr<USoundBase> ProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float Cooldown = 30.0f;

    FAudio_ProximityAudioTrigger()
    {
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        Volume = 1.0f;
        bIsActive = true;
        Cooldown = 30.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_EnvironmentalSoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetCurrentBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdatePlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayProximitySound(const FString& TriggerName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterEnvironmentalVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void AddProximityTrigger(const FString& TriggerName, const FAudio_ProximityAudioTrigger& Trigger);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void RemoveProximityTrigger(const FString& TriggerName);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetWeatherIntensity(float Intensity); // 0.0 = clear, 1.0 = storm

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Settings")
    TMap<EBiomeType, FAudio_BiomeAudioSettings> BiomeAudioSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Proximity Triggers")
    TMap<FString, FAudio_ProximityAudioTrigger> ProximityTriggers;

    UPROPERTY()
    TMap<FString, float> TriggerCooldowns;

    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* WindAudioComponent;

    UPROPERTY()
    UAudioComponent* WeatherAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float MasterEnvironmentalVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float CurrentWeatherIntensity = 0.0f;

    UPROPERTY()
    EBiomeType CurrentBiome = EBiomeType::Forest;

    UPROPERTY()
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY()
    FTimerHandle RandomSoundTimer;

private:
    void InitializeBiomeSettings();
    void PlayRandomNatureSound();
    void UpdateProximityTriggers(const FVector& PlayerLocation);
    void UpdateTimeBasedAudio();
    void UpdateWeatherAudio();
    UAudioComponent* CreateAudioComponent(const FString& ComponentName);
};