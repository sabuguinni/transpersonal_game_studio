#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurBehaviorState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial"),
    Mating      UMETA(DisplayName = "Mating")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Pteranodon  UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Compsognathus UMETA(DisplayName = "Compsognathus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurAnimData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_DinosaurMovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    EAnim_DinosaurBehaviorState BehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "Species")
    EAnim_DinosaurSpecies Species;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AggressionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float AlertnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bNearWater;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bNearFood;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bNearThreat;

    FAnim_DinosaurAnimData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        MovementState = EAnim_DinosaurMovementState::Idle;
        BehaviorState = EAnim_DinosaurBehaviorState::Passive;
        Species = EAnim_DinosaurSpecies::TRex;
        AggressionLevel = 0.0f;
        HealthPercentage = 1.0f;
        HungerLevel = 0.5f;
        AlertnessLevel = 0.0f;
        bNearWater = false;
        bNearFood = false;
        bNearThreat = false;
    }
};

/**
 * Animation Instance para dinossauros
 * Gere comportamentos específicos de cada espécie, estados de caça e reações territoriais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_DinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_DinosaurAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // === DADOS DE ANIMAÇÃO ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data", meta = (AllowPrivateAccess = "true"))
    FAnim_DinosaurAnimData AnimData;

    // === REFERÊNCIAS ===
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    APawn* OwnerPawn;

    // === CONFIGURAÇÕES POR ESPÉCIE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Settings")
    float ChargeSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Settings")
    float AggressionSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Settings")
    float TerritorialRadius;

    // === BLEND SPACES POR ESPÉCIE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* CarnivorousLocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* HerbivorousLocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* FlyingLocomotionBlendSpace;

    // === MONTAGES DE COMPORTAMENTO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* RoarMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* FeedingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DrinkingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* AlertMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* DeathMontage;

public:
    // === FUNÇÕES PÚBLICAS ===
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayBehaviorMontage(EAnim_DinosaurBehaviorState BehaviorType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopBehaviorMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetSpecies(EAnim_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAggressionLevel(float NewAggression);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetHungerLevel(float NewHunger);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerThreatResponse();

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetCurrentSpeed() const { return AnimData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_DinosaurMovementState GetMovementState() const { return AnimData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_DinosaurBehaviorState GetBehaviorState() const { return AnimData.BehaviorState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_DinosaurSpecies GetSpecies() const { return AnimData.Species; }

private:
    // === FUNÇÕES PRIVADAS ===
    void UpdateMovementData(float DeltaTime);
    void UpdateBehaviorState();
    void UpdateEnvironmentalAwareness();
    void UpdateSpeciesSpecificBehavior();
    EAnim_DinosaurMovementState CalculateMovementState() const;
    EAnim_DinosaurBehaviorState CalculateBehaviorState() const;
    UAnimMontage* GetMontageForBehavior(EAnim_DinosaurBehaviorState BehaviorType) const;
    UBlendSpace* GetBlendSpaceForSpecies() const;
    void ConfigureSpeciesSettings();

    // === VARIÁVEIS DE CONTROLO ===
    float LastDirection;
    float TimeInCurrentBehavior;
    float LastAggressionCheck;
    float ThreatResponseCooldown;
    bool bWasMoving;
    FVector LastKnownThreatLocation;
};