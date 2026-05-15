#include "Anim_StateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_StateManager::UAnim_StateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize state data
    CurrentState = FAnim_StateData();
    PreviousState = FAnim_StateData();
    
    // Set default thresholds
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    GroundTraceDistance = 120.0f;
    StateTransitionDelay = 0.1f;
    LastStateChangeTime = 0.0f;
}

void UAnim_StateManager::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    // Initialize state
    UpdateMovementState();
}

void UAnim_StateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Store previous state
    PreviousState = CurrentState;
    
    // Update movement data
    FVector Velocity = MovementComponent->Velocity;
    CurrentState.Speed = Velocity.Size();
    CurrentState.Direction = FMath::Atan2(Velocity.Y, Velocity.X);
    CurrentState.bIsInAir = MovementComponent->IsFalling();
    CurrentState.bIsCrouched = OwnerCharacter->bIsCrouched;
    
    // Update ground distance
    UpdateGroundDistance();
    
    // Update movement state
    UpdateMovementState();
}

void UAnim_StateManager::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_StateManager::UpdateGroundDistance()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, GroundTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        CurrentState.GroundDistance = HitResult.Distance;
    }
    else
    {
        CurrentState.GroundDistance = GroundTraceDistance;
    }
}

void UAnim_StateManager::UpdateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewState = CalculateMovementState();
    
    // Check if we can transition to the new state
    if (CanTransitionTo(NewState))
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // Apply transition delay to prevent rapid state changes
        if (CurrentTime - LastStateChangeTime >= StateTransitionDelay)
        {
            CurrentState.MovementState = NewState;
            LastStateChangeTime = CurrentTime;
        }
    }
}

EAnim_MovementState UAnim_StateManager::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for air states first
    if (CurrentState.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check for crouching
    if (CurrentState.bIsCrouched)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check for swimming
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Ground movement states based on speed
    if (CurrentState.Speed < 10.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (CurrentState.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

bool UAnim_StateManager::CanTransitionTo(EAnim_MovementState NewState) const
{
    return IsValidStateTransition(CurrentState.MovementState, NewState);
}

bool UAnim_StateManager::IsValidStateTransition(EAnim_MovementState From, EAnim_MovementState To) const
{
    // Allow all transitions for now - can be refined later
    // Some examples of restricted transitions:
    // - Can't go from Jumping directly to Running (must land first)
    // - Can't go from Swimming to Walking without exiting water
    
    if (From == To)
    {
        return false; // No need to transition to same state
    }
    
    // Jumping can only transition to Falling or Landing (Idle/Walking)
    if (From == EAnim_MovementState::Jumping && To == EAnim_MovementState::Running)
    {
        return false;
    }
    
    return true;
}

void UAnim_StateManager::SetActionState(EAnim_ActionState NewActionState)
{
    if (CurrentState.ActionState != NewActionState)
    {
        CurrentState.ActionState = NewActionState;
    }
}

void UAnim_StateManager::ForceMovementState(EAnim_MovementState NewState)
{
    CurrentState.MovementState = NewState;
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
}

void UAnim_StateManager::OnJumpStarted()
{
    ForceMovementState(EAnim_MovementState::Jumping);
}

void UAnim_StateManager::OnLanded()
{
    // Transition to appropriate ground state based on speed
    UpdateMovementState();
}

void UAnim_StateManager::OnCrouchStarted()
{
    ForceMovementState(EAnim_MovementState::Crouching);
}

void UAnim_StateManager::OnCrouchEnded()
{
    // Return to appropriate movement state
    UpdateMovementState();
}