#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/MotionMatchingAnimNodeLibrary.h"
#include "AnimationSystemCore.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Sneaking,
    Climbing,
    Swimming,
    Falling,
    Landing,
    Injured,
    Exhausted,
    Frightened,
    Observing
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Nervous,
    Afraid,
    Terrified,
    Curious,
    Cautious,
    Confident,
    Exhausted,
    Injured
};

USTRUCT(BlueprintType)
struct FCharacterAnimationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    ECharacterMovementState MovementState = ECharacterMovementState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Emotion")
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    float TerrainAngle = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    bool bIsOnUneven Terrain = false;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    bool bIsNearDinosaur = false;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float DinosaurThreatLevel = 0.0f;
};

/**
 * Core animation system component for the paleontologist character
 * Handles Motion Matching, IK foot placement, and emotional state blending
 */
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

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationData(const FCharacterAnimationData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FCharacterAnimationData GetCurrentAnimationData() const { return CurrentAnimationData; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EEmotionalState NewState, float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ReactToDinosaur(float ThreatLevel, float Distance);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FCharacterAnimationData CurrentAnimationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float EmotionalBlendSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FearDecayRate = 0.5f;

private:
    void UpdateEmotionalState(float DeltaTime);
    void UpdateFearLevel(float DeltaTime);
};