#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    FQuest_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 3.0f;
        bIsQuestRelated = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FQuest_DialogueTree()
    {
        TreeID = TEXT("");
        NPCName = TEXT("");
        bIsRepeatable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_DialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void RegisterDialogueTree(const FQuest_DialogueTree& DialogueTree);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FQuest_DialogueTree GetDialogueTreeForNPC(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool StartDialogue(const FString& NPCName, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void PlayDialogueLine(const FQuest_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool IsDialogueActive() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest Dialogue")
    TMap<FString, FQuest_DialogueTree> DialogueTrees;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Dialogue")
    FString CurrentNPCName;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Dialogue")
    int32 CurrentLineIndex;

private:
    void InitializeDefaultDialogues();
    bool CanAccessDialogue(const FQuest_DialogueTree& DialogueTree);
};