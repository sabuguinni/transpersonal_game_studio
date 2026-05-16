#include "Anim_MovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MovementController::UAnim_MovementController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    MovingThreshold = 10.0f;
    GroundTraceDistance = 200.0f;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
}

void UAnim_MovementController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponents();
}

void UAnim_MovementController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementValues();
        UpdateMovementState();
        UpdateLocomotionMode();
    }
}

void UAnim_MovementController::CacheComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
    }
}

void UAnim_MovementController::UpdateMovementValues()
{
    if (!MovementComponent)
        return;
        
    // Get current velocity
    MovementData.Velocity = MovementComponent->Velocity;
    
    // Calculate 2D speed (horizontal movement)
    FVector HorizontalVelocity = FVector(MovementData.Velocity.X, MovementData.Velocity.Y, 0.0f);
    MovementData.Speed = HorizontalVelocity.Size();
    
    // Check if moving
    MovementData.bIsMoving = MovementData.Speed > MovingThreshold;
    
    // Calculate direction
    MovementData.Direction = CalculateDirection();
    
    // Check air state
    MovementData.bIsInAir = IsCharacterInAir();
    
    // Check crouch state
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate ground distance
    MovementData.GroundDistance = GetGroundDistance();
}

void UAnim_MovementController::UpdateMovementState()
{
    MovementData.CurrentState = DetermineMovementState();
}

void UAnim_MovementController::UpdateLocomotionMode()
{
    if (MovementData.bIsInAir)
    {
        MovementData.LocomotionMode = EAnim_LocomotionMode::Air;
    }
    else if (MovementComponent && MovementComponent->IsSwimming())
    {
        MovementData.LocomotionMode = EAnim_LocomotionMode::Water;
    }
    else
    {
        MovementData.LocomotionMode = EAnim_LocomotionMode::Ground;
    }
}

FAnim_MovementData UAnim_MovementController::CalculateMovementData()
{
    if (MovementComponent)
    {
        UpdateMovementValues();
        UpdateMovementState();
        UpdateLocomotionMode();
    }
    
    return MovementData;
}

EAnim_MovementState UAnim_MovementController::DetermineMovementState()
{
    if (!MovementComponent)
        return EAnim_MovementState::Idle;
    
    // Check air states first
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0.0f)
            return EAnim_MovementState::Jumping;
        else
            return EAnim_MovementState::Falling;
    }
    
    // Check water state
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check crouch state
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Ground movement states
    if (!MovementData.bIsMoving)
    {
        return EAnim_MovementState::Idle;
    }
    
    if (MovementData.Speed <= WalkSpeedThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else if (MovementData.Speed > RunSpeedThreshold)
    {
        return EAnim_MovementState::Running;
    }
    else
    {
        return EAnim_MovementState::Walking;
    }
}

float UAnim_MovementController::CalculateDirection()
{
    if (!OwnerCharacter || !MovementData.bIsMoving)
        return 0.0f;
    
    // Get character forward vector
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Get movement direction (normalized velocity)
    FVector MovementDirection = MovementData.Velocity.GetSafeNormal2D();
    
    // Calculate angle between forward and movement direction
    float DotProduct = FVector::DotProduct(ForwardVector, MovementDirection);
    float CrossProduct = FVector::CrossProduct(ForwardVector, MovementDirection).Z;
    
    // Convert to angle in degrees
    float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    
    return Angle;
}

bool UAnim_MovementController::IsCharacterInAir()
{
    if (!MovementComponent)
        return false;
    
    return MovementComponent->IsFalling();
}

float UAnim_MovementController::GetGroundDistance()
{
    if (!OwnerCharacter)
        return 0.0f;
    
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, GroundTraceDistance);
    
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
        return FVector::Dist(StartLocation, HitResult.Location);
    }
    
    return GroundTraceDistance;
}

void UAnim_MovementController::TriggerJumpAnimation()
{
    // This can be used to trigger specific jump montages or notify the animation blueprint
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Jump animation triggered for %s"), *OwnerCharacter->GetName());
    }
}

void UAnim_MovementController::TriggerLandAnimation()
{
    // This can be used to trigger specific landing montages or notify the animation blueprint
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Land animation triggered for %s"), *OwnerCharacter->GetName());
    }
}

void UAnim_MovementController::TriggerCrouchAnimation(bool bCrouch)
{
    // This can be used to trigger specific crouch transitions
    if (OwnerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("Crouch animation triggered for %s: %s"), 
               *OwnerCharacter->GetName(), 
               bCrouch ? TEXT("Enter") : TEXT("Exit"));
    }
}

float UAnim_MovementController::CalculateSpeedRatio()
{
    if (!MovementComponent)
        return 0.0f;
    
    float MaxSpeed = MovementComponent->GetMaxSpeed();
    if (MaxSpeed > 0.0f)
    {
        return MovementData.Speed / MaxSpeed;
    }
    
    return 0.0f;
}