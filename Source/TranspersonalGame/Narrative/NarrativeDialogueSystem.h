#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        DisplayDuration = 3.0f;
        QuestID = TEXT("");
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    FNarr_QuestDialogue()
    {
        QuestID = TEXT("");
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& DialogueEntries);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, AActor* TriggerActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ActivateQuestDialogues(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void DeactivateQuestDialogues(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetActiveDialogues() const;

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestStarted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestFailed(const FString& QuestID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> ActiveDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UDataTable* DialogueDataTable;

private:
    void LoadDialogueData();
    void SortDialoguesByPriority();
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeDialogueTrigger : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeDialogueTrigger();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    FString RequiredQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    bool bTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    float TriggerRadius;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Trigger")
    void TriggerDialogue(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Trigger")
    bool CanTrigger(AActor* TriggeringActor) const;

protected:
    virtual void BeginPlay() override;

private:
    bool bHasTriggered;
    UNarrativeDialogueSystem* DialogueSystem;
};

#include "NarrativeDialogueSystem.generated.h"