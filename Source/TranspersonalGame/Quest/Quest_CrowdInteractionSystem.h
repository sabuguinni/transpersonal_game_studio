#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_CrowdInteractionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdMissionType : uint8
{
    HerdProtection,
    MigrationGuide,
    PredatorDistraction,
    ResourceGathering,
    TerritoryDefense
};

USTRUCT(BlueprintType)
struct FQuest_CrowdObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_CrowdMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_CrowdObjective()
    {
        ObjectiveName = TEXT("Default Objective");
        MissionType = EQuest_CrowdMissionType::HerdProtection;
        TargetCrowdSize = 10;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 2000.0f;
        TimeLimit = 300.0f;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdInteractionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdInteractionSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_CrowdObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxSimultaneousObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bSystemEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FVector PlayerLastKnownPosition;

    FTimerHandle ObjectiveUpdateTimer;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartCrowdMission(EQuest_CrowdMissionType MissionType, FVector TargetLocation, int32 CrowdSize = 20);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteObjective(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckObjectiveCompletion(const FQuest_CrowdObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<AActor*> GetCrowdActorsInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHerdProtectionMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateMigrationGuideMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreatePredatorDistractionMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetActiveMissionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ClearAllObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FString GetMissionStatusReport();
};