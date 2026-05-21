#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

class UQuestManager;
class ACrowdSimulationManager;

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
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_BiomeType RequiredBiome;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = "";
        DialogueText = "";
        AudioPath = "";
        QuestID = "";
        RequiredBiome = ENarr_BiomeType::Savana;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 DinosaursSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 QuestsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float SurvivalTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bInDanger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> UnlockedDialogues;

    FNarr_NarrativeContext()
    {
        CurrentBiome = ENarr_BiomeType::Savana;
        DinosaursSeen = 0;
        QuestsCompleted = 0;
        SurvivalTime = 0.0f;
        bInDanger = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDialogueSystem();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetDialogueForContext(const FNarr_NarrativeContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualDialogue(ENarr_BiomeType Biome, const FString& EventType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext);

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& DialogueEntries);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetQuestDialogue(const FString& QuestID);

    // Crowd simulation integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnHerdMigrationDetected(ENarr_BiomeType FromBiome, ENarr_BiomeType ToBiome, int32 HerdSize);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPredatorEncounter(const FString& PredatorType, ENarr_BiomeType Biome);

    // Audio system integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueAudioPlaying() const;

    // Narrative progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnlockDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueUnlocked(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetNarrativeProgress() const;

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY()
    TMap<FString, TArray<FNarr_DialogueEntry>> QuestDialogues;

    UPROPERTY()
    FNarr_NarrativeContext CurrentContext;

    UPROPERTY()
    TArray<FString> UnlockedDialogues;

    UPROPERTY()
    UQuestManager* QuestManagerRef;

    UPROPERTY()
    ACrowdSimulationManager* CrowdManagerRef;

    // Internal methods
    void LoadDialogueDatabase();
    void SetupBiomeDialogues();
    void SetupMigrationDialogues();
    void SetupPredatorDialogues();
    FNarr_DialogueEntry CreateDialogueEntry(const FString& ID, const FString& Speaker, const FString& Text, const FString& Audio = "");
};