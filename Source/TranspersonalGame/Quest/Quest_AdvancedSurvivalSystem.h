#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Quest_AdvancedSurvivalSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalChallengeType : uint8
{
    None = 0,
    WaterGathering = 1,
    HuntingChallenge = 2,
    CraftingMastery = 3,
    ShelterBuilding = 4,
    ResourceManagement = 5,
    TribalDiplomacy = 6,
    TerritoryDefense = 7,
    SeasonalMigration = 8
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None = 0,
    Stone = 1,
    Wood = 2,
    Fiber = 3,
    Food = 4,
    Water = 5,
    Hide = 6,
    Bone = 7,
    Metal = 8
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    EQuest_SurvivalChallengeType ChallengeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    EQuest_ResourceType RequiredResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    int32 TargetAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    FVector TargetLocation;

    FQuest_SurvivalObjective()
    {
        ObjectiveName = TEXT("Unnamed Objective");
        ChallengeType = EQuest_SurvivalChallengeType::None;
        RequiredResource = EQuest_ResourceType::None;
        TargetAmount = 1;
        CurrentAmount = 0;
        TimeLimit = 300.0f; // 5 minutes default
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FQuest_TribalRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Relations")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Relations")
    float RelationshipValue; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Relations")
    bool bIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Relations")
    bool bIsAllied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Relations")
    TArray<FString> CompletedQuests;

    FQuest_TribalRelationship()
    {
        TribeName = TEXT("Unknown Tribe");
        RelationshipValue = 0.0f;
        bIsHostile = false;
        bIsAllied = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_AdvancedSurvivalSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_AdvancedSurvivalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core survival quest management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival System")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival System")
    TArray<FQuest_TribalRelationship> TribalRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival System")
    float SurvivalDifficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival System")
    int32 DaysSurvived;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival System")
    bool bSeasonalChallengesEnabled;

    // Quest generation and management
    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void GenerateRandomSurvivalQuest(EQuest_SurvivalChallengeType ChallengeType);

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void CompleteObjective(int32 ObjectiveIndex, int32 AmountCompleted);

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    bool CheckObjectiveCompletion(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void UpdateTribalRelationship(const FString& TribeName, float RelationshipChange);

    // Resource management
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddResourceToInventory(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    int32 GetResourceAmount(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool HasSufficientResources(EQuest_ResourceType ResourceType, int32 RequiredAmount);

    // Seasonal and environmental challenges
    UFUNCTION(BlueprintCallable, Category = "Environmental Challenges")
    void TriggerSeasonalEvent();

    UFUNCTION(BlueprintCallable, Category = "Environmental Challenges")
    void HandleWeatherChange(const FString& WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Environmental Challenges")
    void SpawnPredatorThreat(const FVector& ThreatLocation);

private:
    // Internal resource tracking
    UPROPERTY()
    TMap<EQuest_ResourceType, int32> PlayerInventory;

    UPROPERTY()
    float LastQuestGenerationTime;

    UPROPERTY()
    float QuestGenerationInterval;

    // Helper functions
    void InitializeDefaultObjectives();
    void UpdateObjectiveProgress(float DeltaTime);
    FQuest_SurvivalObjective CreateHuntingQuest();
    FQuest_SurvivalObjective CreateCraftingQuest();
    FQuest_SurvivalObjective CreateGatheringQuest();
    void NotifyPlayerOfQuestUpdate(const FString& Message);
};