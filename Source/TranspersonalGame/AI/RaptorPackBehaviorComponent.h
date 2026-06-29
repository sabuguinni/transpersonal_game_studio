// RaptorPackBehaviorComponent.h
// Agent #12 — Combat & Enemy AI
// Raptor pack coordination: flanking AI, TRex roar response, pack scatter logic
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "RaptorPackBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — global scope (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Flank       UMETA(DisplayName = "Flank"),
    Charge      UMETA(DisplayName = "Charge"),
    Bite        UMETA(DisplayName = "Bite"),
    Scatter     UMETA(DisplayName = "Scatter"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_FlankRole : uint8
{
    Leader      UMETA(DisplayName = "Leader"),
    LeftFlanker UMETA(DisplayName = "LeftFlanker"),
    RightFlanker UMETA(DisplayName = "RightFlanker"),
    Ambusher    UMETA(DisplayName = "Ambusher")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — global scope
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCombat_RaptorPackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    APawn* RaptorPawn = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_FlankRole FlankRole = ECombat_FlankRole::Leader;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorState CurrentState = ECombat_RaptorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    float LastAttackTime = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    bool bIsAlive = true;
};

USTRUCT(BlueprintType)
struct FCombat_FlankFormation
{
    GENERATED_BODY()

    // Offset from target position for each flank role (relative to target facing)
    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    FVector LeaderOffset = FVector(-300.f, 0.f, 0.f);

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    FVector LeftOffset = FVector(0.f, -400.f, 0.f);

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    FVector RightOffset = FVector(0.f, 400.f, 0.f);

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    FVector AmbushOffset = FVector(500.f, 0.f, 0.f);
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_RaptorPackAttackDelegate,
    APawn*, AttackingRaptor,
    float, DamageDealt);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_RaptorPackScatterDelegate,
    FVector, TRexChargeOrigin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_RaptorPackStateDelegate,
    ECombat_RaptorState, NewState,
    ECombat_FlankRole, RaptorRole);

// ─────────────────────────────────────────────────────────────────────────────
// URaptorPackBehaviorComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Pack Behavior")
class TRANSPERSONALGAME_API URaptorPackBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackBehaviorComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Pack Registration ──────────────────────────────────────────────────

    /** Register a raptor pawn into this pack. Assigns flank role automatically. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void RegisterPackMember(APawn* RaptorPawn);

    /** Remove a raptor from the pack (death, despawn). */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void UnregisterPackMember(APawn* RaptorPawn);

    /** Returns number of living pack members. */
    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    int32 GetLivingMemberCount() const;

    // ── Combat Control ─────────────────────────────────────────────────────

    /** Called when any pack member or external sensor detects the player. Begins coordinated hunt. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void OnPlayerDetected(APawn* PlayerPawn, FVector PlayerLocation);

    /** Called when T-Rex broadcasts a roar (from UNPC_TRexBehaviorComponent::OnRoarBroadcast).
     *  Pack scatters out of T-Rex charge corridor to avoid friendly-fire zone. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void OnTRexRoarReceived(FVector RoarOrigin, float RoarRadius);

    /** Called when T-Rex begins a charge. Pack moves perpendicular to charge vector. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void OnTRexChargeStarted(FVector ChargeDirection, FVector ChargeOrigin);

    /** Force all pack members to scatter — used when combat zone is cleared. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ForceScatter();

    /** Execute a coordinated flank attack on the current target. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ExecuteFlankAttack();

    /** Apply claw damage from a specific raptor. */
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ApplyClawDamage(APawn* AttackingRaptor);

    // ── State Queries ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    ECombat_RaptorState GetPackLeaderState() const;

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    bool IsPackEngaged() const { return bPackEngaged; }

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    APawn* GetCurrentTarget() const { return CurrentTarget; }

    // ── Configuration ──────────────────────────────────────────────────────

    /** Detection radius for player sensing (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float DetectionRadius = 2500.f;

    /** Claw attack damage per hit. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float ClawDamage = 25.f;

    /** Attack cooldown per raptor (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float AttackCooldown = 1.8f;

    /** Melee attack range (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float AttackRange = 180.f;

    /** Radius around T-Rex charge vector that raptors must vacate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float TRexDangerZoneRadius = 600.f;

    /** Flank formation offsets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    FCombat_FlankFormation FlankFormation;

    // ── Delegates ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Combat|RaptorPack|Events")
    FCombat_RaptorPackAttackDelegate OnRaptorAttack;

    UPROPERTY(BlueprintAssignable, Category = "Combat|RaptorPack|Events")
    FCombat_RaptorPackScatterDelegate OnPackScatter;

    UPROPERTY(BlueprintAssignable, Category = "Combat|RaptorPack|Events")
    FCombat_RaptorPackStateDelegate OnRaptorStateChanged;

private:
    // ── Internal State ─────────────────────────────────────────────────────

    UPROPERTY()
    TArray<FCombat_RaptorPackMember> PackMembers;

    UPROPERTY()
    APawn* CurrentTarget = nullptr;

    bool bPackEngaged = false;
    float SenseTickAccumulator = 0.f;
    float SenseTickInterval = 0.12f; // ~8Hz sensing

    // ── Internal Methods ───────────────────────────────────────────────────

    void TickSensePlayer();
    void UpdateFlankPositions();
    void SetMemberState(FCombat_RaptorPackMember& Member, ECombat_RaptorState NewState);
    FVector GetFlankTargetPosition(ECombat_FlankRole Role, FVector TargetLocation, FVector TargetForward) const;
    bool IsInTRexDangerZone(FVector MemberLocation, FVector ChargeOrigin, FVector ChargeDirection) const;
    ECombat_FlankRole AssignFlankRole(int32 MemberIndex) const;
};
