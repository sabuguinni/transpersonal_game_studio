#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Quest_SurvivalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    HuntDinosaur UMETA(DisplayName = "Hunt Dinosaur"),
    GatherResources UMETA(DisplayName = "Gather Resources"),
    CraftTools UMETA(DisplayName = "Craft Tools"),
    ExploreTerritory UMETA(DisplayName = "Explore Territory"),
    DefendCamp UMETA(DisplayName = "Defend Camp"),
    RescueTribalMember UMETA(DisplayName = "Rescue Tribal Member"),
    TrackMigration UMETA(DisplayName = "Track Migration"),
    BuildShelter UMETA(DisplayName = "Build Shelter")
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Novice UMETA(DisplayName = "Novice"),
    Hunter UMETA(DisplayName = "Hunter"),
    Veteran UMETA(DisplayName = "Veteran"),
    Elder UMETA(DisplayName = "Elder"),
    Legendary UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    FQuest_SurvivalMissionData()
    {
        MissionName = TEXT("Unnamed Mission");
        Description = TEXT("No description available");
        MissionType = EQuest_SurvivalMissionType::GatherResources;
        Difficulty = EQuest_MissionDifficulty::Novice;
        ExperienceReward = 100;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 600.0f; // 10 minutes default
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<FQuest_SurvivalMissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<FQuest_SurvivalMissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    float MissionGenerationInterval;

    FTimerHandle MissionGenerationTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool AssignMissionToPlayer(const FQuest_SurvivalMissionData& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void CompleteMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    TArray<FQuest_SurvivalMissionData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FQuest_SurvivalMissionData CreateHuntingMission(const FString& DinosaurType, const FVector& HuntLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FQuest_SurvivalMissionData CreateGatheringMission(const TArray<FString>& ResourceTypes, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FQuest_SurvivalMissionData CreateCraftingMission(const FString& ItemToCraft);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FQuest_SurvivalMissionData CreateExplorationMission(const FVector& ExploreLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void UpdateMissionProgress(int32 MissionIndex, float ProgressPercent);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool IsMissionExpired(const FQuest_SurvivalMissionData& Mission) const;

private:
    void InitializeMissionTemplates();
    void CleanupExpiredMissions();
    EQuest_MissionDifficulty CalculateMissionDifficulty(EQuest_SurvivalMissionType MissionType) const;
    int32 CalculateExperienceReward(EQuest_MissionDifficulty Difficulty) const;
};