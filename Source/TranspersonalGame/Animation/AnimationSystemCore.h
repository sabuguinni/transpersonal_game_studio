#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearch.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, IK, and character-specific animation behaviors
 */

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Cautious,           // Slow, careful movement
    Normal,             // Standard walking
    Panicked,           // Fast, erratic movement when threatened
    Crouched,           // Stealth movement
    Injured,            // Limping, wounded movement
    Exhausted,          // Tired, heavy breathing
    Observing,          // Standing still, looking around
    Hiding              // Pressed against surfaces, minimal movement
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,                // Distant sounds, movement
    Medium,             // Visible predator at distance
    High,               // Close predator, immediate danger
    Extreme             // Active chase, combat
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
    Water
};

USTRUCT(BlueprintType)
struct FCharacterAnimationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    ECharacterMovementState MovementState = ECharacterMovementState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Environment")
    ETerrainType CurrentTerrain = ETerrainType::Flat;

    UPROPERTY(BlueprintReadWrite, Category = "Physical State")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Physical State")
    float HealthLevel = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Emotional State")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Emotional State")
    float ConfidenceLevel = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    FVector MovementDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 0.0f;
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

    // Core Animation Data
    UPROPERTY(BlueprintReadWrite, Category = "Animation Data")
    FCharacterAnimationData AnimationData;

    // Motion Matching Database References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* IdleDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* PanicDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* StealthDatabase;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float PelvisAdjustmentInterpSpeed = 10.0f;

    // Animation Blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float StateTransitionSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    float ThreatResponseSpeed = 8.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateMovementState(ECharacterMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateTerrainType(ETerrainType NewTerrain);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    class UPoseSearchDatabase* GetCurrentDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float GetBlendWeight(ECharacterMovementState State) const;

private:
    // Internal state tracking
    ECharacterMovementState PreviousMovementState;
    EThreatLevel PreviousThreatLevel;
    
    float StateTransitionTimer = 0.0f;
    float ThreatTransitionTimer = 0.0f;

    void UpdateAnimationBlends(float DeltaTime);
    void ProcessThreatResponse(float DeltaTime);
    void UpdatePhysicalState(float DeltaTime);
};