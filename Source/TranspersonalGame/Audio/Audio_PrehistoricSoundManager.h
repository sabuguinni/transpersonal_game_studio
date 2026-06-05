#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "Engine/TriggerBox.h"
#include "Audio_PrehistoricSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest,
    Plains,
    Canyon,
    River,
    Cave,
    Volcanic
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe,
    Caution,
    Danger,
    Extreme
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime = 3.0f;

    FAudio_BiomeAudioData()
    {
        BaseVolume = 0.7f;
        FadeTime = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_ThreatAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatRadius = 1000.0f;

    FAudio_ThreatAudioData()
    {
        IntensityMultiplier = 1.0f;
        ThreatRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_PrehistoricSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_PrehistoricSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ThreatAudioComponent;

    // Biome audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    // Threat audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TMap<EAudio_ThreatLevel, FAudio_ThreatAudioData> ThreatAudioMap;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentThreatIntensity;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ThreatVolume = 0.9f;

    // Update intervals
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BiomeCheckInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ThreatCheckInterval = 0.5f;

private:
    float LastBiomeCheck;
    float LastThreatCheck;
    bool bIsTransitioning;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayDinosaurRoar(const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayFootstepSound(const FVector& Location, bool bIsLarge = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Audio Info")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio Info")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio Info")
    float GetThreatIntensity() const { return CurrentThreatIntensity; }

protected:
    // Internal methods
    void UpdateBiomeAudio();
    void UpdateThreatAudio();
    void TransitionToNewBiome(EAudio_BiomeType NewBiome);
    void TransitionToNewThreat(EAudio_ThreatLevel NewThreat, float Intensity);
    EAudio_BiomeType DetectCurrentBiome();
    EAudio_ThreatLevel DetectCurrentThreatLevel(float& OutIntensity);
    void InitializeAudioMaps();
};