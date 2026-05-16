#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Quest_MissionObjectiveSystem.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class ACrowd_MassEntitySpawner;

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Dinosaur       UMETA(DisplayName = "Hunt Specific Dinosaur"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Explore_Area        UMETA(DisplayName = "Explore Territory"),
    Survive_Time        UMETA(DisplayName = "Survive Duration"),
    Craft_Item          UMETA(DisplayName = "Craft Specific Item"),
    Follow_Crowd        UMETA(DisplayName = "Follow NPC Group"),
    Protect_NPCs        UMETA(DisplayName = "Protect NPCs"),
    Reach_Location      UMETA(DisplayName = "Reach Destination")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Not_Started     UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Swamp"),
    Floresta    UMETA(DisplayName = "Forest"),
    Deserto     UMETA(DisplayName = "Desert"),
    Montanha    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_BiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float ElapsedTime;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        ObjectiveName = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Gather_Resources;
        TargetCount = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        TargetBiome = EQuest_BiomeType::Savana;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
    }
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
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsMainStoryMission;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        Status = EQuest_MissionStatus::Not_Started;
        Priority = 1;
        MissionTimeLimit = 0.0f;
        MissionElapsedTime = 0.0f;
        bIsMainStoryMission = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionObjectiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_MissionObjectiveComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void StartMission(const FQuest_MissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void AbandonMission(const FString& MissionID);

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void CompleteObjective(const FString& MissionID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    bool CheckObjectiveCompletion(const FString& MissionID, const FString& ObjectiveID);

    // Mission Queries
    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    FQuest_MissionData GetMissionByID(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    bool HasActiveMission() const;

    // Crowd Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void RegisterCrowdObjective(const FString& MissionID, const FString& ObjectiveID, ACrowd_MassEntitySpawner* CrowdSpawner);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void CheckCrowdProximity(const FVector& PlayerLocation);

    // Location Tracking
    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    EQuest_BiomeType GetBiomeFromLocation(const FVector& Location);

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest Mission")
    void CreateTestMissions();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest Mission")
    void DebugPrintActiveMissions();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission")
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission")
    TArray<FQuest_MissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Mission")
    float ProximityCheckDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Mission")
    float BiomeCheckRadius;

    // Crowd Integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission")
    TMap<FString, ACrowd_MassEntitySpawner*> CrowdObjectiveMap;

private:
    void UpdateMissionTimers(float DeltaTime);
    void CheckMissionFailures();
    void NotifyObjectiveComplete(const FQuest_ObjectiveData& Objective);
    void NotifyMissionComplete(const FQuest_MissionData& Mission);
    FQuest_MissionData CreateHuntMission();
    FQuest_MissionData CreateGatherMission();
    FQuest_MissionData CreateExploreMission();
    FQuest_MissionData CreateCrowdMission();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MissionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MissionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Distribution
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void AssignMissionToPlayer(ATranspersonalCharacter* Player, const FQuest_MissionData& Mission);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void GenerateCrowdBasedMission();

    // Mission Templates
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FQuest_MissionData CreateSurvivalMission(EQuest_BiomeType Biome, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FQuest_MissionData CreateHuntingMission(EQuest_BiomeType Biome, const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FQuest_MissionData CreateExplorationMission(EQuest_BiomeType Biome, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FQuest_MissionData CreateCrowdInteractionMission(ACrowd_MassEntitySpawner* CrowdSpawner);

    // System Management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest Manager")
    void InitializeMissionSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest Manager")
    void SpawnMissionObjectives();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Manager")
    TArray<FQuest_MissionData> MissionTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Manager")
    int32 MaxActiveMissionsPerPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Manager")
    float MissionGenerationInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Manager")
    float LastMissionGenerationTime;

    // Biome Coordinates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Manager")
    TMap<EQuest_BiomeType, FVector> BiomeLocations;

private:
    void SetupBiomeLocations();
    FString GenerateUniqueID();
    int32 MissionCounter;
};