#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Territorial UMETA(DisplayName = "Territorial"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Drinking    UMETA(DisplayName = "Drinking")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Thirst = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fatigue = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingRange = 1500.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurBehavior CurrentBehavior = ENPC_DinosaurBehavior::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    bool bIsPackHunter = false;

    // Movement Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationSpeed = 90.0f;

    // Territory System
    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    int32 CurrentWaypointIndex = 0;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehavior(ENPC_DinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void DefendTerritory();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void FleeFromThreat(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* FindNearestPrey();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsInTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateStats(float DeltaTime);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle StatsUpdateTimer;
    
    AActor* CurrentTarget = nullptr;
    FVector LastKnownTargetLocation = FVector::ZeroVector;
    float BehaviorTimer = 0.0f;
    
    void UpdateBehavior();
    void ProcessIdleBehavior();
    void ProcessGrazingBehavior();
    void ProcessHuntingBehavior();
    void ProcessTerritorialBehavior();
    void ProcessFleeingBehavior();
    void ProcessSleepingBehavior();
    void ProcessDrinkingBehavior();
    
    void MoveToLocation(FVector TargetLocation, float Speed);
    void RotateTowards(FVector TargetLocation);
    FVector GetRandomLocationInRadius(FVector Center, float Radius);
    void SetupSpeciesDefaults();
};