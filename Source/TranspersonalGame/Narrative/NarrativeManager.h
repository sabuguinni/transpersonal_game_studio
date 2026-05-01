#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForInput,
    Playing,
    Completed
};

UENUM(BlueprintType)
enum class ENarr_NarrativeEvent : uint8
{
    FirstWaterFound,
    FirstDinosaurEncounter,
    FirstNightSurvived,
    FirstToolCrafted,
    FirstShelterBuilt,
    TRexEncounter,
    RaptorPackSighting,
    SafeZoneDiscovered
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerThought;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioFilePath = TEXT("");
        Duration = 0.0f;
        bIsPlayerThought = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_NarrativeEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueLine> TriggerDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasTriggered;

    FNarr_StoryEvent()
    {
        EventType = ENarr_NarrativeEvent::FirstWaterFound;
        EventDescription = TEXT("");
        bHasTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const TArray<FNarr_DialogueLine>& DialogueLines);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_NarrativeEvent EventType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasEventTriggered(ENarr_NarrativeEvent EventType) const;

    // Survival narration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlaySurvivalTip(const FString& TipText, const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayPlayerThought(const FString& ThoughtText, const FString& AudioPath);

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayEnvironmentalNarration(const FString& LocationName, const FString& NarrationText);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentDialogueState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueLine> CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float DialogueTimer;

private:
    void InitializeStoryEvents();
    void AdvanceDialogue();
    FNarr_StoryEvent* FindStoryEvent(ENarr_NarrativeEvent EventType);

    FTimerHandle DialogueTimerHandle;
};