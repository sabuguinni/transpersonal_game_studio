#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
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
    ENarr_BiomeType RequiredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_StoryPhase RequiredStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioFilePath = TEXT("");
        RequiredBiome = ENarr_BiomeType::Savanna;
        RequiredStoryPhase = ENarr_StoryPhase::Awakening;
        bIsQuestDialogue = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueEntry> GreetingDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueEntry> QuestDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueEntry> TradingDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueEntry> CombatDialogues;

    FNarr_NPCDialogueSet()
    {
        NPCName = TEXT("");
        NPCType = ENarr_NPCType::Trader;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetDialogueEntry(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetNPCDialogues(const FString& NPCName, ENarr_DialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueAvailable(const FString& DialogueID, ENarr_BiomeType CurrentBiome, ENarr_StoryPhase CurrentPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetRandomDialogueByContext(ENarr_DialogueContext Context, ENarr_BiomeType Biome);

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY()
    TMap<FString, FNarr_NPCDialogueSet> NPCDialogueSets;

    void InitializeDefaultDialogues();
    void CreateSurvivalDialogues();
    void CreateDangerWarningDialogues();
    void CreateExplorationDialogues();
    void CreateNPCDialogueSets();
};