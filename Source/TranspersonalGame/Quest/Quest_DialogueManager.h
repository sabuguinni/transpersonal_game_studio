#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Quest_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardExperience;

    FQuest_DialogueOption()
    {
        OptionText = TEXT("");
        NextDialogueID = -1;
        bRequiresItem = false;
        RequiredItemName = TEXT("");
        RewardExperience = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuest_DialogueOption> PlayerOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsQuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssociatedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DialogueDuration;

    FQuest_DialogueEntry()
    {
        DialogueID = 0;
        NPCName = TEXT("");
        DialogueText = TEXT("");
        bIsQuestDialogue = false;
        AssociatedQuestID = TEXT("");
        DialogueDuration = 5.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FQuest_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    AActor* CurrentNPC;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* NPC, int32 StartingDialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FQuest_DialogueOption> GetCurrentOptions() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FQuest_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialoguesFromDataTable(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanPlayerAffordOption(const FQuest_DialogueOption& Option) const;

private:
    FQuest_DialogueEntry* FindDialogueByID(int32 DialogueID);
    void ProcessDialogueRewards(const FQuest_DialogueOption& SelectedOption);
    bool CheckDialogueRequirements(const FQuest_DialogueOption& Option) const;
};