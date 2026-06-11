#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FQuest_DialogueNode()
    {
        DialogueText = TEXT("");
        SpeakerName = TEXT("Unknown");
        bIsQuestDialogue = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsActive;

    FQuest_DialogueTree()
    {
        TreeID = TEXT("");
        CurrentNodeIndex = 0;
        bIsActive = false;
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FQuest_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FQuest_DialogueTree* CurrentDialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DialogueAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DialogueSpeed;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueNode GetCurrentDialogueNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentPlayerResponses() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CreateSurvivalDialogues();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void TriggerQuestFromDialogue(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FString& DialogueText, const FString& SpeakerName);

protected:
    void InitializeDefaultDialogues();
    void CreateHunterDialogue();
    void CreateCrafterDialogue();
    void CreateSurvivalGuideDialogue();
    bool AdvanceToNextNode(int32 NextNodeIndex);
};