#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatAI.generated.h"

// Combat AI state for dinosaur enemies
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    PackWaiting UMETA(DisplayName = "PackWaiting")
};

// Dinosaur species type for AI behavior selection
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TyrannosaurusRex UMETA(DisplayName = "TyrannosaurusRex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

// Pack role for social predators (raptors)
UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    LeftFlanker UMETA(DisplayName = "LeftFlanker"),
    RightFlanker UMETA(DisplayName = "RightFlanker"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Distractor  UMETA(DisplayName = "Distractor")
};

// Threat perception data
USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsVisible = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsAudible = false;
};

// Combat attack data
USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float BaseDamage = 25.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ChargeSpeed = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bCanFlank = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;
};

/**
 * UCombat_DinosaurCombatAI
 * Core combat AI component for dinosaur enemies.
 * Implements pack hunting, flanking, ambush, and species-specific behavior.
 * Agent #12 — Combat & Enemy AI
 */
UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Species & Role ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Identity")
    ECombat_PackRole PackRole = ECombat_PackRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Identity")
    bool bIsPackLeader = false;

    // ─── State ────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float StateTimer = 0.0f;

    // ─── Perception ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float SmellRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Perception")
    FCombat_ThreatData CurrentThreat;

    // ─── Attack ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    FCombat_AttackData AttackProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Attack")
    float LastAttackTime = 0.0f;

    // ─── Pack AI ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    TArray<UCombat_DinosaurCombatAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float FlankingAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float PackCoordinationRadius = 1200.0f;

    // ─── Memory ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Memory")
    TArray<FCombat_ThreatData> ThreatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Memory")
    float MemoryDuration = 30.0f;

    // ─── Functions ────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanHearTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatPerception(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecutePackFlank(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void NotifyPackOfThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingPosition(AActor* Target, float AngleOffset) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ApplySpeciesDefaults();

private:
    void TickIdle(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRetreating(float DeltaTime);
    void PurgeStaleMemory(float CurrentTime);
};
