#include "Anim_CharacterMovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterMovementController::UAnim_CharacterMovementController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    DirectionSmoothingSpeed = 10.0f;
    PreviousDirection = 0.0f;
    DirectionSmoothingTimer = 0.0f;

    // Initialize movement state
    CurrentMovementState = FAnim_MovementState();
}

void UAnim_CharacterMovementController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheReferences();
}

void UAnim_CharacterMovementController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementState();
        UpdateAnimationBlueprint();
    }
}

void UAnim_CharacterMovementController::CacheReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
        if (MeshComp)
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
    }
}

void UAnim_CharacterMovementController::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Update speed
    FVector Velocity = MovementComponent->Velocity;
    CurrentMovementState.Speed = Velocity.Size2D();

    // Update direction (relative to character forward)
    if (CurrentMovementState.Speed > 1.0f)
    {
        FVector Forward = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(Forward, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(Forward, VelocityNormalized).Z;
        
        float TargetDirection = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
        
        // Smooth direction changes
        CurrentMovementState.Direction = FMath::FInterpTo(
            PreviousDirection, 
            TargetDirection, 
            GetWorld()->GetDeltaSeconds(), 
            DirectionSmoothingSpeed
        );
        
        PreviousDirection = CurrentMovementState.Direction;
    }
    else
    {
        CurrentMovementState.Direction = 0.0f;
        PreviousDirection = 0.0f;
    }

    // Update air state
    CurrentMovementState.bIsInAir = MovementComponent->IsFalling();

    // Update crouch state
    CurrentMovementState.bIsCrouching = MovementComponent->IsCrouching();

    // Update movement mode
    CurrentMovementState.MovementMode = DetermineMovementMode();
}

EAnim_MovementMode UAnim_CharacterMovementController::DetermineMovementMode()
{
    if (CurrentMovementState.bIsInAir)
    {
        return EAnim_MovementMode::Jumping;
    }
    
    if (CurrentMovementState.bIsCrouching)
    {
        return CurrentMovementState.Speed > 1.0f ? EAnim_MovementMode::CrouchWalking : EAnim_MovementMode::CrouchIdle;
    }

    if (CurrentMovementState.Speed < 1.0f)
    {
        return EAnim_MovementMode::Idle;
    }
    else if (CurrentMovementState.Speed < WalkSpeedThreshold)
    {
        return EAnim_MovementMode::Walking;
    }
    else if (CurrentMovementState.Speed < RunSpeedThreshold)
    {
        return EAnim_MovementMode::Running;
    }
    else
    {
        return EAnim_MovementMode::Sprinting;
    }
}

void UAnim_CharacterMovementController::UpdateAnimationBlueprint()
{
    if (!AnimInstance)
    {
        return;
    }

    // Set animation blueprint variables (these would be consumed by the AnimBP)
    // In a real implementation, these would be exposed as animation blueprint variables
    
    // For now, we'll use the engine's built-in logging to show the system is working
    if (GEngine && GetWorld()->GetTimeSeconds() - DirectionSmoothingTimer > 1.0f)
    {
        DirectionSmoothingTimer = GetWorld()->GetTimeSeconds();
        
        FString ModeString = "";
        switch (CurrentMovementState.MovementMode)
        {
            case EAnim_MovementMode::Idle: ModeString = "Idle"; break;
            case EAnim_MovementMode::Walking: ModeString = "Walking"; break;
            case EAnim_MovementMode::Running: ModeString = "Running"; break;
            case EAnim_MovementMode::Sprinting: ModeString = "Sprinting"; break;
            case EAnim_MovementMode::Jumping: ModeString = "Jumping"; break;
            case EAnim_MovementMode::CrouchIdle: ModeString = "CrouchIdle"; break;
            case EAnim_MovementMode::CrouchWalking: ModeString = "CrouchWalking"; break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Animation State - Mode: %s, Speed: %.1f, Direction: %.1f"), 
               *ModeString, CurrentMovementState.Speed, CurrentMovementState.Direction);
    }
}

void UAnim_CharacterMovementController::PlayJumpAnimation()
{
    if (AnimInstance && JumpMontage)
    {
        AnimInstance->Montage_Play(JumpMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("Playing jump animation montage"));
    }
}

EAnim_MovementMode UAnim_CharacterMovementController::GetCurrentMovementMode() const
{
    return CurrentMovementState.MovementMode;
}

float UAnim_CharacterMovementController::GetMovementSpeed() const
{
    return CurrentMovementState.Speed;
}

float UAnim_CharacterMovementController::GetMovementDirection() const
{
    return CurrentMovementState.Direction;
}

bool UAnim_CharacterMovementController::IsMoving() const
{
    return CurrentMovementState.Speed > 1.0f;
}