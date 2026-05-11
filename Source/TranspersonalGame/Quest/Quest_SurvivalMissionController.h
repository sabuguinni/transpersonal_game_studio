#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    ESurvivalNeed RequiredNeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 TargetAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float DifficultyLevel;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Survival Mission");
        Description = TEXT("Complete survival objective");
        RequiredNeed = ESurvivalNeed::Food;
        TargetAmount = 1;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        DifficultyLevel = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float HealthBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float StaminaBonus;

    FQuest_MissionReward()
    {
        ExperiencePoints = 100;
        HealthBonus = 0.0f;
        StaminaBonus = 0.0f;
    }
};

UCLASS(Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MissionMarker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    bool bAutoGenerateMissions;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void GenerateNewMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool CheckMissionProgress(const FString& MissionName, int32 CurrentProgress);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FQuest_MissionReward CalculateReward(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void StartEmergencyMission(ESurvivalNeed CriticalNeed);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void UpdateMissionDifficulty(float PlayerLevel);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool CanAcceptNewMission() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void ProcessMissionTimeout(float DeltaTime);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    FQuest_SurvivalMission CreateHuntingMission();
    FQuest_SurvivalMission CreateGatheringMission();
    FQuest_SurvivalMission CreateCraftingMission();
    FQuest_SurvivalMission CreateExplorationMission();
    FQuest_SurvivalMission CreateSurvivalMission();

    float LastMissionGenerationTime;
    int32 MissionIdCounter;
};