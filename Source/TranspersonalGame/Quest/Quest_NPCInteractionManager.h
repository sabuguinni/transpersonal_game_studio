#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_NPCInteractionManager.generated.h"

// Forward declarations
class ACrowdSimulationManager;
class UCrowd_PrehistoricNPCManager;

UENUM(BlueprintType)
enum class EQuest_NPCInteractionType : uint8
{
    Trade           UMETA(DisplayName = "Trade Resources"),
    Information     UMETA(DisplayName = "Gather Information"),
    Escort          UMETA(DisplayName = "Escort NPC"),
    Rescue          UMETA(DisplayName = "Rescue NPC"),
    Negotiate       UMETA(DisplayName = "Negotiate Territory"),
    Teaching        UMETA(DisplayName = "Learn Skills"),
    Alliance        UMETA(DisplayName = "Form Alliance"),
    Warning         UMETA(DisplayName = "Receive Warning")
};

USTRUCT(BlueprintType)
struct FQuest_NPCInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    EQuest_NPCInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    FVector NPCLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Interaction")
    float InteractionRange;

    FQuest_NPCInteractionData()
    {
        NPCName = TEXT("Unknown NPC");
        InteractionType = EQuest_NPCInteractionType::Information;
        NPCLocation = FVector::ZeroVector;
        DialogueText = TEXT("Hello, traveler.");
        bIsCompleted = false;
        InteractionRange = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_TribalQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Quest")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Quest")
    TArray<FQuest_NPCInteractionData> InteractionChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Quest")
    int32 CurrentInteractionIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Quest")
    bool bQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Quest")
    float QuestTimeLimit;

    FQuest_TribalQuestData()
    {
        QuestName = TEXT("Tribal Mission");
        TribeName = TEXT("Unknown Tribe");
        CurrentInteractionIndex = 0;
        bQuestActive = false;
        QuestTimeLimit = 3600.0f; // 1 hour
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_NPCInteractionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_NPCInteractionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // NPC Interaction Management
    UFUNCTION(BlueprintCallable, Category = "Quest NPC Interaction")
    void RegisterNPCForInteraction(const FString& NPCName, const FVector& Location, EQuest_NPCInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Interaction")
    bool StartNPCInteraction(const FString& NPCName, AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Interaction")
    void CompleteNPCInteraction(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Interaction")
    TArray<FQuest_NPCInteractionData> GetAvailableInteractions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Interaction")
    FQuest_NPCInteractionData GetNearestInteraction(const FVector& PlayerLocation, float MaxDistance = 1000.0f) const;

    // Tribal Quest System
    UFUNCTION(BlueprintCallable, Category = "Quest Tribal")
    void CreateTribalQuest(const FString& QuestName, const FString& TribeName, const TArray<FQuest_NPCInteractionData>& InteractionChain);

    UFUNCTION(BlueprintCallable, Category = "Quest Tribal")
    bool StartTribalQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Tribal")
    void AdvanceTribalQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Tribal")
    TArray<FQuest_TribalQuestData> GetActiveTribalQuests() const;

    // Integration with Crowd System
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void SyncWithCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void SpawnQuestNPCs();

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FString GetNPCDialogue(const FString& NPCName, EQuest_NPCInteractionType InteractionType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void SetCustomDialogue(const FString& NPCName, const FString& CustomDialogue);

protected:
    // Core data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    TArray<FQuest_NPCInteractionData> RegisteredNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    TArray<FQuest_TribalQuestData> TribalQuests;

    // System references
    UPROPERTY()
    ACrowdSimulationManager* CrowdManager;

    UPROPERTY()
    UCrowd_PrehistoricNPCManager* NPCManager;

    // Internal methods
    void InitializeDefaultInteractions();
    void CreateDefaultTribalQuests();
    bool ValidateNPCInteraction(const FQuest_NPCInteractionData& InteractionData) const;
    void UpdateQuestProgress();

private:
    // Quest tracking
    TMap<FString, float> QuestTimers;
    TMap<FString, int32> InteractionCounts;
    
    // Default dialogue templates
    TMap<EQuest_NPCInteractionType, FString> DefaultDialogues;
    
    void SetupDefaultDialogues();
};