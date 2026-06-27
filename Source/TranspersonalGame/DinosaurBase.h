#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// EEng_DinoSpecies — unique prefix "Eng_" to avoid name collision with other agents
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

// ─────────────────────────────────────────────────────────────────────────────
// EEng_DinoBehaviorState — current AI state of the dinosaur
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EEng_DinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FEng_DinoStats — core stats for a dinosaur species
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    bool bIsPack = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// ADinosaurBase — base class for all dinosaur actors
// Inherits from ACharacter to get movement, capsule, mesh, and NavMesh support
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Species & Stats ───────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinoSpecies Species = EEng_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FEng_DinoStats DinoStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    EEng_DinoBehaviorState BehaviorState = EEng_DinoBehaviorState::Idle;

    // ── Territory ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector HomeLocation = FVector::ZeroVector;

    // ── Combat ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    TObjectPtr<AActor> CurrentTarget = nullptr;

    // ── Blueprint Events ──────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageCauser);
    virtual void OnTakeDamage_Implementation(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Combat")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Behavior")
    void OnTargetDetected(AActor* Target);
    virtual void OnTargetDetected_Implementation(AActor* Target);

    // ── Public API ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void ApplyDamage(float Amount, AActor* Causer);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EEng_DinoBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const { return DinoStats.CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

protected:
    // ── Internal Behavior ─────────────────────────────────────────────────
    virtual void InitializeStats();
    virtual void UpdateBehavior(float DeltaTime);
    virtual void ScanForTargets();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Internal")
    float TimeSinceLastScan = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Internal")
    float ScanInterval = 1.0f;
};
