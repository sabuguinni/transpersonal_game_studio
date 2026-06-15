#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_IntegratedMissionSystem.generated.h"

// Forward declarations
class UCrowdSimulationManager;
class UBiomeManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_IntegratedObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bRequiresCrowdInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString CrowdBehaviorTag;

    FQuest_IntegratedObjective()
    {
        ObjectiveID = "";
        Description = "";
        QuestType = EQuestType::Hunt;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        RequiredCount = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
        bRequiresCrowdInteraction = false;
        CrowdBehaviorTag = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_IntegratedMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_IntegratedObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EMissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RemainingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> CrowdInteractionTags;

    FQuest_IntegratedMission()
    {
        MissionID = "";
        Title = "";
        Description = "";
        Priority = EMissionPriority::Normal;
        TimeLimit = 0.0f;
        RemainingTime = 0.0f;
        ExperienceReward = 100;
        bIsActive = false;
        bIsCompleted = false;
        RequiredBiome = "";
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_IntegratedMissionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_IntegratedMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeMissionSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckObjectiveCompletion(const FString& MissionID, const FString& ObjectiveID);

    // Integrated Systems
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterCrowdInteraction(const FString& CrowdBehaviorTag, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateBiomeContext(const FString& CurrentBiome);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TriggerCrowdBasedObjective(const FString& MissionID, const FString& ObjectiveID);

    // Mission Creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntMission(const FString& TargetSpecies, const FVector& HuntingGrounds);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherMission(const FString& ResourceType, const TArray<FVector>& GatherLocations);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateEscortMission(const FString& NPCName, const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateDefenseMission(const FVector& DefensePoint, int32 WaveCount);

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void PlayQuestNarration(const FString& MissionID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void PlayCompletionReward(const FString& MissionID);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_IntegratedMission> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_IntegratedMission> CompletedMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_IntegratedMission> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionTimeUpdateInterval;

    // System References
    UPROPERTY()
    UCrowdSimulationManager* CrowdManager;

    UPROPERTY()
    UBiomeManager* BiomeManager;

    UPROPERTY()
    ATranspersonalCharacter* PlayerCharacter;

    // Audio URLs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString QuestNarrationURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString CompletionRewardURL;

    // Internal tracking
    FString CurrentBiome;
    TMap<FString, FVector> CrowdInteractionPoints;
    float LastObjectiveCheck;
    float LastTimeUpdate;

private:
    void UpdateMissionTimers(float DeltaTime);
    void CheckObjectiveCompletion();
    void ProcessCrowdInteractions();
    void ValidateMissionRequirements();
    FQuest_IntegratedMission* FindMissionByID(const FString& MissionID);
    FQuest_IntegratedObjective* FindObjectiveInMission(FQuest_IntegratedMission* Mission, const FString& ObjectiveID);
    void CreateDefaultMissions();
    void NotifyMissionStateChange(const FString& MissionID, const FString& NewState);
};