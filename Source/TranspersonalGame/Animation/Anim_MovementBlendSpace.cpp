#include "Anim_MovementBlendSpace.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace1D.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MovementBlendSpace::UAnim_MovementBlendSpace()
{
    // Set default movement thresholds
    IdleThreshold = 5.0f;
    WalkThreshold = 150.0f;
    RunThreshold = 400.0f;
    SprintThreshold = 600.0f;
    
    // Initialize state
    CurrentSpeed = 0.0f;
    BlendSpaceInput = 0.0f;
    bIsMoving = false;
    bIsRunning = false;
    bIsSprinting = false;
    
    PreviousSpeed = 0.0f;
    SpeedChangeRate = 0.0f;
    BlendSpaceSmoothingSpeed = 5.0f;
    
    // Initialize animation references to null
    IdleAnimation = nullptr;
    WalkAnimation = nullptr;
    RunAnimation = nullptr;
    SprintAnimation = nullptr;
    MovementBlendSpace = nullptr;
}

void UAnim_MovementBlendSpace::UpdateMovementState(float Velocity, bool bIsInAir)
{
    // Store previous speed for change rate calculation
    PreviousSpeed = CurrentSpeed;
    CurrentSpeed = FMath::Abs(Velocity);
    
    // Calculate speed change rate
    SpeedChangeRate = CurrentSpeed - PreviousSpeed;
    
    // Update movement flags
    bIsMoving = CurrentSpeed > IdleThreshold;
    bIsRunning = CurrentSpeed > RunThreshold;
    bIsSprinting = CurrentSpeed > SprintThreshold;
    
    // Calculate blend space input
    BlendSpaceInput = CalculateBlendSpaceInput(CurrentSpeed);
    
    // Log state changes for debugging
    if (GEngine)
    {
        FString StateString = bIsSprinting ? TEXT("Sprinting") :
                             bIsRunning ? TEXT("Running") :
                             bIsMoving ? TEXT("Walking") : TEXT("Idle");
        
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green,
            FString::Printf(TEXT("Movement State: %s (Speed: %.1f, Blend: %.2f)"),
                *StateString, CurrentSpeed, BlendSpaceInput));
    }
}

float UAnim_MovementBlendSpace::CalculateBlendSpaceInput(float Speed)
{
    // Normalize speed to 0-1 range for blend space
    if (Speed <= IdleThreshold)
    {
        return 0.0f; // Idle
    }
    else if (Speed <= WalkThreshold)
    {
        // Walk range: 0.0 to 0.33
        return FMath::GetMappedRangeValueClamped(
            FVector2D(IdleThreshold, WalkThreshold),
            FVector2D(0.0f, 0.33f),
            Speed
        );
    }
    else if (Speed <= RunThreshold)
    {
        // Run range: 0.33 to 0.66
        return FMath::GetMappedRangeValueClamped(
            FVector2D(WalkThreshold, RunThreshold),
            FVector2D(0.33f, 0.66f),
            Speed
        );
    }
    else
    {
        // Sprint range: 0.66 to 1.0
        return FMath::GetMappedRangeValueClamped(
            FVector2D(RunThreshold, SprintThreshold),
            FVector2D(0.66f, 1.0f),
            Speed
        );
    }
}

void UAnim_MovementBlendSpace::SetMovementThresholds(float NewWalkThreshold, float NewRunThreshold, float NewSprintThreshold)
{
    // Validate thresholds are in ascending order
    if (NewWalkThreshold > 0.0f && NewRunThreshold > NewWalkThreshold && NewSprintThreshold > NewRunThreshold)
    {
        WalkThreshold = NewWalkThreshold;
        RunThreshold = NewRunThreshold;
        SprintThreshold = NewSprintThreshold;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                FString::Printf(TEXT("Movement thresholds updated: Walk=%.1f, Run=%.1f, Sprint=%.1f"),
                    WalkThreshold, RunThreshold, SprintThreshold));
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                TEXT("Invalid movement thresholds! Must be in ascending order."));
        }
    }
}

bool UAnim_MovementBlendSpace::IsInMovementState(const FString& StateName) const
{
    if (StateName.Equals(TEXT("Idle"), ESearchCase::IgnoreCase))
    {
        return !bIsMoving;
    }
    else if (StateName.Equals(TEXT("Walking"), ESearchCase::IgnoreCase))
    {
        return bIsMoving && !bIsRunning;
    }
    else if (StateName.Equals(TEXT("Running"), ESearchCase::IgnoreCase))
    {
        return bIsRunning && !bIsSprinting;
    }
    else if (StateName.Equals(TEXT("Sprinting"), ESearchCase::IgnoreCase))
    {
        return bIsSprinting;
    }
    
    return false;
}

void UAnim_MovementBlendSpace::SmoothBlendSpaceTransition(float DeltaTime)
{
    // Smooth the blend space input for more natural transitions
    float TargetBlendInput = CalculateBlendSpaceInput(CurrentSpeed);
    
    BlendSpaceInput = FMath::FInterpTo(
        BlendSpaceInput,
        TargetBlendInput,
        DeltaTime,
        BlendSpaceSmoothingSpeed
    );
}