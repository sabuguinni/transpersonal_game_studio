#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, IK, and character-specific animation logic
 * 
 * Design Philosophy:
 * - Every movement tells a story about the character
 * - Fear and vulnerability are communicated through body language
 * - Responsive, weight-based animations using Motion Matching
 * - Terrain adaptation through IK systems
 */

UENUM(BlueprintType)
enum class ECharacterAnimationState : uint8
{
    Idle,
    Cautious,          // Slow, careful movement
    Sneaking,          // Crouched, silent movement
    Walking,           // Normal walking pace
    Jogging,           // Faster movement, still controlled
    Running,           // Panic running
    Climbing,          // Terrain navigation
    Hiding,            // Stationary fear state
    Observing,         // Watching dinosaurs from distance
    Gathering,         // Resource collection animations
    Crafting,          // Tool/weapon creation
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    Flat,
    Uphill,
    Downhill,
    Rocky,
    Muddy,
    Vegetation,
    Water,
    MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FAnimationStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterAnimationState CurrentState = ECharacterAnimationState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.0f; // 0.0 = calm, 1.0 = terrified

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CautionLevel = 0.5f; // Base caution in prehistoric world

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerrainType CurrentTerrain = ETerrainType::Flat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MovementDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsObservingDinosaur = false;

    FAnimationStateData()
    {
        CurrentState = ECharacterAnimationState::Idle;
        MovementSpeed = 0.0f;
        FearLevel = 0.0f;
        CautionLevel = 0.5f;
        CurrentTerrain = ETerrainType::Flat;
        MovementDirection = FVector::ZeroVector;
        bIsHidden = false;
        bIsObservingDinosaur = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Motion Matching Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> CautiousMovementDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> PanicMovementDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    // Animation State Management
    UPROPERTY(BlueprintReadWrite, Category = "Animation State")
    FAnimationStateData CurrentAnimationState;

    // Core Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState(const FAnimationStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UPoseSearchDatabase* GetActiveDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float CalculateBlendTime(ECharacterAnimationState FromState, ECharacterAnimationState ToState) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetTerrainType(ETerrainType NewTerrain);

protected:
    // Internal state management
    UPROPERTY()
    float StateTransitionTimer = 0.0f;

    UPROPERTY()
    ECharacterAnimationState PreviousState = ECharacterAnimationState::Idle;

private:
    // Animation blending parameters
    static constexpr float FAST_BLEND_TIME = 0.2f;
    static constexpr float NORMAL_BLEND_TIME = 0.5f;
    static constexpr float SLOW_BLEND_TIME = 1.0f;
};