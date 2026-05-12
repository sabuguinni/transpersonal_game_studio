#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

// Forward declarations
class UNarr_DialogueComponent;
class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    QuestGiver,
    Merchant,
    Information,
    Warning,
    Story
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None,
    SurvivalLevel,
    QuestCompleted,
    ItemInInventory,
    TimeOfDay,
    PlayerHealth
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_DialogueCondition RequiredCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEndsDialogue;

    FNarr_DialogueOption()
    {
        OptionText = FText::FromString("Continue");
        ResponseText = FText::FromString("I understand.");
        RequiredCondition = ENarr_DialogueCondition::None;
        RequiredValue = 0;
        bEndsDialogue = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText OpeningText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueOption> DialogueOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRepeatable;

    FNarr_DialogueData()
    {
        DialogueID = "default_dialogue";
        SpeakerName = "Unknown";
        DialogueType = ENarr_DialogueType::Greeting;
        OpeningText = FText::FromString("Hello there.");
        bRepeatable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& DialogueID, ATranspersonalCharacter* Player, AActor* Speaker);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueData GetDialogueData(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueOption> GetCurrentDialogueOptions();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentSpeaker() const { return CurrentSpeakerName; }

    // Condition checking
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CheckDialogueCondition(ENarr_DialogueCondition Condition, int32 RequiredValue, ATranspersonalCharacter* Player);

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasStoryEventOccurred(const FString& EventID);

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueAudio(const FString& AudioPath);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentSpeakerName;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ATranspersonalCharacter* CurrentPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    AActor* CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueData CurrentDialogueData;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TSet<FString> CompletedStoryEvents;

private:
    void InitializeDefaultDialogues();
    void CreateSurvivalDialogues();
    void CreateQuestDialogues();
};