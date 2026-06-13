#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ESurvivalNeed RequiredNeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float RequiredNeedValue;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = "";
        DialogueText = "";
        AudioPath = "";
        RequiredNeed = ESurvivalNeed::None;
        RequiredNeedValue = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ConversationState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversation")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversation")
    FString ConversationPartner;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversation")
    TArray<FString> CompletedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conversation")
    bool bInConversation;

    FNarr_ConversationState()
    {
        CurrentDialogueID = "";
        ConversationPartner = "";
        bInConversation = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& NPCName, const FString& StartingDialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableResponses();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInConversation() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasCompletedDialogue(const FString& DialogueID) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FNarr_ConversationState CurrentConversation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> GlobalCompletedDialogues;

    void LoadDefaultDialogues();
    bool CheckDialogueRequirements(const FNarr_DialogueEntry& Dialogue);
    void MarkDialogueCompleted(const FString& DialogueID);
};