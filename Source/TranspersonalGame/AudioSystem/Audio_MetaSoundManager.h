#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "SharedTypes.h"
#include "Audio_MetaSoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EffectsVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EBiomeType CurrentBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float ThreatLevel = 0.0f; // 0-1, affects music intensity
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float TriggerDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float MaxDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    FString AudioEventName;
};

/**
 * MetaSound-based audio manager for dynamic prehistoric soundscapes
 * Handles adaptive music, environmental audio, and proximity-based sound triggers
 */
UCLASS()
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Environment audio control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetEnvironmentSettings(const FAudio_EnvironmentSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateBiomeAudio(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(float Level);

    // Proximity audio system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterProximityTrigger(AActor* TriggerActor, const FAudio_ProximityTrigger& TriggerData);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterProximityTrigger(AActor* TriggerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateProximityAudio(const FVector& PlayerLocation);

    // Dynamic music system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAdaptiveMusic(const FString& MusicLayer);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CrossfadeToLayer(const FString& NewLayer, float FadeTime = 2.0f);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalEffect(const FString& EffectName, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartWeatherAudio(EWeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopWeatherAudio();

    // Narration system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayNarration(const FString& NarrationKey, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopNarration();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsNarrationPlaying() const;

protected:
    // Audio components
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    UAudioComponent* EffectsAudioComponent;

    UPROPERTY()
    UAudioComponent* NarrationAudioComponent;

    // Current environment settings
    UPROPERTY()
    FAudio_EnvironmentSettings CurrentEnvironment;

    // Proximity triggers
    UPROPERTY()
    TMap<AActor*, FAudio_ProximityTrigger> ProximityTriggers;

    // MetaSound assets
    UPROPERTY()
    TMap<FString, UMetaSoundSource*> MetaSoundAssets;

    // Internal methods
    void LoadMetaSoundAssets();
    void UpdateAmbientAudio();
    void UpdateMusicIntensity();
    float CalculateProximityVolume(float Distance, const FAudio_ProximityTrigger& Trigger) const;
    UMetaSoundSource* GetMetaSoundAsset(const FString& AssetName) const;
};