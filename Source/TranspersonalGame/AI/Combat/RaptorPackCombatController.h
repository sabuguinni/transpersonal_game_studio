#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RaptorPackCombatController.generated.h"

UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Decoy       UMETA(DisplayName = "Decoy"),
    FlankLeft   UMETA(DisplayName = "FlankLeft"),
    FlankRight  UMETA(DisplayName = "FlankRight"),
    Ambush      UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_RaptorPackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float DetectionRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FlankOffset = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackCooldown = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FlankCoordinationDelay = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float RetreatHealthThreshold = 0.25f;
};

USTRUCT(BlueprintType)
struct FCombat_TRexCombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float DetectionRange = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float ChargeRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float BiteRange = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float WalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float BiteDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float StompDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float TurnRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|TRex")
    float RoarRadius = 1500.0f;
};

/**
 * ARaptorPackCombatController
 * Controls a single raptor within a coordinated pack combat system.
 * Each raptor has a role (Decoy, FlankLeft, FlankRight, Ambush) and
 * coordinates with pack members via shared target tracking.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorPackCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ARaptorPackCombatController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // === STATE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetCombatState(ECombat_RaptorState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    ECombat_RaptorState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetPackRole(ECombat_RaptorRole NewRole);

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    ECombat_RaptorRole GetPackRole() const { return PackRole; }

    // === COMBAT ACTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void InitiatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ExecuteDecoyCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ExecuteAmbushLeap(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ApplyBiteDamage(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void TriggerRetreat();

    // === PACK COORDINATION ===

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void RegisterPackMember(ARaptorPackCombatController* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void NotifyPackOfTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    bool IsPackCoordinated() const;

    // === DETECTION ===

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    AActor* ScanForPlayer();

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    float GetDistanceToTarget() const;

    // === DATA ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FCombat_RaptorPackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorRole PackRole = ECombat_RaptorRole::Decoy;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Raptor")
    TArray<ARaptorPackCombatController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Raptor")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Raptor")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Raptor")
    bool bIsPackLeader = false;

private:
    ECombat_RaptorState CurrentState = ECombat_RaptorState::Patrol;

    FVector PatrolOrigin;
    FVector FlankDestination;
    float StateTimer = 0.0f;

    void UpdatePatrol(float DeltaTime);
    void UpdateAlert(float DeltaTime);
    void UpdateFlanking(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateRetreat(float DeltaTime);

    FVector CalculateFlankPosition(AActor* Target) const;
    FVector GetRandomPatrolPoint() const;
    bool HasLineOfSightToTarget() const;
};
