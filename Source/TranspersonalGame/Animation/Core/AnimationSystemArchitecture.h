#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemArchitecture.generated.h"

/**
 * Core Animation System Architecture for Transpersonal Game
 * 
 * This system handles:
 * - Motion Matching for fluid character movement
 * - IK foot placement for terrain adaptation
 * - Emotional state-driven animation blending
 * - Procedural fear/stress responses
 * - Dinosaur behavioral animation systems
 */

UENUM(BlueprintType)
enum class ECharacterEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Nervous     UMETA(DisplayName = "Nervous"), 
    Afraid      UMETA(DisplayName = "Afraid"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Focused     UMETA(DisplayName = "Focused")
};

UENUM(BlueprintType)
enum class EMovementContext : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Sneaking    UMETA(DisplayName = "Sneaking"),
    Hiding      UMETA(DisplayName = "Hiding"),
    Injured     UMETA(DisplayName = "Injured"),
    Carrying    UMETA(DisplayName = "Carrying"),
    Climbing    UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct FAnimationStateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    ECharacterEmotionalState EmotionalState = ECharacterEmotionalState::Calm;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    EMovementContext MovementContext = EMovementContext::Normal;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    float StressLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    float FatigueLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    bool bIsBeingHunted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    bool bCanSeeThreats = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    FVector LastKnownThreatDirection = FVector::ZeroVector;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationStateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationStateManager();

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateEmotionalState(ECharacterEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateMovementContext(EMovementContext NewContext);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateStressLevel(float NewStressLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    FAnimationStateData GetCurrentAnimationState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void OnThreatDetected(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void OnThreatLost();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    FAnimationStateData CurrentState;

    UPROPERTY(EditAnywhere, Category = "Animation Tuning")
    float StressDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Animation Tuning")
    float FatigueAccumulationRate = 0.1f;

    void UpdateStressDecay(float DeltaTime);
    void UpdateFatigueAccumulation(float DeltaTime);
};