#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    ESurvivalStat RequiredStat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float RewardAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    FQuest_SurvivalMissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        Description = TEXT("");
        RequiredStat = ESurvivalStat::Health;
        TargetValue = 0.0f;
        RewardAmount = 0.0f;
        bIsCompleted = false;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntingMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FString TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    int32 CurrentKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FVector HuntingZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float HuntingZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    bool bIsCompleted;

    FQuest_HuntingMissionData()
    {
        MissionID = TEXT("");
        TargetSpecies = TEXT("");
        RequiredKills = 0;
        CurrentKills = 0;
        HuntingZoneCenter = FVector::ZeroVector;
        HuntingZoneRadius = 1000.0f;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalMissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartSurvivalMission(const FString& MissionID, ESurvivalStat StatType, float TargetValue, float RewardAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartHuntingMission(const FString& MissionID, const FString& TargetSpecies, int32 RequiredKills, FVector ZoneCenter, float ZoneRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateSurvivalProgress(ESurvivalStat StatType, float CurrentValue);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateHuntingProgress(const FString& Species, FVector KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMissionData> GetActiveSurvivalMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_HuntingMissionData> GetActiveHuntingMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateDefaultMissions();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_SurvivalMissionData> SurvivalMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TArray<FQuest_HuntingMissionData> HuntingMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MissionCheckInterval;

private:
    FTimerHandle MissionUpdateTimer;

    void CheckMissionProgress();
    void NotifyMissionComplete(const FString& MissionID);
    FQuest_SurvivalMissionData* FindSurvivalMission(const FString& MissionID);
    FQuest_HuntingMissionData* FindHuntingMission(const FString& MissionID);
};