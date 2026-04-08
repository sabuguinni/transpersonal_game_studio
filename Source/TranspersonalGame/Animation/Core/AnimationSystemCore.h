#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Implements Motion Matching with adaptive IK for prehistoric survival gameplay
 * 
 * Philosophy: Every movement tells a story of survival, fear, and adaptation
 * Each character has unique body language that defines them before any dialogue
 */

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Idle,
    Cautious,           // Alert but not moving - scanning for threats
    Stalking,           // Moving carefully to avoid detection
    Fleeing,            // Full panic run from predator
    Exhausted,          // Post-flight recovery
    Injured,            // Limping, favoring limbs
    Domesticating,      // Gentle approach to small dinosaurs
    Crafting,           // Tool creation and base building
    Climbing,           // Traversal animations
    Swimming            // Water movement
};

UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    Flat,
    Uneven,
    Steep,
    Muddy,
    Rocky,
    Vegetation,
    Water,
    Ice
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe,               // No predators detected
    Cautious,           // Herbivores nearby
    Dangerous,          // Small predators in area
    Lethal              // Large predators detected
};

USTRUCT(BlueprintType)
struct FCharacterMotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    FVector Acceleration;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    ECharacterState CurrentState;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    ETerrainType TerrainType;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    EThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    float StaminaLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    bool bIsHiding;

    UPROPERTY(BlueprintReadWrite, Category = "Motion")
    bool bIsCrouched;

    FCharacterMotionData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        CurrentState = ECharacterState::Idle;
        TerrainType = ETerrainType::Flat;
        ThreatLevel = EThreatLevel::Safe;
        StaminaLevel = 100.0f;
        FearLevel = 0.0f;
        bIsHiding = false;
        bIsCrouched = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UAnimationSystemCore : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Motion Matching Database Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> CautiousMovementDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> FleeingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    // Current Motion Data
    UPROPERTY(BlueprintReadOnly, Category = "Character Motion")
    FCharacterMotionData MotionData;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float HandIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float SpineAdjustmentAlpha;

    // Fear Response System
    UPROPERTY(BlueprintReadOnly, Category = "Fear Response")
    float HeartRate;

    UPROPERTY(BlueprintReadOnly, Category = "Fear Response")
    float BreathingRate;

    UPROPERTY(BlueprintReadOnly, Category = "Fear Response")
    float TensionLevel;

private:
    // Internal motion analysis
    void UpdateMotionData(float DeltaTime);
    void AnalyzeTerrain();
    void DetectThreats();
    void CalculateFearResponse();
    
    // Database selection logic
    UPoseSearchDatabase* SelectOptimalDatabase() const;
    
    // Character reference
    UPROPERTY()
    TObjectPtr<ACharacter> OwningCharacter;

    // Cached components
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    // Motion history for trajectory prediction
    TArray<FVector> VelocityHistory;
    TArray<FVector> PositionHistory;
    
    static constexpr int32 MaxHistoryFrames = 60; // 1 second at 60fps
};