#include "Core_PlayerMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCore_PlayerMovementComponent::UCore_PlayerMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize movement state
    CurrentMovementState = ECore_MovementState::Walking;
    bIsRunning = false;
    bIsClimbing = false;
    CurrentStaminaDrain = 0.0f;
    EnvironmentalForce = FVector::ZeroVector;
    EnvironmentalForceDuration = 0.0f;
    
    // Set default movement values
    MaxWalkSpeed = MovementSettings.WalkSpeed;
    JumpZVelocity = MovementSettings.JumpZVelocity;
    
    // Configure movement modes
    bCanWalkOffLedges = true;
    bCanWalkOffLedgesWhenCrouching = true;
    SetCrouchedHalfHeight(65.0f);
    
    // Swimming configuration
    MaxSwimSpeed = MovementSettings.SwimSpeed;
    BuoyancyForce = 1.0f;
    
    // Air control
    AirControl = 0.2f;
    AirControlBoostMultiplier = 2.0f;
    AirControlBoostVelocityThreshold = 25.0f;
    
    // Ground friction
    GroundFriction = 8.0f;
    BrakingDecelerationWalking = 2048.0f;
    BrakingDecelerationFalling = 0.0f;
    BrakingDecelerationSwimming = 10.0f;
    BrakingDecelerationFlying = 0.0f;
}

void UCore_PlayerMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to find survival component on the character
    if (ACharacter* Character = GetCharacterOwner())
    {
        SurvivalComponent = Character->FindComponentByClass<UCore_SurvivalComponent>();
        if (!SurvivalComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_PlayerMovementComponent: No SurvivalComponent found on character"));
        }
    }
    
    // Initialize movement speed
    UpdateMovementSpeed();
}

void UCore_PlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle stamina consumption
    HandleStaminaConsumption(DeltaTime);
    
    // Handle environmental forces
    HandleEnvironmentalForces(DeltaTime);
    
    // Update movement based on current state
    UpdateMovementSpeed();
}

void UCore_PlayerMovementComponent::SetMovementState(ECore_MovementState NewState)
{
    if (CurrentMovementState == NewState)
    {
        return;
    }
    
    // Check if transition is valid
    if (!IsValidMovementTransition(CurrentMovementState, NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid movement state transition from %d to %d"), 
               (int32)CurrentMovementState, (int32)NewState);
        return;
    }
    
    ECore_MovementState PreviousState = CurrentMovementState;
    CurrentMovementState = NewState;
    
    // Handle state-specific logic
    switch (NewState)
    {
        case ECore_MovementState::Running:
            bIsRunning = true;
            break;
            
        case ECore_MovementState::Crouching:
            Crouch();
            break;
            
        case ECore_MovementState::Swimming:
            SetMovementMode(MOVE_Swimming);
            break;
            
        case ECore_MovementState::Climbing:
            bIsClimbing = true;
            SetMovementMode(MOVE_Flying); // Use flying mode for climbing
            break;
            
        case ECore_MovementState::Walking:
            bIsRunning = false;
            if (PreviousState == ECore_MovementState::Crouching)
            {
                UnCrouch();
            }
            if (PreviousState == ECore_MovementState::Climbing)
            {
                bIsClimbing = false;
                SetMovementMode(MOVE_Walking);
            }
            break;
            
        case ECore_MovementState::Falling:
            // Handled automatically by movement component
            break;
    }
    
    UpdateMovementModeForState();
    UpdateMovementSpeed();
}

void UCore_PlayerMovementComponent::StartRunning()
{
    if (CanRun())
    {
        SetMovementState(ECore_MovementState::Running);
    }
}

void UCore_PlayerMovementComponent::StopRunning()
{
    if (CurrentMovementState == ECore_MovementState::Running)
    {
        SetMovementState(ECore_MovementState::Walking);
    }
}

void UCore_PlayerMovementComponent::StartCrouching()
{
    SetMovementState(ECore_MovementState::Crouching);
}

void UCore_PlayerMovementComponent::StopCrouching()
{
    if (CurrentMovementState == ECore_MovementState::Crouching)
    {
        SetMovementState(ECore_MovementState::Walking);
    }
}

void UCore_PlayerMovementComponent::StartClimbing()
{
    if (CanClimb())
    {
        SetMovementState(ECore_MovementState::Climbing);
    }
}

void UCore_PlayerMovementComponent::StopClimbing()
{
    if (CurrentMovementState == ECore_MovementState::Climbing)
    {
        SetMovementState(ECore_MovementState::Walking);
    }
}

void UCore_PlayerMovementComponent::SetStaminaComponent(UCore_SurvivalComponent* InStaminaComponent)
{
    SurvivalComponent = InStaminaComponent;
}

bool UCore_PlayerMovementComponent::CanRun() const
{
    if (!SurvivalComponent)
    {
        return true; // Allow running if no survival component
    }
    
    // Check if we have enough stamina to run
    return SurvivalComponent->GetStamina() > 10.0f;
}

bool UCore_PlayerMovementComponent::CanClimb() const
{
    if (!SurvivalComponent)
    {
        return true; // Allow climbing if no survival component
    }
    
    // Check if we have enough stamina to climb
    return SurvivalComponent->GetStamina() > 20.0f;
}

void UCore_PlayerMovementComponent::UpdateMovementForTerrain(float TerrainSteepness, bool bIsWater, bool bIsClimbable)
{
    // Adjust movement based on terrain
    if (bIsWater && CurrentMovementState != ECore_MovementState::Swimming)
    {
        SetMovementState(ECore_MovementState::Swimming);
    }
    else if (!bIsWater && CurrentMovementState == ECore_MovementState::Swimming)
    {
        SetMovementState(ECore_MovementState::Walking);
    }
    
    // Adjust speed based on terrain steepness
    float SpeedMultiplier = 1.0f;
    if (TerrainSteepness > 0.3f) // Steep terrain
    {
        SpeedMultiplier = 0.7f;
    }
    else if (TerrainSteepness > 0.6f) // Very steep terrain
    {
        SpeedMultiplier = 0.5f;
    }
    
    // Apply speed multiplier to current movement settings
    MovementSettings.WalkSpeed = 300.0f * SpeedMultiplier;
    MovementSettings.RunSpeed = 600.0f * SpeedMultiplier;
    MovementSettings.CrouchSpeed = 150.0f * SpeedMultiplier;
    
    UpdateMovementSpeed();
}

void UCore_PlayerMovementComponent::ApplyEnvironmentalForce(FVector Force, float Duration)
{
    EnvironmentalForce = Force;
    EnvironmentalForceDuration = Duration;
}

void UCore_PlayerMovementComponent::UpdateMovementSpeed()
{
    switch (CurrentMovementState)
    {
        case ECore_MovementState::Walking:
            MaxWalkSpeed = MovementSettings.WalkSpeed;
            break;
            
        case ECore_MovementState::Running:
            MaxWalkSpeed = MovementSettings.RunSpeed;
            break;
            
        case ECore_MovementState::Crouching:
            MaxWalkSpeed = MovementSettings.CrouchSpeed;
            break;
            
        case ECore_MovementState::Swimming:
            MaxSwimSpeed = MovementSettings.SwimSpeed;
            break;
            
        case ECore_MovementState::Climbing:
            MaxFlySpeed = MovementSettings.ClimbSpeed;
            break;
            
        case ECore_MovementState::Falling:
            // Use default falling speed
            break;
    }
}

void UCore_PlayerMovementComponent::HandleStaminaConsumption(float DeltaTime)
{
    if (!SurvivalComponent)
    {
        return;
    }
    
    CurrentStaminaDrain = 0.0f;
    
    // Calculate stamina drain based on movement state
    switch (CurrentMovementState)
    {
        case ECore_MovementState::Running:
            CurrentStaminaDrain = MovementSettings.StaminaCostPerSecond;
            break;
            
        case ECore_MovementState::Climbing:
            CurrentStaminaDrain = MovementSettings.StaminaCostPerSecond * 1.5f;
            break;
            
        case ECore_MovementState::Swimming:
            CurrentStaminaDrain = MovementSettings.StaminaCostPerSecond * 0.8f;
            break;
            
        default:
            CurrentStaminaDrain = 0.0f;
            break;
    }
    
    // Apply stamina drain
    if (CurrentStaminaDrain > 0.0f)
    {
        SurvivalComponent->ModifyStamina(-CurrentStaminaDrain * DeltaTime);
        
        // Stop running/climbing if out of stamina
        if (SurvivalComponent->GetStamina() <= 0.0f)
        {
            if (CurrentMovementState == ECore_MovementState::Running)
            {
                SetMovementState(ECore_MovementState::Walking);
            }
            else if (CurrentMovementState == ECore_MovementState::Climbing)
            {
                SetMovementState(ECore_MovementState::Falling);
            }
        }
    }
}

void UCore_PlayerMovementComponent::HandleEnvironmentalForces(float DeltaTime)
{
    if (EnvironmentalForceDuration > 0.0f)
    {
        // Apply environmental force
        if (ACharacter* Character = GetCharacterOwner())
        {
            Character->LaunchCharacter(EnvironmentalForce * DeltaTime, false, false);
        }
        
        EnvironmentalForceDuration -= DeltaTime;
        
        if (EnvironmentalForceDuration <= 0.0f)
        {
            EnvironmentalForce = FVector::ZeroVector;
        }
    }
}

void UCore_PlayerMovementComponent::UpdateMovementModeForState()
{
    switch (CurrentMovementState)
    {
        case ECore_MovementState::Swimming:
            SetMovementMode(MOVE_Swimming);
            break;
            
        case ECore_MovementState::Climbing:
            SetMovementMode(MOVE_Flying);
            break;
            
        case ECore_MovementState::Falling:
            SetMovementMode(MOVE_Falling);
            break;
            
        default:
            SetMovementMode(MOVE_Walking);
            break;
    }
}

bool UCore_PlayerMovementComponent::IsValidMovementTransition(ECore_MovementState FromState, ECore_MovementState ToState) const
{
    // Define valid state transitions
    switch (FromState)
    {
        case ECore_MovementState::Walking:
            return true; // Can transition to any state from walking
            
        case ECore_MovementState::Running:
            return ToState != ECore_MovementState::Climbing; // Can't climb while running
            
        case ECore_MovementState::Crouching:
            return ToState != ECore_MovementState::Running && ToState != ECore_MovementState::Climbing;
            
        case ECore_MovementState::Swimming:
            return ToState != ECore_MovementState::Climbing; // Can't climb while swimming
            
        case ECore_MovementState::Climbing:
            return true; // Can transition to any state from climbing
            
        case ECore_MovementState::Falling:
            return ToState != ECore_MovementState::Climbing; // Can't climb while falling
    }
    
    return false;
}