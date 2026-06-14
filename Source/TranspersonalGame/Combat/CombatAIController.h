#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Defend      UMETA(DisplayName = "Defend"),
    Ambush      UMETA(DisplayName = "Ambush"),
    Flank       UMETA(DisplayName = "Flank"),
    Retreat     UMETA(DisplayName = "Retreat")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    FCombat_ThreatAssessment()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RallyPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_TacticalState PackTactic = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinationRadius = 1000.0f;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        RallyPoint = FVector::ZeroVector;
        PackTactic = ECombat_TacticalState::Patrol;
        CoordinationRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardComponent* BlackboardComponent;

    // Combat State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState CurrentTacticalState = ECombat_TacticalState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_PackCoordination PackData;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Aggressiveness = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Courage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float FleeThreshold = 2000.0f;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetPrimaryThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* ThreatActor) const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void UpdatePackCoordination();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    bool IsPackLeader() const;

    // Tactical Behaviors
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector CalculateFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    FVector CalculateAmbushPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteTacticalManeuver(ECombat_TacticalState NewTactic);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_TacticalState GetCurrentTacticalState() const { return CurrentTacticalState; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    const TArray<FCombat_ThreatAssessment>& GetKnownThreats() const { return KnownThreats; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    const FCombat_PackCoordination& GetPackData() const { return PackData; }

private:
    // Internal timers
    float ThreatAssessmentTimer = 0.0f;
    float PackCoordinationTimer = 0.0f;
    float TacticalUpdateTimer = 0.0f;

    // Update intervals
    static constexpr float THREAT_ASSESSMENT_INTERVAL = 0.5f;
    static constexpr float PACK_COORDINATION_INTERVAL = 1.0f;
    static constexpr float TACTICAL_UPDATE_INTERVAL = 0.25f;

    // Helper functions
    void InitializePerception();
    void InitializeBehaviorTree();
    void CleanupOldThreats();
    TArray<AActor*> FindNearbyPackMembers() const;
    bool IsValidThreat(AActor* Actor) const;
};