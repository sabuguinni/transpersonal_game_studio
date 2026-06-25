// DinosaurBase.h
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260625_005
// Base pawn class for all dinosaurs. Holds SurvivalComponent for hunger/territory tracking.
// Designed for 60fps PC / 30fps console with LOD-aware tick throttling.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "Core/Survival/SurvivalComponent.h"
#include "DinosaurBase.generated.h"

// ── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Unknown         UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EPerf_DinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Territorial UMETA(DisplayName = "Territorial"),
    Dead        UMETA(DisplayName = "Dead")
};

// ── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float Mass = 1000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_DinoLODConfig
{
    GENERATED_BODY()

    // Distance at which tick rate drops to 2s interval (medium LOD)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Performance")
    float MediumLODDistance = 3000.0f;

    // Distance at which tick rate drops to 5s interval (far LOD)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Performance")
    float FarLODDistance = 6000.0f;

    // Distance at which AI is completely suspended (culled)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Performance")
    float CullDistance = 12000.0f;

    // Tick interval at medium LOD (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Performance")
    float MediumTickInterval = 2.0f;

    // Tick interval at far LOD (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Performance")
    float FarTickInterval = 5.0f;
};

// ── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPerf_OnDinoDied, ADinosaurBase*, Dinosaur, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPerf_OnDinoStateChanged, ADinosaurBase*, Dinosaur, EPerf_DinoBehaviorState, NewState);

// ── Class ────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Dinosaurs), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

    // ── Species & Stats ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    EPerf_DinoSpecies Species = EPerf_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    FPerf_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Performance")
    FPerf_DinoLODConfig LODConfig;

    // ── Behavior State ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Behavior",
              meta = (AllowPrivateAccess = "true"))
    EPerf_DinoBehaviorState BehaviorState = EPerf_DinoBehaviorState::Idle;

    UFUNCTION(BlueprintCallable, Category = "Dino|Behavior")
    void SetBehaviorState(EPerf_DinoBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dino|Behavior")
    EPerf_DinoBehaviorState GetBehaviorState() const { return BehaviorState; }

    // ── Health ───────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Health",
              meta = (AllowPrivateAccess = "true"))
    float CurrentHealth = 500.0f;

    UFUNCTION(BlueprintCallable, Category = "Dino|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dino|Health")
    bool IsDead() const { return BehaviorState == EPerf_DinoBehaviorState::Dead; }

    // ── Survival Component ───────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Survival",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USurvivalComponent> SurvivalComp;

    // ── Delegates ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Dino|Events")
    FPerf_OnDinoDied OnDinoDied;

    UPROPERTY(BlueprintAssignable, Category = "Dino|Events")
    FPerf_OnDinoStateChanged OnDinoStateChanged;

    // ── Performance: LOD-aware tick throttling ───────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dino|Performance")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Dino|Performance")
    void UpdateLODTickRate();

protected:
    // ── Internal ─────────────────────────────────────────────────────────────

    void HandleDeath(AActor* Killer);
    void UpdateBehaviorAI(float DeltaTime);

    // Accumulated time for LOD tick throttling
    float LODTickAccumulator = 0.0f;
    float CurrentLODTickInterval = 0.1f;

    // Cached player reference (updated lazily)
    UPROPERTY()
    TObjectPtr<APawn> CachedPlayerPawn;

    float PlayerCacheTimer = 0.0f;
    static constexpr float PlayerCacheInterval = 2.0f;
};
