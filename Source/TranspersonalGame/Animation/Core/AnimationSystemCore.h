#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, IK, and procedural animation behaviors
 */

UENUM(BlueprintType)
enum class ECharacterAnimationState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Hiding,
    Climbing,
    Swimming,
    Injured,
    Dying
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Foraging,
    Hunting,
    Drinking,
    Sleeping,
    Territorial,
    Fleeing,
    Aggressive,
    Curious,
    Domesticated
};

USTRUCT(BlueprintType)
struct FAnimationPersonalityTraits
{
    GENERATED_BODY()

    // Movimento base
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float MovementSpeed = 1.0f;

    // Nervosismo - afeta frequência de olhar ao redor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Nervousness = 0.5f;

    // Confiança - afeta postura corporal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence = 0.5f;

    // Agressividade - afeta animações de ameaça
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    // Curiosidade - afeta investigação de objetos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;
};

USTRUCT(BlueprintType)
struct FDinosaurPhysicalVariation
{
    GENERATED_BODY()

    // Variações físicas que afetam animação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float SizeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.9", ClampMax = "1.1"))
    float LimbLength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.9", ClampMax = "1.1"))
    float HeadSize = 1.0f;

    // Ferimentos ou defeitos que afetam movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasLimp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LimpingLimb = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "0.9"))
    float LimpSeverity = 0.0f;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Sistema de personalidade para animações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Personality")
    FAnimationPersonalityTraits PersonalityTraits;

    // Variações físicas para dinossauros
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variation")
    FDinosaurPhysicalVariation PhysicalVariation;

    // Estado atual de animação
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    ECharacterAnimationState CurrentAnimationState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EDinosaurBehaviorState CurrentBehaviorState;

    // Funções para mudança de estado
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationState(ECharacterAnimationState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    // Sistema de medo e tensão
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentFearLevel = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Fear System")
    void UpdateFearLevel(float DeltaFear);

    // Sistema de domesticação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DomesticationLevel = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void UpdateDomestication(float DeltaTime, bool bPlayerNearby, bool bPlayerThreatening);

private:
    // Timers para comportamentos procedurais
    float IdleBehaviorTimer = 0.0f;
    float LookAroundTimer = 0.0f;
    float PostureAdjustmentTimer = 0.0f;

    void UpdateProceduralBehaviors(float DeltaTime);
    void TriggerIdleBehavior();
    void AdjustPostureBasedOnPersonality();
};