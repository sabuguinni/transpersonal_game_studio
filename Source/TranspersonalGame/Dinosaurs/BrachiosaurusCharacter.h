#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "BrachiosaurusCharacter.generated.h"

/**
 * ABrachiosaurusCharacter
 * Large herbivore dinosaur — herd behaviour, flees carnivores, high HP.
 * Biome: River/Wetland zone. Herd size: 2-4 individuals.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABrachiosaurusCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ABrachiosaurusCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Herd System ──────────────────────────────────────────────────────
    /** Radius within which other Brachiosaurus are considered herd members */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Herd")
    float HerdRadius = 3000.0f;

    /** Maximum herd size this individual will join */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Herd")
    int32 MaxHerdSize = 4;

    /** Current herd members (populated at BeginPlay + updated every 5s) */
    UPROPERTY(BlueprintReadOnly, Category = "Brachio|Herd")
    TArray<ABrachiosaurusCharacter*> HerdMembers;

    /** Is this the herd leader? (elected by proximity order) */
    UPROPERTY(BlueprintReadOnly, Category = "Brachio|Herd")
    bool bIsHerdLeader = false;

    // ── Flee System ──────────────────────────────────────────────────────
    /** Distance at which Brachio detects and flees carnivores */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Flee")
    float CarnivoreDetectionRadius = 2500.0f;

    /** Speed multiplier when fleeing (applied to base WalkSpeed) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Flee")
    float FleeSpeedMultiplier = 1.8f;

    /** Duration to keep fleeing after losing sight of carnivore (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Flee")
    float FleeDuration = 12.0f;

    /** Is currently fleeing? */
    UPROPERTY(BlueprintReadOnly, Category = "Brachio|Flee")
    bool bIsFleeing = false;

    // ── Stomp Attack ─────────────────────────────────────────────────────
    /** Stomp radius — damages anything within this range when triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Combat")
    float StompRadius = 400.0f;

    /** Stomp damage (defensive only — Brachio doesn't hunt) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Combat")
    float StompDamage = 120.0f;

    /** Cooldown between stomps (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Combat")
    float StompCooldown = 6.0f;

    // ── Grazing ──────────────────────────────────────────────────────────
    /** Is currently grazing (idle animation state) */
    UPROPERTY(BlueprintReadOnly, Category = "Brachio|Behaviour")
    bool bIsGrazing = false;

    /** Time spent grazing before moving to next point (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachio|Behaviour")
    float GrazeDuration = 8.0f;

    // ── Public API ───────────────────────────────────────────────────────
    /** Perform defensive stomp — damages nearby actors in StompRadius */
    UFUNCTION(BlueprintCallable, Category = "Brachio|Combat")
    void PerformStomp();

    /** Alert herd members to flee in direction away from threat */
    UFUNCTION(BlueprintCallable, Category = "Brachio|Herd")
    void AlertHerd(FVector ThreatLocation);

    /** Update herd membership list */
    UFUNCTION(BlueprintCallable, Category = "Brachio|Herd")
    void UpdateHerdMembers();

    /** Check for nearby carnivores and trigger flee if found */
    UFUNCTION(BlueprintCallable, Category = "Brachio|Flee")
    void CheckForCarnivores();

protected:
    virtual void OnDeath() override;

private:
    float LastStompTime = 0.0f;
    float FleeTimer = 0.0f;
    float HerdUpdateTimer = 0.0f;
    float GrazeTimer = 0.0f;
    FVector FleeDirection = FVector::ZeroVector;

    void UpdateFleeState(float DeltaTime);
    void UpdateGrazingState(float DeltaTime);
    void ElectHerdLeader();
};
