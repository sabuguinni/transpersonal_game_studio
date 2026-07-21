#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_CrowdMissionManager.generated.h"

// Forward declarations
class ACrowdSimulationManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    FQuest_CrowdMission()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        Description = TEXT("");
        QuestType = EQuestType::Hunt;
        TargetLocation = FVector::ZeroVector;
        RequiredCrowdSize = 5;
        MissionRadius = 1000.0f;
        bIsActive = false;
        bIsCompleted = false;
        TimeLimit = 300.0f;
        ElapsedTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdMissionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdMissionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core mission management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions")
    TArray<FQuest_CrowdMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions")
    TArray<FQuest_CrowdMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions")
    int32 MaxActiveMissions;

    // Crowd integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ACrowdSimulationManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CrowdDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MinCrowdForMission;

    // Mission types
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void CreateEscortMission(FVector StartLocation, FVector EndLocation, int32 CrowdSize);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void CreateCrowdControlMission(FVector CongestionPoint, int32 MaxCrowdDensity);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void CreateGatheringMission(FVector ResourceLocation, int32 RequiredGatherers);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void CreateDefenseMission(FVector DefensePoint, int32 DefenderCount);

    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    void UpdateMissionProgress(const FString& MissionID, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool CheckMissionCompletion(const FString& MissionID);

    // Crowd interaction
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetCrowdCountInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<AActor*> GetCrowdActorsInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void DirectCrowdToLocation(FVector TargetLocation, int32 CrowdCount);

    // Mission validation
    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool ValidateMissionRequirements(const FQuest_CrowdMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    FString GenerateMissionID();

    // Visual components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MissionMarker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* MissionTrigger;

private:
    // Internal tracking
    int32 MissionCounter;
    float LastMissionCheck;
    bool bSystemInitialized;

    // Helper functions
    void InitializeMissionSystem();
    void CleanupCompletedMissions();
    FVector FindOptimalMissionLocation();
    bool IsCrowdAvailable(int32 RequiredSize);
};