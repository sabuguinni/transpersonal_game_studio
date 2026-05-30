#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

class UNarrativeCharacterSystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresQuestCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredQuestID;

    FNarr_DialogueNode()
    {
        DialogueText = TEXT("");
        SpeakerName = TEXT("Unknown");
        TriggerCondition = ENarr_DialogueTrigger::Always;
        bRequiresQuestCompletion = false;
        RequiredQuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterType AssociatedCharacterType;

    FNarr_DialogueTree()
    {
        TreeID = TEXT("");
        TreeName = TEXT("");
        RootNodeID = 0;
        AssociatedCharacterType = ENarr_CharacterType::TribalElder;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<ENarr_DinosaurSpecies> NearbyDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerFearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHealthLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 DaysInWorld;

    FNarr_DialogueContext()
    {
        CurrentBiome = ENarr_BiomeType::Savana;
        PlayerFearLevel = 0.0f;
        PlayerHealthLevel = 100.0f;
        DaysInWorld = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FNarr_DialogueContext CurrentContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString ActiveTreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    UNarrativeCharacterSystem* CharacterSystemRef;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& TreeID, const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateDialogueContext(const FNarr_DialogueContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetContextualDialogue(ENarr_CharacterType CharacterType, const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GenerateContextualGreeting(ENarr_CharacterType CharacterType, const FNarr_DialogueContext& Context);

protected:
    void InitializeDefaultDialogueTrees();
    bool EvaluateDialogueTrigger(ENarr_DialogueTrigger Trigger, const FNarr_DialogueContext& Context);
    FNarr_DialogueTree* FindDialogueTree(const FString& TreeID);
    FNarr_DialogueNode* FindDialogueNode(const FString& TreeID, int32 NodeID);
};