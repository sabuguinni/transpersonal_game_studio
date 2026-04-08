#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Jurassic Game
 * Handles Motion Matching, IK, and procedural animation systems
 * 
 * Design Philosophy:
 * - Every movement tells a story
 * - Weight and intention in every gesture  
 * - Unique personality per creature
 * - Constant sense of vulnerability for player
 */

UENUM(BlueprintType)
enum class ECreatureAnimationState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Locomotion UMETA(DisplayName = "Locomotion"), 
    Hunting UMETA(DisplayName = "Hunting"),
    Feeding UMETA(DisplayName = "Feeding"),
    Resting UMETA(DisplayName = "Resting"),
    Alert UMETA(DisplayName = "Alert"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Domesticated UMETA(DisplayName = "Domesticated"),
    Injured UMETA(DisplayName = "Injured")
};

UENUM(BlueprintType)
enum class EPlayerAnimationState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Crouching UMETA(DisplayName = "Crouching"),
    Hiding UMETA(DisplayName = "Hiding"),
    Climbing UMETA(DisplayName = "Climbing"),
    Crafting UMETA(DisplayName = "Crafting"),
    Gathering UMETA(DisplayName = "Gathering"),
    Fearful UMETA(DisplayName = "Fearful"),
    Exhausted UMETA(DisplayName = "Exhausted")
};

USTRUCT(BlueprintType)
struct FCreaturePersonality
{
    GENERATED_BODY()

    // Physical variation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float SizeVariation = 1.0f; // 0.8 to 1.2 scale multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float PostureVariation = 0.0f; // -0.2 to 0.2 spine curvature

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float LimpSeverity = 0.0f; // 0.0 to 1.0 limp intensity

    // Behavioral parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggressiveness = 0.5f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Nervousness = 0.5f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Curiosity = 0.5f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DomesticationLevel = 0.0f; // 0.0 to 1.0

    // Animation timing modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float MovementSpeedMultiplier = 1.0f; // 0.7 to 1.3

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float ReactionTime = 0.5f; // 0.2 to 1.0 seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float IdleVariationFrequency = 1.0f; // 0.5 to 2.0
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Motion Matching Database Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeMotionMatchingDatabases();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetActiveDatabase(const FString& DatabaseName);

    // Creature Animation Management
    UFUNCTION(BlueprintCallable, Category = "Creature Animation")
    void ApplyCreaturePersonality(USkeletalMeshComponent* SkeletalMesh, const FCreaturePersonality& Personality);

    UFUNCTION(BlueprintCallable, Category = "Creature Animation")
    void UpdateCreatureAnimationState(USkeletalMeshComponent* SkeletalMesh, ECreatureAnimationState NewState);

    // Player Animation Management
    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void UpdatePlayerAnimationState(USkeletalMeshComponent* PlayerMesh, EPlayerAnimationState NewState);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void ApplyFearResponse(USkeletalMeshComponent* PlayerMesh, float FearIntensity);

    // IK System Management
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableFootIK(USkeletalMeshComponent* SkeletalMesh, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateTerrainAdaptation(USkeletalMeshComponent* SkeletalMesh, const FVector& GroundNormal);

protected:
    // Motion Matching Databases
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    TMap<FString, class UPoseSearchDatabase*> MotionMatchingDatabases;

    // Animation Blueprint Classes
    UPROPERTY(EditDefaultsOnly, Category = "Animation Blueprints")
    TSubclassOf<class UAnimInstance> PlayerAnimBPClass;

    UPROPERTY(EditDefaultsOnly, Category = "Animation Blueprints")
    TSubclassOf<class UAnimInstance> CreatureAnimBPClass;

    // IK Rig Assets
    UPROPERTY(EditDefaultsOnly, Category = "IK Rigs")
    class UIKRigDefinition* PlayerIKRig;

    UPROPERTY(EditDefaultsOnly, Category = "IK Rigs")
    TMap<FString, class UIKRigDefinition*> CreatureIKRigs;

private:
    void InitializeDefaultPersonalities();
    void SetupMotionMatchingSchemas();
    void ConfigureIKSolvers();
};