#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Quest_SurvivalObjectiveManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalObjectiveType : uint8
{
    FindWater,
    BuildShelter,
    GatherFood,
    CreateFire,
    CraftTool,
    EscapePredator,
    ExploreArea,
    SurviveNight
};

UENUM(BlueprintType)
enum class EQuest_SurvivalPriority : uint8
{
    Critical,
    High,
    Medium,
    Low
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    EQuest_SurvivalObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    EQuest_SurvivalPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float RemainingTime;

    FQuest_SurvivalObjective()
    {
        ObjectiveType = EQuest_SurvivalObjectiveType::FindWater;
        ObjectiveDescription = TEXT("Find a water source");
        Priority = EQuest_SurvivalPriority::High;
        TimeLimit = 300.0f;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        bIsCompleted = false;
        bIsActive = false;
        RemainingTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalObjectiveManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalObjectiveManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Survival objective management
    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void CreateSurvivalObjective(EQuest_SurvivalObjectiveType ObjectiveType, FVector Location, float TimeLimit = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void CompleteObjective(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void FailObjective(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    bool CheckObjectiveCompletion(int32 ObjectiveIndex, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void UpdateObjectivePriorities();

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    FQuest_SurvivalObjective GetHighestPriorityObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    TArray<FQuest_SurvivalObjective> GetActiveObjectives();

    // Environmental condition monitoring
    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void MonitorEnvironmentalThreats();

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void CreateEmergencyObjective(EQuest_SurvivalObjectiveType ObjectiveType, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void HandleNightfall();

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void HandleWeatherChange();

    // Player condition monitoring
    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void MonitorPlayerNeeds(float Health, float Hunger, float Thirst, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Quest Survival")
    void CreateNeedBasedObjective(float Health, float Hunger, float Thirst);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    TArray<FQuest_SurvivalObjective> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    int32 MaxActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    float ObjectiveUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    bool bNightSurvivalMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Survival")
    float CurrentTimeOfDay;

    // Timer handles
    FTimerHandle ObjectiveTimerHandle;
    FTimerHandle ThreatMonitorHandle;
    FTimerHandle EnvironmentCheckHandle;

private:
    void UpdateObjectiveTimers(float DeltaTime);
    void CheckForExpiredObjectives();
    void SortObjectivesByPriority();
    FVector FindNearestWaterSource();
    FVector FindSafeShelterLocation();
    bool IsLocationSafe(FVector Location);
    void SpawnObjectiveMarker(FVector Location, EQuest_SurvivalObjectiveType ObjectiveType);
};