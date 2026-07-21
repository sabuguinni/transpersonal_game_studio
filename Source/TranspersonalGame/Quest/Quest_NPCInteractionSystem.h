#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Quest_NPCInteractionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_NPCRole : uint8
{
    VillageElder    UMETA(DisplayName = "Village Elder"),
    HunterGuide     UMETA(DisplayName = "Hunter Guide"),
    Tracker         UMETA(DisplayName = "Tracker"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class EQuest_NPCMood : uint8
{
    Friendly        UMETA(DisplayName = "Friendly"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Worried         UMETA(DisplayName = "Worried"),
    Angry           UMETA(DisplayName = "Angry"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Excited         UMETA(DisplayName = "Excited"),
    Desperate       UMETA(DisplayName = "Desperate")
};

USTRUCT(BlueprintType)
struct FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuest_NPCMood RequiredMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FQuest_NPCDialogue()
    {
        DialogueText = TEXT("Hello, traveler.");
        SpeakerName = TEXT("Unknown");
        RequiredMood = EQuest_NPCMood::Neutral;
        bIsQuestRelated = false;
        QuestID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FQuest_NPCQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardExperience;

    FQuest_NPCQuestData()
    {
        QuestID = TEXT("");
        QuestTitle = TEXT("");
        QuestDescription = TEXT("");
        bIsAvailable = false;
        bIsCompleted = false;
        RewardExperience = 100;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_NPCInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_NPCInteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EQuest_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EQuest_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bCanInteract;

    // Dialogue System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_NPCDialogue> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FQuest_NPCDialogue CurrentDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bInConversation;

    // Quest System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_NPCQuestData> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasActiveQuest;

    // Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanPlayerInteract(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartInteraction(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_NPCDialogue GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogue(const FString& DialogueText, EQuest_NPCMood Mood);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_NPCQuestData> GetAvailableQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GiveQuestToPlayer(const FString& QuestID, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void SetNPCMood(EQuest_NPCMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    FString GetNPCRoleString();

private:
    AActor* InteractingPlayer;
    float LastInteractionTime;

    void UpdateDialogueBasedOnMood();
    void InitializeQuestData();
    void InitializeDialogues();
};