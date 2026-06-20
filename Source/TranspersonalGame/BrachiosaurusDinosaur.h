#pragma once
// BrachiosaurusDinosaur.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_008
// Brachiosaurus — massive herbivore, passive unless cornered

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "BrachiosaurusDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABrachiosaurusDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ABrachiosaurusDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Combat ───────────────────────────────────────────────────────────────

    /** Tail sweep — AoE knockback in 600 radius, 80 damage */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TailSweep();

    /** Stomp — ground pound AoE in 400 radius, 120 damage + stun */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void Stomp();

    /** Override: only attacks when cornered (health < 30%) */
    virtual void Attack_Implementation(AActor* Target) override;

    /** Override: death triggers herd alert */
    virtual void OnDeath_Implementation() override;

    // ─── Herd Behaviour ───────────────────────────────────────────────────────

    /** Register as part of a herd */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Herd")
    void JoinHerd(ABrachiosaurusDinosaur* HerdLeader);

    /** Broadcast danger to nearby herd members */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Herd")
    void AlertHerd(AActor* Threat);

    // ─── Properties ───────────────────────────────────────────────────────────

    /** Health threshold below which Brachio becomes aggressive (0.0-1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behaviour")
    float AggressionThreshold = 0.30f;

    /** Stomp AoE radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float StompRadius = 400.0f;

    /** Tail sweep AoE radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float TailSweepRadius = 600.0f;

    /** Stomp cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float StompCooldown = 8.0f;

    /** Whether stomp is on cooldown */
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Combat", meta = (AllowPrivateAccess = "true"))
    bool bStompOnCooldown = false;

    /** Herd leader reference */
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Herd", meta = (AllowPrivateAccess = "true"))
    ABrachiosaurusDinosaur* HerdLeader = nullptr;

    /** Number of herd members (leader only) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Herd")
    int32 HerdSize = 0;

    /** Whether this is the herd leader */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Herd")
    bool bIsHerdLeader = false;

private:
    FTimerHandle StompCooldownHandle;
    FTimerHandle TailSweepCooldownHandle;
    bool bTailSweepOnCooldown = false;

    void ResetStompCooldown();
    void ResetTailSweepCooldown();
    bool IsAlive() const;
    bool IsCornered() const;
};
