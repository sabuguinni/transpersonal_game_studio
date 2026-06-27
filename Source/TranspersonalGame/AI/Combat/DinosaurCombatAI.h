#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// Combat AI State Machine — Agent #12 Combat & Enemy AI
// Implements tactical combat behaviour for all dinosaur species
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRadius = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChargeSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float BaseDamage = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bCanFlank = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankAngleDeg = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float StalkDuration = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TerritoryRadius = 5000.f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastSeenTime = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Species Configuration ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    FCombat_DinoSpeciesTraits SpeciesTraits;

    // ── Runtime State ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float CurrentHealth = 500.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    TArray<FCombat_ThreatEntry> ThreatList;

    // ── Pack Coordination ──────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    int32 MaxPackSize = 4;

    // ── Public API ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsHostile() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankPosition(AActor* Target, float SideOffset) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BroadcastPackAlert(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    // ── Static Species Presets ─────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI|Species")
    static FCombat_DinoSpeciesTraits GetTRexTraits();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Species")
    static FCombat_DinoSpeciesTraits GetRaptorTraits();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Species")
    static FCombat_DinoSpeciesTraits GetTriceratopsTraits();

private:
    // ── Internal Tick Logic ────────────────────────────────
    void TickIdle(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRetreating(float DeltaTime);

    void EvaluateThreatList();
    void PruneStaleThreatEntries();
    bool HasLineOfSightToTarget() const;
    float GetDistanceToTarget() const;
    void AttemptAttack();
    void SelectFlankRole();

    float StateTimer = 0.f;
    float ThreatEvalTimer = 0.f;
    bool bIsFlankLeft = false;
    FVector HomeLocation = FVector::ZeroVector;
    int32 PackRoleIndex = 0;
};
