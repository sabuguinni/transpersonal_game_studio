// RaptorCharacter.h
// Performance Optimizer #04 — Cycle AUTO_004
// Pack hunter raptor: fast, coordinated, flanking AI

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "RaptorCharacter.generated.h"

// Pack signal types for raptor coordination
UENUM(BlueprintType)
enum class EPerf_RaptorPackSignal : uint8
{
    None        UMETA(DisplayName = "None"),
    FlankLeft   UMETA(DisplayName = "Flank Left"),
    FlankRight  UMETA(DisplayName = "Flank Right"),
    Distract    UMETA(DisplayName = "Distract"),
    Strike      UMETA(DisplayName = "Strike"),
    Retreat     UMETA(DisplayName = "Retreat"),
};

// Raptor pack role
UENUM(BlueprintType)
enum class EPerf_RaptorPackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha — leads attack"),
    Flanker     UMETA(DisplayName = "Flanker — circles prey"),
    Distractor  UMETA(DisplayName = "Distractor — draws attention"),
    Ambusher    UMETA(DisplayName = "Ambusher — waits for opening"),
};

// Per-raptor pack state
USTRUCT(BlueprintType)
struct FPerf_RaptorPackState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    EPerf_RaptorPackRole Role = EPerf_RaptorPackRole::Flanker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    EPerf_RaptorPackSignal CurrentSignal = EPerf_RaptorPackSignal::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<AActor> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float FlankAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float CoordinationRadius = 1200.0f;
};

/**
 * ARaptorCharacter — Velociraptor pack hunter
 * Fast (900 UU/s), coordinated, flanking attack patterns.
 * Pack of 3 raptors is more dangerous than 1 T-Rex for the player.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Raptor Character"))
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

    // === PACK COORDINATION ===

    /** Current pack state for this raptor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    FPerf_RaptorPackState PackState;

    /** All raptors in this pack (set by pack spawner or AI) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    TArray<TWeakObjectPtr<ARaptorCharacter>> PackMembers;

    /** Maximum pack size — raptors ignore prey if pack is full */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackSize = 4;

    // === COMBAT STATS ===

    /** Leap attack range — raptor jumps onto prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapAttackRange = 350.0f;

    /** Leap attack speed multiplier during jump */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapSpeedMultiplier = 2.2f;

    /** Cooldown between leap attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown = 4.0f;

    /** Claw slash damage per hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float ClawDamage = 35.0f;

    // === MOVEMENT ===

    /** Whether raptor is currently in leap */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|State",
              meta = (AllowPrivateAccess = "true"))
    bool bIsLeaping = false;

    /** Whether raptor is circling prey (flanking behavior) */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|State",
              meta = (AllowPrivateAccess = "true"))
    bool bIsCircling = false;

    // === PACK FUNCTIONS ===

    /** Broadcast signal to all pack members within CoordinationRadius */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void BroadcastPackSignal(EPerf_RaptorPackSignal Signal);

    /** Receive signal from pack leader or another pack member */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ReceivePackSignal(EPerf_RaptorPackSignal Signal, ARaptorCharacter* Sender);

    /** Calculate optimal flank position around target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    FVector CalculateFlankPosition(AActor* Target, float AngleDegrees) const;

    /** Join an existing pack */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    bool JoinPack(ARaptorCharacter* PackLeader);

    /** Dissolve pack (alpha dies or pack scatters) */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void DissolvePack();

    // === COMBAT FUNCTIONS ===

    /** Perform leap attack on target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeapAttack(AActor* Target);

    /** Called when leap lands on target */
    UFUNCTION(BlueprintNativeEvent, Category = "Raptor|Combat")
    void OnLeapLanded(AActor* Target);
    virtual void OnLeapLanded_Implementation(AActor* Target);

    /** Claw slash — melee attack when in range */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformClawSlash();

    // === OVERRIDES ===
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

protected:
    /** Timer handle for leap cooldown */
    FTimerHandle LeapCooldownTimer;

    /** Whether leap is on cooldown */
    bool bLeapOnCooldown = false;

    /** Reset leap cooldown */
    void ResetLeapCooldown();

    /** Update circling behavior — move around prey at flank angle */
    void UpdateCirclingBehavior(float DeltaTime);
};
