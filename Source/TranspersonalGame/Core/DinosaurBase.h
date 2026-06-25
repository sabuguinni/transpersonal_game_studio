#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "DinosaurBase.generated.h"

// ============================================================
// ECore_DinoSpecies — unique species enum (Core_ prefix, Rule 2)
// MUST be at global scope (Rule 1)
// ============================================================
UENUM(BlueprintType)
enum class ECore_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops   UMETA(DisplayName = "Protoceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Tsintaosaurus   UMETA(DisplayName = "Tsintaosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Dilophosaurus   UMETA(DisplayName = "Dilophosaurus"),
    Compsognathus   UMETA(DisplayName = "Compsognathus"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Mosasaurus      UMETA(DisplayName = "Mosasaurus"),
};

// ============================================================
// ECore_DinoBehaviorState — current AI state
// ============================================================
UENUM(BlueprintType)
enum class ECore_DinoBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting"),
    Territorial UMETA(DisplayName = "Territorial"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ============================================================
// ECore_DietType — herbivore / carnivore / omnivore
// ============================================================
UENUM(BlueprintType)
enum class ECore_DietType : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

// ============================================================
// FCore_DinoStats — base stats struct (global scope, Rule 1)
// ============================================================
USTRUCT(BlueprintType)
struct FCore_DinoStats
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
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float SprintSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float Mass = 1000.0f;
};

// ============================================================
// ADinosaurBase — base class for all dinosaur pawns
// ============================================================
UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ---- Species & Diet ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ECore_DinoSpecies Species = ECore_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ECore_DietType DietType = ECore_DietType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName CommonName = NAME_None;

    // ---- Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FCore_DinoStats Stats;

    // ---- Behavior State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
        meta = (AllowPrivateAccess = "true"))
    ECore_DinoBehaviorState BehaviorState = ECore_DinoBehaviorState::Idle;

    // ---- Detection sphere ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* DetectionSphere;

    // ---- Territory sphere ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TerritorySphere;

    // ---- Pack behavior ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    float PackCohesionRadius = 1500.0f;

    // ---- Biome compatibility ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Biome")
    TArray<FName> AllowedBiomes;

    // ---- Overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Core behavior UFUNCTIONs ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(ECore_DinoBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    ECore_DinoBehaviorState GetBehaviorState() const { return BehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsAlive() const { return Stats.CurrentHealth > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    bool IsHungry() const { return Stats.Hunger < 30.0f; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    bool IsThreatened(AActor* PotentialThreat) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|AI")
    void OnDetectPrey(AActor* Prey);
    virtual void OnDetectPrey_Implementation(AActor* Prey);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|AI")
    void OnDetectThreat(AActor* Threat);
    virtual void OnDetectThreat_Implementation(AActor* Threat);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Combat")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Debug", CallInEditor)
    void DrawDebugInfo();

protected:
    // Internal tick helpers
    void TickHunger(float DeltaTime);
    void TickBehavior(float DeltaTime);

    // Detection overlap callbacks
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // Behavior timer
    float BehaviorTickInterval = 0.5f;
    float BehaviorTickAccumulator = 0.0f;
};
