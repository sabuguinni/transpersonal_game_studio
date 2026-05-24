#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    Hunt_Dinosaur       UMETA(DisplayName = "Hunt Dinosaur"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Build_Shelter       UMETA(DisplayName = "Build Shelter"),
    Explore_Territory   UMETA(DisplayName = "Explore Territory"),
    Defend_Camp         UMETA(DisplayName = "Defend Camp"),
    Craft_Tools         UMETA(DisplayName = "Craft Tools"),
    Find_Water          UMETA(DisplayName = "Find Water Source"),
    Escape_Predator     UMETA(DisplayName = "Escape Predator")
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Easy        UMETA(DisplayName = "Easy"),
    Medium      UMETA(DisplayName = "Medium"),
    Hard        UMETA(DisplayName = "Hard"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalMissionData
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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float RemainingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    FQuest_SurvivalMissionData()
    {
        MissionName = TEXT("Unknown Mission");
        Description = TEXT("No description");
        MissionType = EQuest_SurvivalMissionType::Gather_Resources;
        Difficulty = EQuest_MissionDifficulty::Easy;
        RequiredCount = 1;
        CurrentProgress = 0;
        TimeLimit = 300.0f;
        RemainingTime = 300.0f;
        bIsActive = false;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalMissionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void StartMission(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void UpdateMissionProgress(int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    bool IsMissionActive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    FQuest_SurvivalMissionData GetCurrentMission() const;

    // Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void GenerateDinosaurHuntMission(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void GenerateResourceGatheringMission(const TArray<FString>& Resources);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void GenerateExplorationMission(const FVector& TargetArea);

    // Mission Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnMissionStarted(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnMissionCompleted(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnMissionFailed(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnMissionProgressUpdated(const FQuest_SurvivalMissionData& MissionData);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Data")
    FQuest_SurvivalMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    int32 MaxActiveMissions;

private:
    FTimerHandle MissionTimerHandle;
    FTimerHandle GenerationTimerHandle;

    void UpdateMissionTimer();
    void OnMissionTimeExpired();
    void SetupMissionData(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty);
    FVector GetRandomExplorationTarget() const;
    TArray<FString> GetRandomResourceList() const;
    FString GetRandomDinosaurType() const;
};