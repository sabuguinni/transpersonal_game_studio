#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_MissionManager.generated.h"

// Forward declarations
class UQuest_NPCInteractionManager;
class UQuest_SurvivalMissionManager;
class UQuest_DinosaurHuntManager;

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt            UMETA(DisplayName = "Hunt Mission"),
    Gather          UMETA(DisplayName = "Gather Mission"),
    Explore         UMETA(DisplayName = "Explore Mission"),
    Survive         UMETA(DisplayName = "Survive Mission"),
    Craft           UMETA(DisplayName = "Craft Mission"),
    Defend          UMETA(DisplayName = "Defend Mission"),
    Rescue          UMETA(DisplayName = "Rescue Mission"),
    Trade           UMETA(DisplayName = "Trade Mission")
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

UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    float CompletionRadius;

    FQuest_MissionObjective()
    {
        Description = TEXT("Unknown Objective");
        TargetCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EQuest_MissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString NPCGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FVector MissionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EBiomeType RequiredBiome;

    FQuest_MissionData()
    {
        MissionID = TEXT("MISSION_000");
        Title = TEXT("Unknown Mission");
        Description = TEXT("No description available");
        MissionType = EQuest_MissionType::Survive;
        Status = EQuest_MissionStatus::NotStarted;
        Priority = EQuest_MissionPriority::Medium;
        TimeLimit = 0.0f;
        TimeRemaining = 0.0f;
        ExperienceReward = 100;
        NPCGiver = TEXT("Unknown");
        MissionLocation = FVector::ZeroVector;
        RequiredBiome = EBiomeType::Savana;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnMissionStatusChanged, const FQuest_MissionData&, MissionData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnObjectiveCompleted, const FQuest_MissionObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnMissionCompleted, const FQuest_MissionData&, MissionData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_MissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_MissionManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    FString CreateMission(EQuest_MissionType MissionType, EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    bool AbandonMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    void UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    bool IsObjectiveCompleted(const FString& MissionID, int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    float GetMissionProgress(const FString& MissionID);

    // Mission Queries
    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    TArray<FQuest_MissionData> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    TArray<FQuest_MissionData> GetMissionsByType(EQuest_MissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    TArray<FQuest_MissionData> GetMissionsByPriority(EQuest_MissionPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    FQuest_MissionData GetMissionData(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    bool HasActiveMission(const FString& MissionID);

    // Auto-generation
    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    void GenerateRandomMissions(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    void GenerateBiomeSpecificMissions(EBiomeType Biome, int32 Count);

    // Integration with crowd system
    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    void CheckCrowdInteractions(const FVector& PlayerLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission Manager")
    TArray<AActor*> GetNearbyQuestNPCs(const FVector& Location, float Radius);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnMissionStatusChanged OnMissionStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnMissionCompleted OnMissionCompleted;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission Manager")
    TMap<FString, FQuest_MissionData> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission Manager")
    TMap<FString, FQuest_MissionData> CompletedMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission Manager")
    int32 NextMissionID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission Manager")
    float MissionUpdateInterval;

    FTimerHandle MissionUpdateTimer;

    // Helper functions
    FQuest_MissionData CreateHuntMission(EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);
    FQuest_MissionData CreateGatherMission(EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);
    FQuest_MissionData CreateExploreMission(EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);
    FQuest_MissionData CreateSurviveMission(EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);
    FQuest_MissionData CreateCraftMission(EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);
    FQuest_MissionData CreateDefendMission(EQuest_MissionPriority Priority, const FString& NPCGiver, EBiomeType Biome);

    UFUNCTION()
    void UpdateMissions();

    FVector GetBiomeLocation(EBiomeType Biome);
    FString GenerateUniqueMissionID();
};