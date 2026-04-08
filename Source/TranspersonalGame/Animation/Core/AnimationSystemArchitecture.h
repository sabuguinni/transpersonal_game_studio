#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "AnimationSystemArchitecture.generated.h"

/**
 * Core Animation System Architecture for Transpersonal Game
 * 
 * This system is built around the principle that every movement tells a story.
 * Each character - human or dinosaur - has a unique movement signature that
 * defines their personality before any dialogue or interaction occurs.
 * 
 * Architecture Philosophy:
 * - Motion Matching for fluid, context-aware locomotion
 * - Procedural IK for terrain adaptation
 * - Behavioral animation layers for species-specific traits
 * - Individual variation system for unique dinosaur personalities
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // Human archetypes
    Survivor_Cautious       UMETA(DisplayName = "Cautious Survivor"),
    Survivor_Desperate      UMETA(DisplayName = "Desperate Survivor"),
    Survivor_Experienced    UMETA(DisplayName = "Experienced Survivor"),
    
    // Dinosaur behavioral archetypes
    Predator_Apex          UMETA(DisplayName = "Apex Predator"),
    Predator_Pack          UMETA(DisplayName = "Pack Hunter"),
    Predator_Ambush        UMETA(DisplayName = "Ambush Predator"),
    Herbivore_Herd         UMETA(DisplayName = "Herd Herbivore"),
    Herbivore_Solitary     UMETA(DisplayName = "Solitary Herbivore"),
    Herbivore_Defensive    UMETA(DisplayName = "Defensive Herbivore"),
    Scavenger_Opportunist  UMETA(DisplayName = "Opportunist Scavenger"),
    
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EAnimationLayer : uint8
{
    Base_Locomotion        UMETA(DisplayName = "Base Locomotion"),
    Terrain_Adaptation     UMETA(DisplayName = "Terrain Adaptation"),
    Behavioral_Traits      UMETA(DisplayName = "Behavioral Traits"),
    Individual_Variation   UMETA(DisplayName = "Individual Variation"),
    Emotional_State        UMETA(DisplayName = "Emotional State"),
    Environmental_Response UMETA(DisplayName = "Environmental Response"),
    
    MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnimationPersonality
{
    GENERATED_BODY()

    // Core personality traits that affect movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Alertness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fatigue = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fear = 0.0f;
    
    // Physical traits that affect movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float MovementSpeedMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float StepLengthVariation = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.9", ClampMax = "1.1"))
    float PostureVariation = 1.0f;
    
    FAnimationPersonality()
    {
        // Default neutral personality
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMotionMatchingConfig
{
    GENERATED_BODY()

    // Motion Matching Database for this character type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UPoseSearchDatabase> PoseDatabase;
    
    // Schema defining what features to match
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UPoseSearchSchema> SearchSchema;
    
    // Blend time for transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float DefaultBlendTime = 0.2f;
    
    // Context-specific blend times
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> ContextBlendTimes;
};

/**
 * Base class for all animation instances in the game
 * Provides common functionality for Motion Matching and personality-driven animation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

protected:
    // Core animation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    ECharacterArchetype CharacterArchetype = ECharacterArchetype::Survivor_Cautious;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    FAnimationPersonality Personality;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingConfig MotionMatchingConfig;
    
    // Current animation state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FVector Velocity;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float Speed;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float Direction;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInAir;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsMoving;
    
    // Terrain adaptation
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKOffset;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKOffset;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

public:
    // Animation Blueprint interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
    
    // Personality system
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetPersonality(const FAnimationPersonality& NewPersonality);
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnimationPersonality GetPersonality() const { return Personality; }
    
    // Motion Matching helpers
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetContextualBlendTime(const FString& Context) const;
    
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingDatabase(UPoseSearchDatabase* NewDatabase);

protected:
    // Update functions called each frame
    virtual void UpdateMovementData();
    virtual void UpdateTerrainAdaptation();
    virtual void UpdatePersonalityInfluence();
    
    // IK calculation helpers
    void CalculateFootIK(const FName& FootBoneName, float& OutOffset, FRotator& OutRotation);
    FVector GetFootLocation(const FName& FootBoneName) const;
    
private:
    // Cached references
    UPROPERTY()
    TObjectPtr<class APawn> OwningPawn;
    
    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;
    
    // Personality influence cache
    float PersonalitySpeedMultiplier = 1.0f;
    float PersonalityAnimationScale = 1.0f;
};