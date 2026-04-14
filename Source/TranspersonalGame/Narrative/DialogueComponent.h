#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "DialogueComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceClipPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_EmotionalState EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 MinConsciousnessLevel;

    FNarr_DialogueNode()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::GetEmpty();
        VoiceClipPath = TEXT("");
        EmotionalTone = ENarr_EmotionalState::Neutral;
        bRequiresConsciousnessLevel = false;
        MinConsciousnessLevel = 0;
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
    int32 RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterArchetype SpeakerArchetype;

    FNarr_DialogueTree()
    {
        TreeName = TEXT("DefaultTree");
        RootNodeID = 0;
        SpeakerArchetype = ENarr_CharacterArchetype::Neutral;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, SpeakerName, const FText&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, bool, bCompletedSuccessfully);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerResponseSelected, int32, ResponseIndex, const FString&, ResponseText, int32, NextNodeID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

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
    void SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceToNextNode(int32 NodeID);

    // Dialogue Tree Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueTree(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTree(const FNarr_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogueTree(const FString& TreeName) const;

    // State Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentNode() const { return CurrentNode; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
    TArray<FString> GetCurrentResponses() const { return CurrentNode.PlayerResponses; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnPlayerResponseSelected OnPlayerResponseSelected;

protected:
    // Dialogue State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    bool bDialogueActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueNode CurrentNode;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentTreeName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentNodeID;

    // Dialogue Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    TMap<FString, FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    class UDataTable* DialogueDataTable;

    // Helper Functions
    bool ValidateNodeTransition(int32 FromNodeID, int32 ToNodeID) const;
    bool CheckConsciousnessRequirement(const FNarr_DialogueNode& Node) const;
    void ProcessDialogueEffects(const FNarr_DialogueNode& Node);
};