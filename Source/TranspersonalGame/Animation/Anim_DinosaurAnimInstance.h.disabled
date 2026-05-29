#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Alert       UMETA(DisplayName = "Alert"),
    Roaring     UMETA(DisplayName = "Roaring"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_DinosaurState CurrentState = EAnim_DinosaurState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Species")
    EAnim_DinosaurSpecies Species = EAnim_DinosaurSpecies::TRex;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAggressive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float HealthPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float HungerLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float AlertLevel = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_DinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_DinosaurAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_DinosaurData DinosaurData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    bool bShouldPlayIdleVariation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* RoarMontage = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* EatMontage = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* DeathMontage = nullptr;

public:
    // Animation control functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayRoarAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayEatAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayDeathAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetDinosaurState(EAnim_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetDinosaurSpecies(EAnim_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetAggressionLevel(float AggressionLevel);

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_DinosaurState GetCurrentState() const { return DinosaurData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetMovementSpeed() const { return DinosaurData.Speed; }

private:
    void UpdateMovementData();
    void UpdateStateLogic();
    void UpdateSpeciesSpecificBehavior();

    APawn* OwnerPawn = nullptr;
    UCharacterMovementComponent* MovementComponent = nullptr;

    float IdleTimer = 0.0f;
    float StateChangeTimer = 0.0f;
    float LastAttackTime = 0.0f;
};