#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue system for survival-focused narrative
// NO spiritual/mystical content - only practical survival dialogue

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    FieldNotes          UMETA(DisplayName = "Field Notes"),
    SafetyAlert         UMETA(DisplayName = "Safety Alert"),
    WeatherWarning      UMETA(DisplayName = "Weather Warning"),
    Discovery           UMETA(DisplayName = "Discovery"),
    ThreatAlert         UMETA(DisplayName = "Threat Alert"),
    SurvivalTip         UMETA(DisplayName = "Survival Tip"),
    ResourceInfo        UMETA(DisplayName = "Resource Info"),
    BiomeDescription    UMETA(DisplayName = "Biome Description")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    Paleontologist      UMETA(DisplayName = "Paleontologist"),
    SafetyOfficer       UMETA(DisplayName = "Safety Officer"),
    WeatherStation      UMETA(DisplayName = "Weather Station"),
    FieldGuide          UMETA(DisplayName = "Field Guide"),
    EmergencySystem     UMETA(DisplayName = "Emergency System"),
    Narrator            UMETA(DisplayName = "Narrator")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SpeakerType Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType RelevantBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_ThreatLevel UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> TriggerConditions;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        DialogueType = ENarr_DialogueType::FieldNotes;
        Speaker = ENarr_SpeakerType::Paleontologist;
        DialogueText = "";
        AudioFilePath = "";
        Duration = 0.0f;
        RelevantBiome = EEng_BiomeType::Forest;
        UrgencyLevel = EEng_ThreatLevel::Safe;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DelayBetweenEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bLooping;

    FNarr_DialogueSequence()
    {
        SequenceID = "";
        bAutoPlay = false;
        DelayBetweenEntries = 2.0f;
        bLooping = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core dialogue system functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PauseDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResumeDialogue();

    // Contextual dialogue triggers
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerBiomeDialogue(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerThreatDialogue(EEng_ThreatLevel ThreatLevel, EEng_DinosaurSpecies DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerWeatherDialogue(EEng_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDiscoveryDialogue(const FString& DiscoveryType);

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& DialogueSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentDialogueID() const;

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetAudioVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetSubtitlesEnabled(bool bEnabled);

protected:
    // Dialogue data storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueSequence> SequenceDatabase;

    // Current playback state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPaused;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float CurrentPlaybackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float TotalDialogueDuration;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bSubtitlesEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bAutoPlayContextualDialogue;

    // Audio component for playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

private:
    // Internal dialogue processing
    void ProcessDialogueQueue();
    void OnDialogueFinished();
    bool CheckDialogueTriggerConditions(const FNarr_DialogueEntry& DialogueEntry);
    void LoadDialogueAudio(const FString& AudioFilePath);

    // Dialogue queue for sequences
    TArray<FNarr_DialogueEntry> DialogueQueue;
    int32 CurrentQueueIndex;
    float SequenceTimer;
    float DelayTimer;
    bool bProcessingSequence;
};