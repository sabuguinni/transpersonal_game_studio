#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

// ============================================================
// EEng_DinoSpecies — unique species enum (Eng_ prefix)
// ============================================================
UENUM(BlueprintType)
enum class EEng_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus")
};

// ============================================================
// EEng_DinoBehaviorState — current AI behavior state
// ============================================================
UENUM(BlueprintType)
enum class EEng_DinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// FEng_DinoStats — survival stats for a dinosaur
// ============================================================
USTRUCT(BlueprintType)
struct FEng_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float CurrentHunger = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 1000.0f;

    FEng_DinoStats() {}
};

// ============================================================
// ADinosaurBase — base class for all dinosaur pawns
// ============================================================
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Species & Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinoSpecies Species = EEng_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesDisplayName = TEXT("Unknown Dinosaur");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsPackHunter = false;

    // ---- Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinoStats DinoStats;

    // ---- Behavior State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EEng_DinoBehaviorState BehaviorState = EEng_DinoBehaviorState::Idle;

    // ---- Target ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* CurrentTarget = nullptr;

    // ---- Blueprint-callable functions ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDinoHit(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EEng_DinoBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void DetectNearbyThreats();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void HealDino(float Amount);

    // ---- Events (override in subclasses) ----
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinoSpotTarget(AActor* SpottedTarget);
    virtual void OnDinoSpotTarget_Implementation(AActor* SpottedTarget);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinoDeath();
    virtual void OnDinoDeath_Implementation();

protected:
    // Internal tick helpers
    void UpdateHunger(float DeltaTime);
    void UpdateBehaviorAI(float DeltaTime);

    float HungerDecayRate = 2.0f;   // hunger units per second
    float AIUpdateInterval = 0.5f;  // seconds between AI ticks
    float AIUpdateAccumulator = 0.0f;
};
