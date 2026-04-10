#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> Prerequisites;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueID, const FString&, SpeakerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeEvent, const FString&, EventID, const FText&, EventDescription);

UCLASS()
class TRANSPERSONALGAME_API UNarrativeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectDialogueResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventTriggered(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FDialogueEntry GetDialogueEntry(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadDialogueTable(UDataTable* DialogueDataTable);

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeEvent OnNarrativeEvent;

protected:
    UPROPERTY()
    UDataTable* DialogueTable;

    UPROPERTY()
    TMap<FString, FNarrativeEvent> NarrativeEvents;

    UPROPERTY()
    FString CurrentDialogueID;

    UPROPERTY()
    AActor* CurrentSpeaker;

    UPROPERTY()
    bool bIsDialogueActive;

private:
    void InitializeNarrativeEvents();
    void ProcessDialogueLogic(const FString& DialogueID);
};