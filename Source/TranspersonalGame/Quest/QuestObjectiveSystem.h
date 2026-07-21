// QuestObjectiveSystem.h — Agent #14 Quest & Mission Designer
// Cycle: AUTO_20260702_005
// Defines quest objective tracking, types, and completion evaluation.
// All types prefixed with Quest_ to avoid collisions per UE5 compilation rules.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestObjectiveSystem.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt            UMETA(DisplayName = "Hunt Target"),
    Gather          UMETA(DisplayName = "Gather Resources"),
    Explore         UMETA(DisplayName = "Explore Location"),
    Survive         UMETA(DisplayName = "Survive Duration"),
    Craft           UMETA(DisplayName = "Craft Item"),
    Defend          UMETA(DisplayName = "Defend Location"),
    Rescue          UMETA(DisplayName = "Rescue NPC"),
    Track           UMETA(DisplayName = "Track Animal"),
    Escape          UMETA(DisplayName = "Escape Danger"),
    Deliver         UMETA(DisplayName = "Deliver Item")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Optional        UMETA(DisplayName = "Optional — Skippable")
};

UENUM(BlueprintType)
enum class EQuest_Priority : uint8
{
    Critical        UMETA(DisplayName = "Critical — Blocks Progression"),
    Primary         UMETA(DisplayName = "Primary Objective"),
    Secondary       UMETA(DisplayName = "Secondary Objective"),
    Bonus           UMETA(DisplayName = "Bonus — Extra Reward")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_Priority Priority;

    // How many units required (kills, items gathered, seconds survived, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 RequiredCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Objective")
    int32 CurrentCount;

    // Optional: target location for exploration/defend objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float TargetRadius;

    // Optional: target actor tag for hunt/track objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName TargetActorTag;

    // Time limit in seconds (0 = no limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float TimeLimitSeconds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Objective")
    float ElapsedSeconds;

    FQuest_ObjectiveData()
        : ObjectiveID(NAME_None)
        , ObjectiveType(EQuest_ObjectiveType::Gather)
        , State(EQuest_ObjectiveState::Inactive)
        , Priority(EQuest_Priority::Primary)
        , RequiredCount(1)
        , CurrentCount(0)
        , TargetLocation(FVector::ZeroVector)
        , TargetRadius(500.f)
        , TargetActorTag(NAME_None)
        , TimeLimitSeconds(0.f)
        , ElapsedSeconds(0.f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_RewardPackage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 BoneMaterialCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 HideCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 FlintCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    bool bUnlocksNewArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FName UnlockedAreaTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FText RewardNarration;

    FQuest_RewardPackage()
        : BoneMaterialCount(0)
        , HideCount(0)
        , FlintCount(0)
        , bUnlocksNewArea(false)
        , UnlockedAreaTag(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    FName MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    FText MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    FText MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    FQuest_RewardPackage Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    EQuest_ObjectiveState MissionState;

    // NPC who gives this mission
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    FName QuestGiverTag;

    // Prerequisite mission IDs that must be completed first
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Mission")
    TArray<FName> PrerequisiteMissionIDs;

    FQuest_MissionData()
        : MissionID(NAME_None)
        , MissionState(EQuest_ObjectiveState::Inactive)
        , QuestGiverTag(NAME_None)
    {}
};

// ─── Quest Objective Manager Actor ───────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Quest")
class TRANSPERSONALGAME_API AQuest_ObjectiveManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Mission Registration ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Mission")
    void RegisterMission(const FQuest_MissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Quest|Mission")
    bool ActivateMission(FName MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Mission")
    bool CompleteMission(FName MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Mission")
    bool FailMission(FName MissionID);

    // ── Objective Progress ────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportProgress(FName MissionID, FName ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportKill(FName ActorTag);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportGather(FName ResourceTag, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportLocationReached(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportCraft(FName ItemTag);

    // ── Query ─────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    bool IsMissionActive(FName MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    bool IsMissionComplete(FName MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    FQuest_ObjectiveData GetObjectiveStatus(FName MissionID, FName ObjectiveID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    int32 GetTotalCompletedMissions() const;

    // ── Delegates / Events ────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionActivated, FName, MissionID, FText, MissionTitle);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnMissionActivated OnMissionActivated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionCompleted, FName, MissionID, FQuest_RewardPackage, Reward);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnMissionCompleted OnMissionCompleted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, MissionID, FQuest_ObjectiveData, Objective);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, FName, MissionID);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnMissionFailed OnMissionFailed;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State",
              meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_MissionData> AllMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State",
              meta = (AllowPrivateAccess = "true"))
    int32 CompletedMissionCount;

private:
    void EvaluateObjectiveCompletion(FQuest_MissionData& Mission);
    void TickTimedObjectives(FQuest_MissionData& Mission, float DeltaTime);
    FQuest_MissionData* FindMission(FName MissionID);
};
