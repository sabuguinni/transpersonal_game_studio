#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    None = 0,
    TribalElder,
    ScoutWarning,
    AncientWisdom,
    CombatNarrator,
    EnvironmentalHint,
    DangerAlert,
    QuestGiver,
    Storyteller
};

UENUM(BlueprintType)
enum class ENarr_TriggerCondition : uint8
{
    None = 0,
    PlayerEnterArea,
    DinosaurNearby,
    LowHealth,
    FirstTime,
    QuestActive,
    TimeOfDay,
    WeatherChange,
    CombatStarted
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsUrgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_TriggerCondition TriggerCondition;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        SpeakerType = ENarr_DialogueType::None;
        AudioURL = TEXT("");
        Duration = 0.0f;
        bIsUrgent = false;
        TriggerCondition = ENarr_TriggerCondition::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 Priority;

    FNarr_NarrativeEvent()
    {
        EventName = TEXT("");
        bPlayOnce = true;
        CooldownTime = 30.0f;
        Priority = 1;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventName, ENarr_TriggerCondition Condition);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanPlayEvent(const FString& EventName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayRandomDialogue(ENarr_DialogueType SpeakerType);

    // Event management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkEventPlayed(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetAllEvents();

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_NarrativeEvent> RegisteredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, float> EventCooldowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSet<FString> PlayedOnceEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bDialogueEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCurrentlyPlayingDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CurrentDialogueEndTime;

    // Predefined dialogue lines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> TribalElderLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> ScoutWarningLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> AncientWisdomLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> CombatNarratorLines;

private:
    void InitializeDialogueLines();
    void UpdateDialogueState();
    FNarr_DialogueLine GetRandomDialogue(ENarr_DialogueType SpeakerType) const;
};