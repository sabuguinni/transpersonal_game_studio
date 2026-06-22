#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurCombatAIController.generated.h"

// Combat role enum — defines dinosaur tactical behaviour
UENUM(BlueprintType)
enum class ECombat_DinoRole : uint8
{
    ApexPredator    UMETA(DisplayName = "Apex Predator"),   // TRex — solo, devastating, slow
    PackHunter      UMETA(DisplayName = "Pack Hunter"),     // Raptor — fast, flanks, coordinates
    PassiveHerbivore UMETA(DisplayName = "Passive Herbivore"), // Brachio — ignores unless cornered
    Scavenger       UMETA(DisplayName = "Scavenger"),       // Opportunistic, flees strong threats
    TerritorialGuard UMETA(DisplayName = "Territorial Guard")  // Defends fixed zone aggressively
};

// Combat state machine
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),     // Heard/smelled player — searching
    Stalking    UMETA(DisplayName = "Stalking"),    // Has LOS — approaching cautiously
    Charging    UMETA(DisplayName = "Charging"),    // Full sprint attack
    Attacking   UMETA(DisplayName = "Attacking"),   // In melee range — dealing damage
    Flanking    UMETA(DisplayName = "Flanking"),    // Pack hunter circling for position
    Retreating  UMETA(DisplayName = "Retreating"),  // Injured — pulling back
    Dead        UMETA(DisplayName = "Dead")
};

// Per-species combat parameters
USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SmellRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    ECombat_DinoRole Role = ECombat_DinoRole::PackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    float FleeHealthThreshold = 0.25f;   // Flee when HP drops below 25%

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    bool bCoordinatesWithPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behaviour")
    float PackAlertRadius = 1500.0f;     // Radius to alert pack members
};

/**
 * ADinosaurCombatAIController
 * AI Controller for all dinosaur species in the prehistoric survival game.
 * Implements tactical combat: detection, stalking, charging, pack coordination.
 * Builds on UNPCBehaviorComponent foundation from Agent #11.
 */
UCLASS(ClassGroup = "TranspersonalGame|CombatAI", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- Combat State ---
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoState GetCombatState() const { return CurrentCombatState; }

    // --- Detection ---
    UFUNCTION(BlueprintCallable, Category = "Combat|Detection")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Detection")
    bool CanHearPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Detection")
    float GetDistanceToPlayer() const;

    // --- Pack Coordination ---
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AlertPackMembers();

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackAlert(AActor* ThreatActor, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AssignFlankingPosition(int32 FlankIndex, int32 TotalPackSize);

    // --- Species Configuration ---
    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ConfigureAsApexPredator();

    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ConfigureAsPackHunter();

    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ConfigureAsPassiveHerbivore();

    // --- Damage ---
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    void OnDinoTakeDamage(float DamageAmount);

protected:
    // --- State Machine Ticks ---
    void TickIdle(float DeltaTime);
    void TickPatrolling(float DeltaTime);
    void TickAlerted(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickRetreating(float DeltaTime);

    // --- Internal Helpers ---
    AActor* FindPlayerActor() const;
    FVector GetFlankingPosition(int32 FlankIndex, int32 TotalPackSize) const;
    bool IsPlayerInRadius(float Radius) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoSpeciesParams SpeciesParams;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentCombatState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState PreviousState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Detection", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Detection", meta = (AllowPrivateAccess = "true"))
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    float AlertTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    float AttackCooldownTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack", meta = (AllowPrivateAccess = "true"))
    int32 AssignedFlankIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack", meta = (AllowPrivateAccess = "true"))
    bool bIsPackAlerted = false;

    // Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Patrol", meta = (AllowPrivateAccess = "true"))
    float PatrolWaitTimer = 0.0f;

    static constexpr float PATROL_WAIT_TIME = 3.0f;
    static constexpr float ALERT_DURATION = 8.0f;
    static constexpr float STALK_TO_CHARGE_DISTANCE = 400.0f;
};
