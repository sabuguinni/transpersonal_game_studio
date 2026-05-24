#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QuestTypes.h"
#include "SurvivalMissionManager.generated.h"

class ATranspersonalCharacter;
class ASurvivalQuestNPC;
class AQuestTrigger;

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Survival_Water      UMETA(DisplayName = "Find Water"),
    Survival_Food       UMETA(DisplayName = "Gather Food"),
    Survival_Shelter    UMETA(DisplayName = "Build Shelter"),
    Survival_Fire       UMETA(DisplayName = "Create Fire"),
    Survival_Tools      UMETA(DisplayName = "Craft Tools"),
    Hunt_Herbivore      UMETA(DisplayName = "Hunt Herbivore"),
    Hunt_Predator       UMETA(DisplayName = "Survive Predator"),
    Explore_Territory   UMETA(DisplayName = "Explore Territory"),
    Rescue_Survivor     UMETA(DisplayName = "Rescue Survivor"),
    Defend_Camp         UMETA(DisplayName = "Defend Camp")
};

UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Optional    UMETA(DisplayName = "Optional")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    FQuest_MissionObjective()
    {
        ObjectiveText = TEXT("");
        bIsCompleted = false;
        bIsOptional = false;
        TargetCount = 1;
        CurrentCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TargetRadius;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        MissionType = EQuest_MissionType::Survival_Water;
        Priority = EQuest_MissionPriority::Medium;
        bIsActive = false;
        bIsCompleted = false;
        TimeLimit = 0.0f;
        TimeRemaining = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, const FQuest_SurvivalMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FQuest_SurvivalMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, const FQuest_SurvivalMission&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FQuest_SurvivalMission&, Mission, int32, ObjectiveIndex);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USurvivalMissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    USurvivalMissionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void StartMission(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void FailMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void UpdateObjective(const FString& MissionName, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool IsMissionActive(const FString& MissionName) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FQuest_SurvivalMission GetMissionByName(const FString& MissionName) const;

    // Pre-defined survival missions
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CreateWaterMission(const FVector& WaterLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CreateFoodMission(const FVector& FoodLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CreateShelterMission(const FVector& ShelterLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CreateHuntMission(const FVector& HuntLocation, EQuest_MissionType HuntType);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CreateExploreMission(const FVector& ExploreLocation);

    // Mission events
    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionStarted OnMissionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionFailed OnMissionFailed;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // Player interaction
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CheckPlayerLocation(ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void OnPlayerReachedLocation(const FVector& Location, float Radius);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMission> FailedMissions;

    // Timer management
    UPROPERTY()
    TMap<FString, FTimerHandle> MissionTimers;

    // Helper functions
    void OnMissionTimeExpired(FString MissionName);
    bool CheckMissionCompletion(FQuest_SurvivalMission& Mission);
    void BroadcastMissionUpdate(const FQuest_SurvivalMission& Mission);

private:
    // Internal mission tracking
    int32 FindMissionIndex(const FString& MissionName) const;
    void RemoveFromActiveMissions(const FString& MissionName);
    void SetupMissionTimer(const FQuest_SurvivalMission& Mission);
};