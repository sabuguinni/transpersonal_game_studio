// VelociraptorCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Velociraptor: fast pack-hunter, flanking AI, leap attack

#pragma once

#include "CoreMinimal.h"
#include "Core/DinosaurBase.h"
#include "VelociraptorCharacter.generated.h"

/**
 * AVelociraptorCharacter
 *
 * Species-specific subclass of ADinosaurBase for Velociraptor.
 * Key traits:
 *   - Pack hunter: coordinates with nearby raptors to flank prey
 *   - Leap attack: closes distance in one jump, applies bleed DoT
 *   - High speed (900 cm/s), low health (400), high aggression
 *   - Calls for pack reinforcements when target spotted
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API AVelociraptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorCharacter();

    // ─── Pack Behaviour ────────────────────────────────────────────────────────

    /** Radius within which this raptor calls for pack members */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCallRadius = 2500.f;

    /** Max number of pack members that will respond to a call */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackResponseCount = 3;

    /** True when this raptor is the designated alpha of its pack */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsPackAlpha = false;

    /** Weak reference to the alpha — non-alpha raptors follow alpha's target */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    TWeakObjectPtr<AVelociraptorCharacter> PackAlpha;

    // ─── Leap Attack ───────────────────────────────────────────────────────────

    /** Horizontal distance at which the leap attack is triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapAttackRange = 350.f;

    /** Impulse applied to the raptor during a leap */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapImpulse = 1200.f;

    /** Bleed damage per second applied after a successful leap */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float BleedDamagePerSecond = 15.f;

    /** Duration of bleed DoT in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float BleedDuration = 5.f;

    /** Cooldown between leap attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown = 4.f;

    // ─── Flanking ──────────────────────────────────────────────────────────────

    /** Angle offset from the target's forward vector for flanking approach */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float FlankAngleDegrees = 90.f;

    /** Distance from target at which flanking position is calculated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float FlankRadius = 400.f;

    // ─── Blueprint Events ──────────────────────────────────────────────────────

    /** Fired when this raptor performs a leap attack */
    UFUNCTION(BlueprintNativeEvent, Category = "Raptor|Events")
    void OnLeapAttack(AActor* Target);
    virtual void OnLeapAttack_Implementation(AActor* Target);

    /** Fired when this raptor calls for pack reinforcements */
    UFUNCTION(BlueprintNativeEvent, Category = "Raptor|Events")
    void OnPackCallIssued();
    virtual void OnPackCallIssued_Implementation();

    /** Fired when a pack member responds to this raptor's call */
    UFUNCTION(BlueprintNativeEvent, Category = "Raptor|Events")
    void OnPackMemberArrived(AVelociraptorCharacter* Member);
    virtual void OnPackMemberArrived_Implementation(AVelociraptorCharacter* Member);

    // ─── Public API ────────────────────────────────────────────────────────────

    /** Called by pack alpha to assign a flanking position index (0 = left, 1 = right, 2 = rear) */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void AssignFlankPosition(int32 FlankIndex, AActor* SharedTarget);

    /** Returns true if leap attack is off cooldown and target is in range */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    bool CanLeap() const;

    /** Executes the leap attack toward the target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void ExecuteLeapAttack();

    /** Broadcasts a pack call to nearby raptors */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void BroadcastPackCall();

protected:
    // ─── ACharacter overrides ──────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── ADinosaurBase overrides ───────────────────────────────────────────────
    virtual void PerformAttack_Implementation() override;

private:
    // ─── Internal state ────────────────────────────────────────────────────────

    float LastLeapTime = -999.f;
    bool bIsLeaping = false;
    int32 AssignedFlankIndex = -1;
    AActor* FlankTarget = nullptr;

    /** Accumulated bleed time on current target */
    float BleedTimeRemaining = 0.f;
    AActor* BleedTarget = nullptr;

    /** Elect alpha: the raptor with highest health in the pack becomes alpha */
    void ElectPackAlpha();

    /** Move toward the assigned flanking position relative to FlankTarget */
    void MoveToFlankPosition(float DeltaTime);

    /** Apply ongoing bleed DoT */
    void TickBleed(float DeltaTime);

    /** Scan for nearby raptors and coordinate pack attack */
    void CoordinatePackAttack();
};
