#pragma once
// DinosaurBehaviorComponent.h
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260629_004
// Behavioral brain for all dinosaur species — patrol, alert, flee, attack, daily routine

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

// ============================================================
// ENUMS — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Forage      UMETA(DisplayName = "Forage"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Rest        UMETA(DisplayName = "Rest"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpeciesRole : uint8
{
    Apex        UMETA(DisplayName = "Apex Predator"),    // TRex — solitary, territory 5000u
    PackHunter  UMETA(DisplayName = "Pack Hunter"),      // Raptor — group tactics
    Herbivore   UMETA(DisplayName = "Herbivore"),        // Brachiosaurus — flee, herd
    Scavenger   UMETA(DisplayName = "Scavenger"),        // Opportunistic, follows kills
    Ambush      UMETA(DisplayName = "Ambush Predator")   // Waits in cover, burst attack
};

UENUM(BlueprintType)
enum class ENPC_ThreatResponse : uint8
{
    Ignore      UMETA(DisplayName = "Ignore"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Flee        UMETA(DisplayName = "Flee"),
    Attack      UMETA(DisplayName = "Attack")
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_DinoMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Dino Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Dino Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Dino Memory")
    float ThreatLevel = 0.0f;          // 0.0 = none, 1.0 = critical

    UPROPERTY(BlueprintReadWrite, Category = "Dino Memory")
    float TimeStamp = 0.0f;            // Game time when threat was last seen

    UPROPERTY(BlueprintReadWrite, Category = "Dino Memory")
    bool bIsVisible = false;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    float WaitDuration = 3.0f;         // Seconds to wait at this point

    UPROPERTY(BlueprintReadWrite, Category = "Patrol")
    bool bIsRestPoint = false;         // Dino rests here (night routine)
};

USTRUCT(BlueprintType)
struct FNPC_DailyScheduleSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Daily Schedule")
    float StartHour = 0.0f;            // 0-24 game hours

    UPROPERTY(BlueprintReadWrite, Category = "Daily Schedule")
    float EndHour = 6.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Daily Schedule")
    ENPC_DinoState ScheduledState = ENPC_DinoState::Rest;

    UPROPERTY(BlueprintReadWrite, Category = "Daily Schedule")
    FVector PreferredZoneCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Daily Schedule")
    float ZoneRadius = 1000.0f;
};

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Behavior Component")
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Species Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ENPC_DinoSpeciesRole SpeciesRole = ENPC_DinoSpeciesRole::Apex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString SpeciesName = TEXT("Tyrannosaurus Rex");

    // ---- State Machine ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    ENPC_DinoState CurrentState = ENPC_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    ENPC_DinoState PreviousState = ENPC_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    float TimeInCurrentState = 0.0f;

    // ---- Perception Ranges ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0", ClampMax = "10000"))
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0", ClampMax = "180"))
    float SightAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0", ClampMax = "5000"))
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0", ClampMax = "1000"))
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0", ClampMax = "2000"))
    float ChaseRange = 5000.0f;

    // ---- Territory ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (ClampMin = "100", ClampMax = "20000"))
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bDefendsTerritory = true;

    // ---- Patrol ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "50", ClampMax = "2000"))
    float PatrolAcceptanceRadius = 200.0f;

    // ---- Memory ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_DinoMemory> ThreatMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "5", ClampMax = "120"))
    float MemoryDecaySeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxMemoryEntries = 5;

    // ---- Daily Routine ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_DailyScheduleSlot> DailySchedule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Daily Routine")
    float CurrentGameHour = 12.0f;     // 0-24

    // ---- Pack Behavior ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesionRadius = 800.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader = false;

    // ---- Combat Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.2f;  // Flee when HP < 20%

    // ---- Current Target ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
    float DistanceToTarget = 0.0f;

    // ============================================================
    // PUBLIC API — Blueprint callable
    // ============================================================

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void SetState(ENPC_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    ENPC_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void AddThreatMemory(AActor* ThreatActor, float ThreatLevel, FVector LastLocation);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void ClearThreatMemory(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    float GetHighestThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    AActor* GetHighestThreatActor() const;

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    bool IsPlayerInSightRange() const;

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void SetupTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void SetupRaptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void SetupBrachiosaurusBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void GeneratePatrolPoints(FVector Center, float Radius, int32 NumPoints);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void SetPackLeader(bool bLeader) { bIsPackLeader = bLeader; }

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void AddPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void BroadcastAlertToPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void SetGameHour(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Dino Behavior")
    void BuildDefaultDailySchedule();

    // ============================================================
    // PRIVATE IMPLEMENTATION
    // ============================================================

private:
    void TickStateMachine(float DeltaTime);
    void TickPerception(float DeltaTime);
    void TickDailyRoutine(float DeltaTime);
    void PurgeExpiredMemories();
    void TransitionToScheduledState();

    ENPC_ThreatResponse EvaluateThreatResponse(float ThreatLevel) const;

    float PerceptionTickInterval = 0.2f;   // Perception runs at 5Hz
    float PerceptionTickAccumulator = 0.0f;

    float DailyTickInterval = 1.0f;        // Daily routine checks at 1Hz
    float DailyTickAccumulator = 0.0f;
};
