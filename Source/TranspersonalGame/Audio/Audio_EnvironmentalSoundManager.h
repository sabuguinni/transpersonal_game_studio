#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    class USoundCue* AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<class USoundWave*> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundInterval = 15.0f;

    FAudio_BiomeAudioSettings()
    {
        AmbientSoundCue = nullptr;
        BaseVolume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        RandomSoundInterval = 15.0f;
    }
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert"),
    Cave        UMETA(DisplayName = "Cave"),
    River       UMETA(DisplayName = "River")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SecondaryAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioSettings> BiomeAudioSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TransitionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeDetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableRandomSounds = true;

private:
    float RandomSoundTimer;
    bool bPrimaryActive;
    float FadeTimer;
    float FadeDuration;
    EAudio_BiomeType TargetBiome;

public:
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayRandomBiomeSound();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    EAudio_BiomeType DetectCurrentBiome();

    UFUNCTION(BlueprintPure, Category = "Environmental Audio")
    float GetCurrentVolume() const;

    UFUNCTION(BlueprintPure, Category = "Environmental Audio")
    bool IsTransitioning() const { return bIsTransitioning; }

protected:
    UFUNCTION()
    void UpdateBiomeTransition(float DeltaTime);

    UFUNCTION()
    void StartBiomeTransition(EAudio_BiomeType NewBiome);

    UFUNCTION()
    void CompleteBiomeTransition();

    UFUNCTION()
    void UpdateRandomSounds(float DeltaTime);
};