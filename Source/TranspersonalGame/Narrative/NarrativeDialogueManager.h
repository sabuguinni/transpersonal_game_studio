#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueEntry()
        : Priority(1.0f)
        , bIsRepeatable(false)
        , TriggerType(ENarr_DialogueTrigger::Manual)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    ENarr_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    ENarr_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    TArray<FString> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context")
    int32 SurvivalDays;

    FNarr_NarrativeContext()
        : CurrentBiome(ENarr_BiomeType::Savana)
        , TimeOfDay(ENarr_TimeOfDay::Day)
        , ThreatLevel(0.0f)
        , SurvivalDays(0)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FNarr_DialogueEntry&, DialogueEntry, const FNarr_NarrativeContext&, Context);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, const FNarr_NarrativeContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_NarrativeContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeContext(const FNarr_NarrativeContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetDialogueEntry(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetContextualDialogues(ENarr_DialogueTrigger TriggerType, const FNarr_NarrativeContext& Context) const;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueTriggered OnDialogueTriggered;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
    class UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_NarrativeContext CurrentContext;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> PlayedDialogues;

private:
    void LoadDialogueData();
    bool ShouldPlayDialogue(const FNarr_DialogueEntry& Entry, const FNarr_NarrativeContext& Context) const;
    float CalculateDialoguePriority(const FNarr_DialogueEntry& Entry, const FNarr_NarrativeContext& Context) const;
};