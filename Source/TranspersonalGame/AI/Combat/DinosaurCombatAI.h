
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/DamageType.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — all prefixed Combat_ to avoid global name collisions
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Stalking     UMETA(DisplayName = "Stalking"),
    Charging     UMETA(DisplayName = "Charging"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Retreating   UMETA(DisplayName = "Retreating"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    PackHunt     UMETA(DisplayName = "PackHunt")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl    UMETA(DisplayName = "Pterodactyl")
};

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha    UMETA(DisplayName = "Alpha — leads charge"),
    Flanker  UMETA(DisplayName = "Flanker — circles target"),
    Ambusher UMETA(DisplayName = "Ambusher — waits in cover"),
    Herder   UMETA(DisplayName = "Herder — drives prey toward pack")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float BiteDamage = 75.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ClawDamage = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 900.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRange = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float HearingRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    AActor* MemberActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_PackRole Role = ECombat_PackRole::Flanker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FVector AssignedFlankPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsInPosition = false;
};

// ============================================================
// DAMAGE TYPE
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCombat_DinosaurDamageType : public UDamageType
{
    GENERATED_BODY()

public:
    UCombat_DinosaurDamageType();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DamageType")
    float ArmorPenetration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DamageType")
    bool bCausesBleed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DamageType")
    float BleedDamagePerSecond = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DamageType")
    float BleedDuration = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DamageType")
    bool bCausesKnockback = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DamageType")
    float KnockbackForce = 1000.f;
};

// ============================================================
// MAIN COMPONENT
// ============================================================

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    // --- Species & State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_PackRole PackRole = ECombat_PackRole::Alpha;

    // --- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    // --- Target ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Target")
    AActor* CurrentTarget = nullptr;

    // --- Pack ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius = 2000.f;

    // --- Timers ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Timers")
    float TimeSinceLastAttack = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Timers")
    float TimeSinceTargetSeen = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Timers")
    float GiveUpChaseTime = 12.f;

    // --- UActorComponent interface ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- State machine ---
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage(float DamageAmount);

    // --- Pack coordination ---
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void InitiatePackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    FVector CalculateFlankPosition(AActor* Target, int32 FlankIndex, int32 TotalFlankers);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastPackSignal(ECombat_DinoState SignalState);

    // --- Species presets ---
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplySpeciesPreset(ECombat_DinoSpecies InSpecies);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInAttackRange() const;

private:
    void TickIdle(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRetreating(float DeltaTime);
    void TickPackHunt(float DeltaTime);

    FVector HomeLocation;
    float StateTimer = 0.f;
};
