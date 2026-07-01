#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Unknown             UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Attacking   UMETA(DisplayName = "Attacking")
};

USTRUCT(BlueprintType)
struct FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Mass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsPackHunter = false;
};

/**
 * ADinosaurBase — Base class for all dinosaur pawns in the prehistoric survival game.
 * Provides species identity, survival stats, behavior state machine, and patrol logic.
 * All specific dinosaur types (TRex, Raptor, Brachiosaurus) inherit from this class.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Species Identity ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurSpecies Species = EEng_DinosaurSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FName DinosaurName = NAME_None;

    // --- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinosaurStats DinoStats;

    // --- Behavior State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState BehaviorState = EEng_DinosaurBehaviorState::Idle;

    // --- Patrol Waypoints ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Patrol")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Patrol")
    int32 CurrentWaypointIndex = 0;

    // --- Target ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    AActor* CurrentTarget = nullptr;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState GetBehaviorState() const { return BehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void AddPatrolWaypoint(FVector WaypointLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Patrol")
    void ClearPatrolWaypoints();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Events")
    void OnDinosaurDeath();
    virtual void OnDinosaurDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dinosaur|Events")
    void OnTargetDetected(AActor* DetectedActor);
    virtual void OnTargetDetected_Implementation(AActor* DetectedActor);

protected:
    // --- Internal Behavior Logic ---
    void UpdatePatrol(float DeltaTime);
    void UpdateHunting(float DeltaTime);
    void ScanForTargets();
    bool IsTargetInRange(AActor* Target, float Range) const;
    void MoveToNextWaypoint();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Patrol")
    float WaypointAcceptanceRadius = 150.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Behavior")
    float BehaviorUpdateInterval = 0.5f;

    float BehaviorUpdateTimer = 0.0f;
};
