#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Audio_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundCue* PrimaryAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundCue* SecondaryAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundCue* WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AudioRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    bool bUseDistanceAttenuation;

    FAudio_BiomeAudioData()
    {
        PrimaryAmbientSound = nullptr;
        SecondaryAmbientSound = nullptr;
        WeatherSound = nullptr;
        AmbientVolume = 0.7f;
        AudioRange = 15000.0f;
        bUseDistanceAttenuation = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* SecondaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WeatherAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EBiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bAutoDetectBiome;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void TransitionToBiome(EBiomeType TargetBiome, float TransitionDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetWeatherAudio(USoundCue* WeatherSound, float Volume = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void StopAllAudio();

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    EBiomeType DetectBiomeFromLocation(FVector WorldLocation);

    UFUNCTION(BlueprintPure, Category = "Biome Audio")
    bool IsAudioPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void InitializeBiomeAudioData();

protected:
    UFUNCTION()
    void OnAudioFinished();

    void UpdateAudioComponents();
    void FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float Duration);

private:
    bool bIsTransitioning;
    float TransitionTimer;
    float TransitionDuration;
    EBiomeType TransitionTargetBiome;
    
    // Biome detection areas (center coordinates)
    TMap<EBiomeType, FVector> BiomeCenters;
    
    void SetupBiomeCenters();
    float CalculateDistanceToBiomeCenter(EBiomeType Biome, FVector Location);
};