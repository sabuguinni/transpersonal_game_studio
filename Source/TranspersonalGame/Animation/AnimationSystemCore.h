#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game Studio
 * Handles Motion Matching, IK Systems, and Procedural Animation
 * 
 * Design Philosophy:
 * - Every movement tells a story about the character
 * - Weight and intention in every gesture
 * - Seamless blending between states without breaking immersion
 */

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Crouching UMETA(DisplayName = "Crouching"),
    Hiding UMETA(DisplayName = "Hiding"),
    Climbing UMETA(DisplayName = "Climbing"),
    Swimming UMETA(DisplayName = "Swimming"),
    Injured UMETA(DisplayName = "Injured"),
    Exhausted UMETA(DisplayName = "Exhausted"),
    Panicked UMETA(DisplayName = "Panicked")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Grazing UMETA(DisplayName = "Grazing"),
    Drinking UMETA(DisplayName = "Drinking"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Fighting UMETA(DisplayName = "Fighting"),
    Socializing UMETA(DisplayName = "Socializing"),
    Nesting UMETA(DisplayName = "Nesting"),
    Domesticated_Calm UMETA(DisplayName = "Domesticated Calm"),
    Domesticated_Playful UMETA(DisplayName = "Domesticated Playful")
};

USTRUCT(BlueprintType)
struct FAnimationPersonality
{
    GENERATED_BODY()

    // Core personality traits that affect animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Nervousness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialTendency = 0.5f;

    // Physical characteristics that affect movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float AnimationSpeedVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LimpSeverity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UniqueIdentifier;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Motion Matching Database Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeMotionMatchingSystem();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCharacterMovementState(ECharacterMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetDinosaurBehaviorState(EDinosaurBehaviorState NewState);

    // Personality-driven animation
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void ApplyAnimationPersonality(const FAnimationPersonality& Personality);

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateTerrainAdaptation(float SurfaceAngle, float SurfaceRoughness);

    // Domestication system
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateDomesticationLevel(float DomesticationProgress);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    class UPoseSearchDatabase* HumanLocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    class UPoseSearchDatabase* DinosaurBehaviorDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    class UPoseSearchSchema* DefaultMotionSchema;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK System")
    class UIKRigDefinition* HumanIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK System")
    class UIKRigDefinition* DinosaurIKRig;

private:
    ECharacterMovementState CurrentMovementState;
    EDinosaurBehaviorState CurrentBehaviorState;
    FAnimationPersonality CurrentPersonality;
    
    float TerrainAngle;
    float TerrainRoughness;
    float DomesticationLevel;
};