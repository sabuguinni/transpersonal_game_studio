#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_MissionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    FQuest_MissionObjective()
    {
        ObjectiveText = TEXT("Default Objective");
        bCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Mission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    FQuest_Mission()
    {
        MissionName = TEXT("Default Mission");
        Description = TEXT("A basic survival mission");
        bActive = false;
        bCompleted = false;
        ExperienceReward = 100;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AQuest_MissionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MissionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ManagerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Mission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Mission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float ObjectiveCheckInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FQuest_Mission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& MissionName, int32 ObjectiveIndex, bool bComplete);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckObjectiveCompletion(const FQuest_MissionObjective& Objective, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_Mission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_Mission GetMissionByName(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntMission(const FString& DinosaurType, const FVector& HuntLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherMission(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExplorationMission(const FVector& ExploreLocation, float ExploreRadius);

private:
    void CheckAllObjectives();
    void GenerateRandomMissions();
    FTimer ObjectiveCheckTimer;
};