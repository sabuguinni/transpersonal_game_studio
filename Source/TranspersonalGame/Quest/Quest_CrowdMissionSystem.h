#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Quest_CrowdMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdMissionType : uint8
{
    HerdDinosaurs UMETA(DisplayName = "Herd Dinosaurs"),
    DefendTerritory UMETA(DisplayName = "Defend Territory"),
    EscortGroup UMETA(DisplayName = "Escort Group"),
    HuntPack UMETA(DisplayName = "Hunt Pack"),
    ScavengeResources UMETA(DisplayName = "Scavenge Resources")
};

UENUM(BlueprintType)
enum class EQuest_CrowdMissionStatus : uint8
{
    Inactive UMETA(DisplayName = "Inactive"),
    Active UMETA(DisplayName = "Active"),
    InProgress UMETA(DisplayName = "In Progress"),
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdMissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 CurrentCrowdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bRequiresPlayerPresence;

    FQuest_CrowdMissionData()
    {
        MissionName = TEXT("Default Mission");
        Description = TEXT("Complete the crowd mission");
        MissionType = EQuest_CrowdMissionType::HerdDinosaurs;
        Status = EQuest_CrowdMissionStatus::Inactive;
        RequiredCrowdSize = 5;
        MissionRadius = 2000.0f;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 300.0f;
        ElapsedTime = 0.0f;
        CurrentCrowdCount = 0;
        bRequiresPlayerPresence = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdMissionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdMissionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_CrowdMissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_CrowdMissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxConcurrentMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    class APlayerController* PlayerController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<AActor*> CrowdActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float LastMissionCheckTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void StartCrowdMission(const FQuest_CrowdMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CompleteMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void FailMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CheckMissionProgress(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateCrowdCount();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<AActor*> GetNearbyActors(FVector Location, float Radius, TSubclassOf<AActor> ActorClass);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_CrowdMissionData CreateHerdMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_CrowdMissionData CreateDefenseMission(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_CrowdMissionData CreateEscortMission(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool IsPlayerInMissionArea(const FQuest_CrowdMissionData& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CleanupCompletedMissions();

    UFUNCTION(BlueprintPure, Category = "Mission System")
    int32 GetActiveMissionCount() const { return ActiveMissions.Num(); }

    UFUNCTION(BlueprintPure, Category = "Mission System")
    TArray<FQuest_CrowdMissionData> GetActiveMissions() const { return ActiveMissions; }

private:
    void UpdateMissionTimers(float DeltaTime);
    void CheckMissionCompletion();
    void ValidateCrowdActors();
    FVector GetRandomLocationInRadius(FVector Center, float Radius);
};

#include "Quest_CrowdMissionSystem.generated.h"