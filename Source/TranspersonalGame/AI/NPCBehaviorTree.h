#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NPCBehaviorTree.generated.h"

// ============================================================
// Enums — NPC_prefix to avoid conflicts
// ============================================================

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

// ============================================================
// Structs — NPC_prefix
// ============================================================

USTRUCT(BlueprintType)
struct FNPC_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    float WaitTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    bool bIsAlertPoint = false;
};

USTRUCT(BlueprintType)
struct FNPC_PerceptionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    bool bCanSeePlayer = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    bool bCanHearPlayer = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    float DistanceToPlayer = 99999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Perception")
    float TimeSinceLastSeen = 0.0f;
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float DetectRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float ChaseRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MoveSpeed_Patrol = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MoveSpeed_Chase = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    int32 PackSize = 1;
};

// ============================================================
// ANPCBehaviorController — AIController subclass
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANPCBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPCBehaviorController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentState; }

    // ---- Patrol ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void SetPatrolWaypoints(const TArray<FNPC_PatrolWaypoint>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
    void AdvanceToNextWaypoint();

    // ---- Perception ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
    void UpdatePerception(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "NPC|Perception")
    const FNPC_PerceptionData& GetPerceptionData() const { return PerceptionData; }

    // ---- Combat ----
    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
    bool CanAttack() const;

    // ---- Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig BehaviorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
    TArray<FNPC_PatrolWaypoint> PatrolWaypoints;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

private:
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;
    FNPC_PerceptionData PerceptionData;

    int32 CurrentWaypointIndex = 0;
    float WaypointWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;

    float AttackCooldownTimer = 0.0f;

    APawn* PlayerPawn = nullptr;

    // Internal state handlers
    void HandleIdleState(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleAlertState(float DeltaTime);
    void HandleChaseState(float DeltaTime);
    void HandleAttackState(float DeltaTime);
    void HandleFleeState(float DeltaTime);

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    APawn* FindPlayerPawn() const;
};
