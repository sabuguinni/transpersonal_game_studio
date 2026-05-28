#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Quest_SurvivalMissionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    HunterGatherer UMETA(DisplayName = "Hunter Gatherer"),
    ShelterBuilder UMETA(DisplayName = "Shelter Builder"),
    WaterFinder UMETA(DisplayName = "Water Finder"),
    FireKeeper UMETA(DisplayName = "Fire Keeper"),
    ToolCrafter UMETA(DisplayName = "Tool Crafter"),
    TerritoryDefender UMETA(DisplayName = "Territory Defender")
};

UENUM(BlueprintType)
enum class EQuest_SurvivalPriority : uint8
{
    Critical UMETA(DisplayName = "Critical"),
    High UMETA(DisplayName = "High"),
    Medium UMETA(DisplayName = "Medium"),
    Low UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float RequiredRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredResourceCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    FQuest_SurvivalMissionData()
    {
        MissionName = TEXT("Unknown Mission");
        MissionType = EQuest_SurvivalMissionType::HunterGatherer;
        Priority = EQuest_SurvivalPriority::Medium;
        TargetLocation = FVector::ZeroVector;
        RequiredRadius = 500.0f;
        RequiredResourceCount = 1;
        TimeLimit = 300.0f;
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

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CreateSurvivalMission(const FString& MissionName, EQuest_SurvivalMissionType MissionType, const FVector& Location, EQuest_SurvivalPriority Priority = EQuest_SurvivalPriority::Medium);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CancelMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    TArray<FQuest_SurvivalMissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    FQuest_SurvivalMissionData GetMissionByName(const FString& MissionName) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    TArray<FQuest_SurvivalMissionData> GetMissionsByType(EQuest_SurvivalMissionType MissionType) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    TArray<FQuest_SurvivalMissionData> GetMissionsByPriority(EQuest_SurvivalPriority Priority) const;

    // Mission validation
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    bool ValidateMissionCompletion(const FString& MissionName, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void UpdateMissionProgress(float DeltaTime);

    // Resource tracking
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void RegisterResourceCollection(const FString& ResourceType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void RegisterShelterConstruction(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void RegisterFireCreation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void RegisterToolCrafting(const FString& ToolType);

    // Debug and testing
    UFUNCTION(BlueprintCallable, Category = "Survival Missions", CallInEditor)
    void CreateTestSurvivalMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions", CallInEditor)
    void ClearAllMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions", CallInEditor)
    void LogMissionStatus();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Missions")
    TArray<FQuest_SurvivalMissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Missions")
    TArray<FQuest_SurvivalMissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoCreateMissions;

private:
    FTimerHandle MissionUpdateTimer;
    
    void ProcessMissionTimeouts();
    void AutoGenerateMissions();
    FVector FindSuitableLocationForMission(EQuest_SurvivalMissionType MissionType);
    int32 FindMissionIndex(const FString& MissionName) const;
};