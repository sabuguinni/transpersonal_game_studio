#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"), 
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Extreme     UMETA(DisplayName = "Extreme Danger")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> WindLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> CreatureLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatVolumeMultiplier = 1.5f;

    FAudio_BiomeAudioProfile()
    {
        BaseVolume = 0.7f;
        ThreatVolumeMultiplier = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_ThreatAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    FAudio_ThreatAudioData()
    {
        TriggerDistance = 2000.0f;
        Volume = 1.0f;
        bLooping = false;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_EnvironmentalSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_EnvironmentalSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Biome Audio Management
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayThreatSound(const FString& ThreatType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopAllThreatSounds();

    // Day/Night Cycle Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetTimeOfDay(float NormalizedTime); // 0.0 = midnight, 0.5 = noon

    // Weather Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetWeatherIntensity(float Intensity); // 0.0 = clear, 1.0 = storm

protected:
    // Biome Audio Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profiles")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioProfile> BiomeProfiles;

    // Threat Audio Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TMap<FString, FAudio_ThreatAudioData> ThreatSounds;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> WindAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> CreatureAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> ThreatAudioComponent;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentWeatherIntensity;

private:
    void InitializeBiomeProfiles();
    void InitializeThreatSounds();
    void UpdateAmbientAudio();
    void UpdateVolumeBasedOnThreat();
    float CalculateVolumeMultiplier() const;
    
    // Timer for periodic audio updates
    FTimerHandle AudioUpdateTimer;
    void PeriodicAudioUpdate();
};