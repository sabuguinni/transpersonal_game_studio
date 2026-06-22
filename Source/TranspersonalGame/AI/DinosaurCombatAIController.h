#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NPCBehaviorComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Combat AI — ECombat_ThreatTier
// Global threat escalation tiers for music/AI intensity
// ============================================================
UENUM(BlueprintType)
enum class ECombat_ThreatTier : uint8
{
    None    UMETA(DisplayName = "No Threat"),
    Low     UMETA(DisplayName = "Low — 1 dino"),
    Medium  UMETA(DisplayName = "Medium — 2-3 dinos"),
    High    UMETA(DisplayName = "High — 4+ dinos / Pack")
};

// ============================================================
// Combat AI — ECombat_AttackType
// Types of attack a dinosaur can perform
// ============================================================
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw Swipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    TailSwipe   UMETA(DisplayName = "Tail Swipe"),
    Charge      UMETA(DisplayName = "Charge")
};

// ============================================================
// Combat AI — FCombat_AttackData
// Parameters for a single attack action
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float DamageAmount = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Cooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bIsAOE = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AOERadius = 0.0f;
};

// ============================================================
// FCombat_PackState
// Shared state for pack-hunting coordination
// ============================================================
USTRUCT(BlueprintType)
struct FCombat_PackState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bPackChasingPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 ActivePackMembers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackBroadcastRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float TimeSinceLastBroadcast = 0.0f;
};

// ============================================================
// ADinosaurCombatAIController
// AIController subclass for dinosaur combat behaviour.
// Reads ENPC_BehaviorState from UNPCBehaviorComponent and
// drives movement, attacks, and pack coordination.
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --------------------------------------------------------
    // Combat Configuration
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_AttackData PrimaryAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_AttackData SecondaryAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float PatrolMoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float ChaseMoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AcceptanceRadius = 150.0f;

    // --------------------------------------------------------
    // Runtime State
    // --------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_ThreatTier CurrentThreatTier = ECombat_ThreatTier::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    bool bCanAttack = true;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    FCombat_PackState PackState;

    // --------------------------------------------------------
    // Patrol Waypoints
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 CurrentWaypointIndex = 0;

    // --------------------------------------------------------
    // Combat Functions
    // --------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target, const FCombat_AttackData& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastPackAlert(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void MoveToNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplySpeciesAttackDefaults();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_ThreatTier CalculateThreatTier() const;

private:
    UPROPERTY()
    UNPCBehaviorComponent* BehaviorComp = nullptr;

    UPROPERTY()
    AActor* PlayerActor = nullptr;

    void UpdateCombatMovement(float DeltaTime);
    void UpdateAttackCooldown(float DeltaTime);
    void FindPlayerActor();
    void SyncStateFromBehaviorComp();
};
