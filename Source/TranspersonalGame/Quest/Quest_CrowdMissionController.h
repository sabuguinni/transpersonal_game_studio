#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../AI/Crowd/CrowdSimulationManager.h"
#include "../Core/SharedTypes.h"
#include "Quest_CrowdMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdMissionType : uint8
{
    TribalGathering     UMETA(DisplayName = "Tribal Gathering"),
    MigrationEscort     UMETA(DisplayName = "Migration Escort"),
    CrowdControl        UMETA(DisplayName = "Crowd Control"),
    SocialMediation     UMETA(DisplayName = "Social Mediation"),
    GroupHunt           UMETA(DisplayName = "Group Hunt"),
    CommunityDefense    UMETA(DisplayName = "Community Defense"),
    ResourceSharing     UMETA(DisplayName = "Resource Sharing"),
    CulturalExchange    UMETA(DisplayName = "Cultural Exchange")
};

UENUM(BlueprintType)
enum class EQuest_CrowdDensity : uint8
{
    Sparse      UMETA(DisplayName = "Sparse (5-15 NPCs)"),
    Moderate    UMETA(DisplayName = "Moderate (16-50 NPCs)"),
    Dense       UMETA(DisplayName = "Dense (51-150 NPCs)"),
    Massive     UMETA(DisplayName = "Massive (151-500 NPCs)"),
    Extreme     UMETA(DisplayName = "Extreme (500+ NPCs)")
};

USTRUCT(BlueprintType)
struct FQuest_CrowdMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdDensity CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector CentralLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 TargetNPCCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> ObjectiveDescriptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionPercentage;

    FQuest_CrowdMissionData()
    {
        MissionName = TEXT("Unnamed Mission");
        MissionType = EQuest_CrowdMissionType::TribalGathering;
        CrowdDensity = EQuest_CrowdDensity::Moderate;
        CentralLocation = FVector::ZeroVector;
        MissionRadius = 1000.0f;
        TargetNPCCount = 25;
        MissionDuration = 300.0f;
        bIsActive = false;
        CompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdMissionController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    FQuest_CrowdMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<FQuest_CrowdMissionData> MissionQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    UCrowdSimulationManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    bool bAutoStartNextMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<AActor*> ParticipatingNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TArray<FVector> GatheringPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    float NPCSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    int32 MaxConcurrentMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Control")
    TMap<EQuest_CrowdMissionType, float> MissionTypeWeights;

public:
    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void StartCrowdMission(const FQuest_CrowdMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void AbortMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    FQuest_CrowdMissionData GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void SpawnMissionNPCs(int32 NPCCount, const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void UpdateMissionProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    bool IsMissionComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    float GetMissionTimeRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void AddObjectiveToCurrentMission(const FString& ObjectiveDescription);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void SetCrowdDensityTarget(EQuest_CrowdDensity NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    TArray<AActor*> GetNPCsInMissionRadius() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void ConfigureMissionBehavior(EQuest_CrowdMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void QueueMission(const FQuest_CrowdMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission Control")
    void ProcessMissionQueue();

private:
    FTimerHandle MissionUpdateTimer;
    float MissionStartTime;
    int32 CurrentNPCCount;
    bool bMissionInitialized;

    void InitializeMissionType(EQuest_CrowdMissionType MissionType);
    void SetupTribalGathering();
    void SetupMigrationEscort();
    void SetupCrowdControl();
    void SetupSocialMediation();
    void SetupGroupHunt();
    void SetupCommunityDefense();
    void SetupResourceSharing();
    void SetupCulturalExchange();
    void CleanupMission();
    void ValidateMissionParameters();
};