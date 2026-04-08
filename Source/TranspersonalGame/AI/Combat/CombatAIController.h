#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UDinosaurCombatComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Feeding         UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None            UMETA(DisplayName = "None"),
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadOnly)
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    bool bHasLineOfSight = false;

    UPROPERTY(BlueprintReadOnly)
    float ThreatScore = 0.0f;
};

/**
 * Advanced AI Controller for dinosaur combat behavior
 * Implements tactical decision making, threat assessment, and dynamic combat states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    class UDinosaurCombatComponent* CombatComponent;

    // Perception Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
    class UAISenseConfig_Damage* DamageConfig;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintPure, Category = "Combat")
    FThreatAssessment GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateBlackboardKeys();

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFleeFromTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFleeDirection(AActor* ThreatSource) const;

    // Pack Behavior (for pack hunters)
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<ACombatAIController*> GetNearbyPackMembers(float Radius = 1000.0f) const;

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState = ECombatState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<FThreatAssessment> ThreatList;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritorialRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeDistance = 1000.0f;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FGameplayTag PackTag;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<ACombatAIController*> PackMembers;

    // Timing
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

private:
    void InitializePerception();
    void UpdateThreatAssessment();
    void CleanupOldThreats();
    float CalculateThreatScore(AActor* Actor) const;
    void HandleCombatStateTransition(ECombatState NewState);
};