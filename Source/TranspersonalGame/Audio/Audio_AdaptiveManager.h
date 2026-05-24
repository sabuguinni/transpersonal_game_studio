#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveManager.generated.h"

// Audio intensity levels for adaptive music
UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm = 0,
    Tense = 1,
    Danger = 2,
    Combat = 3,
    Terror = 4
};

// Audio biome types for environmental sounds
UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest = 0,
    Swamp = 1,
    Savanna = 2,
    Desert = 3,
    SnowyMountain = 4
};

// Audio event configuration
USTRUCT(BlueprintType)
struct FAudio_EventConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    float FadeInTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Event")
    float FadeOutTime = 1.0f;

    FAudio_EventConfig()
    {
        EventName = TEXT("DefaultEvent");
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = false;
        FadeInTime = 0.5f;
        FadeOutTime = 1.0f;
    }
};

// Adaptive music layer
USTRUCT(BlueprintType)
struct FAudio_MusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Layer")
    EAudio_IntensityLevel IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Layer")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Layer")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Layer")
    float CrossfadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Layer")
    bool bIsActive = false;

    FAudio_MusicLayer()
    {
        IntensityLevel = EAudio_IntensityLevel::Calm;
        BaseVolume = 0.7f;
        CrossfadeTime = 2.0f;
        bIsActive = false;
    }
};

/**
 * Adaptive Audio Manager - Controls dynamic music and environmental audio
 * Responds to player fear level, dinosaur proximity, and biome changes
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === ADAPTIVE MUSIC SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetIntensityLevel(EAudio_IntensityLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateMusicBasedOnFear(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void TriggerDinosaurProximityMusic(float Distance, bool bIsLargeDinosaur);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetBiomeAmbience(EAudio_BiomeType BiomeType);

    // === AUDIO EVENT SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Audio Events")
    void PlayAudioEvent(const FString& EventName, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio Events")
    void StopAudioEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Audio Events")
    void RegisterAudioEvent(const FAudio_EventConfig& EventConfig);

    // === ENVIRONMENTAL AUDIO ===
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(float WindIntensity, float RainIntensity, bool bThunderActive);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayFootstepSound(FVector Location, bool bIsPlayerFootstep);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayDinosaurVocalization(FVector Location, const FString& DinosaurType, float IntensityLevel);

    // === NARRATIVE AUDIO INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeVoiceLine(const FString& VoiceLineID, float DelayTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetNarrativeAudioVolume(float VolumeMultiplier);

protected:
    // === MUSIC LAYERS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TArray<FAudio_MusicLayer> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    EAudio_IntensityLevel CurrentIntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float IntensityChangeThreshold = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float MusicUpdateInterval = 0.5f;

    // === AUDIO COMPONENTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SFXAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* VoiceAudioComponent;

    // === BIOME AUDIO ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, TSoftObjectPtr<USoundCue>> BiomeAmbienceTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType CurrentBiome;

    // === AUDIO EVENTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Events")
    TMap<FString, FAudio_EventConfig> RegisteredAudioEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Events")
    TMap<FString, UAudioComponent*> ActiveAudioEvents;

    // === INTERNAL STATE ===
    
    UPROPERTY()
    float LastMusicUpdateTime;

    UPROPERTY()
    float CurrentFearLevel;

    UPROPERTY()
    bool bIsNearLargeDinosaur;

    UPROPERTY()
    float DinosaurProximityDistance;

private:
    void InitializeAudioComponents();
    void UpdateAdaptiveMusic(float DeltaTime);
    void CrossfadeToMusicLayer(const FAudio_MusicLayer& NewLayer);
    EAudio_IntensityLevel CalculateIntensityFromFear(float FearLevel);
    void CleanupFinishedAudioEvents();
};