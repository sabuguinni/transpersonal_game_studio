#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    None = 0,
    TribalElder,
    ScoutWarrior,
    HuntLeader,
    ShamanNarrator,
    QuestGiver,
    Merchant,
    Warning,
    Discovery,
    Combat,
    Environmental
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Manual = 0,
    Proximity,
    QuestStart,
    QuestComplete,
    CombatStart,
    CombatEnd,
    Discovery,
    TimeOfDay,
    Weather,
    PlayerHealth
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    float MinPlayerHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bRequiresDayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float Cooldown;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        DialogueText = FText::GetEmpty();
        SpeakerType = ENarr_DialogueType::None;
        CharacterName = "";
        TriggerType = ENarr_DialogueTrigger::Manual;
        MinPlayerHealth = 0.0f;
        bRequiresDayTime = false;
        Duration = 5.0f;
        Cooldown = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ActiveDialogue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    FNarr_DialogueEntry DialogueData;

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    float StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    AActor* TriggeringActor;

    FNarr_ActiveDialogue()
    {
        StartTime = 0.0f;
        bIsPlaying = false;
        TriggeringActor = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FNarr_DialogueEntry&, DialogueEntry, AActor*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueFinished, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueInterrupted, const FString&, DialogueID, const FString&, Reason);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Dialogue Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue(const FString& Reason = "Manual Stop");

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_ActiveDialogue GetCurrentDialogue() const;

    // Dialogue Registration
    UFUNCTION(BlueprintCallable, Category = "Dialogue Setup")
    void RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Setup")
    void LoadDialogueTable(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Setup")
    void RegisterPresetDialogues();

    // Trigger System
    UFUNCTION(BlueprintCallable, Category = "Dialogue Triggers")
    void CheckProximityTriggers(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Triggers")
    void TriggerQuestDialogue(const FString& QuestID, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Triggers")
    void TriggerCombatDialogue(bool bCombatStart, AActor* Enemy = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Triggers")
    void TriggerEnvironmentalDialogue(const FString& EnvironmentType);

    // Condition Checking
    UFUNCTION(BlueprintCallable, Category = "Dialogue Conditions")
    bool CanPlayDialogue(const FNarr_DialogueEntry& DialogueEntry) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Conditions")
    bool IsDialogueOnCooldown(const FString& DialogueID) const;

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopDialogueAudio();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueFinished OnDialogueFinished;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueInterrupted OnDialogueInterrupted;

protected:
    // Internal Data
    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> RegisteredDialogues;

    UPROPERTY()
    FNarr_ActiveDialogue CurrentDialogue;

    UPROPERTY()
    TMap<FString, float> DialogueCooldowns;

    UPROPERTY()
    class UAudioComponent* DialogueAudioComponent;

    // Timer Handles
    FTimerHandle DialogueTimerHandle;
    FTimerHandle CooldownTimerHandle;

    // Internal Functions
    void OnDialogueTimerComplete();
    void UpdateCooldowns();
    void CleanupFinishedDialogue();

private:
    bool bIsInitialized;
    float LastDialogueTime;
};