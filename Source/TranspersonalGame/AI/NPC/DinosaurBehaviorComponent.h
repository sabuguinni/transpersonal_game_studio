#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — global scope (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Alert       UMETA(DisplayName = "Alert"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Patrol waypoint struct
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLookAround = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// Species trait data
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_DinoSpeciesTraits
{
    GENERATED_BODY()

    /** Detection radius for player/prey (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float DetectionRadius = 3000.0f;

    /** Attack range (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float AttackRange = 300.0f;

    /** Chase speed (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float ChaseSpeed = 800.0f;

    /** Patrol speed (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float PatrolSpeed = 250.0f;

    /** Base damage per attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float AttackDamage = 40.0f;

    /** Attack cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float AttackCooldown = 2.0f;

    /** Is this a pack hunter? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    bool bIsPackHunter = false;

    /** Does this species flee from larger predators? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    bool bFleeFromPredators = false;

    /** Is herbivore (grazes, only attacks when threatened) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    bool bIsHerbivore = false;

    /** Territorial radius — attacks anything entering this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    float TerritorialRadius = 1500.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Main component
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    // ── Species & state ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State",
        meta = (AllowPrivateAccess = "true"))
    ENPC_DinoState CurrentState = ENPC_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Traits")
    FNPC_DinoSpeciesTraits Traits;

    // ── Patrol ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolWaypoint> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    // ── Threat tracking ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    AActor* ThreatTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    float DistanceToThreat = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Threat")
    float LastAttackTime = 0.0f;

    // ── Memory ───────────────────────────────────────────────────────────────

    /** Last known player position (updated when player is in detection range) */
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    /** How long ago the dino last saw the player (seconds) */
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeSinceLastSighting = 9999.0f;

    /** Memory decay — after this many seconds without sighting, dino returns to patrol */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecaySeconds = 15.0f;

    // ── Pack behavior ────────────────────────────────────────────────────────

    /** Other pack members (for Raptors) */
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Pack")
    TArray<AActor*> PackMembers;

    /** Pack leader (nullptr if this dino IS the leader) */
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Pack")
    AActor* PackLeader = nullptr;

    // ── UActorComponent overrides ────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetState(ENPC_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AdvancePatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetCurrentWaypointLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void AlertPackMembers();

    /** Apply default species traits based on Species enum */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "NPC|Behavior")
    void ApplySpeciesDefaults();

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    ENPC_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsAggressive() const;

    UFUNCTION(BlueprintPure, Category = "NPC|State")
    bool IsPatrolling() const { return CurrentState == ENPC_DinoState::Patrolling; }

private:
    void TickBehavior(float DeltaTime);
    void TickThreatDetection();
    void TickMemoryDecay(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickGrazing(float DeltaTime);

    AActor* FindPlayerActor() const;

    float PatrolWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;
};
