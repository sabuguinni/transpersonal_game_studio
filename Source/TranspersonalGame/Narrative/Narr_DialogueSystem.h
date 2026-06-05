#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue response options for player choices
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RelationshipChange;

    FNarr_DialogueOption()
    {
        OptionText = TEXT("Continue...");
        NextNodeID = -1;
        bRequiresItem = false;
        RequiredItemName = TEXT("");
        RelationshipChange = 0;
    }
};

// Individual dialogue node containing NPC speech and player options
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueOption> PlayerOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bEndsConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CrowdDensityModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_TribalRank RequiredRank;

    FNarr_DialogueNode()
    {
        NodeID = 0;
        NPCText = TEXT("Greetings, traveler.");
        NPCName = TEXT("Tribal Member");
        bEndsConversation = false;
        CrowdDensityModifier = 1.0f;
        RequiredRank = ENarr_TribalRank::Outcast;
    }
};

// Complete dialogue tree for an NPC character
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCCharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 StartNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAdaptsToPlayerRank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAdaptsToCrowdDensity;

    FNarr_DialogueTree()
    {
        TreeName = TEXT("Basic Conversation");
        NPCCharacterName = TEXT("Unknown NPC");
        StartNodeID = 0;
        bAdaptsToPlayerRank = true;
        bAdaptsToCrowdDensity = false;
    }
};

// Dialogue system component for managing conversations
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue tree management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FNarr_DialogueTree CurrentDialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bIsInConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    AActor* ConversationPartner;

    // Crowd integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    float CrowdInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    int32 NearbyCrowdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    bool bModifiesDialogueBasedOnCrowd;

    // Relationship tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TMap<FString, int32> NPCRelationshipLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    ENarr_TribalRank PlayerTribalRank;

    // Dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(AActor* NPCActor, const FString& DialogueTreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateCrowdInfluence();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ModifyNPCRelationship(const FString& NPCName, int32 RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    int32 GetNPCRelationshipLevel(const FString& NPCName);

private:
    void InitializeDefaultDialogues();
    FNarr_DialogueNode* FindDialogueNode(int32 NodeID);
    bool ValidateDialogueOption(const FNarr_DialogueOption& Option);
};

// Dialogue trigger actor for zone-based conversations
UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

public:
    // Trigger properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    FString AssociatedDialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    FString TriggerNPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    bool bRequiresSpecificCrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    int32 MinCrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    int32 MaxCrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Trigger")
    bool bHasBeenTriggered;

    // Trigger events
    UFUNCTION()
    void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION()
    void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Trigger")
    bool CheckCrowdDensityRequirements();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Trigger")
    void TriggerDialogue(AActor* PlayerActor);
};

// Narrative manager for coordinating story events
UCLASS()
class TRANSPERSONALGAME_API ANarr_NarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NarrativeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Story progression tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TMap<FString, bool> StoryFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<FString> CompletedStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    FString CurrentMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 PlayerStoryProgress;

    // Crowd integration for narrative
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Narrative")
    bool bAdaptsStoryToCrowdBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Narrative")
    float CrowdStoryInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Narrative")
    TArray<FString> CrowdTriggeredStoryEvents;

    // Dialogue tree management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Management")
    TMap<FString, FNarr_DialogueTree> GlobalDialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Management")
    TArray<ANarr_DialogueTrigger*> ActiveDialogueTriggers;

    // Narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteStoryBeat(const FString& StoryBeatName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& StoryBeatName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FString& TreeName, const FNarr_DialogueTree& DialogueTree);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueTree GetDialogueTree(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateStoryBasedOnCrowdBehavior();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerCrowdStoryEvent(const FString& EventName);

private:
    void InitializeDefaultStoryFlags();
    void InitializeDefaultDialogueTrees();
    void MonitorCrowdForStoryTriggers();
};