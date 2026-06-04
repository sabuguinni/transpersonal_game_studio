#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_TacticalMissionController.generated.h"

UENUM(BlueprintType)
enum class EQuest_TacticalMissionType : uint8
{
    None = 0,
    DefendPosition,
    AssaultTarget,
    Reconnaissance,
    Extraction,
    Sabotage,
    Escort,
    Ambush,
    Infiltration
};

UENUM(BlueprintType)
enum class EQuest_ThreatLevel : uint8
{
    Minimal = 0,
    Low,
    Moderate,
    High,
    Extreme,
    Lethal
};

UENUM(BlueprintType)
enum class EQuest_SquadRole : uint8
{
    Leader = 0,
    Scout,
    Hunter,
    Guardian,
    Specialist,
    Support
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TacticalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    EQuest_TacticalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    EQuest_ThreatLevel ThreatAssessment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    int32 RequiredSquadSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Objective")
    bool bIsCompleted;

    FQuest_TacticalObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        MissionType = EQuest_TacticalMissionType::None;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        ThreatAssessment = EQuest_ThreatLevel::Minimal;
        RequiredSquadSize = 1;
        TimeLimit = 600.0f;
        bIsActive = false;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SquadMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    FString MemberID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    EQuest_SquadRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    float StaminaPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad Member")
    bool bIsInCombat;

    FQuest_SquadMember()
    {
        MemberID = TEXT("");
        Role = EQuest_SquadRole::Leader;
        Position = FVector::ZeroVector;
        HealthPercentage = 100.0f;
        StaminaPercentage = 100.0f;
        bIsAlive = true;
        bIsInCombat = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_TacticalMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_TacticalMissionController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    TArray<FQuest_TacticalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    TArray<FQuest_SquadMember> SquadMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    FString CurrentMissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    EQuest_TacticalMissionType CurrentMissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    float MissionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    float ThreatAssessmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    int32 MaxSquadSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Mission Controller")
    bool bMissionInProgress;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    bool StartTacticalMission(const FString& MissionID, EQuest_TacticalMissionType MissionType, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void CompleteTacticalMission();

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void AbortTacticalMission();

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    bool AddSquadMember(const FString& MemberID, EQuest_SquadRole Role, const FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void RemoveSquadMember(const FString& MemberID);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void UpdateSquadMemberStatus(const FString& MemberID, float Health, float Stamina, bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    EQuest_ThreatLevel AssessThreatLevel(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    FQuest_TacticalObjective CreateDefensiveObjective(const FVector& DefensePoint, float DefenseRadius);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    FQuest_TacticalObjective CreateOffensiveObjective(const FVector& TargetPoint, EQuest_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    FQuest_TacticalObjective CreateReconObjective(const FVector& ScoutLocation, float IntelRadius);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void UpdateObjectiveProgress(const FString& ObjectiveID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    bool IsObjectiveComplete(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    TArray<FQuest_SquadMember> GetSquadMembersInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    FVector CalculateOptimalSquadFormation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void IssueSquadCommand(const FString& Command, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    float CalculateMissionSuccessProbability();

    UFUNCTION(BlueprintCallable, Category = "Tactical Mission")
    void GenerateDynamicObjectives(const FVector& MissionArea, int32 ObjectiveCount);

private:
    void UpdateThreatAssessment();
    void MonitorSquadHealth();
    void CheckObjectiveCompletion();
    FString GenerateObjectiveID();
    void LogTacticalEvent(const FString& Event);
};