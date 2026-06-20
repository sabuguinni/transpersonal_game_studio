#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Navigation/PathFollowingComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ─── Combat Phase ────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_Phase : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Circling        UMETA(DisplayName = "Circling"),
    Retreating      UMETA(DisplayName = "Retreating"),
    PackCoordinate  UMETA(DisplayName = "PackCoordinate")
};

// ─── Attack Type ─────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Charge      UMETA(DisplayName = "Charge"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
    Pounce      UMETA(DisplayName = "Pounce")
};

// ─── Tactical Role (pack coordination) ───────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),    // leads charge
    Flanker     UMETA(DisplayName = "Flanker"),  // circles left/right
    Ambusher    UMETA(DisplayName = "Ambusher"), // waits for opening
    Distractor  UMETA(DisplayName = "Distractor")// draws attention
};

// ─── Combat Stats ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CirclingRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold = 0.25f;  // flee below 25% HP

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType PrimaryAttack = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanPackHunt = false;
};

// ─── Pack Signal ──────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalRole AssignedRole = ECombat_TacticalRole::Flanker;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float SignalTimestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsActive = false;
};

// ─── Controller ───────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // ── Perception callbacks ──────────────────────────────────────────────────
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // ── Combat phase control ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombatPhase(ECombat_Phase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BeginCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CircleTarget(AActor* Target, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastPackSignal(ECombat_TacticalRole RoleForOthers);

    // ── State queries ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_Phase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetDistanceToTarget() const;

    // ── Pack coordination ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReceivePackSignal(const FCombat_PackSignal& Signal);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool HasPackSupport() const { return NearbyPackMembers.Num() > 0; }

protected:
    // ── Perception ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception",
              meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    // ── Combat state ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat",
              meta = (AllowPrivateAccess = "true"))
    ECombat_Phase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Combat",
              meta = (AllowPrivateAccess = "true"))
    ECombat_TacticalRole MyTacticalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DinoStats CombatStats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat",
              meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat",
              meta = (AllowPrivateAccess = "true"))
    TArray<ADinosaurCombatAIController*> NearbyPackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat",
              meta = (AllowPrivateAccess = "true"))
    FCombat_PackSignal ActivePackSignal;

    // ── Timers ────────────────────────────────────────────────────────────────
    float AttackCooldownRemaining;
    float CirclingAngle;
    float PhaseTimer;
    bool bChargeInitiated;

private:
    void TickCombatPhase(float DeltaTime);
    void ScanForPackMembers();
    bool ShouldRetreat() const;
    FVector GetCirclingPosition(float Angle) const;
};
