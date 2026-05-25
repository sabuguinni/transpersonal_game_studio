#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UQuest_MissionObjective;

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather Resources"),
    Explore     UMETA(DisplayName = "Explore Area"),
    Build       UMETA(DisplayName = "Build Structure"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Escape      UMETA(DisplayName = "Escape Danger"),
    Rescue      UMETA(DisplayName = "Rescue NPC"),
    Defend      UMETA(DisplayName = "Defend Location")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_Priority : uint8
{
    Low         UMETA(DisplayName = "Low Priority"),
    Normal      UMETA(DisplayName = "Normal Priority"),
    High        UMETA(DisplayName = "High Priority"),
    Critical    UMETA(DisplayName = "Critical Priority")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> ObjectiveList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString RewardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredLevel;

    FQuest_MissionData()
    {
        MissionID = "";
        MissionName = "";
        Description = "";
        MissionType = EQuest_MissionType::Hunt;
        Status = EQuest_MissionStatus::Inactive;
        Priority = EQuest_Priority::Normal;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 0.0f;
        Progress = 0.0f;
        RewardDescription = "";
        bIsRepeatable = false;
        RequiredLevel = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector ObjectiveLocation;

    FQuest_SurvivalObjective()
    {
        ObjectiveID = "";
        Description = "";
        bIsCompleted = false;
        bIsOptional = false;
        TargetValue = 1.0f;
        CurrentValue = 0.0f;
        TargetActorClass = "";
        ObjectiveLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MissionTrigger : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MissionTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FQuest_MissionData MissionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bAutoActivateOnTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TriggerRadius;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

public:
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void ActivateMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void DeactivateMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsMissionActive() const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    FQuest_MissionData GetMissionData() const { return MissionData; }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_SurvivalMissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Missions")
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Missions")
    TArray<FQuest_MissionData> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Missions")
    TArray<FQuest_MissionData> AvailableMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Missions")
    FQuest_MissionData CurrentPrimaryMission;

    UPROPERTY(BlueprintReadOnly, Category = "Missions")
    bool bHasActivePrimaryMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MissionCheckInterval;

    FTimerHandle MissionUpdateTimer;

public:
    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool AbandonMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateMissionProgress(const FString& MissionID, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, float Value);

    // Mission Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission System")
    TArray<FQuest_MissionData> GetActiveMissions() const { return ActiveMissions; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission System")
    TArray<FQuest_MissionData> GetAvailableMissions() const { return AvailableMissions; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission System")
    FQuest_MissionData GetMissionByID(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission System")
    bool HasActiveMission(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission System")
    FQuest_MissionData GetPrimaryMission() const { return CurrentPrimaryMission; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission System")
    bool HasActivePrimaryMission() const { return bHasActivePrimaryMission; }

    // Mission Creation
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData CreateHuntMission(const FString& TargetSpecies, const FVector& HuntLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData CreateGatherMission(const FString& ResourceType, int32 Quantity, const FVector& GatherLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData CreateExploreMission(const FString& AreaName, const FVector& ExploreLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData CreateSurvivalMission(float SurvivalDuration, const FVector& SurvivalLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData CreateBuildMission(const FString& StructureType, const FVector& BuildLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData CreateEscapeMission(const FVector& EscapeFromLocation, const FVector& EscapeToLocation, float TimeLimit);

    // Event System
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void OnPlayerKillDinosaur(const FString& DinosaurSpecies, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void OnPlayerGatherResource(const FString& ResourceType, int32 Quantity, const FVector& GatherLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void OnPlayerEnterArea(const FString& AreaName, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void OnPlayerBuildStructure(const FString& StructureType, const FVector& BuildLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void OnPlayerSurviveTime(float SurvivedTime);

protected:
    void UpdateMissions();
    void CheckMissionTimeouts();
    void ProcessMissionEvents();
    
    FQuest_MissionData* FindMissionByID(const FString& MissionID);
    void AddMissionToAvailable(const FQuest_MissionData& Mission);
    void RemoveMissionFromActive(const FString& MissionID);
    
    FString GenerateUniqueMissionID();
    void InitializeDefaultMissions();
};