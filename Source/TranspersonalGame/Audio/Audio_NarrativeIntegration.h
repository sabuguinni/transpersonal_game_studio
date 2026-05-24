#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Actor.h"
#include "Audio_NarrativeIntegration.generated.h"

UENUM(BlueprintType)
enum class EAudio_NarrativeContext : uint8
{
    Atmospheric     UMETA(DisplayName = "Atmospheric"),
    Survival        UMETA(DisplayName = "Survival"),
    Danger          UMETA(DisplayName = "Danger"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Crafting        UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    EAudio_NarrativeContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Volume;

    FAudio_NarrativeClip()
    {
        ClipName = TEXT("");
        AudioURL = TEXT("");
        Context = EAudio_NarrativeContext::Atmospheric;
        Duration = 0.0f;
        bIsLooping = false;
        Volume = 1.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_NarrativeIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_NarrativeIntegration();

protected:
    virtual void BeginPlay() override;

    // Narrative audio clips database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TArray<FAudio_NarrativeClip> NarrativeClips;

    // Audio components for playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> SecondaryAudioComponent;

    // Current playback state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsPlayingNarrative;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_NarrativeContext CurrentContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeInDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeOutDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SpatialRange;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core narrative audio functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeClip(const FString& ClipName, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayContextualNarrative(EAudio_NarrativeContext Context, bool bRandomSelection = true);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarrativeAudio(bool bFadeOut = true);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void RegisterNarrativeClip(const FAudio_NarrativeClip& NewClip);

    // Integration with narrative triggers
    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void OnNarrativeTriggerEntered(const FString& TriggerID, EAudio_NarrativeContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative Integration")
    void OnNarrativeTriggerExited(const FString& TriggerID);

    // Audio state queries
    UFUNCTION(BlueprintPure, Category = "Audio State")
    bool IsPlayingNarrative() const { return bIsPlayingNarrative; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_NarrativeContext GetCurrentContext() const { return CurrentContext; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    TArray<FAudio_NarrativeClip> GetClipsByContext(EAudio_NarrativeContext Context) const;

private:
    // Internal audio management
    void InitializeAudioComponents();
    void LoadPredefinedNarrativeClips();
    FAudio_NarrativeClip* FindClipByName(const FString& ClipName);
    void HandleAudioFinished();

    // Fade management
    void StartFadeIn(UAudioComponent* AudioComp, float Duration);
    void StartFadeOut(UAudioComponent* AudioComp, float Duration);

    // Current fade state
    bool bIsFading;
    float FadeTimer;
    float FadeTargetDuration;
    UAudioComponent* FadingComponent;
    bool bFadeIn;
};