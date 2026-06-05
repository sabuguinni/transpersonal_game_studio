#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalCondition RequiredCondition;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioFilePath = TEXT("");
        Duration = 3.0f;
        RequiredCondition = ESurvivalCondition::Healthy;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ConversationState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    int32 ConversationStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    bool bHasMetBefore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    TArray<FString> CompletedTopics;

    FNarr_ConversationState()
    {
        NPCName = TEXT("");
        ConversationStage = 0;
        bHasMetBefore = false;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartConversation(const FString& NPCName, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FText> GetAvailableResponses() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectResponse(int32 ResponseIndex);

    // Conversation state management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateConversationState(const FString& NPCName, int32 NewStage);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_ConversationState GetConversationState(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void MarkTopicCompleted(const FString& NPCName, const FString& TopicName);

    // Survival-based dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> GetSurvivalDialogue(ESurvivalCondition PlayerCondition) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanAccessDialogue(const FNarr_DialogueEntry& Dialogue, ESurvivalCondition PlayerCondition) const;

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    class UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bInConversation;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentNPC;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    TMap<FString, FNarr_ConversationState> ConversationStates;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

private:
    void LoadDialogueData();
    void InitializeNPCStates();
    FNarr_DialogueEntry SelectAppropriateDialogue(const FString& NPCName, ESurvivalCondition PlayerCondition) const;
};