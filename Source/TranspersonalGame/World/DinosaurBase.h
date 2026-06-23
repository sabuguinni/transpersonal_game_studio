// DinosaurBase.h
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260623_007
// Base class for all dinosaur pawns in the prehistoric survival world.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — UHT requirement) ───────────────────────────────────

UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    Unknown       UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "T-Rex"),
    Velociraptor  UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops   UMETA(DisplayName = "Triceratops"),
    Stegosaurus   UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl   UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus  UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus   UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class EDinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Resting     UMETA(DisplayName = "Resting"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinoBiome : uint8
{
    Grassland   UMETA(DisplayName = "Grassland"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Tundra      UMETA(DisplayName = "Tundra")
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FDinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    float BaseMaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    float BaseMaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    float BaseAttackDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    float BaseWalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    float BaseSprintSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    bool bIsPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Traits")
    EDinoBiome PreferredBiome = EDinoBiome::Grassland;
};

// ─── Delegate ─────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinoStateChanged, EDinoState, OldState, EDinoState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinoDied, ADinosaurBase*, Dinosaur);

// ─── Class ────────────────────────────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
    meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Lifecycle ──────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ── Species ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinoSpecies DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FDinoSpeciesTraits SpeciesTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinoBiome PreferredBiome;

    // ── State ─────────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDinoState DinoState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    EDinoBiome CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAggressive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    float ThreatLevel;

    // ── Health & Stamina ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate;

    // ── Combat ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsPredator;

    // ── Territory ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float PatrolRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector HomeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    bool bHomeLocked;

    // ── AI Perception ─────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    // ── Delegates ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinoStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinoDied OnDied;

    // ── Blueprint-callable functions ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Survival")
    void Feed(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|State")
    void SetDinoState(EDinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Territory")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Biome")
    bool IsInPreferredBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Biome")
    void UpdateCurrentBiome();

    // ── Overridable events (BlueprintNativeEvent) ─────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinoHit(float DamageAmount, AActor* DamageCauser);
    virtual void OnDinoHit_Implementation(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinoDied();
    virtual void OnDinoDied_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAttackPerformed(AActor* Target);
    virtual void OnAttackPerformed_Implementation(AActor* Target);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnThreatDetected(AActor* ThreatActor);
    virtual void OnThreatDetected_Implementation(AActor* ThreatActor);

#if WITH_EDITOR
    UPROPERTY(EditAnywhere, Category = "Dinosaur|Debug")
    bool bDrawDebugInfo = false;
#endif

private:
    FTimerHandle HungerTimerHandle;
    FTimerHandle StaminaTimerHandle;
    float LastAttackTime;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void TickHunger();
    void RegenStamina();
};
