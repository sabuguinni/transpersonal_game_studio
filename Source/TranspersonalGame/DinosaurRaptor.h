#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "DinosaurRaptor.generated.h"

/**
 * AVelociraptorDinosaur — pack predator
 * Fast, low-health flanking predator. Works in groups of 3.
 * Agent #12 (Combat AI) reads PackRole and PackLeader to coordinate flanking.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Velociraptor"))
class TRANSPERSONALGAME_API AVelociraptorDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Pack coordination ────────────────────────────────────────────────────

    /** Role in the pack: 0=Leader, 1=LeftFlanker, 2=RightFlanker */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 PackRole;

    /** Reference to the pack leader (nullptr if this IS the leader) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    AVelociraptorDinosaur* PackLeader;

    /** Registered pack members (leader only) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    TArray<AVelociraptorDinosaur*> PackMembers;

    /** Maximum pack size before splitting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackSize;

    // ── Jump attack ──────────────────────────────────────────────────────────

    /** Jump attack damage (base) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float JumpAttackDamage;

    /** Horizontal distance to trigger jump attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float JumpAttackRange;

    /** Cooldown between jump attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float JumpAttackCooldown;

    /** True while jump attack is in progress */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Combat")
    bool bIsJumping;

    // ── Flanking ─────────────────────────────────────────────────────────────

    /** Flanking offset from target (set by AI controller) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    FVector FlankOffset;

    /** Coordination radius — raptors within this range share target info */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float CoordinationRadius;

    // ── Methods ──────────────────────────────────────────────────────────────

    /**
     * Perform a leap attack at the target actor.
     * Launches the raptor with AddImpulse toward the target.
     * Called by Agent #12 Behavior Tree via UFUNCTION.
     */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformJumpAttack(AActor* Target);

    /**
     * Coordinate flanking positions with pack members.
     * Leader calls this; flankers receive updated FlankOffset.
     * Called by Agent #12 Behavior Tree.
     */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void CoordinateFlank(AActor* Target);

    /**
     * Register a new pack member with the leader.
     * Returns true if pack is not full.
     */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    bool RegisterPackMember(AVelociraptorDinosaur* NewMember);

    /**
     * Get the computed world-space flanking position for this raptor.
     * Flankers call this to know where to move.
     */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    FVector GetFlankPosition() const;

private:
    float JumpAttackTimer;

    FTimerHandle JumpResetTimer;

    void ResetJumpState();
};
