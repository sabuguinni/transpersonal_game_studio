#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_SurvivalMissionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    Hunt_DinosaurPack       UMETA(DisplayName = "Hunt Dinosaur Pack"),
    Gather_Resources        UMETA(DisplayName = "Gather Resources"),
    Build_Shelter           UMETA(DisplayName = "Build Shelter"),
    Explore_Territory       UMETA(DisplayName = "Explore Territory"),
    Defend_Camp             UMETA(DisplayName = "Defend Camp"),
    Rescue_Survivor         UMETA(DisplayName = "Rescue Survivor"),
    Track_Migration         UMETA(DisplayName = "Track Migration"),
    Craft_Tools             UMETA(DisplayName = "Craft Tools"),
    Find_Water              UMETA(DisplayName = "Find Water Source"),
    Escape_Predator         UMETA(DisplayName = "Escape Predator")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    FQuest_SurvivalObjective()
    {
        Description = TEXT("Default Objective");
        TargetCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_SurvivalObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector MissionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Default Mission");
        MissionDescription = TEXT("A basic survival mission");
        MissionType = EQuest_SurvivalMissionType::Gather_Resources;
        Status = EQuest_MissionStatus::NotStarted;
        TimeLimit = 600.0f; // 10 minutes
        ElapsedTime = 0.0f;
        Priority = 1;
        MissionLocation = FVector::ZeroVector;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailMission(int32 MissionIndex, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AbandonMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(int32 MissionIndex, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckObjectiveCompletion(int32 MissionIndex, int32 ObjectiveIndex, const FVector& PlayerLocation);

    // Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GenerateHuntMission(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GenerateGatherMission(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GenerateExploreMission(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GenerateDefenseMission(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GenerateRescueMission(const FVector& Location);

    // Mission Queries
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMission GetMissionByIndex(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetActiveMissionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool HasActiveMissionOfType(EQuest_SurvivalMissionType MissionType) const;

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerEnteredArea(const FVector& PlayerLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerKilledDinosaur(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerGatheredResource(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerCraftedItem(const FString& ItemType);

protected:
    // Mission Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> AvailableMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionGenerationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<EQuest_SurvivalMissionType> AllowedMissionTypes;

    // Internal State
    UPROPERTY()
    float LastMissionGenerationTime;

    UPROPERTY()
    class ATranspersonalCharacter* PlayerCharacter;

    // Mission Generation Helpers
    void GenerateRandomMissions();
    void CleanupExpiredMissions();
    FVector GetRandomLocationNearPlayer(float MinDistance, float MaxDistance) const;
    bool IsLocationSafe(const FVector& Location) const;
    void NotifyPlayerOfMissionUpdate(const FString& Message);
};