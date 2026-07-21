#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../Narrative/Narr_SurvivalNarrativeCore.h"
#include "AudioTypes.h"
#include "Audio_NarrativeIntegration.generated.h"

UENUM(BlueprintType)
enum class EAudio_NarrativeIntensity : uint8
{
    Calm = 0        UMETA(DisplayName = "Calm"),
    Tension = 1     UMETA(DisplayName = "Tension"),
    Danger = 2      UMETA(DisplayName = "Danger"),
    Combat = 3      UMETA(DisplayName = "Combat"),
    Discovery = 4   UMETA(DisplayName = "Discovery")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    EAudio_NarrativeIntensity CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bIsInStoryLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float ThreatLevel;

    FAudio_NarrativeAudioState()
    {
        CurrentIntensity = EAudio_NarrativeIntensity::Calm;
        IntensityLevel = 0.0f;
        CurrentPhase = TEXT("Awakening");
        bIsInStoryLocation = false;
        ThreatLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TSoftObjectPtr<USoundCue> CalmMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TSoftObjectPtr<USoundCue> TensionMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TSoftObjectPtr<USoundCue> DangerMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TSoftObjectPtr<USoundCue> CombatMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TSoftObjectPtr<USoundCue> DiscoveryMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float IntensityThreshold;

    FAudio_NarrativeAudioConfig()
    {
        TransitionDuration = 2.0f;
        IntensityThreshold = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_NarrativeIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_NarrativeIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FAudio_NarrativeAudioState CurrentAudioState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FAudio_NarrativeAudioConfig AudioConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float UpdateInterval;

    FTimerHandle AudioUpdateTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void OnNarrativePhaseChanged(const FString& NewPhase, float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void OnThreatLevelChanged(float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void OnStoryLocationEntered(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void OnStoryLocationExited();

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void UpdateAudioIntensity();

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void TransitionToIntensity(EAudio_NarrativeIntensity NewIntensity);

    UFUNCTION(BlueprintPure, Category = "Narrative Audio")
    EAudio_NarrativeIntensity GetCurrentIntensity() const { return CurrentAudioState.CurrentIntensity; }

    UFUNCTION(BlueprintPure, Category = "Narrative Audio")
    float GetIntensityLevel() const { return CurrentAudioState.IntensityLevel; }

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetAmbienceVolume(float Volume);

private:
    void UpdateAudioState();
    void PlayMusicForIntensity(EAudio_NarrativeIntensity Intensity);
    USoundCue* GetSoundCueForIntensity(EAudio_NarrativeIntensity Intensity);
    void CrossfadeMusic(USoundCue* NewMusic);
};