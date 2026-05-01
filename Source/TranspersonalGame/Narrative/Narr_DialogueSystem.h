#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerCondition;

    FNarr_DialogueNode()
    {
        DialogueText = TEXT("");
        SpeakerName = TEXT("Unknown");
        AudioPath = TEXT("");
        DisplayDuration = 3.0f;
        bIsPlayerChoice = false;
        TriggerCondition = ENarr_DialogueTrigger::Proximity;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCType NPCType;

    FNarr_DialogueTree()
    {
        TreeName = TEXT("DefaultTree");
        CurrentNodeIndex = 0;
        bIsActive = false;
        NPCType = ENarr_NPCType::TribalElder;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue(int32 ChoiceIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode() const;

    // Tree Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTree(const FNarr_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RemoveDialogueTree(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueTree* GetDialogueTree(const FString& TreeName);

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

    // NPC Integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetNPCType(ENarr_NPCType NewNPCType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    ENarr_NPCType GetNPCType() const;

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerQuestDialogue(EQuest_MissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CompleteQuestDialogue(EQuest_MissionType MissionType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueTree* CurrentTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UAudioComponent* DialogueAudioComponent;

private:
    void InitializeDefaultDialogueTrees();
    void CreateElderDialogueTree();
    void CreateHunterDialogueTree();
    void CreateGathererDialogueTree();
    void CreateShamanDialogueTree();
    
    FNarr_DialogueNode CreateDialogueNode(const FString& Text, const FString& Speaker, 
                                         const FString& AudioPath = TEXT(""), 
                                         float Duration = 3.0f, 
                                         bool bIsChoice = false);
};