#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "SharedTypes.h"
#include "Eng_DinosaurBase.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float CurrentStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackRange = 200.0f;

    FEng_DinosaurStats()
    {
        MaxHealth = 100.0f;
        CurrentHealth = 100.0f;
        MaxStamina = 100.0f;
        CurrentStamina = 100.0f;
        AttackDamage = 25.0f;
        MovementSpeed = 300.0f;
        DetectionRange = 1000.0f;
        AttackRange = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_DinosaurBase : public APawn
{
    GENERATED_BODY()

public:
    AEng_DinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UFloatingPawnMovement* MovementComponent;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurSpecies Species = EDinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurBehavior CurrentBehavior = EDinosaurBehavior::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EBiomeType PreferredBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FEng_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    bool bIsAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float HungerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float ThirstLevel = 0.5f;

    // AI and Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FVector PatrolCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float StateChangeTimer = 0.0f;

    // Blueprint Events
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetDinosaurSpecies(EDinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetBehaviorState(EDinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool IsInPreferredBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void ConsumeStamina(float StaminaCost);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool CanPerformAction() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    FVector GetRandomPatrolLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void StopPatrolling();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur")
    void OnHealthChanged(float NewHealth, float MaxHealth);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur")
    void OnBehaviorChanged(EDinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur")
    void OnTargetDetected(APawn* DetectedTarget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur")
    void OnDeath();

protected:
    virtual void UpdateBehavior(float DeltaTime);
    virtual void UpdateStats(float DeltaTime);
    virtual void CheckForTargets();
    virtual void HandleMovement(float DeltaTime);
    
    void InitializeDinosaurMesh();
    void SetupStatsForSpecies();

private:
    float LastBehaviorUpdateTime = 0.0f;
    float BehaviorUpdateInterval = 2.0f;
    
    bool bIsDead = false;
    FVector LastKnownTargetLocation = FVector::ZeroVector;
};

#include "Eng_DinosaurBase.generated.h"