#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Narrator        UMETA(DisplayName = "Survival Narrator"),
    Warning         UMETA(DisplayName = "Danger Warning"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TrackerGuide    UMETA(DisplayName = "Tracker Guide"),
    SystemMessage   UMETA(DisplayName = "System Message")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    PlayerProximity     UMETA(DisplayName = "Player Proximity"),
    DinosaurSighting    UMETA(DisplayName = "Dinosaur Sighting"),
    DangerDetected      UMETA(DisplayName = "Danger Detected"),
    LocationDiscovered  UMETA(DisplayName = "Location Discovered"),
    QuestProgress       UMETA(DisplayName = "Quest Progress")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueEntry()
    {
        DialogueText = TEXT("");
        VoiceType = ENarr_DialogueType::Narrator;
        AudioCue = nullptr;
        DisplayDuration = 5.0f;
        Priority = 1;
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
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        TriggerType = ENarr_DialogueTrigger::PlayerProximity;
        bPlayOnce = false;
        CooldownTime = 30.0f;
    }
};

/**
 * Dialogue System for prehistoric survival narrative
 * Manages contextual voice lines, warnings, and survival guidance
 */
UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FString& SequenceID, bool bForcePlay = false);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueEntry(const FNarr_DialogueEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    // Trigger-based dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerProximityDialogue(const FVector& PlayerLocation, const FVector& TriggerLocation, float Range = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDangerWarning(const FString& DinosaurType, const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerLocationDiscovery(const FString& LocationName);

    // Sequence management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UnregisterDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueSequence GetDialogueSequence(const FString& SequenceID) const;

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetVoiceTypeVolume(ENarr_DialogueType VoiceType, float Volume);

protected:
    // Internal dialogue management
    void InitializeDefaultSequences();
    void UpdateDialogueQueue(float DeltaTime);
    bool CanPlayDialogue(const FString& SequenceID) const;
    void OnDialogueFinished();

    // Audio component management
    UAudioComponent* GetOrCreateAudioComponent();
    void CleanupAudioComponents();

private:
    // Dialogue storage
    UPROPERTY()
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY()
    TArray<FNarr_DialogueEntry> DialogueQueue;

    UPROPERTY()
    FNarr_DialogueEntry CurrentDialogue;

    // Audio components
    UPROPERTY()
    TArray<UAudioComponent*> AudioComponents;

    UPROPERTY()
    UAudioComponent* CurrentAudioComponent;

    // Playback state
    bool bIsPlaying;
    float CurrentDialogueTime;
    int32 CurrentQueueIndex;

    // Cooldown tracking
    TMap<FString, float> SequenceCooldowns;
    TSet<FString> PlayedOnceSequences;

    // Volume settings
    float MasterVolume;
    TMap<ENarr_DialogueType, float> VoiceTypeVolumes;

    // Timing
    float LastUpdateTime;
};