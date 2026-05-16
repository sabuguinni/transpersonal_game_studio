#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredItemID;

    FQuest_DialogueNode()
    {
        NodeID = TEXT("");
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        bIsQuestNode = false;
        QuestID = TEXT("");
        bRequiresItem = false;
        RequiredItemID = TEXT("");
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
    TArray<FQuest_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsActive;

    FQuest_DialogueTree()
    {
        TreeID = TEXT("");
        NPCName = TEXT("Tribal Elder");
        CurrentNodeID = TEXT("start");
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FQuest_DialogueTree DialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanInteract;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueNode GetCurrentNode();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInRange(AActor* Player);

protected:
    virtual void BeginPlay() override;

private:
    void InitializeDefaultDialogue();
    FQuest_DialogueNode* FindNodeByID(const FString& NodeID);
};

UCLASS()
class TRANSPERSONALGAME_API UQuest_DialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FQuest_DialogueTree> ActiveDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FQuest_DialogueTree* CurrentDialogue;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool RegisterDialogueTree(const FQuest_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueTree* GetDialogueByID(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogueWithNPC(const FString& NPCID, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive();

private:
    void CreateDefaultDialogues();
};