#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// Combat AI — Dinosaur Combat State Machine
// Agent #12 — Combat & Enemy AI Agent
// Cycle: PROD_CYCLE_AUTO_20260626_009
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Charge      UMETA(DisplayName = "Charge"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Flanking    UMETA(DisplayName = "Flanking"),
    PackHunt    UMETA(DisplayName = "PackHunt")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float Health = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ReactionTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FlankingAngle = 45.0f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float DistanceToThreat = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void OnTakeDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteChargeAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void BroadcastPackAlert();

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    bool bDebugDrawDetection = false;

    // --- Runtime State ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_ThreatAssessment CurrentThreat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float StateTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsAlerted = false;

private:
    void UpdateIdleState(float DeltaTime);
    void UpdateAlertState(float DeltaTime);
    void UpdateStalkState(float DeltaTime);
    void UpdateChargeState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateRetreatState(float DeltaTime);
    void UpdateFlankingState(float DeltaTime);
    void UpdatePackHuntState(float DeltaTime);

    AActor* FindNearestPlayer() const;
    float CalculateThreatLevel(AActor* Target) const;
    FVector GetFlankPosition(AActor* Target, float AngleOffset) const;

    UPROPERTY()
    AActor* CachedPlayerActor;

    float DetectionCheckTimer = 0.0f;
    float AttackCooldown = 0.0f;
    static constexpr float DETECTION_CHECK_INTERVAL = 0.25f;
    static constexpr float ATTACK_COOLDOWN_DURATION = 2.0f;
};
