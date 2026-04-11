#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessLevel : uint8
{
    Unaware = 0,
    Awakening = 1,
    Aware = 2,
    Enlightened = 3,
    Transcendent = 4
};

UENUM(BlueprintType)
enum class EStoryBeat : uint8
{
    Introduction,
    FirstAwakening,
    SpiritualCrisis,
    Deepening,
    Integration,
    Transcendence
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessLevel RequiredConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DialogueChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ChoiceConsequences;

    FDialogueEntry()
    {
        CharacterName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioAssetPath = TEXT("");
        RequiredConsciousnessLevel = EConsciousnessLevel::Unaware;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStoryBeat StoryBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UnlockedDialogues;

    FNarrativeEvent()
    {
        EventID = TEXT("");
        EventDescription = FText::GetEmpty();
        StoryBeat = EStoryBeat::Introduction;
        ConsciousnessImpact = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEvent, const FNarrativeEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessChanged, EConsciousnessLevel, OldLevel, EConsciousnessLevel, NewLevel);

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateConsciousnessLevel(float DeltaConsciousness);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    EConsciousnessLevel GetCurrentConsciousnessLevel() const { return CurrentConsciousnessLevel; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetConsciousnessProgress() const { return ConsciousnessProgress; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    EStoryBeat GetCurrentStoryBeat() const { return CurrentStoryBeat; }

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogueChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryBeat();

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FString> GetUnlockedDialogues() const { return UnlockedDialogues; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsDialogueUnlocked(const FString& DialogueID) const;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnNarrativeEvent OnNarrativeEvent;

    UPROPERTY(BlueprintAssignable)
    FOnConsciousnessChanged OnConsciousnessChanged;

protected:
    // Data tables
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
    UDataTable* NarrativeEventsDataTable;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EConsciousnessLevel CurrentConsciousnessLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float ConsciousnessProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EStoryBeat CurrentStoryBeat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TArray<FString> CompletedEvents;

    // Dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FDialogueEntry CurrentDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    AActor* CurrentSpeaker;

private:
    void LoadNarrativeData();
    void ProcessNarrativeEvent(const FNarrativeEvent& Event);
    EConsciousnessLevel CalculateConsciousnessLevel(float Progress) const;
    void UpdateStoryBeatBasedOnConsciousness();
};