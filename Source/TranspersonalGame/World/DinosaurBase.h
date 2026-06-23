// DinosaurBase.h
// Core Systems Programmer #03 — P2 Dinosaur Base Class
// Prehistoric survival game — realistic dinosaur behaviour foundation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — UHT requirement) ──────────────────────────────────

UENUM(BlueprintType)
enum class ECore_DinoSpecies : uint8
{
    Unknown        UMETA(DisplayName = "Unknown"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor   UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl    UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus   UMETA(DisplayName = "Ankylosaurus"),
};

UENUM(BlueprintType)
enum class ECore_DinoBehaviourState : uint8
{
    Idle       UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting    UMETA(DisplayName = "Hunting"),
    Fleeing    UMETA(DisplayName = "Fleeing"),
    Feeding    UMETA(DisplayName = "Feeding"),
    Resting    UMETA(DisplayName = "Resting"),
    Dead       UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class ECore_DietType : uint8
{
    Carnivore  UMETA(DisplayName = "Carnivore"),
    Herbivore  UMETA(DisplayName = "Herbivore"),
    Omnivore   UMETA(DisplayName = "Omnivore"),
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCore_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MaxHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float CurrentHealth = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackDamage = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float WalkSpeed = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float RunSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float Mass = 6000.f;  // kg — realistic T-Rex ~8000kg
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnDinoDeathDelegate,
    class ADinosaurBase*, Dinosaur,
    AActor*, Killer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnDinoStateChangedDelegate,
    class ADinosaurBase*, Dinosaur,
    ECore_DinoBehaviourState, NewState);

// ─── Class ────────────────────────────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
    meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    // ── Species & Diet ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dino|Identity")
    ECore_DinoSpecies Species = ECore_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dino|Identity")
    ECore_DietType DietType = ECore_DietType::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dino|Identity")
    FString SpeciesDisplayName = TEXT("Unknown Dinosaur");

    // ── Stats ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    FCore_DinoStats DinoStats;

    // ── Behaviour State ─────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Behaviour")
    ECore_DinoBehaviourState BehaviourState = ECore_DinoBehaviourState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Behaviour")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Behaviour")
    bool bIsAggressive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Behaviour")
    bool bIsPackHunter = false;

    // ── Delegates ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Dino|Events")
    FCore_OnDinoDeathDelegate OnDinoDeath;

    UPROPERTY(BlueprintAssignable, Category = "Dino|Events")
    FCore_OnDinoStateChangedDelegate OnStateChanged;

    // ── Overrides ───────────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ── UFunctions ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dino|Behaviour")
    void SetBehaviourState(ECore_DinoBehaviourState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dino|Combat")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dino|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dino|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dino|Behaviour")
    void RegisterWithBiomeManager();

    UFUNCTION(BlueprintCallable, Category = "Dino|Behaviour")
    AActor* FindNearestPrey() const;

protected:
    virtual void Die(AActor* Killer);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Components",
        meta = (AllowPrivateAccess = "true"))
    class USphereComponent* DetectionSphere = nullptr;

    float TimeSinceLastAttack = 0.f;
    static constexpr float AttackCooldown = 1.5f;
};
