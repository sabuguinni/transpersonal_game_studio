#include "PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    PreviousMovementState = EAnim_MovementState::Idle;
    
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    AnimationBlendSpeed = 5.0f;
    DirectionBlendSpeed = 8.0f;
    
    LastGroundedTime = 0.0f;
    bWasInAir = false;
    LastVelocity = FVector::ZeroVector;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    // Initialize blend parameters
    BlendParameters = FAnim_BlendParameters();
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: BeginPlay completed"));
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!MovementComponent || !SkeletalMeshComponent)
    {
        return;
    }
    
    UpdateAnimationState(DeltaTime);
}

void UPrimitiveAnimationController::InitializeComponents()
{
    // Get the character owner
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Owner is not a Character"));
        return;
    }
    
    // Get movement component
    MovementComponent = Character->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No CharacterMovementComponent found"));
        return;
    }
    
    // Get skeletal mesh component
    SkeletalMeshComponent = Character->GetMesh();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: No SkeletalMeshComponent found"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Components initialized successfully"));
}

void UPrimitiveAnimationController::UpdateAnimationState(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Store previous state
    PreviousMovementState = CurrentMovementState;
    
    // Calculate new movement state
    CalculateMovementState();
    
    // Update blend parameters
    UpdateBlendParameters(DeltaTime);
    
    // Smooth blend values
    SmoothBlendValues(DeltaTime);
    
    // Track air time
    if (MovementComponent->IsFalling())
    {
        if (!bWasInAir)
        {
            bWasInAir = true;
            UE_LOG(LogTemp, Log, TEXT("Character entered air state"));
        }
    }
    else
    {
        if (bWasInAir)
        {
            bWasInAir = false;
            LastGroundedTime = GetWorld()->GetTimeSeconds();
            TriggerLandAnimation();
        }
    }
    
    // Store velocity for next frame
    LastVelocity = MovementComponent->Velocity;
}

void UPrimitiveAnimationController::CalculateMovementState()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Check if in air
    if (MovementComponent->IsFalling())
    {
        if (MovementComponent->Velocity.Z > 0)
        {
            CurrentMovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            CurrentMovementState = EAnim_MovementState::Falling;
        }
        return;
    }
    
    // Check if crouching
    if (MovementComponent->IsCrouching())
    {
        CurrentMovementState = EAnim_MovementState::Crouching;
        return;
    }
    
    // Calculate horizontal speed
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    float Speed = HorizontalVelocity.Size();
    
    // Determine movement state based on speed
    if (Speed < WalkSpeedThreshold)
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
    else if (Speed < RunSpeedThreshold)
    {
        CurrentMovementState = EAnim_MovementState::Walking;
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Running;
    }
}

void UPrimitiveAnimationController::UpdateBlendParameters(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate speed
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    float CurrentSpeed = HorizontalVelocity.Size();
    
    // Update speed parameter
    BlendParameters.Speed = CurrentSpeed;
    
    // Update direction parameter
    BlendParameters.Direction = CalculateMovementDirection();
    
    // Update air state
    BlendParameters.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouch state
    BlendParameters.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate lean amount based on velocity change
    FVector VelocityDelta = MovementComponent->Velocity - LastVelocity;
    VelocityDelta.Z = 0.0f;
    BlendParameters.LeanAmount = FMath::Clamp(VelocityDelta.Size() / 1000.0f, -1.0f, 1.0f);
}

void UPrimitiveAnimationController::SmoothBlendValues(float DeltaTime)
{
    // Smooth speed transitions
    float TargetSpeed = BlendParameters.Speed;
    BlendParameters.Speed = FMath::FInterpTo(BlendParameters.Speed, TargetSpeed, DeltaTime, AnimationBlendSpeed);
    
    // Smooth direction transitions
    float TargetDirection = BlendParameters.Direction;
    BlendParameters.Direction = FMath::FInterpTo(BlendParameters.Direction, TargetDirection, DeltaTime, DirectionBlendSpeed);
    
    // Smooth lean amount
    BlendParameters.LeanAmount = FMath::FInterpTo(BlendParameters.LeanAmount, 0.0f, DeltaTime, 2.0f);
}

bool UPrimitiveAnimationController::IsMoving() const
{
    if (!MovementComponent)
    {
        return false;
    }
    
    FVector HorizontalVelocity = MovementComponent->Velocity;
    HorizontalVelocity.Z = 0.0f;
    return HorizontalVelocity.Size() > WalkSpeedThreshold;
}

float UPrimitiveAnimationController::CalculateMovementDirection() const
{
    if (!MovementComponent || !IsMoving())
    {
        return 0.0f;
    }
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return 0.0f;
    }
    
    // Get movement direction relative to character forward
    FVector Velocity = MovementComponent->Velocity;
    Velocity.Z = 0.0f;
    Velocity.Normalize();
    
    FVector Forward = Character->GetActorForwardVector();
    Forward.Z = 0.0f;
    Forward.Normalize();
    
    // Calculate angle between forward and velocity
    float DotProduct = FVector::DotProduct(Forward, Velocity);
    float CrossProduct = FVector::CrossProduct(Forward, Velocity).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

void UPrimitiveAnimationController::TriggerJumpAnimation()
{
    CurrentMovementState = EAnim_MovementState::Jumping;
    UE_LOG(LogTemp, Log, TEXT("Jump animation triggered"));
}

void UPrimitiveAnimationController::TriggerLandAnimation()
{
    UE_LOG(LogTemp, Log, TEXT("Land animation triggered"));
    
    // Reset to appropriate ground state
    if (IsMoving())
    {
        CalculateMovementState();
    }
    else
    {
        CurrentMovementState = EAnim_MovementState::Idle;
    }
}

void UPrimitiveAnimationController::SetCrouchState(bool bShouldCrouch)
{
    if (!MovementComponent)
    {
        return;
    }
    
    if (bShouldCrouch)
    {
        MovementComponent->Crouch();
        CurrentMovementState = EAnim_MovementState::Crouching;
    }
    else
    {
        MovementComponent->UnCrouch();
        CalculateMovementState();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crouch state set to: %s"), bShouldCrouch ? TEXT("true") : TEXT("false"));
}