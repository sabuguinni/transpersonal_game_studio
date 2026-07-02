#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "RaptorBehavior.generated.h"

// Pack role for coordinated hunting
UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),      // Pack leader, coordinates attack
    Flanker     UMETA(DisplayName = "Flanker"),    // Flanks left or right
    Distractor  UMETA(DisplayName = "Distractor"), // Holds prey attention
    Scout       UMETA(DisplayName = "Scout")       // Detects prey, alerts pack
};

// Raptor FSM states
UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Scout       UMETA(DisplayName = "Scout"),
    PackAssemble UMETA(DisplayName = "PackAssemble"),
    Flank       UMETA(DisplayName = "Flank"),
    Lunge       UMETA(DisplayName = "Lunge"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Feeding     UMETA(DisplayName = "Feeding")
};

// Shared pack target data — broadcast to all pack members
USTRUCT(BlueprintType)
struct FCombat_PackTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    float DetectedAtTime = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    bool bIsConfirmed = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    int32 PackMembersEngaged = 0;
};

// Per-raptor stats
USTRUCT(BlueprintType)
struct FCombat_RaptorStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float MaxHealth = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float CurrentHealth = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackDamage = 35.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackRange = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float PatrolSpeed = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float ChaseSpeed = 650.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float LungeSpeed = 900.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float DetectionRange = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float HearingRange = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FieldOfViewDegrees = 75.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackCooldown = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FlankOffset = 350.f; // Distance from target when flanking
};

/**
 * URaptorBehaviorComponent
 * Implements pack-coordinated hunting AI for Velociraptor-type enemies.
 * Each raptor has a role (Alpha/Flanker/Distractor) and shares a pack target.
 * Pack members coordinate via FCombat_PackTarget broadcast through the Alpha.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Behavior Component")
class TRANSPERSONALGAME_API URaptorBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Pack Management ---

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetPackRole(ECombat_RaptorRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void RegisterPackMember(URaptorBehaviorComponent* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void BroadcastPackTarget(AActor* Target, FVector LastKnownLoc);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ReceivePackTarget(const FCombat_PackTarget& PackTarget);

    // --- State Machine ---

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetState(ECombat_RaptorState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    ECombat_RaptorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    ECombat_RaptorRole GetPackRole() const { return PackRole; }

    // --- Combat Actions ---

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void PerformLunge();

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void PerformFlank();

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void TakeDamage_Raptor(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    bool IsPackReadyToAttack() const;

    // --- Detection ---

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    AActor* ScanForPrey();

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat|Raptor")
    bool CanHearTarget(AActor* Target) const;

    // --- Stats ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FCombat_RaptorStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorRole PackRole = ECombat_RaptorRole::Flanker;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Raptor", meta = (AllowPrivateAccess = "true"))
    FCombat_PackTarget SharedPackTarget;

private:
    ECombat_RaptorState CurrentState = ECombat_RaptorState::Idle;

    // Pack members (Alpha maintains this list)
    UPROPERTY()
    TArray<URaptorBehaviorComponent*> PackMembers;

    // Patrol
    FVector HomeLocation;
    FVector PatrolTarget;
    float PatrolWaitTimer = 0.f;

    // Combat timers
    float AttackCooldownTimer = 0.f;
    float FlankTimer = 0.f;
    float StateTimer = 0.f;

    // State tick functions
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickScout(float DeltaTime);
    void TickPackAssemble(float DeltaTime);
    void TickFlank(float DeltaTime);
    void TickLunge(float DeltaTime);
    void TickRetreat(float DeltaTime);

    // Helpers
    FVector GetFlankPosition() const;
    FVector GetRandomPatrolPoint() const;
    void MoveToLocation(const FVector& Destination, float Speed);
    bool IsWithinRange(AActor* Target, float Range) const;
};
