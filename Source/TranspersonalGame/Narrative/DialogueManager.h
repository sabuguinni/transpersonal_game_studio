#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("Narrator");
        DialogueText = FText::FromString(TEXT(""));
        AudioPath = TEXT("");
        Context = ENarr_DialogueContext::Narration;
        DisplayDuration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_QuestType TriggerQuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        TriggerQuestType = ENarr_QuestType::Exploration;
        bIsRepeatable = false;
        Priority = 1;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueID, const FText&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, const FString&, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueResponseSelected, const FString&, DialogueID, int32, ResponseIndex, const FString&, NextDialogueID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueEntry(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerQuestDialogue(ENarr_QuestType QuestType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterQuestCompletion(ENarr_QuestType QuestType, bool bSuccess);

    // Dialogue data management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueData();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueSequence GetDialogueSequence(const FString& SequenceID) const;

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FString GetCurrentDialogueID() const { return CurrentDialogueID; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogueEntry() const { return CurrentDialogueEntry; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueCompleted OnDialogueCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueResponseSelected OnDialogueResponseSelected;

protected:
    // Dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    FString CurrentSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    FNarr_DialogueEntry CurrentDialogueEntry;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    int32 CurrentDialogueIndex;

    // Dialogue data
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedSequences;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* DialogueAudioComponent;

    // Timer handles
    FTimerHandle DialogueTimerHandle;

private:
    // Internal dialogue flow
    void ProcessNextDialogue();
    void CompleteDialogueSequence();
    void InitializeDefaultDialogues();
    
    // Quest dialogue mapping
    TMap<ENarr_QuestType, TArray<FString>> QuestDialogueMap;
};

#include "DialogueManager.generated.h"