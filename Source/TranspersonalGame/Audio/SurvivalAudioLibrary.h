#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "SurvivalAudioLibrary.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentalSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    EEnvironmentalAudioType AudioType;

    FAudio_EnvironmentalSound()
    {
        BaseVolume = 1.0f;
        MinDistance = 400.0f;
        MaxDistance = 4000.0f;
        bIsLooping = true;
        AudioType = EEnvironmentalAudioType::Forest;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SurvivalFeedback
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> HungerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> ThirstSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> FearSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float IntensityThreshold;

    FAudio_SurvivalFeedback()
    {
        IntensityThreshold = 0.7f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API USurvivalAudioLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Survival Audio", meta = (CallInEditor))
    static void PlayEnvironmentalSound(const FVector& Location, EEnvironmentalAudioType SoundType, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void PlaySurvivalFeedbackSound(ESurvivalStat StatType, float StatValue, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static UAudioComponent* CreateDynamicAudioComponent(AActor* Owner, USoundBase* Sound, bool bAutoPlay = true);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void UpdateAudioIntensityBasedOnThreat(float ThreatLevel, UAudioComponent* AudioComponent);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void PlayFootstepSound(const FVector& Location, ETerrainType TerrainType, bool bIsRunning = false);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void PlayCraftingSound(ECraftingAction ActionType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void SetupAmbientAudioZone(const FVector& Center, float Radius, EEnvironmentalAudioType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void StopAmbientSoundsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio", meta = (CallInEditor))
    static void InitializePrehistoricSoundLibrary();

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    static void PlayWeatherSound(EWeatherType WeatherType, float Intensity, const FVector& Location);

private:
    static TMap<EEnvironmentalAudioType, FAudio_EnvironmentalSound> EnvironmentalSoundMap;
    static FAudio_SurvivalFeedback SurvivalFeedbackSounds;
    static bool bIsInitialized;

    static void LoadEnvironmentalSounds();
    static void LoadSurvivalFeedbackSounds();
    static USoundCue* GetSoundForTerrainType(ETerrainType TerrainType, bool bIsRunning);
    static USoundCue* GetCraftingSoundForAction(ECraftingAction ActionType);
};