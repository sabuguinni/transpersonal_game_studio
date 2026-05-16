#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

// Survival mission types for prehistoric gameplay
UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    Hunt_Dinosaur       UMETA(DisplayName = "Hunt Dinosaur"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Build_Shelter       UMETA(DisplayName = "Build Shelter"),
    Explore_Territory   UMETA(DisplayName = "Explore Territory"),
    Defend_Camp         UMETA(DisplayName = "Defend Camp"),
    Track_Migration     UMETA(DisplayName = "Track Migration"),
    Craft_Tools         UMETA(DisplayName = "Craft Tools"),
    Find_Water          UMETA(DisplayName = "Find Water Source"),
    Escape_Predator     UMETA(DisplayName = "Escape Predator"),
    Rescue_Survivor     UMETA(DisplayName = "Rescue Survivor")
};

// Survival mission objectives
UENUM(BlueprintType)
enum class EQuest_SurvivalObjective : uint8
{
    Kill_Target         UMETA(DisplayName = "Kill Target"),
    Collect_Items       UMETA(DisplayName = "Collect Items"),
    Reach_Location      UMETA(DisplayName = "Reach Location"),
    Survive_Duration    UMETA(DisplayName = "Survive Duration"),
    Craft_Item          UMETA(DisplayName = "Craft Item"),
    Interact_Object     UMETA(DisplayName = "Interact Object"),
    Avoid_Detection     UMETA(DisplayName = "Avoid Detection"),
    Follow_Trail        UMETA(DisplayName = "Follow Trail"),
    Defend_Position     UMETA(DisplayName = "Defend Position"),
    Escort_NPC          UMETA(DisplayName = "Escort NPC")
};

// Survival mission difficulty levels
UENUM(BlueprintType)
enum class EQuest_SurvivalDifficulty : uint8
{
    Novice              UMETA(DisplayName = "Novice Hunter"),
    Experienced         UMETA(DisplayName = "Experienced Survivor"),
    Expert              UMETA(DisplayName = "Expert Tracker"),
    Master              UMETA(DisplayName = "Master Hunter"),
    Legendary           UMETA(DisplayName = "Legendary Survivor")
};

// Survival mission reward types
UENUM(BlueprintType)
enum class EQuest_SurvivalReward : uint8
{
    Food_Rations        UMETA(DisplayName = "Food Rations"),
    Crafting_Materials  UMETA(DisplayName = "Crafting Materials"),
    Tool_Upgrade        UMETA(DisplayName = "Tool Upgrade"),
    Territory_Access    UMETA(DisplayName = "Territory Access"),
    Tribe_Standing      UMETA(DisplayName = "Tribe Standing"),
    Survival_Knowledge  UMETA(DisplayName = "Survival Knowledge"),
    Weapon_Blueprint    UMETA(DisplayName = "Weapon Blueprint"),
    Safe_Passage        UMETA(DisplayName = "Safe Passage"),
    Resource_Cache      UMETA(DisplayName = "Resource Cache"),
    Ally_Support        UMETA(DisplayName = "Ally Support")
};

// Survival mission data structure
USTRUCT(BlueprintType)
struct FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalObjective PrimaryObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<EQuest_SurvivalObjective> SecondaryObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalReward RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RewardQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionPercentage;

    FQuest_SurvivalMissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        MissionType = EQuest_SurvivalMissionType::Hunt_Dinosaur;
        PrimaryObjective = EQuest_SurvivalObjective::Kill_Target;
        Difficulty = EQuest_SurvivalDifficulty::Novice;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 300.0f;
        RequiredQuantity = 1;
        TargetSpecies = TEXT("");
        RewardType = EQuest_SurvivalReward::Food_Rations;
        RewardQuantity = 1;
        bIsActive = false;
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

// Survival mission progress tracking
USTRUCT(BlueprintType)
struct FQuest_SurvivalProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 KillCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 ItemsCollected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float DistanceTraveled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float SurvivalTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    bool bLocationReached;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    bool bObjectInteracted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    bool bDetectionAvoided;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> CompletedObjectives;

    FQuest_SurvivalProgress()
    {
        MissionID = TEXT("");
        KillCount = 0;
        ItemsCollected = 0;
        DistanceTraveled = 0.0f;
        SurvivalTime = 0.0f;
        bLocationReached = false;
        bObjectInteracted = false;
        bDetectionAvoided = true;
    }
};

/**
 * Survival Mission System for prehistoric dinosaur survival gameplay
 * Manages hunting, gathering, crafting, and exploration missions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void InitializeMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString CreateHuntingMission(const FString& TargetSpecies, int32 Quantity, EQuest_SurvivalDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString CreateGatheringMission(const TArray<FString>& RequiredItems, int32 Quantity, float TimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString CreateExplorationMission(const FVector& TargetLocation, float Radius, EQuest_SurvivalDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString CreateCraftingMission(const FString& ItemToCraft, const TArray<FString>& RequiredMaterials);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString CreateDefenseMission(const FVector& DefenseLocation, float Duration, int32 WaveCount);

    // Mission progress tracking
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void UpdateMissionProgress(const FString& MissionID, const FQuest_SurvivalProgress& Progress);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool CheckMissionCompletion(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void FailMission(const FString& MissionID, const FString& FailureReason);

    // Mission queries
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    TArray<FQuest_SurvivalMissionData> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FQuest_SurvivalMissionData GetMissionData(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    TArray<FQuest_SurvivalMissionData> GetMissionsByType(EQuest_SurvivalMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    TArray<FQuest_SurvivalMissionData> GetMissionsByDifficulty(EQuest_SurvivalDifficulty Difficulty);

    // Mission rewards
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void GrantMissionReward(const FString& MissionID, class APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void ProcessReward(EQuest_SurvivalReward RewardType, int32 Quantity, class APlayerController* PlayerController);

    // Mission generation
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void GenerateRandomMissions(int32 Count, EQuest_SurvivalDifficulty MaxDifficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FQuest_SurvivalMissionData GenerateHuntingMission(EQuest_SurvivalDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FQuest_SurvivalMissionData GenerateGatheringMission(EQuest_SurvivalDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FQuest_SurvivalMissionData GenerateExplorationMission(EQuest_SurvivalDifficulty Difficulty);

protected:
    // Mission storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMissionData> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMissionData> CompletedMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TMap<FString, FQuest_SurvivalProgress> MissionProgress;

    // Mission configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    TArray<FString> AvailableSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    TArray<FString> GatherableItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Config")
    TArray<FVector> ExplorationPoints;

    // Internal systems
    UPROPERTY()
    FString NextMissionID;

    UPROPERTY()
    float LastUpdateTime;

private:
    // Helper functions
    FString GenerateUniqueMissionID();
    EQuest_SurvivalDifficulty GetRandomDifficulty(EQuest_SurvivalDifficulty MaxDifficulty);
    FVector GetRandomExplorationPoint();
    FString GetRandomSpecies();
    TArray<FString> GetRandomGatherableItems(int32 Count);
    float CalculateMissionDifficulty(const FQuest_SurvivalMissionData& Mission);
    bool ValidateMissionRequirements(const FQuest_SurvivalMissionData& Mission);
};