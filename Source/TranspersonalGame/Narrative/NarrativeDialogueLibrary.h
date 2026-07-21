#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "NarrativeDialogueLibrary.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ENarr_SurvivalContext TriggerContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::GetEmpty();
        TriggerContext = ENarr_SurvivalContext::Exploration;
        Priority = 1.0f;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
    ENarr_SurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
    TArray<FString> RequiredDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
    bool bIsTriggered;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        EventDescription = FText::GetEmpty();
        Context = ENarr_SurvivalContext::Exploration;
        bIsTriggered = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarrativeDialogueLibrary : public UObject
{
    GENERATED_BODY()

public:
    UNarrativeDialogueLibrary();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    static FNarr_DialogueEntry GetDialogueByContext(ENarr_SurvivalContext Context, float PlayerFear = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    static TArray<FNarr_DialogueEntry> GetAllDialogueForContext(ENarr_SurvivalContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    static void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    static bool IsEventTriggered(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    static void InitializeNarrativeDatabase();

private:
    static TArray<FNarr_DialogueEntry> DialogueDatabase;
    static TArray<FNarr_NarrativeEvent> EventDatabase;
    static bool bIsInitialized;

    static void LoadSurvivalDialogue();
    static void LoadCombatDialogue();
    static void LoadExplorationDialogue();
};