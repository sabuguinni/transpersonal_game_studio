#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "AudioSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping;

    FAudio_BiomeAudioData()
    {
        AmbientSound = nullptr;
        Volume = 1.0f;
        FadeTime = 2.0f;
        bLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepVolume;

    FAudio_DinosaurAudioData()
    {
        RoarSound = nullptr;
        FootstepSound = nullptr;
        BreathingSound = nullptr;
        ThreatRadius = 5000.0f;
        FootstepVolume = 0.8f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateBiomeAudio(EBiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurAudio(EDinosaurSpecies Species, const FVector& DinosaurLocation, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepAudio(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDamageAudio(float DamageAmount, const FVector& ImpactLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSFXVolume(float Volume);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EDinosaurSpecies, FAudio_DinosaurAudioData> DinosaurAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    USoundCue* PlayerFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    USoundCue* DamageSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    USoundCue* HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float SFXVolume;

private:
    UPROPERTY()
    UAudioComponent* CurrentAmbientComponent;

    UPROPERTY()
    EBiomeType CurrentBiome;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    void CleanupInactiveComponents();
    UAudioComponent* CreateAudioComponent(USoundCue* SoundCue, const FVector& Location, float Volume, bool bLooping = false);
    float CalculateDistanceAttenuation(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance);
};