#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "DinosaurBase.generated.h"

/**
 * Base class for all dinosaur pawns in the game
 * Provides common functionality for movement, health, and behavior
 */
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
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UFloatingPawnMovement* MovementComponent;

    // Dinosaur Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float DetectionRange;

    // Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsAggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur")
    void SetAggressionState(bool bAggressive);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur")
    void OnHealthChanged(float NewHealth, float MaxHealthValue);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dinosaur")
    void OnDeath();

protected:
    UFUNCTION()
    void UpdateBehavior(float DeltaTime);

    UFUNCTION()
    void UpdateHunger(float DeltaTime);
};