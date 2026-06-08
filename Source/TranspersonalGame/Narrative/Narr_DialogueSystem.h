#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FNarr_DialogueNode()
        : SpeakerName(TEXT(""))
        , DialogueText(TEXT(""))
        , bIsQuestNode(false)
        , QuestID(TEXT(""))
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentNodeIndex;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , CurrentNodeIndex(0)
    {
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& TreeID, class AActor* Speaker, class AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CreateHunterDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CreateElderDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CreateScoutDialogue();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    class AActor* CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    class AActor* CurrentListener;

private:
    void ProcessCurrentNode();
    void AdvanceToNextNode(int32 NextIndex);
};