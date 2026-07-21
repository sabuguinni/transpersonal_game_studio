#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Narr_DialogueNode.h"
#include "Narr_ConversationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConversationStarted, class AActor*, NPC, const FString&, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueNodeChanged, UNarr_DialogueNode*, NewNode);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_ConversationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_ConversationManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Conversation")
    FOnConversationStarted OnConversationStarted;

    UPROPERTY(BlueprintAssignable, Category = "Conversation")
    FOnConversationEnded OnConversationEnded;

    UPROPERTY(BlueprintAssignable, Category = "Conversation")
    FOnDialogueNodeChanged OnDialogueNodeChanged;

    UFUNCTION(BlueprintCallable, Category = "Conversation")
    bool StartConversation(AActor* NPC, const FString& ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Conversation")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Conversation")
    bool SelectDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Conversation")
    UNarr_DialogueNode* GetCurrentNode() const { return CurrentDialogueNode; }

    UFUNCTION(BlueprintCallable, Category = "Conversation")
    bool IsInConversation() const { return bIsInConversation; }

    UFUNCTION(BlueprintCallable, Category = "Conversation")
    AActor* GetCurrentNPC() const { return CurrentNPC; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Conversation")
    bool bIsInConversation;

    UPROPERTY(BlueprintReadOnly, Category = "Conversation")
    AActor* CurrentNPC;

    UPROPERTY(BlueprintReadOnly, Category = "Conversation")
    FString CurrentConversationID;

    UPROPERTY(BlueprintReadOnly, Category = "Conversation")
    UNarr_DialogueNode* CurrentDialogueNode;

    UPROPERTY(BlueprintReadOnly, Category = "Conversation")
    TMap<FString, UNarr_DialogueNode*> DialogueNodes;

private:
    void LoadConversationData(const FString& ConversationID);
    void NavigateToNode(const FString& NodeID);
    TArray<FString> GetPlayerInventory() const;
    void CreateSampleDialogue();
};