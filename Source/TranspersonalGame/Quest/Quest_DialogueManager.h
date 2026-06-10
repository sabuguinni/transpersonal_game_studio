#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Quest_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestToGive;

    FQuest_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioFilePath = TEXT("");
        bIsQuestGiver = false;
        QuestToGive = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FQuest_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString DefaultGreeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString QuestCompleteResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString QuestInProgressResponse;

    FQuest_NPCDialogueSet()
    {
        NPCName = TEXT("");
        DefaultGreeting = TEXT("");
        QuestCompleteResponse = TEXT("");
        QuestInProgressResponse = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_DialogueManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FQuest_DialogueEntry GetDialogueEntry(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    TArray<FQuest_DialogueEntry> GetNPCDialogues(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FString GetNPCGreeting(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool StartDialogue(const FString& NPCName, const FString& PlayerID);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void EndDialogue(const FString& PlayerID);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool IsInDialogue(const FString& PlayerID) const;

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FString GetQuestDialogue(const FString& QuestID, const FString& DialogueType);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void RegisterQuestGiver(const FString& NPCName, const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool IsQuestGiver(const FString& NPCName) const;

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FString GetDialogueAudioPath(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void PlayDialogueAudio(const FString& DialogueID);

protected:
    // Dialogue Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, FQuest_NPCDialogueSet> NPCDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, FQuest_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue Data")
    TMap<FString, FString> QuestGivers;

    // Active Dialogues
    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogues")
    TMap<FString, FString> ActiveDialogues; // PlayerID -> NPCName

    // Initialization
    void InitializeDialogueDatabase();
    void LoadNPCDialogues();
    void SetupQuestGivers();

    // Helper Functions
    FQuest_DialogueEntry CreateDialogueEntry(const FString& ID, const FString& Speaker, 
                                           const FString& Text, const FString& AudioPath = TEXT(""));
    void AddNPCDialogue(const FString& NPCName, const FQuest_DialogueEntry& Entry);
};