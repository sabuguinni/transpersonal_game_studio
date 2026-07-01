// NPCBehaviorSystem.h
// NPC Behavior Agent #11 — Cycle AUTO_20260701_004
// Behavior Tree system for T-Rex, Raptor pack, and Herbivore herd AI
// Priority: P2 — DINOSAUR AI (Behavior trees, species traits, pack dynamics)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "NPCBehaviorSystem.generated.h"

// ============================================================
// ENUMS — NPC_prefix to avoid global name collisions
// ============================================================

UENUM(BlueprintType)
enum class ENPC_DinoAIState : uint8
{
    Resting         UMETA(DisplayName = "Resting"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Alert           UMETA(DisplayName = "Alert"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Chase           UMETA(DisplayName = "Chase"),
    Attack          UMETA(DisplayName = "Attack"),
    Flee            UMETA(DisplayName = "Flee"),
    Feed            UMETA(DisplayName = "Feed"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None            UMETA(DisplayName = "None"),
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_PatrolRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTimeAtWaypoint = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bLoopRoute = true;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FNPC_PackState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    AActor* AlphaLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    FVector FlankTargetLeft = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Pack")
    FVector FlankTargetRight = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FNPC_DinoSensoryConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Sensory")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Sensory")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Sensory")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Sensory")
    float SmellRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Sensory")
    bool bMotionSensitive = false;
};

// ============================================================
// UNPC_DinoAIController — Base AI controller for all dinosaurs
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinoAIController : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinoAIController();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    ENPC_DinoAIState CurrentState = ENPC_DinoAIState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    FNPC_PatrolRoute PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    FNPC_MemoryEntry PlayerMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    FNPC_DinoSensoryConfig SensoryConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    float ChaseRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    float PatrolRadius = 5000.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|AI")
    void SetAIState(ENPC_DinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|AI")
    ENPC_DinoAIState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|AI")
    void UpdatePlayerMemory(FVector PlayerLocation, float ThreatScore);

    UFUNCTION(BlueprintCallable, Category = "NPC|AI")
    bool CanSeeTarget(FVector TargetLocation, FVector SelfLocation) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|AI")
    FVector GetNextPatrolWaypoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|AI")
    void TickBehavior(float DeltaTime, FVector SelfLocation, FVector PlayerLocation);
};

// ============================================================
// ANPC_TRexController — T-Rex specific AI (patrol + ambush)
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API ANPC_TRexController : public AActor
{
    GENERATED_BODY()

public:
    ANPC_TRexController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|TRex",
              meta = (AllowPrivateAccess = "true"))
    UNPC_DinoAIController* AIController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|TRex")
    float RoarCooldown = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ExecuteRoar();

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void StartChase(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|TRex")
    void ExecuteAttack(AActor* Target);

private:
    float RoarTimer = 0.0f;
    AActor* CurrentTarget = nullptr;
};

// ============================================================
// ANPC_RaptorPackController — Raptor pack flanking AI
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API ANPC_RaptorPackController : public AActor
{
    GENERATED_BODY()

public:
    ANPC_RaptorPackController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Raptor",
              meta = (AllowPrivateAccess = "true"))
    FNPC_PackState PackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Raptor")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Raptor")
    float FlankAngleDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Raptor")
    float CoordinationRadius = 2000.0f;

    UFUNCTION(BlueprintCallable, Category = "NPC|Raptor")
    void InitiatePackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC|Raptor")
    void CalculateFlankPositions(FVector TargetLocation, FVector ApproachDirection);

    UFUNCTION(BlueprintCallable, Category = "NPC|Raptor")
    void SignalPackMembers(ENPC_DinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Raptor")
    bool IsPackCoordinated() const;
};

// ============================================================
// ANPC_HerbivoreHerdManager — Herbivore herd behavior
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API ANPC_HerbivoreHerdManager : public AActor
{
    GENERATED_BODY()

public:
    ANPC_HerbivoreHerdManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Herd")
    int32 HerdSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Herd")
    float GrazingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Herd")
    float FleeThreshold = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Herd")
    float StampedeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Herd")
    ENPC_DinoSpecies HerbivoreSpecies = ENPC_DinoSpecies::Brachiosaurus;

    UFUNCTION(BlueprintCallable, Category = "NPC|Herd")
    void TriggerStampede(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Herd")
    void ResumeGrazing();

    UFUNCTION(BlueprintCallable, Category = "NPC|Herd")
    FVector GetSafeFleeDirection(FVector ThreatLocation) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Herd")
    bool IsThreatNearby(FVector ThreatLocation) const;

private:
    bool bIsStampeding = false;
    FVector StampedeDirection = FVector::ZeroVector;
    float StampedeTimer = 0.0f;
};
