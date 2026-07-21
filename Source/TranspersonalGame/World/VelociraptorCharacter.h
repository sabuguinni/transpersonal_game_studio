#pragma once

#include "CoreMinimal.h"
#include "World/DinosaurBase.h"
#include "VelociraptorCharacter.generated.h"

/**
 * AVelociraptorCharacter
 * Pack hunter — coordinates attacks with nearby raptors within PackRadius.
 * Shares CurrentTarget pointer via PackCoordinator scan every PackSyncInterval seconds.
 * Inherits full AI state machine (Idle/Wander/Hunt/Flee/Attack) from ADinosaurBase.
 *
 * Performance notes (Agent #4):
 *   - PackSync runs on a timer (not Tick) at PackSyncInterval=2.0s
 *   - GetAllActorsOfClass scan is bounded to PackRadius sphere overlap (not world-wide)
 *   - Tick interval set to 0.1s via SetActorTickInterval in BeginPlay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Pack Coordination ──────────────────────────────────────────────────

    /** Radius within which this raptor detects and syncs with pack members */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackRadius = 800.0f;

    /** How often (seconds) this raptor scans for pack members to share target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackSyncInterval = 2.0f;

    /** Minimum pack size to trigger coordinated flank attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MinPackSizeForFlank = 2;

    /** Offset angle (degrees) for flanking position relative to pack leader */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float FlankAngleOffset = 60.0f;

    /** Whether this raptor is the current pack leader (highest health in pack) */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsPackLeader = false;

    /** Current pack leader reference (nullptr if this is leader) */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    AVelociraptorCharacter* PackLeader = nullptr;

    // ── Leap Attack ────────────────────────────────────────────────────────

    /** Leap attack range — raptors jump onto prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapRange = 350.0f;

    /** Damage dealt by a successful leap attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapDamage = 60.0f;

    /** Cooldown between leap attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown = 4.0f;

    /** Whether a leap is currently in progress */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Combat")
    bool bLeapActive = false;

    // ── Pack Coordination Methods ──────────────────────────────────────────

    /** Scan for nearby raptors and synchronize CurrentTarget with the pack */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void SyncPackTarget();

    /** Elect pack leader (highest health raptor in pack) */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ElectPackLeader(const TArray<AVelociraptorCharacter*>& PackMembers);

    /** Calculate flanking position offset from pack leader's target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    FVector GetFlankPosition(int32 MemberIndex, int32 TotalMembers) const;

    /** Execute leap attack toward CurrentTarget */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeapAttack();

    /** Returns true if pack has enough members to coordinate a flank */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Raptor|Pack")
    bool CanFlankAttack() const;

    /** Returns current pack members within PackRadius */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    TArray<AVelociraptorCharacter*> GetNearbyPackMembers() const;

protected:
    virtual void InitializeSpeciesStats() override;

private:
    /** Timer handle for periodic pack sync */
    FTimerHandle PackSyncTimerHandle;

    /** Timer handle for leap cooldown */
    FTimerHandle LeapCooldownTimerHandle;

    /** Timestamp of last leap attack */
    float LastLeapTime = -999.0f;

    /** Cached pack members (refreshed every PackSyncInterval) */
    TArray<AVelociraptorCharacter*> CachedPackMembers;

    /** Internal: reset leap active flag after animation completes */
    void ResetLeapActive();
};
