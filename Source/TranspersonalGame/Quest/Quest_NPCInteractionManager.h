#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_NPCInteractionManager.generated.h"

class ATranspersonalCharacter;
class UQuest_DialogueSystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    FQuest_NPCDialogue()
    {
        DialogueText = TEXT("");
        NPCName = TEXT("Unknown");
        DialogueDuration = 5.0f;
        bIsQuestRelated = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ESurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ESurvivalUrgencyLevel UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RequiredItems;

    FQuest_NPCQuestData()
    {
        QuestID = TEXT("");
        QuestTitle = TEXT("Survival Task");
        QuestDescription = TEXT("Complete this task to survive");
        MissionType = ESurvivalMissionType::GatherWater;
        UrgencyLevel = ESurvivalUrgencyLevel::Medium;
        TargetLocation = FVector::ZeroVector;
        RewardExperience = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_NPCInteractionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_NPCInteractionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // NPC Interaction System
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void InitiateNPCDialogue(AActor* NPCActor, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndNPCDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool IsPlayerInInteractionRange(AActor* NPCActor, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void RegisterNPCQuestGiver(AActor* NPCActor, const FQuest_NPCQuestData& QuestData);

    // Quest Assignment System
    UFUNCTION(BlueprintCallable, Category = "Quest Assignment")
    void AssignQuestToPlayer(ATranspersonalCharacter* Player, const FQuest_NPCQuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest Assignment")
    bool CanAssignQuest(ATranspersonalCharacter* Player, const FQuest_NPCQuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest Assignment")
    void CompleteQuestForPlayer(ATranspersonalCharacter* Player, const FString& QuestID);

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNPCDialogue(const FQuest_NPCDialogue& DialogueData);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProcessPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableResponses();

protected:
    // Core Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float DialogueDisplayTime;

    // NPC Registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPCs")
    TMap<AActor*, FQuest_NPCQuestData> RegisteredNPCs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPCs")
    TArray<AActor*> ActiveNPCs;

    // Current Interaction State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    AActor* CurrentInteractingNPC;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ATranspersonalCharacter* CurrentPlayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsInDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FQuest_NPCDialogue CurrentDialogue;

    // Quest Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quests")
    TMap<FString, FQuest_NPCQuestData> AvailableQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quests")
    TMap<ATranspersonalCharacter*, TArray<FString>> PlayerActiveQuests;

private:
    // Internal Methods
    void UpdateNPCInteractions();
    void CheckPlayerProximity();
    FQuest_NPCDialogue GenerateContextualDialogue(AActor* NPCActor, ATranspersonalCharacter* Player);
    bool ValidateQuestRequirements(ATranspersonalCharacter* Player, const FQuest_NPCQuestData& QuestData);
};