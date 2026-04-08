#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "Animation/AnimNode_MotionMatching.h"
#include "Animation/AnimNode_PoseSearchHistoryCollector.h"
#include "../Core/AnimationSystemCore.h"
#include "MotionMatchingController.generated.h"

/**
 * Motion Matching Controller for Jurassic Survival Game
 * 
 * Handles dynamic pose selection based on:
 * - Current movement state and velocity
 * - Emotional state (fear, aggression, curiosity)
 * - Environmental context (terrain, nearby threats)
 * - Individual personality variations
 * 
 * Based on RDR2's approach where every step tells a story about the character
 */

USTRUCT(BlueprintType)
struct FMotionMatchingQuery
{
    GENERATED_BODY()

    // Movement parameters
    UPROPERTY(BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;
    
    UPROPERTY(BlueprintReadWrite)
    FVector Acceleration = FVector::ZeroVector;
    
    UPROPERTY(BlueprintReadWrite)
    float Speed = 0.0f;
    
    UPROPERTY(BlueprintReadWrite)
    float Direction = 0.0f;
    
    // Emotional state influences
    UPROPERTY(BlueprintReadWrite)
    EEmotionalState CurrentEmotionalState = EEmotionalState::Calm;
    
    UPROPERTY(BlueprintReadWrite)
    float FearLevel = 0.0f;
    
    UPROPERTY(BlueprintReadWrite)
    float AlertnessLevel = 0.0f;
    
    // Environmental context
    UPROPERTY(BlueprintReadWrite)
    float TerrainSlope = 0.0f;
    
    UPROPERTY(BlueprintReadWrite)
    bool bIsInDenseVegetation = false;
    
    UPROPERTY(BlueprintReadWrite)
    bool bNearWater = false;
    
    UPROPERTY(BlueprintReadWrite)
    bool bHasNearbyThreats = false;
    
    // Individual variations
    UPROPERTY(BlueprintReadWrite)
    FCreaturePersonality Personality;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMotionMatchingController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UMotionMatchingController();

    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingQuery();
    
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* SelectOptimalDatabase();
    
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateBlendTime(const FAnimNode_MotionMatching& MotionMatchingNode);

    // Personality-based animation modifiers
    UFUNCTION(BlueprintCallable, Category = "Animation Modifiers")
    float GetPersonalitySpeedModifier() const;
    
    UFUNCTION(BlueprintCallable, Category = "Animation Modifiers")
    float GetPersonalityPostureOffset() const;
    
    UFUNCTION(BlueprintCallable, Category = "Animation Modifiers")
    float GetStepLengthModifier() const;

protected:
    // Core References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<UAnimationSystemCore> AnimationSystem;
    
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ACharacter> OwningCharacter;
    
    // Motion Matching State
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FMotionMatchingQuery CurrentQuery;
    
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> ActiveDatabase;
    
    // Creature Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Setup")
    ECreatureArchetype CreatureType = ECreatureArchetype::Paleontologist;
    
    UPROPERTY(BlueprintReadOnly, Category = "Creature Setup")
    FCreaturePersonality CreaturePersonality;
    
    // Animation State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float MovementSpeed = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float MovementDirection = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsMoving = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsCrouching = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInCombat = false;
    
    // Environmental Awareness
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    TArray<AActor*> NearbyThreats;
    
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float CurrentFearLevel = 0.0f;
    
    // Motion Matching Databases
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching Databases")
    TObjectPtr<UPoseSearchDatabase> IdleDatabase;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching Databases")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching Databases")
    TObjectPtr<UPoseSearchDatabase> StealthDatabase;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching Databases")
    TObjectPtr<UPoseSearchDatabase> AlertDatabase;
    
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching Databases")
    TObjectPtr<UPoseSearchDatabase> FearDatabase;

private:
    // Internal state tracking
    FVector PreviousLocation = FVector::ZeroVector;
    FVector CurrentVelocity = FVector::ZeroVector;
    float DeltaTime = 0.0f;
    
    // Helper functions
    void UpdateMovementState();
    void UpdateEmotionalState();
    void UpdateEnvironmentalAwareness();
    void ApplyPersonalityVariations();
};