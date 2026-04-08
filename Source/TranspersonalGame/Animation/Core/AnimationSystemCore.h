#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, procedural animations, and character-specific behaviors
 * 
 * Design Philosophy:
 * - Every movement tells a story about the character
 * - Motion Matching for natural human locomotion
 * - Procedural systems for dinosaur uniqueness
 * - IK for terrain adaptation
 */

UENUM(BlueprintType)
enum class ECharacterAnimationState : uint8
{
    // Human States
    Idle_Calm           UMETA(DisplayName = "Idle - Calm"),
    Idle_Alert          UMETA(DisplayName = "Idle - Alert"), 
    Idle_Exhausted      UMETA(DisplayName = "Idle - Exhausted"),
    Walk_Cautious       UMETA(DisplayName = "Walk - Cautious"),
    Walk_Confident      UMETA(DisplayName = "Walk - Confident"),
    Run_Panicked        UMETA(DisplayName = "Run - Panicked"),
    Run_Determined      UMETA(DisplayName = "Run - Determined"),
    Crouch_Hide         UMETA(DisplayName = "Crouch - Hide"),
    Crouch_Observe      UMETA(DisplayName = "Crouch - Observe"),
    
    // Dinosaur States
    Dino_Idle_Grazing   UMETA(DisplayName = "Dinosaur - Grazing"),
    Dino_Idle_Resting   UMETA(DisplayName = "Dinosaur - Resting"),
    Dino_Walk_Patrol    UMETA(DisplayName = "Dinosaur - Patrol Walk"),
    Dino_Run_Hunt       UMETA(DisplayName = "Dinosaur - Hunting Run"),
    Dino_Run_Flee       UMETA(DisplayName = "Dinosaur - Fleeing"),
    Dino_Alert_Scanning UMETA(DisplayName = "Dinosaur - Alert Scan"),
    Dino_Aggressive     UMETA(DisplayName = "Dinosaur - Aggressive"),
    Dino_Curious        UMETA(DisplayName = "Dinosaur - Curious")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Anxious     UMETA(DisplayName = "Anxious"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Confident   UMETA(DisplayName = "Confident"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Curious     UMETA(DisplayName = "Curious"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Submissive  UMETA(DisplayName = "Submissive")
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    // Motion Matching Database for this character type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* MotionDatabase;
    
    // Emotional state influences animation selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState CurrentEmotionalState;
    
    // Fatigue affects movement speed and posture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FatigueLevel;
    
    // Fear level affects animation choices
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel;
    
    // Confidence affects posture and movement boldness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidenceLevel;
    
    // Unique movement characteristics for this individual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MovementPersonality; // X=Boldness, Y=Precision, Z=Energy
    
    FCharacterAnimationProfile()
    {
        MotionDatabase = nullptr;
        CurrentEmotionalState = EEmotionalState::Calm;
        FatigueLevel = 0.0f;
        FearLevel = 0.0f;
        ConfidenceLevel = 0.5f;
        MovementPersonality = FVector(0.5f, 0.5f, 0.5f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Initialize the animation system for a character
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeForCharacter(USkeletalMeshComponent* SkeletalMesh, const FCharacterAnimationProfile& Profile);
    
    // Update animation state based on gameplay context
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateAnimationState(ECharacterAnimationState NewState, float DeltaTime);
    
    // Update emotional state (affects animation choices)
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateEmotionalState(EEmotionalState NewState, float TransitionTime = 1.0f);
    
    // Get current animation database for Motion Matching
    UFUNCTION(BlueprintPure, Category = "Animation System")
    class UPoseSearchDatabase* GetCurrentMotionDatabase() const;
    
    // Calculate blend weights for emotional animations
    UFUNCTION(BlueprintPure, Category = "Animation System")
    float CalculateEmotionalBlendWeight(EEmotionalState TargetState) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FCharacterAnimationProfile CurrentProfile;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ECharacterAnimationState CurrentAnimationState;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float StateTransitionTime;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float EmotionalTransitionProgress;

private:
    void UpdateEmotionalTransition(float DeltaTime);
    ECharacterAnimationState SelectOptimalAnimationState() const;
};