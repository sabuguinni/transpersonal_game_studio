#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_InteractiveQuestSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_InteractionType : uint8
{
    Dialogue        UMETA(DisplayName = "Dialogue"),
    QuestGiver      UMETA(DisplayName = "Quest Giver"),
    QuestReceiver   UMETA(DisplayName = "Quest Receiver"),
    Vendor          UMETA(DisplayName = "Vendor"),
    Informant       UMETA(DisplayName = "Informant"),
    Companion       UMETA(DisplayName = "Companion")
};

UENUM(BlueprintType)
enum class EQuest_NPCMood : uint8
{
    Friendly        UMETA(DisplayName = "Friendly"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Suspicious      UMETA(DisplayName = "Suspicious"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Excited         UMETA(DisplayName = "Excited")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString OptionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bLeadsToQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RelationshipChange;

    FQuest_DialogueOption()
    {
        OptionText = TEXT("");
        ResponseText = TEXT("");
        bLeadsToQuest = false;
        QuestID = TEXT("");
        RelationshipChange = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Friendliness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Helpfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Suspicion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Knowledge;

    FQuest_NPCPersonality()
    {
        Friendliness = 0.5f;
        Helpfulness = 0.5f;
        Suspicion = 0.3f;
        Courage = 0.5f;
        Knowledge = 0.4f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractiveNPC : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    FString NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    EQuest_InteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    EQuest_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Info")
    FQuest_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueOption> DialogueOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<FString> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float PlayerRelationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    int32 InteractionCount;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartInteraction(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ProcessDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    TArray<FString> GetAvailableDialogueOptions();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CanGiveQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GiveQuestToPlayer(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Relationship")
    void ModifyRelationship(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Mood")
    void UpdateMoodBasedOnRelationship();

private:
    void GenerateContextualDialogue();
    void UpdatePersonalityBasedResponses();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_InteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_InteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteract;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionPrompt;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsPlayerInRange();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EnableInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void DisableInteraction();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionAvailable();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionUnavailable();

private:
    UPROPERTY()
    AActor* CachedPlayerActor;

    void CheckPlayerProximity();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractiveQuestBoard : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveQuestBoard();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Board")
    TArray<FString> PostedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Board")
    int32 MaxQuestCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Board")
    float QuestRefreshTime;

    UFUNCTION(BlueprintCallable, Category = "Quest Board")
    void RefreshQuestBoard();

    UFUNCTION(BlueprintCallable, Category = "Quest Board")
    TArray<FString> GetAvailableQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest Board")
    void AddQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Board")
    void RemoveQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Board")
    void PostCommunityQuest(const FString& QuestID, int32 Priority);

private:
    FTimerHandle QuestRefreshTimer;
    void AutoRefreshQuests();
};