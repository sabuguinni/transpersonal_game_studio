#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurType : uint8
{
    Carnivore       UMETA(DisplayName = "Carnivore"),
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Omnivore        UMETA(DisplayName = "Omnivore"),
    Scavenger       UMETA(DisplayName = "Scavenger")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehavior : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Fear = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public APawn
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AttackSphere;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurType DinosaurType = ENPC_DinosaurType::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurBehavior CurrentBehavior = ENPC_DinosaurBehavior::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FString SpeciesName = TEXT("Unknown Dinosaur");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float AttackRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float AttackDamage = 25.0f;

    // AI Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float IdleTime = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float CurrentIdleTime = 0.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetBehavior(ENPC_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void StartFleeing(AActor* ThreatSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void AddPatrolPoint(FVector Point);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void ClearPatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool IsPlayerInRange(float Range);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void MoveTowardsTarget(FVector TargetLocation, float DeltaTime);

protected:
    // AI Update Functions
    void UpdateAI(float DeltaTime);
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);

    // Detection
    UFUNCTION()
    void OnDetectionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    float LastBehaviorChangeTime = 0.0f;
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    bool bHasValidTarget = false;
};