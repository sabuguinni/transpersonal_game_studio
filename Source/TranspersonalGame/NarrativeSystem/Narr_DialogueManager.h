#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    HuntInstruction,
    SurvivalWarning,
    ResourceGuidance,
    ThreatAlert,
    QuestBriefing,
    TribeWisdom
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    HuntMaster,
    SurvivalGuide,
    Scout,
    Elder,
    Crafter,
    Warrior
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> TriggerConditions;

    FNarr_DialogueEntry()
    {
        SpeakerRole = ENarr_CharacterRole::Elder;
        DialogueType = ENarr_DialogueType::TribeWisdom;
        Duration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float SurvivalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 SuccessfulHunts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasMetElders;

    FNarr_NarrativeState()
    {
        CurrentChapter = 1;
        SurvivalExperience = 0.0f;
        SuccessfulHunts = 0;
        bHasMetElders = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue system
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueByType(ENarr_DialogueType DialogueType, ENarr_CharacterRole SpeakerRole);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueUnlocked(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UnlockDialogue(const FString& DialogueID);

    // Narrative progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceChapter(int32 NewChapter);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestCompletion(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateSurvivalExperience(float ExperienceGained);

    // Contextual dialogue triggers
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerHuntDialogue(const FString& DinosaurType, int32 PackSize);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDangerWarning(const FString& ThreatType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerResourceGuidance(const FString& ResourceType, bool bFound);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Narrative")
    const FNarr_NarrativeState& GetNarrativeState() const { return NarrativeState; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> GetAvailableDialogues() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NarrativeState NarrativeState;

    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY()
    class UAudioComponent* CurrentDialogueAudio;

private:
    void LoadDialogueDatabase();
    void PlayDialogueAudio(const FNarr_DialogueEntry& DialogueEntry);
    bool CheckDialogueConditions(const FNarr_DialogueEntry& DialogueEntry) const;
    void LogDialogueEvent(const FString& DialogueID, const FString& Context);
};