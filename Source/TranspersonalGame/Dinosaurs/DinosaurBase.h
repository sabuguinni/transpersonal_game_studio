#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class EEng_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
};

UENUM(BlueprintType)
enum class EEng_DinosaurDiet : uint8
{
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Omnivore    UMETA(DisplayName = "Omnivore"),
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
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SprintSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Mass = 500.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FEng_DinosaurStats DinoStats;

    // Species name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesName = TEXT("Unknown Dinosaur");

    // Diet type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EEng_DinosaurDiet Diet = EEng_DinosaurDiet::Carnivore;

    // Current behavior state
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState BehaviorState = EEng_DinosaurBehaviorState::Idle;

    // Is aggressive toward player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    bool bIsAggressive = true;

    // Current target actor
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    AActor* CurrentTarget = nullptr;

    // Apply damage to this dinosaur
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void TakeDinosaurDamage(float DamageAmount, AActor* DamageCauser);

    // Check if dinosaur is alive
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    bool IsAlive() const;

    // Set behavior state
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    // Get detection radius
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    float GetDetectionRadius() const;

    // Called when dinosaur dies
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDeath();
    virtual void OnDinosaurDeath_Implementation();

    // Called when dinosaur detects player
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnPlayerDetected(AActor* DetectedPlayer);
    virtual void OnPlayerDetected_Implementation(AActor* DetectedPlayer);

protected:
    // Simple AI tick — scan for player
    void UpdateBehaviorAI(float DeltaTime);

    // Timer for AI updates (runs every 0.5s)
    float AIUpdateTimer = 0.0f;
    static constexpr float AIUpdateInterval = 0.5f;
};
