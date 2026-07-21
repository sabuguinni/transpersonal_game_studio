#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

class ANPC_DinosaurPawn;

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurBehaviorState CurrentState = ENPC_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsInPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<ANPC_DinosaurPawn*> PackMembers;

    FNPC_DinosaurBehaviorState()
    {
        CurrentState = ENPC_DinosaurBehaviorState::Idle;
        StateTimer = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        AlertLevel = 0.0f;
        bIsInPack = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MaxDetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    int32 PreferredPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsHerbivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float TerritoryRadius = 5000.0f;

    FNPC_DinosaurSpeciesTraits()
    {
        Species = ENPC_DinosaurSpecies::TRex;
        MaxDetectionRange = 2000.0f;
        AttackRange = 300.0f;
        MovementSpeed = 400.0f;
        AggressionLevel = 0.5f;
        bIsPackHunter = false;
        PreferredPackSize = 1;
        bIsHerbivore = false;
        TerritoryRadius = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeBehavior(ENPC_DinosaurSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    ENPC_DinosaurBehaviorState GetCurrentBehaviorState() const;

    // Detection and targeting
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* DetectNearbyThreats();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* DetectNearbyPrey();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInRange(float Range) const;

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void JoinPack(ANPC_DinosaurPawn* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdatePackBehavior();

    // Territory and patrol
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetTerritoryCenter(FVector Center);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsInTerritory(FVector Location) const;

    // Combat behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldAttack(AActor* Target) const;

    // Environmental reactions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ReactToWeather(ENPC_WeatherType Weather);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ReactToTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ReactToNoise(FVector NoiseLocation, float NoiseLevel);

protected:
    // Behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
    FNPC_DinosaurBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurSpeciesTraits SpeciesTraits;

    // Territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 5000.0f;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float PatrolTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float DetectionTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float CombatTimer = 0.0f;

    // References
    UPROPERTY()
    class APawn* OwnerPawn;

    UPROPERTY()
    class APlayerController* PlayerController;

private:
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateCombatBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);

    void InitializeSpeciesTraits();
    bool CanDetectActor(AActor* Target) const;
    float CalculateThreatLevel(AActor* Target) const;
};