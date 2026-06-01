#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Quest_NPCInteractionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_NPCInteractionType : uint8
{
    None UMETA(DisplayName = "None"),
    Trade UMETA(DisplayName = "Trade"),
    Information UMETA(DisplayName = "Information"),
    QuestGiver UMETA(DisplayName = "Quest Giver"),
    Companion UMETA(DisplayName = "Companion"),
    Hostile UMETA(DisplayName = "Hostile"),
    Neutral UMETA(DisplayName = "Neutral"),
    Ally UMETA(DisplayName = "Ally"),
    Merchant UMETA(DisplayName = "Merchant")
};

UENUM(BlueprintType)
enum class EQuest_NPCMood : uint8
{
    Friendly UMETA(DisplayName = "Friendly"),
    Neutral UMETA(DisplayName = "Neutral"),
    Suspicious UMETA(DisplayName = "Suspicious"),
    Hostile UMETA(DisplayName = "Hostile"),
    Fearful UMETA(DisplayName = "Fearful"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Helpful UMETA(DisplayName = "Helpful"),
    Indifferent UMETA(DisplayName = "Indifferent")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuest_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 DialogueID;

    FQuest_NPCDialogueData()
    {
        DialogueText = TEXT("Hello, traveler.");
        PlayerResponses.Add(TEXT("Hello"));
        PlayerResponses.Add(TEXT("Goodbye"));
        NPCName = TEXT("Unknown NPC");
        CurrentMood = EQuest_NPCMood::Neutral;
        bIsQuestRelated = false;
        DialogueID = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EQuest_NPCInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EQuest_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float RelationshipLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FQuest_NPCDialogueData> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanTrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bHasQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    int32 LastInteractionTime;

    FQuest_NPCInteractionData()
    {
        NPCName = TEXT("Unknown NPC");
        InteractionType = EQuest_NPCInteractionType::Neutral;
        CurrentMood = EQuest_NPCMood::Neutral;
        RelationshipLevel = 0.0f;
        bCanTrade = false;
        bHasQuest = false;
        InteractionDistance = 200.0f;
        LastInteractionTime = 0;
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

    // NPC Registration
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void RegisterNPC(const FString& NPCName, EQuest_NPCInteractionType InteractionType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void UnregisterNPC(const FString& NPCName);

    // Interaction Management
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool StartInteraction(const FString& NPCName, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndInteraction(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool IsInInteraction() const;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FQuest_NPCDialogueData GetCurrentDialogue(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SelectPlayerResponse(const FString& NPCName, int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void AddDialogueToNPC(const FString& NPCName, const FQuest_NPCDialogueData& DialogueData);

    // Relationship System
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void ModifyRelationship(const FString& NPCName, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    float GetRelationshipLevel(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SetNPCMood(const FString& NPCName, EQuest_NPCMood NewMood);

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SetNPCHasQuest(const FString& NPCName, bool bHasQuest);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool DoesNPCHaveQuest(const FString& NPCName) const;

    // Proximity Detection
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    TArray<FString> GetNearbyNPCs(const FVector& PlayerLocation, float SearchRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FString GetClosestInteractableNPC(const FVector& PlayerLocation) const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Interaction")
    void OnInteractionStarted(const FString& NPCName, const FQuest_NPCInteractionData& InteractionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Interaction")
    void OnInteractionEnded(const FString& NPCName);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Interaction")
    void OnDialogueChanged(const FQuest_NPCDialogueData& DialogueData);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Interaction")
    void OnRelationshipChanged(const FString& NPCName, float NewRelationshipLevel);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    TMap<FString, FQuest_NPCInteractionData> RegisteredNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    TMap<FString, FVector> NPCLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction State")
    FString CurrentInteractionNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction State")
    bool bIsCurrentlyInteracting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float DefaultInteractionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float RelationshipDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    int32 MaxDialogueHistory;

private:
    void InitializeDefaultDialogues();
    void UpdateNPCRelationships(float DeltaTime);
    FQuest_NPCDialogueData CreateDefaultDialogue(EQuest_NPCInteractionType InteractionType, EQuest_NPCMood Mood);
    void LogInteractionEvent(const FString& Event, const FString& NPCName);
};