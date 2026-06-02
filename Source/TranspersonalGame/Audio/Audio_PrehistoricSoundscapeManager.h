#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Audio_PrehistoricSoundscapeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeAudioType : uint8
{
    Forest          UMETA(DisplayName = "Dense Forest"),
    Plains          UMETA(DisplayName = "Open Plains"),
    Swamp           UMETA(DisplayName = "Wetland Swamp"),
    Mountain        UMETA(DisplayName = "Rocky Mountains"),
    River           UMETA(DisplayName = "River Valley"),
    Cave            UMETA(DisplayName = "Underground Cave")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe Zone"),
    Caution         UMETA(DisplayName = "Caution Required"),
    Danger          UMETA(DisplayName = "Active Threat"),
    Extreme         UMETA(DisplayName = "Extreme Danger")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeSoundscape
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeAudioType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> InsectSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> BirdCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> WindSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeDistance = 2000.0f;

    FAudio_BiomeSoundscape()
    {
        BiomeType = EAudio_BiomeAudioType::Forest;
        BaseVolume = 0.7f;
        FadeDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_DangerAudioCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_DangerLevel DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> HeartbeatIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioPriority = 1.0f;

    FAudio_DangerAudioCue()
    {
        DangerLevel = EAudio_DangerLevel::Safe;
        TriggerDistance = 1500.0f;
        AudioPriority = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_PrehistoricSoundscapeManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_PrehistoricSoundscapeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* BiomeAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* DangerAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* NarrativeAudioComponent;

    // Biome soundscape configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_BiomeSoundscape> BiomeSoundscapes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeAudioType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BiomeTransitionTime = 3.0f;

    // Danger audio system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger Audio")
    TArray<FAudio_DangerAudioCue> DangerAudioCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger Audio")
    EAudio_DangerLevel CurrentDangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger Audio")
    float DangerDetectionRadius = 2500.0f;

    // Narrative voice integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TMap<FString, TSoftObjectPtr<USoundWave>> NarrativeVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bNarrativeAudioEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float NarrativeVolume = 0.8f;

public:
    // Biome audio control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetBiomeAudio(EAudio_BiomeAudioType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void TransitionToBiome(EAudio_BiomeAudioType TargetBiome, float TransitionDuration = 3.0f);

    // Danger audio system
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetDangerLevel(EAudio_DangerLevel NewDangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void TriggerDangerAudio(EAudio_DangerLevel DangerLevel, FVector ThreatLocation);

    // Narrative voice playback
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayNarrativeVoiceLine(const FString& VoiceLineKey);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StopNarrativeAudio();

    // Audio state queries
    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_BiomeAudioType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    bool IsNarrativeAudioPlaying() const;

private:
    // Internal audio management
    void UpdateBiomeAudio(float DeltaTime);
    void UpdateDangerAudio(float DeltaTime);
    void DetectNearbyThreats();
    
    FAudio_BiomeSoundscape* GetBiomeSoundscape(EAudio_BiomeAudioType BiomeType);
    FAudio_DangerAudioCue* GetDangerAudioCue(EAudio_DangerLevel DangerLevel);

    // Audio transition state
    bool bTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 3.0f;
    EAudio_BiomeAudioType TransitionTargetBiome;
    
    // Cached player reference
    UPROPERTY()
    class APawn* CachedPlayer;
    
    // Audio update timers
    float BiomeUpdateTimer = 0.0f;
    float DangerUpdateTimer = 0.0f;
    const float BiomeUpdateInterval = 0.5f;
    const float DangerUpdateInterval = 0.1f;
};