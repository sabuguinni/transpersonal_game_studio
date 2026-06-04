#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    GatherWater,
    HuntPrey,
    BuildShelter,
    EscapePredator,
    FindFood,
    CraftTool,
    ExploreTerritory,
    DefendCamp
};

UENUM(BlueprintType)
enum class EQuest_SurvivalUrgency : uint8
{
    Low,
    Medium,
    High,
    Critical,
    LifeThreatening
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_SurvivalUrgency UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionReward;

    FQuest_SurvivalMissionData()
    {
        MissionType = EQuest_SurvivalMissionType::GatherWater;
        UrgencyLevel = EQuest_SurvivalUrgency::Medium;
        MissionDescription = TEXT("Survival mission");
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 300.0f;
        RequiredQuantity = 1;
        bIsCompleted = false;
        CompletionReward = 10.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalMissionController : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    TArray<FQuest_SurvivalMissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    TArray<FQuest_SurvivalMissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float MissionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float PlayerHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float PlayerHungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float PlayerThirstThreshold;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CreateSurvivalMission(EQuest_SurvivalMissionType MissionType, EQuest_SurvivalUrgency Urgency, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CompleteMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool CheckMissionCompletion(const FQuest_SurvivalMissionData& Mission);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void GenerateUrgentMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    FString GetMissionDescription(EQuest_SurvivalMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    EQuest_SurvivalUrgency CalculateUrgencyLevel(float PlayerHealth, float PlayerHunger, float PlayerThirst);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    TArray<FQuest_SurvivalMissionData> GetActiveMissions() const { return ActiveMissions; }

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    int32 GetActiveMissionCount() const { return ActiveMissions.Num(); }

private:
    FTimerHandle MissionCheckTimer;
    
    void CheckPlayerSurvivalStatus();
    void CleanupExpiredMissions();
    FVector FindNearestResourceLocation(EQuest_SurvivalMissionType ResourceType);
};