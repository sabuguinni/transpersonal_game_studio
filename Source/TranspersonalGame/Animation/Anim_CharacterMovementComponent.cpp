#include "Anim_CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_CharacterMovementComponent::UAnim_CharacterMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Set reasonable defaults for prehistoric survival movement
    MaxWalkSpeed = 400.0f;
    MaxWalkSpeedCrouched = 150.0f;
    JumpZVelocity = 500.0f;
    AirControl = 0.2f;
    GroundFriction = 8.0f;
    BrakingDecelerationWalking = 2048.0f;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    PreviousMovementState = EAnim_MovementState::Idle;
}

void UAnim_CharacterMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize movement data
    UpdateMovementData(0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Anim_CharacterMovementComponent initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UAnim_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update animation-specific movement data
    UpdateMovementData(DeltaTime);
}

void UAnim_CharacterMovementComponent::UpdateMovementData(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }

    // Update basic movement properties
    MovementData.Velocity = Velocity;
    MovementData.Speed = Velocity.Size2D();
    MovementData.bIsInAir = IsFalling();
    MovementData.bIsCrouching = IsCrouching();
    
    // Calculate movement direction relative to actor forward
    if (MovementData.Speed > IdleThreshold)
    {
        FVector ForwardVector = GetOwner()->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        // Calculate angle between forward and velocity
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        MovementData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update ground distance for foot IK
    UpdateGroundDistance();
    
    // Update movement state
    EAnim_MovementState NewState = CalculateMovementState();
    if (NewState != MovementData.MovementState)
    {
        EAnim_MovementState OldState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        
        // Trigger Blueprint event
        OnMovementStateChanged(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)OldState, (int32)NewState);
    }
}

void UAnim_CharacterMovementComponent::UpdateGroundDistance()
{
    if (!GetOwner())
    {
        MovementData.GroundDistance = 0.0f;
        return;
    }

    // Trace downward from character base to find ground distance
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, GroundTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        MovementData.GroundDistance = HitResult.Distance;
    }
    else
    {
        MovementData.GroundDistance = GroundTraceDistance;
    }
}

EAnim_MovementState UAnim_CharacterMovementComponent::CalculateMovementState() const
{
    // Priority order: Air states first, then ground states
    if (MovementData.bIsInAir)
    {
        if (Velocity.Z > 0.0f)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Swimming state
    if (IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Crouching state
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Ground movement states based on speed
    if (MovementData.Speed <= IdleThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed <= WalkToRunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
    
    // Log movement mode changes for debugging
    UE_LOG(LogTemp, Log, TEXT("Movement mode changed from %d to %d"), 
           (int32)PreviousMovementMode, (int32)MovementMode);
}

void UAnim_CharacterMovementComponent::OnLanded(const FHitResult& Hit)
{
    Super::OnLanded(Hit);
    
    // Trigger Blueprint event for animation
    OnLanded(Hit);
    
    UE_LOG(LogTemp, Log, TEXT("Character landed on %s"), 
           Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("Unknown"));
}