#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

// Dialogue line data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
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
    TArray<FString> ResponseOptions;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
    }
};

// Dialogue tree node
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueLine DialogueLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEndNode;

    FNarr_DialogueNode()
    {
        NodeID = 0;
        bIsEndNode = false;
    }
};

// Complete dialogue tree
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RootNodeID;

    FNarr_DialogueTree()
    {
        TreeName = TEXT("");
        RootNodeID = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

    // Dialogue trees storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_DialogueTree> DialogueTrees;

    // Current active dialogue
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    FNarr_DialogueTree* CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    int32 CurrentNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bDialogueActive;

public:
    // Dialogue management functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue(int32 ResponseIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentResponseOptions();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    // Dialogue tree management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTree(const FNarr_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueTree* FindDialogueTree(const FString& TreeName);

    // Initialize default dialogue trees
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dialogue")
    void InitializeDefaultDialogues();

private:
    FNarr_DialogueNode* FindNodeByID(int32 NodeID);
    void CreateTribalElderDialogue();
    void CreateScoutDialogue();
    void CreateHunterDialogue();
};