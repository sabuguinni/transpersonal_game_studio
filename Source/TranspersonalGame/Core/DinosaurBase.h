#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "SharedTypes.h"
#include "DinosaurBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurStateChanged, EEng_DinosaurBehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurDamaged, float, Damage, AActor*, DamageSource);

/**
 * Base class for all dinosaur types in the game
 * Provides common functionality for AI, movement, and survival mechanics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public APawn
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFloatingPawnMovement* MovementComponent;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EEng_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EEng_DinosaurSize SizeCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EEng_DinosaurDiet DietType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    // Behavior State
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Behavior")
    EEng_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Behavior")
    float FleeThreshold;

    // Territory and Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    bool bIsPackAnimal;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Pack")
    TArray<ADinosaurBase*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur|Pack")
    ADinosaurBase* PackLeader;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinosaurStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur|Events")
    FOnDinosaurDamaged OnDamaged;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Core")
    void SetBehaviorState(EEng_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Core")
    EEng_DinosaurBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void ModifyHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    void ModifyStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetHungerPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Stats")
    float GetStaminaPercentage() const;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    bool CanDetectTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void StartFleeing(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Behavior")
    void StartResting();

    // Territory Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    void SetTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Territory")
    FVector GetRandomLocationInTerritory() const;

    // Pack Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void JoinPack(ADinosaurBase* Leader);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void AddPackMember(ADinosaurBase* Member);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    void RemovePackMember(ADinosaurBase* Member);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Pack")
    bool IsPackLeader() const;

protected:
    // Internal Functions
    virtual void InitializeStats();
    virtual void UpdateBehavior(float DeltaTime);
    virtual void UpdateStats(float DeltaTime);
    virtual void HandleStateTransition(EEng_DinosaurBehaviorState OldState, EEng_DinosaurBehaviorState NewState);

    // Behavior Implementation
    virtual void ExecuteIdleBehavior(float DeltaTime);
    virtual void ExecutePatrolBehavior(float DeltaTime);
    virtual void ExecuteHuntingBehavior(float DeltaTime);
    virtual void ExecuteFleeingBehavior(float DeltaTime);
    virtual void ExecuteRestingBehavior(float DeltaTime);
    virtual void ExecuteFeedingBehavior(float DeltaTime);

    // Utility Functions
    AActor* FindNearestThreat() const;
    AActor* FindNearestPrey() const;
    FVector GetRandomPatrolLocation() const;
    bool ShouldFlee() const;
    bool ShouldHunt() const;
    bool ShouldRest() const;

private:
    // Internal state tracking
    float StateTimer;
    FVector LastKnownThreatLocation;
    AActor* CurrentTarget;
    FVector PatrolDestination;
    
    // Timers for stat regeneration
    float HealthRegenTimer;
    float StaminaRegenTimer;
    float HungerDecayTimer;
};