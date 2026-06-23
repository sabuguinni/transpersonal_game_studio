#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CombatAIComponent.generated.h"

// Combat AI state enum — prefixed Combat_ to avoid collisions
UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Dead        UMETA(DisplayName = "Dead")
};

// Combat threat level
UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Extreme UMETA(DisplayName = "Extreme")
};

// Combat AI config struct
USTRUCT(BlueprintType)
struct FCombat_AIConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bCanFlanking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 PackSize = 1;
};

// Combat event data
USTRUCT(BlueprintType)
struct FCombat_ThreatEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatDistance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float Timestamp = 0.0f;
};

/**
 * UCombatAIComponent — attaches to dinosaur/enemy pawns.
 * Manages state machine, threat detection, flanking logic.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnThreatDetected(AActor* ThreatActor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnThreatLost();

    // --- Attack Logic ---
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool TryAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlankingManeuver(AActor* Target);

    // --- Pack Coordination ---
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void NotifyPackMembers(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ReceivePackAlert(const FCombat_ThreatEvent& ThreatEvent);

    // --- Queries ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    float GetThreatLevel() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_AIConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_ThreatLevel CurrentThreatLevel;

private:
    UPROPERTY()
    ECombat_AIState CurrentState;

    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    float LastAttackTime;

    UPROPERTY()
    float StateEntryTime;

    UPROPERTY()
    FVector FlankTargetLocation;

    UPROPERTY()
    TArray<FCombat_ThreatEvent> ThreatHistory;

    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateAlertedState(float DeltaTime);
    void UpdateChasingState(float DeltaTime);
    void UpdateAttackingState(float DeltaTime);
    void UpdateFlankingState(float DeltaTime);
    void UpdateRetreatingState(float DeltaTime);

    FVector CalculateFlankPosition(AActor* Target) const;
    ECombat_ThreatLevel EvaluateThreat(float Distance) const;
    bool CanSeeTarget(AActor* Target) const;
};
