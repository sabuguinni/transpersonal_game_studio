#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Ambush          UMETA(DisplayName = "Ambush"),
    Attack          UMETA(DisplayName = "Attack"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Flank           UMETA(DisplayName = "Flank"),
    Coordinate      UMETA(DisplayName = "Coordinate"),
    Defend          UMETA(DisplayName = "Defend")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurRole : uint8
{
    Alpha           UMETA(DisplayName = "Alpha"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Scout           UMETA(DisplayName = "Scout"),
    Ambusher        UMETA(DisplayName = "Ambusher"),
    Defender        UMETA(DisplayName = "Defender"),
    Support         UMETA(DisplayName = "Support")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_TacticalState CurrentState = ECombat_TacticalState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_DinosaurRole Role = ECombat_DinosaurRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float PackCoordination = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanSeeTarget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float DistanceToTarget = 0.0f;

    FCombat_TacticalInfo()
    {
        CurrentState = ECombat_TacticalState::Idle;
        Role = ECombat_DinosaurRole::Hunter;
        AggressionLevel = 0.5f;
        FearLevel = 0.0f;
        PackCoordination = 0.7f;
        LastKnownTargetLocation = FVector::ZeroVector;
        TimeSinceLastSighting = 0.0f;
        bIsInCombat = false;
        bCanSeeTarget = false;
        DistanceToTarget = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector FormationCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float FormationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsCoordinatedAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float CoordinationTimer = 0.0f;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        FormationCenter = FVector::ZeroVector;
        FormationRadius = 500.0f;
        bIsCoordinatedAttack = false;
        CoordinationTimer = 0.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalInfo TacticalInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior", meta = (AllowPrivateAccess = "true"))
    FCombat_PackCoordination PackInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings", meta = (AllowPrivateAccess = "true"))
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings", meta = (AllowPrivateAccess = "true"))
    float FlankingDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings", meta = (AllowPrivateAccess = "true"))
    float RetreatHealthThreshold = 0.3f;

    // Tactical decision functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTargetInfo(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ECombat_TacticalState DetermineBestTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateAmbushPosition(AActor* Target);

    // Pack coordination functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UCombat_TacticalAI* PackLeaderAI);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackFormationPosition();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackLeader() const;

    // Combat behavior functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlank(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteFlank(AActor* Target);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool HasLineOfSight(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<AActor*> FindNearbyAllies(float SearchRadius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<AActor*> FindNearbyEnemies(float SearchRadius = 1000.0f) const;

private:
    // Internal state tracking
    AActor* CurrentTarget = nullptr;
    float StateTimer = 0.0f;
    float LastDecisionTime = 0.0f;
    float DecisionCooldown = 1.0f;

    // AI Controller reference
    UPROPERTY()
    AAIController* AIController = nullptr;

    // Blackboard keys for behavior tree integration
    FName TargetActorKey = TEXT("TargetActor");
    FName TacticalStateKey = TEXT("TacticalState");
    FName PackLeaderKey = TEXT("PackLeader");
    FName FlankPositionKey = TEXT("FlankPosition");

    // Internal helper functions
    void InitializeAIController();
    void UpdateBlackboardValues();
    void ProcessTacticalDecision(float DeltaTime);
    bool IsValidTarget(AActor* Target) const;
    void HandleStateTransition(ECombat_TacticalState NewState);
    void UpdatePackFormation();
    FVector FindSafeRetreatPosition() const;
    bool IsPositionSafe(const FVector& Position) const;
};