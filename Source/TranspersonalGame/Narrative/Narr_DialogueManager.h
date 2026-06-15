#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float AudioDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioAssetPath = TEXT("");
        AudioDuration = 0.0f;
        bIsQuestRelated = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ConversationState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FString CurrentDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FString ConversationPartner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    TArray<FString> DialogueHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    float ConversationStartTime;

    FNarr_ConversationState()
    {
        CurrentDialogueID = TEXT("");
        ConversationPartner = TEXT("");
        bIsActive = false;
        ConversationStartTime = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, SpeakerName, const FText&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, ConversationPartner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResponseSelected, const FString&, ResponseText, const FString&, NextDialogueID, bool, bEndsConversation);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnResponseSelected OnResponseSelected;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& NPCName, const FString& InitialDialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentResponseOptions() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsConversationActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueData(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Conversation State")
    FNarr_ConversationState CurrentConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DefaultAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoAdvanceDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AutoAdvanceDelay;

private:
    FNarr_DialogueEntry* FindDialogueEntry(const FString& DialogueID);
    void ProcessDialogueEntry(const FNarr_DialogueEntry& Entry);
    void HandleQuestDialogue(const FNarr_DialogueEntry& Entry);
    void UpdateConversationHistory(const FString& DialogueID);

    FTimerHandle AutoAdvanceTimer;
    void AutoAdvanceDialogue();
};