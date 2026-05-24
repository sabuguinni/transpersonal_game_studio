#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "NarrativeVoiceManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_VoiceType : uint8
{
    PrimordialNarrator      UMETA(DisplayName = "Primordial Narrator"),
    SpiritGuide            UMETA(DisplayName = "Spirit Guide"),
    AncientWisdom          UMETA(DisplayName = "Ancient Wisdom"),
    ConsciousnessVoice     UMETA(DisplayName = "Consciousness Voice"),
    NatureSpirit           UMETA(DisplayName = "Nature Spirit")
};

UENUM(BlueprintType)
enum class EAudio_NarrativeContext : uint8
{
    GameIntro              UMETA(DisplayName = "Game Introduction"),
    LevelTransition        UMETA(DisplayName = "Level Transition"),
    ConsciousnessShift     UMETA(DisplayName = "Consciousness Shift"),
    SpiritualGuidance      UMETA(DisplayName = "Spiritual Guidance"),
    AncientMemory          UMETA(DisplayName = "Ancient Memory"),
    Transformation         UMETA(DisplayName = "Transformation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    EAudio_VoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    EAudio_NarrativeContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    TSoftObjectPtr<USoundWave> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float SpiritualResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    bool bIsLoopable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float FadeInDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float FadeOutDuration;

    FAudio_VoiceLine()
    {
        LineID = TEXT("");
        NarrativeText = FText::GetEmpty();
        VoiceType = EAudio_VoiceType::PrimordialNarrator;
        Context = EAudio_NarrativeContext::GameIntro;
        EmotionalIntensity = 0.5f;
        SpiritualResonance = 0.5f;
        bIsLoopable = false;
        FadeInDuration = 1.0f;
        FadeOutDuration = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_VoiceSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Sequence")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Sequence")
    TArray<FAudio_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Sequence")
    float DelayBetweenLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Sequence")
    bool bAutoAdvance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Sequence")
    EAudio_NarrativeContext SequenceContext;

    FAudio_VoiceSequence()
    {
        SequenceID = TEXT("");
        DelayBetweenLines = 2.0f;
        bAutoAdvance = true;
        SequenceContext = EAudio_NarrativeContext::GameIntro;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceLineStarted, const FString&, LineID, EAudio_VoiceType, VoiceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceLineCompleted, const FString&, LineID, EAudio_VoiceType, VoiceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceSequenceCompleted, const FString&, SequenceID, EAudio_NarrativeContext, Context);

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeVoiceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeVoiceManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Voice Line Management
    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void PlayVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void PlayVoiceLineByID(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void PlayVoiceSequence(const FAudio_VoiceSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void PlayVoiceSequenceByID(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void StopCurrentVoice();

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void StopAllVoices();

    // Voice Configuration
    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void RegisterVoiceSequence(const FAudio_VoiceSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void SetVoiceVolume(EAudio_VoiceType VoiceType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void SetGlobalNarrativeVolume(float Volume);

    // Context-Based Voice Selection
    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    void PlayContextualVoice(EAudio_NarrativeContext Context, float SpiritualIntensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Narrative Voice")
    TArray<FAudio_VoiceLine> GetVoicesForContext(EAudio_NarrativeContext Context) const;

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Narrative Voice")
    bool IsVoicePlaying() const;

    UFUNCTION(BlueprintPure, Category = "Narrative Voice")
    EAudio_VoiceType GetCurrentVoiceType() const;

    UFUNCTION(BlueprintPure, Category = "Narrative Voice")
    FString GetCurrentLineID() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative Voice Events")
    FOnVoiceLineStarted OnVoiceLineStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Voice Events")
    FOnVoiceLineCompleted OnVoiceLineCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Voice Events")
    FOnVoiceSequenceCompleted OnVoiceSequenceCompleted;

protected:
    // Voice Line Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Configuration")
    TMap<FString, FAudio_VoiceLine> RegisteredVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Configuration")
    TMap<FString, FAudio_VoiceSequence> RegisteredSequences;

    // Audio Components for different voice types
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudio_VoiceType, UAudioComponent*> VoiceAudioComponents;

    // Current playback state
    UPROPERTY(BlueprintReadOnly, Category = "Playback State")
    FAudio_VoiceLine CurrentVoiceLine;

    UPROPERTY(BlueprintReadOnly, Category = "Playback State")
    FAudio_VoiceSequence CurrentSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Playback State")
    int32 CurrentSequenceIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Playback State")
    bool bIsPlayingSequence;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_VoiceType, float> VoiceTypeVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float GlobalNarrativeVolume;

    // Timing
    UPROPERTY()
    float SequenceTimer;

    UPROPERTY()
    bool bWaitingForNextLine;

private:
    void InitializeVoiceComponents();
    void PlayNextLineInSequence();
    UAudioComponent* GetAudioComponentForVoiceType(EAudio_VoiceType VoiceType);
    void OnVoiceLineFinished();

    UFUNCTION()
    void HandleAudioFinished();
};