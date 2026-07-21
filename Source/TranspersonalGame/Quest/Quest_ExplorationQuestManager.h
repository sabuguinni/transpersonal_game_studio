#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Quest_ExplorationQuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ExplorationObjectiveType : uint8
{
    DiscoverLocation    UMETA(DisplayName = "Discover Location"),
    CollectItem        UMETA(DisplayName = "Collect Item"),
    ReachElevation     UMETA(DisplayName = "Reach Elevation"),
    CrossTerrain       UMETA(DisplayName = "Cross Terrain"),
    FindResource       UMETA(DisplayName = "Find Resource")
};

USTRUCT(BlueprintType)
struct FQuest_ExplorationObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    EQuest_ExplorationObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
    int32 ExperienceReward;

    FQuest_ExplorationObjective()
    {
        ObjectiveName = TEXT("Unknown Objective");
        Description = TEXT("Explore the unknown");
        ObjectiveType = EQuest_ExplorationObjectiveType::DiscoverLocation;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        bIsCompleted = false;
        ExperienceReward = 100;
    }
};

USTRUCT(BlueprintType)
struct FQuest_ExplorationQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ExplorationObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TotalExperienceReward;

    FQuest_ExplorationQuest()
    {
        QuestName = TEXT("Exploration Quest");
        QuestDescription = TEXT("Explore the prehistoric world");
        bIsActive = false;
        bIsCompleted = false;
        TotalExperienceReward = 500;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ExplorationQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ExplorationQuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_ExplorationQuest> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_ExplorationQuest> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    TArray<AActor*> DiscoveryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
    TArray<ATriggerBox*> ExplorationTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bDebugMode;

    FTimerHandle ObjectiveCheckTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartExplorationQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CompleteObjective(const FString& QuestName, const FString& ObjectiveName);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool IsQuestActive(const FString& QuestName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool IsObjectiveCompleted(const FString& QuestName, const FString& ObjectiveName) const;

    UFUNCTION(BlueprintCallable, Category = "Discovery")
    void RegisterDiscoveryItem(AActor* DiscoveryActor);

    UFUNCTION(BlueprintCallable, Category = "Discovery")
    void RegisterExplorationTrigger(ATriggerBox* TriggerBox);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void CheckPlayerProximityToObjectives(APawn* PlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Quest Progress")
    float GetQuestCompletionPercentage(const FString& QuestName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Progress")
    TArray<FString> GetActiveQuestNames() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    int32 CalculateQuestReward(const FString& QuestName) const;

protected:
    UFUNCTION()
    void CheckObjectiveProgress();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnQuestStarted(const FString& QuestName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnObjectiveCompleted(const FString& QuestName, const FString& ObjectiveName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnQuestCompleted(const FString& QuestName, int32 ExperienceReward);

    void InitializeDefaultQuests();
    bool CheckLocationObjective(const FQuest_ExplorationObjective& Objective, APawn* PlayerPawn);
    void MarkObjectiveComplete(FQuest_ExplorationQuest& Quest, FQuest_ExplorationObjective& Objective);
};