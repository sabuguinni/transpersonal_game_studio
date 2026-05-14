#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundWave>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MinRandomInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MaxRandomInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float VolumeMultiplier = 1.0f;

    FAudio_BiomeAudioData()
    {
        MinRandomInterval = 10.0f;
        MaxRandomInterval = 30.0f;
        VolumeMultiplier = 1.0f;
    }
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Cave        UMETA(DisplayName = "Cave")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* RandomSoundComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* WeatherAudioComponent;

    // Biome audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Forest;

    // Weather audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> RainSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> ThunderSound;

    // Audio control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EffectsVolume = 0.8f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayRandomBiomeSound();

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StartRain(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StopRain();

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void PlayThunder();

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetEffectsVolume(float Volume);

private:
    // Timer handles
    FTimerHandle RandomSoundTimer;
    
    // Internal functions
    void InitializeBiomeAudio();
    void UpdateAmbientAudio();
    void ScheduleNextRandomSound();
    void LoadAudioAssets();

    // Current state
    bool bIsRaining = false;
    float CurrentRainIntensity = 0.0f;
};