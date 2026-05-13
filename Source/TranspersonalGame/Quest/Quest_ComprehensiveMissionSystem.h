#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "../SharedTypes.h"
#include "Quest_ComprehensiveMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Tutorial        UMETA(DisplayName = "Tutorial Mission"),
    Survival        UMETA(DisplayName = "Survival Mission"),
    Hunting         UMETA(DisplayName = "Hunting Mission"),
    Crafting        UMETA(DisplayName = "Crafting Mission"),
    Exploration     UMETA(DisplayName = "Exploration Mission"),
    Combat          UMETA(DisplayName = "Combat Mission"),
    Resource        UMETA(DisplayName = "Resource Mission"),
    Narrative       UMETA(DisplayName = "Narrative Mission"),
    Challenge       UMETA(DisplayName = "Challenge Mission"),
    Emergency       UMETA(DisplayName = "Emergency Mission")
};

UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Low             UMETA(DisplayName = "Low Priority"),
    Normal          UMETA(DisplayName = "Normal Priority"),
    High            UMETA(DisplayName = "High Priority"),
    Critical        UMETA(DisplayName = "Critical Priority"),
    Emergency       UMETA(DisplayName = "Emergency Priority")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Expired         UMETA(DisplayName = "Expired")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsRepeatable;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        MissionType = EQuest_MissionType::Tutorial;
        Priority = EQuest_MissionPriority::Normal;
        Status = EQuest_MissionStatus::Inactive;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        RequiredLevel = 1;
        TimeLimit = 0.0f;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ESurvivalNeed RequiredNeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    bool bIsCompleted;

    FQuest_SurvivalObjective()
    {
        ObjectiveID = TEXT("");
        ObjectiveDescription = TEXT("");
        RequiredNeed = ESurvivalNeed::Health;
        TargetValue = 100.0f;
        CurrentProgress = 0.0f;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ComprehensiveMissionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ComprehensiveMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void InitializeMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CreateMission(const FQuest_MissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_MissionData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData GetMissionByID(const FString& MissionID) const;

    // Survival Integration
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CreateSurvivalMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void UpdateSurvivalProgress(ESurvivalNeed NeedType, float NewValue);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    bool CheckSurvivalObjectives(const FString& MissionID);

    // Dynamic Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Dynamic Missions")
    void GenerateEmergencyMissions();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Missions")
    void GenerateResourceMissions();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Missions")
    void GenerateHuntingMissions();

    // Mission Progression
    UFUNCTION(BlueprintCallable, Category = "Mission Progression")
    void UpdateMissionProgress(const FString& MissionID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Mission Progression")
    float CalculateMissionCompletion(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Progression")
    bool CheckMissionPrerequisites(const FString& MissionID) const;

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void IntegrateWithQuestSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void SyncWithCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void UpdateNarrativeProgression();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MissionMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* MissionLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_MissionData> AllMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_MissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Data")
    TArray<FQuest_SurvivalObjective> SurvivalObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float EmergencyMissionThreshold;

private:
    float LastUpdateTime;
    int32 NextMissionID;

    void UpdateActiveMissions(float DeltaTime);
    void CheckMissionTimeouts(float DeltaTime);
    void ProcessMissionTriggers();
    FString GenerateUniqueMissionID();
    void SetupMissionVisuals(const FQuest_MissionData& Mission);
    void CleanupExpiredMissions();
};