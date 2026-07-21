#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Quest_PrehistoricSurvivalMissions.generated.h"

class ATranspersonalCharacter;
class UQuestManager;

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    HuntPrey            UMETA(DisplayName = "Hunt Prey"),
    GatherResources     UMETA(DisplayName = "Gather Resources"),
    BuildShelter        UMETA(DisplayName = "Build Shelter"),
    DefendTerritory     UMETA(DisplayName = "Defend Territory"),
    ExploreRegion       UMETA(DisplayName = "Explore Region"),
    SurviveStorm        UMETA(DisplayName = "Survive Storm"),
    TrackMigration      UMETA(DisplayName = "Track Migration"),
    CraftTools          UMETA(DisplayName = "Craft Tools"),
    RescueTribalMember  UMETA(DisplayName = "Rescue Tribal Member"),
    EstablishTrade      UMETA(DisplayName = "Establish Trade")
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Novice      UMETA(DisplayName = "Novice"),
    Apprentice  UMETA(DisplayName = "Apprentice"),
    Skilled     UMETA(DisplayName = "Skilled"),
    Expert      UMETA(DisplayName = "Expert"),
    Master      UMETA(DisplayName = "Master")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    FQuest_SurvivalMissionData()
    {
        MissionName = TEXT("Unknown Mission");
        MissionDescription = TEXT("No description available");
        MissionType = EQuest_SurvivalMissionType::GatherResources;
        Difficulty = EQuest_MissionDifficulty::Novice;
        RequiredLevel = 1;
        TimeLimit = 300.0f;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 1000.0f;
        ExperienceReward = 100;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 CurrentObjectiveIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<bool> ObjectiveCompletionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float ProgressPercentage;

    FQuest_MissionProgress()
    {
        bIsActive = false;
        bIsCompleted = false;
        ElapsedTime = 0.0f;
        CurrentObjectiveIndex = 0;
        ProgressPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_PrehistoricSurvivalMissions : public AActor
{
    GENERATED_BODY()

public:
    AQuest_PrehistoricSurvivalMissions();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    FQuest_SurvivalMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    FQuest_MissionProgress MissionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    bool bAutoGenerateMissions;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void InitializeMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool StartMission(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateMissionProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CheckMissionObjectives();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_SurvivalMissionData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_SurvivalMissionData GetCurrentMission() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool IsMissionActive() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    float GetMissionTimeRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CreateHuntMission(const FString& PreyType, const FVector& HuntingGrounds);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CreateGatherMission(const TArray<FString>& ResourceTypes, int32 RequiredAmount);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CreateExplorationMission(const FVector& TargetArea, float ExplorationRadius);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CreateDefenseMission(const FVector& DefensePoint, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CreateCraftingMission(const TArray<FString>& ItemsToCraft);

private:
    UPROPERTY()
    ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY()
    UQuestManager* QuestManager;

    FTimerHandle MissionUpdateTimer;
    FTimerHandle MissionGenerationTimer;

    void SetupMissionTemplates();
    void ValidateMissionRequirements();
    void CalculateMissionRewards();
    FVector FindSuitableMissionLocation(EQuest_SurvivalMissionType MissionType);
    void NotifyMissionUpdate();
    void HandleMissionTimeout();
    bool CheckPlayerRequirements(const FQuest_SurvivalMissionData& Mission);
    void SpawnMissionActors();
    void CleanupMissionActors();
};

#include "Quest_PrehistoricSurvivalMissions.generated.h"