#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Quest_SurvivalMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    None UMETA(DisplayName = "None"),
    GatherFood UMETA(DisplayName = "Gather Food"),
    FindWater UMETA(DisplayName = "Find Water"),
    BuildShelter UMETA(DisplayName = "Build Shelter"),
    CraftTool UMETA(DisplayName = "Craft Tool"),
    HuntPrey UMETA(DisplayName = "Hunt Prey"),
    EscapePredator UMETA(DisplayName = "Escape Predator"),
    ExploreArea UMETA(DisplayName = "Explore Area"),
    DefendTerritory UMETA(DisplayName = "Defend Territory")
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
    int32 TargetQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float DifficultyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    FQuest_SurvivalMissionData()
    {
        MissionName = TEXT("Unknown Mission");
        Description = TEXT("No description available");
        MissionType = EQuest_SurvivalMissionType::None;
        TargetQuantity = 1;
        TimeLimit = 300.0f; // 5 minutes default
        DifficultyLevel = 1.0f;
        TargetLocation = FVector::ZeroVector;
        bIsActive = false;
        bIsCompleted = false;
    }
};

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

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void StartMission(EQuest_SurvivalMissionType MissionType, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool IsMissionActive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FQuest_SurvivalMissionData GetCurrentMission() const;

    // Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void GenerateMissionBasedOnPlayerState();

    // Progress Tracking
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void UpdateMissionProgress(int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    float GetMissionProgress() const;

    // Event Handlers
    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Mission")
    void OnMissionStarted(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Mission")
    void OnMissionCompleted(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Mission")
    void OnMissionFailed(const FQuest_SurvivalMissionData& MissionData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Mission")
    void OnMissionProgressUpdated(float ProgressPercentage);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FQuest_SurvivalMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_SurvivalMissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float MissionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    int32 MaxSimultaneousMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Progress")
    float MissionTimer;

private:
    void InitializeMissionTemplates();
    void CheckMissionTimeout();
    FQuest_SurvivalMissionData CreateMissionFromType(EQuest_SurvivalMissionType Type, const FVector& Location);
    void LogMissionEvent(const FString& Event, const FString& Details);
};