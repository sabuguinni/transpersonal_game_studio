#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Combat_AIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Attack      UMETA(DisplayName = "Attack"),
    Flank       UMETA(DisplayName = "Flank"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Coordinate  UMETA(DisplayName = "Coordinate")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EngagementRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_AIState::Idle;
        ThreatLevel = ECombat_ThreatLevel::None;
        PrimaryTarget = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
        EngagementRange = 1500.0f;
        FlankingDistance = 800.0f;
        bIsPackLeader = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Combat Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold = 0.3f;

public:
    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCombatState() const { return TacticalData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetPrimaryTarget() const { return TacticalData.PrimaryTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemovePackMember(AActor* Member);

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessTacticalDecision();
    void ExecuteFlankingManeuver();
    void HandleRetreat();
    void UpdatePackCoordination();

private:
    float LastTacticalUpdate = 0.0f;
    float TacticalUpdateInterval = 0.5f;
    float LastCoordinationUpdate = 0.0f;
    float CoordinationUpdateInterval = 1.0f;
};