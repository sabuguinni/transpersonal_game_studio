#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../Quest/Quest_MissionFramework.h"
#include "../SharedTypes.h"
#include "Narr_QuestDialogueIntegration.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    ENarr_EmotionalState EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    EQuest_MissionType AssociatedMissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    bool bIsQuestComplete;

    FNarr_QuestDialogueNode()
    {
        DialogueText = TEXT("");
        CharacterName = TEXT("");
        EmotionalTone = ENarr_EmotionalState::Neutral;
        AssociatedMissionType = EQuest_MissionType::Hunt;
        bIsQuestComplete = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_MissionBriefing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString BriefingTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString BriefingDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString ObjectiveDetails;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString WarningText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    EQuest_DifficultyLevel DifficultyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    float EstimatedDuration;

    FNarr_MissionBriefing()
    {
        BriefingTitle = TEXT("");
        BriefingDescription = TEXT("");
        ObjectiveDetails = TEXT("");
        WarningText = TEXT("");
        MissionType = EQuest_MissionType::Hunt;
        DifficultyLevel = EQuest_DifficultyLevel::Easy;
        EstimatedDuration = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_QuestDialogueIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_QuestDialogueIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    TArray<FNarr_QuestDialogueNode> QuestDialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefings")
    TArray<FNarr_MissionBriefing> MissionBriefings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Voices")
    TMap<FString, FString> CharacterVoiceMapping;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FNarr_QuestDialogueNode GetQuestDialogue(EQuest_MissionType MissionType, bool bIsComplete);

    UFUNCTION(BlueprintCallable, Category = "Mission Briefing")
    FNarr_MissionBriefing GetMissionBriefing(EQuest_MissionType MissionType, EQuest_DifficultyLevel Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void InitializeQuestDialogues();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void UpdateQuestProgress(EQuest_MissionType MissionType, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    FString GetCharacterVoiceID(const FString& CharacterName);

private:
    void SetupHuntMissionDialogues();
    void SetupGatherMissionDialogues();
    void SetupExploreMissionDialogues();
    void SetupCraftMissionDialogues();
    void SetupRescueMissionDialogues();
    void SetupDefenseMissionDialogues();
    void SetupDiscoveryMissionDialogues();

    void CreateMissionBriefings();
    void SetupCharacterVoices();
};