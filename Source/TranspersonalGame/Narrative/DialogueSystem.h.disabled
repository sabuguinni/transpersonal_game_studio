#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEndNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    FNarr_DialogueNode()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        bIsEndNode = false;
        AudioAssetPath = TEXT("");
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
    int32 StartNodeID;

    FNarr_DialogueTree()
    {
        TreeName = TEXT("");
        StartNodeID = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentTreeIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentNodeIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentNode();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTree(const FNarr_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogueTrees();

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueFromDataTable(class UDataTable* DialogueDataTable);

private:
    void InitializeDefaultDialogues();
    int32 FindTreeByName(const FString& TreeName);
};