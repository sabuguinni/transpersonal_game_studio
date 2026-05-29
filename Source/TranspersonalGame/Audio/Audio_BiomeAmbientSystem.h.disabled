#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Audio_BiomeAmbientSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Cave        UMETA(DisplayName = "Cave"),
    River       UMETA(DisplayName = "River")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    TArray<TSoftObjectPtr<USoundCue>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float RandomSoundInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float RandomSoundVariance = 10.0f;

    FAudio_BiomeAudioProfile()
    {
        BaseVolume = 0.7f;
        FadeDistance = 2000.0f;
        RandomSoundInterval = 15.0f;
        RandomSoundVariance = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_BiomeAmbientSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_BiomeAmbientSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* RandomAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profiles")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioProfile> BiomeAudioProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudibleDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bUseTimeOfDayVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float NightVolumeMultiplier = 0.6f;

private:
    float RandomSoundTimer = 0.0f;
    float NextRandomSoundTime = 0.0f;
    bool bIsPlayerInRange = false;
    float CurrentVolumeMultiplier = 1.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetBiomeType(EAudio_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateAudioBasedOnPlayerDistance(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void PlayRandomBiomeSound();

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetTimeOfDayMultiplier(float TimeMultiplier);

private:
    void InitializeBiomeProfiles();
    void UpdateAmbientAudio();
    void UpdateRandomSounds(float DeltaTime);
    float CalculateDistanceVolume(FVector PlayerLocation);
    void LoadAudioAssets();
};