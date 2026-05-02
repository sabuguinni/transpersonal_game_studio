#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    FindWater           UMETA(DisplayName = "Find Water Source"),
    GatherResources     UMETA(DisplayName = "Gather Resources"),
    HuntPrey           UMETA(DisplayName = "Hunt Small Prey"),
    SurviveDanger      UMETA(DisplayName = "Survive Danger Zone"),
    CraftTool          UMETA(DisplayName = "Craft Survival Tool"),
    ExploreArea        UMETA(DisplayName = "Explore New Area")
};

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    NotStarted         UMETA(DisplayName = "Not Started"),
    Active             UMETA(DisplayName = "Active"),
    Completed          UMETA(DisplayName = "Completed"),
    Failed             UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    FQuest_Objective()
    {
        ObjectiveID = "";
        Title = "";
        Description = "";
        ObjectiveType = EQuest_ObjectiveType::FindWater;
        Status = EQuest_Status::NotStarted;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        RequiredCount = 1;
        CurrentCount = 0;
        bIsOptional = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status MissionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Priority;

    FQuest_SurvivalMission()
    {
        MissionID = "";
        MissionName = "";
        MissionDescription = "";
        MissionStatus = EQuest_Status::NotStarted;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeSurvivalQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& MissionID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckObjectiveCompletion(const FVector& PlayerLocation, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GetMissionByID(const FString& MissionID) const;

    // Survival-specific quest functions
    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void CreateWaterSourceQuest();

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void CreateHuntingQuest();

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void CreateResourceGatheringQuest();

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void CreateSurvivalChallengeQuest();

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void OnPlayerEnterDangerZone();

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void OnPlayerFindWaterSource();

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void OnPlayerHuntSuccess();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    TArray<FQuest_SurvivalMission> AllMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bAutoStartFirstMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float DefaultObjectiveRadius;

private:
    float LastObjectiveCheckTime;
    
    void CheckAllObjectives();
    void ProcessMissionCompletion(FQuest_SurvivalMission& Mission);
    FQuest_Objective CreateWaterObjective();
    FQuest_Objective CreateHuntObjective();
    FQuest_Objective CreateGatherObjective();
};