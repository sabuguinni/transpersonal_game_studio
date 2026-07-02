// RaptorDinosaur.h
// Performance Optimizer #04 — Cycle AUTO_20260702_008
// Pack predator dinosaur — Velociraptor-class hunter with coordinated pack AI
// Performance: URO-compatible, distance-based tick throttle, LOD-aware

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "RaptorDinosaur.generated.h"

// Pack role enum — determines behavior priority in group hunts
UENUM(BlueprintType)
enum class EPerf_RaptorPackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),    // Leads charge, highest priority target
    Flanker     UMETA(DisplayName = "Flanker"),  // Circles prey, cuts off escape
    Distractor  UMETA(DisplayName = "Distractor") // Feints to draw attention
};

// Pack coordination signal
USTRUCT(BlueprintType)
struct FPerf_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    AActor* Target = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    EPerf_RaptorPackRole AssignedRole = EPerf_RaptorPackRole::Alpha;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    FVector FlankPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bHuntActive = false;
};

/**
 * ARaptorDinosaur — Velociraptor-class pack predator
 *
 * Performance design:
 * - Tick throttled by distance to player (URO-compatible)
 * - Pack signals broadcast via timer (not every tick)
 * - Sphere overlaps cached, not run every frame
 * - LOD-aware: AI disabled beyond 6000 units
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
    meta = (DisplayName = "Raptor Dinosaur"))
class TRANSPERSONALGAME_API ARaptorDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorDinosaur();

    // --- Pack System ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    EPerf_RaptorPackRole PackRole = EPerf_RaptorPackRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bPackHunter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackDetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float CoordinatedAttackBonus = 1.5f; // Damage multiplier when 2+ raptors attack same target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MaxPackSize = 5;

    // --- Species Stats ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float LeapAttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float LeapAttackCooldown = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FlankingRadius = 500.0f; // Distance flankers maintain from prey

    // --- Performance: Distance-based tick throttle ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AIDisableDistance = 6000.0f; // Beyond this, AI ticks are suspended

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TickThrottleNear = 0.05f;  // Tick interval when player < 2000 units

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TickThrottleFar = 0.25f;   // Tick interval when player 2000-6000 units

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PackSignalBroadcastInterval = 0.5f; // How often pack signals are sent

    // --- Core Overrides ---

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // --- Pack Behavior ---

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BroadcastHuntSignal(AActor* PreyTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void ReceiveHuntSignal(const FPerf_PackSignal& Signal);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AssignFlankPosition(AActor* PreyTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<ARaptorDinosaur*> FindNearbyPackMembers();

    // --- Attack Behaviors ---

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformLeapAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintNativeEvent, Category = "Combat")
    void OnPackHuntBegin(AActor* PreyTarget);
    virtual void OnPackHuntBegin_Implementation(AActor* PreyTarget);

    // --- Performance Utilities ---

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateTickRate(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinAIRange() const;

protected:
    // Internal state
    bool bLeapOnCooldown = false;
    bool bHuntActive = false;
    FPerf_PackSignal CurrentPackSignal;

    // Cached player ref (updated periodically, not every tick)
    UPROPERTY()
    AActor* CachedPlayerActor = nullptr;

    float TimeSinceLastPlayerScan = 0.0f;
    float PlayerScanInterval = 1.0f;

    // Timers
    FTimerHandle LeapCooldownTimer;
    FTimerHandle PackSignalTimer;
    FTimerHandle PlayerScanTimer;

    void ScanForPlayer();
    void OnLeapCooldownEnd();
    void TickPackSignal();
};
