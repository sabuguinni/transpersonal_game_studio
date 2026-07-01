#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Pterodactylus       UMETA(DisplayName = "Pterodactylus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
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

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
};

// ============================================================
// Structs — must be at global scope (UE5 UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FDinosaurStats
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
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float HungerDecayRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float TerritoryRadius = 3000.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinosaurSpecies Species = EDinosaurSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinosaurDiet Diet = EDinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    FString SpeciesName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsFlying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    bool bIsAquatic = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    float ScaleMultiplier = 1.0f;
};

// ============================================================
// ADinosaurBase — Base class for all dinosaurs
// ============================================================

UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = "Dinosaurs",
       meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ---- Overrides ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Species & Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FDinosaurSpeciesData SpeciesData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FDinosaurStats Stats;

    // ---- Behavior State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinosaurBehaviorState BehaviorState = EDinosaurBehaviorState::Idle;

    // ---- AI Components ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAIPerceptionComponent> PerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    // ---- Target ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    TObjectPtr<AActor> CurrentTarget;

    // ---- Pack ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Pack")
    TArray<TObjectPtr<ADinosaurBase>> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    TObjectPtr<ADinosaurBase> PackLeader;

    // ---- Territory ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    bool bHasTerritory = true;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnDinosaurDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnTargetDetected(AActor* DetectedActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnAttack(AActor* AttackTarget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur|Events")
    void OnBehaviorStateChanged(EDinosaurBehaviorState NewState);

    // ---- Native Functions ----
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    bool IsPlayerInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    bool IsHungry() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void JoinPack(ADinosaurBase* Leader);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void AlertPack(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    bool IsWithinTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    void SetTerritoryCenter(const FVector& Center);

protected:
    virtual void InitializeSpeciesDefaults();
    virtual void UpdateHunger(float DeltaTime);
    virtual void HandleDeath();

    UPROPERTY()
    bool bIsDead = false;

    UPROPERTY()
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Dinosaur|Combat")
    float AttackCooldown = 1.5f;
};
