#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Anim_DinosaurController.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Alert           UMETA(DisplayName = "Alert"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Roaming         UMETA(DisplayName = "Roaming")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Compsognathus   UMETA(DisplayName = "Compsognathus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurAnimationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Species")
    EAnim_DinosaurSpecies Species;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float HealthPercentage;

    FAnim_DinosaurAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        CurrentState = EAnim_DinosaurState::Idle;
        Species = EAnim_DinosaurSpecies::TRex;
        AggressionLevel = 0.5f;
        HungerLevel = 0.3f;
        FearLevel = 0.0f;
        bIsInCombat = false;
        HealthPercentage = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_DinosaurController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_DinosaurController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Pawn Reference")
    class APawn* OwningPawn;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_DinosaurAnimationData AnimationData;

    // Animation Montages for different behaviors
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* RoarMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* FeedingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* DrinkingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    class UAnimMontage* DeathMontage;

    // Movement thresholds (species-specific)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
    float HuntSpeedThreshold;

private:
    void UpdateMovementData();
    void UpdateBehaviorState();
    EAnim_DinosaurState CalculateDinosaurState();
    void SetSpeciesSpecificSettings();

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayBehaviorAction(EAnim_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetDinosaurSpecies(EAnim_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void TriggerAttack();

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FAnim_DinosaurAnimationData GetAnimationData() const { return AnimationData; }

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetAggressionLevel(float NewAggression);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetHungerLevel(float NewHunger);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetFearLevel(float NewFear);
};