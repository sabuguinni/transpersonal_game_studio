#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Audio_MetaSoundsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    class USoundBase* SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float PitchMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeOutTime;

    FAudio_SoundLayer()
    {
        SoundAsset = nullptr;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        bLooping = true;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_SoundLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_SoundLayer> WeatherLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float CrossfadeDistance;

    FAudio_BiomeAudioConfig()
    {
        BiomeType = EBiomeType::Temperate_Forest;
        MaxAudibleDistance = 2000.0f;
        CrossfadeDistance = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UAudioComponent*> AudioLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TArray<FAudio_BiomeAudioConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float GlobalVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float UpdateFrequency;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWeatherType CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float DistanceToPlayer;

private:
    float LastUpdateTime;
    class APawn* CachedPlayerPawn;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetBiomeAudio(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetWeatherAudio(EWeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateAudioLayers();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeInLayer(int32 LayerIndex, float FadeTime);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeOutLayer(int32 LayerIndex, float FadeTime);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetGlobalVolume(float NewVolume);

    UFUNCTION(BlueprintPure, Category = "Audio Query")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Audio Query")
    bool IsPlayerInRange() const;

protected:
    void InitializeAudioLayers();
    void UpdatePlayerDistance();
    FAudio_BiomeAudioConfig* GetBiomeConfig(EBiomeType BiomeType);
};