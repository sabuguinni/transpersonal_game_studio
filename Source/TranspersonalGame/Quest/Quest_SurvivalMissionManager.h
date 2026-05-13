#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EResourceType RequiredResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    FQuest_MissionObjective()
    {
        ObjectiveText = TEXT("Survive");
        RequiredResource = EResourceType::Stone;
        RequiredAmount = 1;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EBiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MissionPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Survival Mission");
        MissionDescription = TEXT("Complete survival objectives");
        TargetBiome = EBiomeType::Temperate;
        MissionPriority = 1;
        TimeLimit = 600.0f;
        bIsActive = false;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalMissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjective(const FString& MissionName, int32 ObjectiveIndex, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateGatheringMission(EResourceType ResourceType, int32 Amount, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateHuntingMission(EDinosaurSpecies TargetSpecies, FVector HuntingGrounds);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateExplorationMission(EBiomeType BiomeToExplore, FVector ExploreCenter);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateCraftingMission(EResourceType CraftedItem, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateShelterMission(FVector BuildLocation, float ShelterSize);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_SurvivalMission GetMissionByName(const FString& MissionName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsMissionCompleted(const FString& MissionName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckLocationObjectives(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckResourceObjectives(EResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GenerateRandomSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CleanupCompletedMissions();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FTimerHandle MissionGenerationTimer;

private:
    void InitializeDefaultMissions();
    void OnMissionGenerationTimer();
    FQuest_SurvivalMission CreateRandomGatheringMission();
    FQuest_SurvivalMission CreateRandomHuntingMission();
    FQuest_SurvivalMission CreateRandomExplorationMission();
};