#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RaptorCombatComponent.generated.h"

// === Raptor Combat State Machine ===
UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    PackCall        UMETA(DisplayName = "PackCall")
};

// === Raptor Role in Pack ===
UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    FlankLeft   UMETA(DisplayName = "FlankLeft"),
    FlankRight  UMETA(DisplayName = "FlankRight"),
    Ambush      UMETA(DisplayName = "Ambush")
};

// === Raptor Sensory Data ===
USTRUCT(BlueprintType)
struct FCombat_RaptorSensory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Sensory")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Sensory")
    float HearingRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Sensory")
    float FieldOfViewDegrees = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Sensory")
    float AggroRange = 800.0f;
};

// === Raptor Combat Stats ===
USTRUCT(BlueprintType)
struct FCombat_RaptorStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float MaxHealth = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float CurrentHealth = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float ClawDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float BiteDamage = 55.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float AttackCooldown = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float ChaseSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float FlankSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor|Stats")
    float AttackRange = 180.0f;
};

// === Pack Coordination Signal ===
USTRUCT(BlueprintType)
struct FCombat_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_RaptorRole AssignedRole = ECombat_RaptorRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bPackEngaged = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 PackMembersAlive = 3;
};

/**
 * URaptorCombatComponent
 * Implements pack-based tactical combat AI for Velociraptor enemies.
 * Raptors coordinate flanking maneuvers, share target data, and adapt
 * attack patterns based on pack size and player behavior.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Combat Component")
class TRANSPERSONALGAME_API URaptorCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorCombatComponent();

    // === State ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    ECombat_RaptorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorRole PackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FCombat_RaptorSensory Sensory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FCombat_RaptorStats Stats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    FCombat_PackSignal PackSignal;

    // === Target ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    FVector FlankDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FlankOffset = 400.0f;

    // === Pack Communication ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<URaptorCombatComponent*> PackMembers;

    // === Public API ===
    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetPackRole(ECombat_RaptorRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void EngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ReceivePackSignal(const FCombat_PackSignal& Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void BroadcastPackSignal();

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    float TakeDamage_Raptor(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    ECombat_RaptorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    FVector CalculateFlankPosition(FVector TargetLocation, bool bLeftFlank) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateStateMachine(float DeltaTime);
    void UpdateStalkingBehavior(float DeltaTime);
    void UpdateFlankingBehavior(float DeltaTime);
    void UpdateChargingBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateRetreatBehavior(float DeltaTime);
    void DetectPlayer();
    void ExecuteAttack();
    void TransitionToState(ECombat_RaptorState NewState);

    FVector PatrolOrigin;
    float StateTimer;
    bool bHasLineOfSight;
};
