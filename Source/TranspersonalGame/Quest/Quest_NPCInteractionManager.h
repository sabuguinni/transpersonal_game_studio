#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_NPCInteractionManager.generated.h"

class ATranspersonalCharacter;
class UQuest_MissionObjective;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    bool bIsQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Dialogue")
    FString QuestID;

    FQuest_NPCDialogue()
    {
        NPCName = TEXT("");
        DialogueText = TEXT("");
        bIsQuestGiver = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    EQuestObjectiveType CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float ActivityTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<FString> AvailableQuests;

    FQuest_NPCBehaviorState()
    {
        NPCID = TEXT("");
        CurrentActivity = EQuestObjectiveType::Hunt;
        TargetLocation = FVector::ZeroVector;
        ActivityTimer = 0.0f;
        bIsInteractable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_NPCInteractionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_NPCInteractionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void RegisterNPC(const FString& NPCID, AActor* NPCActor, const FQuest_NPCBehaviorState& InitialState);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void UnregisterNPC(const FString& NPCID);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool StartNPCInteraction(const FString& NPCID, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void EndNPCInteraction(const FString& NPCID);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    FQuest_NPCDialogue GetNPCDialogue(const FString& NPCID);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void UpdateNPCBehavior(const FString& NPCID, const FQuest_NPCBehaviorState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    TArray<FString> GetNearbyInteractableNPCs(const FVector& PlayerLocation, float InteractionRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void SetNPCQuestAvailability(const FString& NPCID, const FString& QuestID, bool bIsAvailable);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool CanNPCGiveQuest(const FString& NPCID, const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void ProcessNPCDailyRoutines(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void SpawnTribalNPCs();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC")
    TMap<FString, AActor*> RegisteredNPCs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC")
    TMap<FString, FQuest_NPCBehaviorState> NPCBehaviorStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC")
    TMap<FString, FQuest_NPCDialogue> NPCDialogues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPC")
    FString CurrentInteractingNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest NPC")
    float NPCUpdateInterval;

private:
    void InitializeTribalNPCDialogues();
    void UpdateNPCActivity(const FString& NPCID, float DeltaTime);
    FVector GetRandomPatrolLocation(const FVector& BaseLocation, float PatrolRadius = 1000.0f);
};