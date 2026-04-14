#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "NarrativeTypes.generated.h"

/**
 * Dialogue emotion types for character expression
 */
UENUM(BlueprintType)
enum class ENarr_DialogueEmotion : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Happy       UMETA(DisplayName = "Happy"),
    Sad         UMETA(DisplayName = "Sad"),
    Angry       UMETA(DisplayName = "Angry"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Surprised   UMETA(DisplayName = "Surprised"),
    Mystical    UMETA(DisplayName = "Mystical"),
    Wise        UMETA(DisplayName = "Wise"),
    Determined  UMETA(DisplayName = "Determined")
};

/**
 * Quest states for progression tracking
 */
UENUM(BlueprintType)
enum class ENarr_QuestState : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Hidden      UMETA(DisplayName = "Hidden")
};

/**
 * Consciousness awakening levels
 */
UENUM(BlueprintType)
enum class ENarr_ConsciousnessLevel : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    Stirring    UMETA(DisplayName = "Stirring"),
    Awakening   UMETA(DisplayName = "Awakening"),
    Aware       UMETA(DisplayName = "Aware"),
    Enlightened UMETA(DisplayName = "Enlightened"),
    Transcendent UMETA(DisplayName = "Transcendent")
};

/**
 * Dialogue choice consequence types
 */
UENUM(BlueprintType)
enum class ENarr_ChoiceConsequence : uint8
{
    None        UMETA(DisplayName = "None"),
    Minor       UMETA(DisplayName = "Minor"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Major       UMETA(DisplayName = "Major"),
    Permanent   UMETA(DisplayName = "Permanent")
};

/**
 * Single dialogue line data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueEmotion Emotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<class USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<class UAnimMontage> FacialAnimation;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        Emotion = ENarr_DialogueEmotion::Neutral;
        Duration = 3.0f;
        VoiceClip = nullptr;
        FacialAnimation = nullptr;
    }
};

/**
 * Dialogue choice option
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    ENarr_ChoiceConsequence ConsequenceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> SetFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    int32 ConsciousnessRequirement;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::GetEmpty();
        NextDialogueID = TEXT("");
        ConsequenceLevel = ENarr_ChoiceConsequence::None;
        ConsciousnessRequirement = 0;
    }
};

/**
 * Complete dialogue node
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AssociatedQuestID;

    FNarr_DialogueNode()
    {
        DialogueID = TEXT("");
        bIsQuestDialogue = false;
        AssociatedQuestID = TEXT("");
    }
};

/**
 * Quest objective data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    FNarr_QuestObjective()
    {
        ObjectiveID = TEXT("");
        ObjectiveDescription = FText::GetEmpty();
        bIsCompleted = false;
        bIsOptional = false;
        TargetCount = 1;
        CurrentCount = 0;
    }
};

/**
 * Complete quest data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ENarr_QuestState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ConsciousnessReward;

    FNarr_QuestData()
    {
        QuestID = TEXT("");
        QuestTitle = FText::GetEmpty();
        QuestDescription = FText::GetEmpty();
        CurrentState = ENarr_QuestState::NotStarted;
        ExperienceReward = 0;
        ConsciousnessReward = 0;
    }
};

/**
 * Character lore and background data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterLore
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_ConsciousnessLevel ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString TribeAffiliation;

    FNarr_CharacterLore()
    {
        CharacterID = TEXT("");
        CharacterName = FText::GetEmpty();
        BackgroundStory = FText::GetEmpty();
        ConsciousnessLevel = ENarr_ConsciousnessLevel::Dormant;
        TribeAffiliation = TEXT("");
    }
};

/**
 * Data table row for dialogue data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueNode DialogueData;
};

/**
 * Data table row for quest data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FNarr_QuestData QuestData;
};

/**
 * Data table row for character lore
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FNarr_CharacterLore CharacterData;
};