// AnimationBlendSpaceConfig.h
// Animation Agent #10 — PROD_CYCLE_AUTO_20260628_007
// Blend space configuration structs for player and dinosaur locomotion
// These drive the UE5 BlendSpace assets created via Python

#pragma once

#include "CoreMinimal.h"
#include "AnimationBlendSpaceConfig.generated.h"

// ─── Player Locomotion Blend Space ───────────────────────────────────────────
// 2D Blend Space: X = Direction (-180 to 180), Y = Speed (0 to 600)
// Axes:
//   X-axis: Direction (strafe angle)
//   Y-axis: Speed (cm/s)
//
// Sample points:
//   (0,   0)   → Idle
//   (0,  150)  → Walk_Fwd
//   (90, 150)  → Walk_Right
//   (-90,150)  → Walk_Left
//   (180,150)  → Walk_Bwd
//   (0,  350)  → Run_Fwd
//   (90, 350)  → Run_Right
//   (-90,350)  → Run_Left
//   (0,  550)  → Sprint_Fwd

USTRUCT(BlueprintType)
struct FAnim_BlendSpaceSample
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpace")
    float DirectionAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpace")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpace")
    FName AnimationSlotName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|BlendSpace")
    float PlayRate = 1.0f;

    FAnim_BlendSpaceSample() {}
    FAnim_BlendSpaceSample(float Dir, float Spd, FName Slot, float Rate = 1.0f)
        : DirectionAngle(Dir), Speed(Spd), AnimationSlotName(Slot), PlayRate(Rate) {}
};

// ─── Dinosaur Locomotion Blend Space ─────────────────────────────────────────
// 1D Blend Space: X = Speed (0 to MaxSpeed per species)
// Samples: Idle → Walk → Trot → Run → Sprint

USTRUCT(BlueprintType)
struct FAnim_DinoBlendSpaceSample
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|DinoBlendSpace")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|DinoBlendSpace")
    FName AnimationSlotName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|DinoBlendSpace")
    float PlayRate = 1.0f;

    FAnim_DinoBlendSpaceSample() {}
    FAnim_DinoBlendSpaceSample(float Spd, FName Slot, float Rate = 1.0f)
        : Speed(Spd), AnimationSlotName(Slot), PlayRate(Rate) {}
};

// ─── Blend Space Config Asset (data-driven) ───────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_BlendSpaceConfig : public UObject
{
    GENERATED_BODY()

public:

    UAnim_BlendSpaceConfig();

    // Player locomotion samples (2D blend space)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Player")
    TArray<FAnim_BlendSpaceSample> PlayerLocomotionSamples;

    // Dinosaur locomotion samples (1D blend space, per species)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Dinosaur")
    TArray<FAnim_DinoBlendSpaceSample> DinoLocomotionSamples;

    // Speed thresholds for state transitions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Thresholds")
    float WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Thresholds")
    float RunSpeedThreshold = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Thresholds")
    float SprintSpeedThreshold = 400.0f;

    // Blend time between states (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Blending")
    float IdleToWalkBlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Blending")
    float WalkToRunBlendTime = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Blending")
    float RunToSprintBlendTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Blending")
    float AirToGroundBlendTime = 0.25f;

    // Foot IK settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
    bool bEnableFootIKByDefault = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
    float FootIKTraceDistance = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
    float FootIKInterpSpeed = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|FootIK")
    float MaxFootIKOffset = 20.0f;

    // Lean settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Lean")
    float MaxLeanAngle = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Lean")
    float LeanInterpSpeed = 6.0f;

    // Populate default player samples
    UFUNCTION(BlueprintCallable, Category = "Animation|Config")
    void PopulateDefaultPlayerSamples();

    // Populate default dinosaur samples
    UFUNCTION(BlueprintCallable, Category = "Animation|Config")
    void PopulateDefaultDinoSamples();
};
