#include "SurvivalAnimationBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

USurvivalAnimationBlueprint::USurvivalAnimationBlueprint()
{
    // Initialize animation state variables
    bIsMoving = false;
    bIsRunning = false;
    bIsCrouching = false;
    bIsJumping = false;
    bIsFearing = false;
    bIsGathering = false;
    bIsCrafting = false;
    bIsHiding = false;
    bIsClimbing = false;

    // Initialize movement parameters
    Speed = 0.0f;
    Direction = 0.0f;
    FearLevel = 0.0f;

    // Initialize animation assets to nullptr
    IdleAnimation = nullptr;
    WalkAnimation = nullptr;
    RunAnimation = nullptr;
    JumpAnimation = nullptr;
    CrouchAnimation = nullptr;
    FearAnimation = nullptr;

    LocomotionBlendSpace = nullptr;
    CrouchBlendSpace = nullptr;

    GatheringMontage = nullptr;
    CraftingMontage = nullptr;
    ClimbingMontage = nullptr;

    // Initialize timing and transition parameters
    LastUpdateTime = 0.0f;
    StateChangeThreshold = 0.1f;
    BlendInTime = 0.2f;
    BlendOutTime = 0.2f;
    FearTransitionSpeed = 2.0f;
    MovementTransitionSpeed = 1.5f;
}

void USurvivalAnimationBlueprint::UpdateMovementState(float NewSpeed, float NewDirection)
{
    // Update movement parameters
    Speed = NewSpeed;
    Direction = NewDirection;

    // Determine movement state based on speed
    bIsMoving = Speed > 1.0f;
    bIsRunning = Speed > 300.0f; // Running threshold

    // Log state changes for debugging
    if (GEngine)
    {
        FString StateInfo = FString::Printf(
            TEXT("Movement State - Speed: %.1f, Direction: %.1f, Moving: %s, Running: %s"),
            Speed, Direction,
            bIsMoving ? TEXT("Yes") : TEXT("No"),
            bIsRunning ? TEXT("Yes") : TEXT("No")
        );
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, StateInfo);
    }
}

void USurvivalAnimationBlueprint::SetFearState(bool bNewFearing, float NewFearLevel)
{
    bIsFearing = bNewFearing;
    FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);

    // Fear overrides other states
    if (bIsFearing)
    {
        // Stop survival actions when afraid
        StopAllSurvivalActions();
        
        // Increase movement speed when afraid
        if (bIsMoving && FearLevel > 0.5f)
        {
            bIsRunning = true;
        }
    }

    // Log fear state changes
    if (GEngine)
    {
        FString FearInfo = FString::Printf(
            TEXT("Fear State - Fearing: %s, Level: %.2f"),
            bIsFearing ? TEXT("Yes") : TEXT("No"),
            FearLevel
        );
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FearInfo);
    }
}

void USurvivalAnimationBlueprint::PlaySurvivalAction(const FString& ActionName)
{
    // Reset all survival action states
    StopAllSurvivalActions();

    // Set the appropriate action state
    if (ActionName == TEXT("Gathering"))
    {
        bIsGathering = true;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Started Gathering Animation"));
        }
    }
    else if (ActionName == TEXT("Crafting"))
    {
        bIsCrafting = true;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Started Crafting Animation"));
        }
    }
    else if (ActionName == TEXT("Hiding"))
    {
        bIsHiding = true;
        bIsCrouching = true; // Hiding implies crouching
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Started Hiding Animation"));
        }
    }
    else if (ActionName == TEXT("Climbing"))
    {
        bIsClimbing = true;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Started Climbing Animation"));
        }
    }
}

void USurvivalAnimationBlueprint::StopAllSurvivalActions()
{
    bIsGathering = false;
    bIsCrafting = false;
    bIsHiding = false;
    bIsClimbing = false;

    // Only reset crouching if not hiding
    if (!bIsHiding)
    {
        bIsCrouching = false;
    }
}

bool USurvivalAnimationBlueprint::ShouldEnterFearState() const
{
    // Enter fear state if fear level is above threshold
    return FearLevel > 0.3f;
}

bool USurvivalAnimationBlueprint::ShouldEnterRunState() const
{
    // Enter run state if moving fast or afraid
    return (Speed > 300.0f) || (bIsFearing && bIsMoving);
}

float USurvivalAnimationBlueprint::GetBlendSpaceX() const
{
    // X axis represents speed (0 = idle, 1 = walk, 2 = run)
    if (!bIsMoving)
    {
        return 0.0f; // Idle
    }
    else if (bIsRunning || (bIsFearing && bIsMoving))
    {
        return 2.0f; // Run
    }
    else
    {
        return 1.0f; // Walk
    }
}

float USurvivalAnimationBlueprint::GetBlendSpaceY() const
{
    // Y axis represents direction (-1 = left, 0 = forward, 1 = right)
    // Normalize direction to blend space range
    return FMath::Clamp(Direction / 180.0f, -1.0f, 1.0f);
}