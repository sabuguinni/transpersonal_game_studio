#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Quest_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_DialogueType : uint8
{
    QuestGiver = 0,
    QuestComplete,
    QuestFailed,
    Hint,
    Lore,
    Trade,
    Warning
};

UENUM(BlueprintType)
enum class EQuest_NPCPersonality : uint8
{
    Gruff = 0,
    Wise,
    Cautious,
    Aggressive,
    Friendly,
    Mysterious
};

USTRUCT(BlueprintType)
struct FQuest_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuest_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    FQuest_DialogueLine()
    {
        DialogueText = TEXT("");
        DialogueType = EQuest_DialogueType::QuestGiver;
        AudioPath = TEXT("");
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_NPCDialogue : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EQuest_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FQuest_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString QuestID;

    FQuest_NPCDialogue()
    {
        NPCName = TEXT("Unknown");
        Personality = EQuest_NPCPersonality::Friendly;
        QuestID = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool StartDialogue(const FString& NPCName, class AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool PlayDialogueLine(const FQuest_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    TArray<FString> GetPlayerResponses(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FString GetCurrentSpeaker() const { return CurrentSpeaker; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    class UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FQuest_NPCDialogue CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    class AActor* CurrentPlayer;

private:
    void LoadDialogueData();
    void ProcessDialogueChoice(int32 ChoiceIndex);
    void TriggerQuestEvent(const FString& QuestID);
};