#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> TriggerConditions;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        DialogueText = FText::GetEmpty();
        SpeakerName = TEXT("");
        Duration = 3.0f;
        DialogueType = ENarr_DialogueType::Narration;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        BeatTitle = FText::GetEmpty();
        BeatDescription = FText::GetEmpty();
        Phase = ENarr_StoryPhase::Discovery;
        bIsCompleted = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueAvailable(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueCondition(const FString& ConditionID, bool bValue);

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const;

    // Environmental Narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEnvironmentalNarration(const FVector& Location, const FString& NarrationType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeContext(ENarr_BiomeType Biome, ENarr_TimeOfDay TimeOfDay, ENarr_WeatherType Weather);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    class UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, bool> DialogueConditions;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Context")
    ENarr_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Context")
    ENarr_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Context")
    ENarr_WeatherType CurrentWeather;

private:
    void InitializeStoryBeats();
    void LoadDialogueData();
    bool EvaluateConditions(const TArray<FString>& Conditions) const;
    void BroadcastDialogue(const FNarr_DialogueEntry& DialogueEntry, AActor* Speaker);
};