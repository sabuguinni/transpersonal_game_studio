#include "Anim_MovementController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MovementController::UAnim_MovementController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize default values
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 500.0f;
    MovementSmoothingSpeed = 10.0f;
    
    PreviousSpeed = 0.0f;
    SmoothSpeed = 0.0f;
    PreviousState = EAnim_MovementState::Idle;

    // Initialize movement data
    MovementData = FAnim_MovementData();
}

void UAnim_MovementController::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner character and components
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
        {
            AnimInstance = MeshComp->GetAnimInstance();
        }
        
        if (!MovementComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_MovementController: No CharacterMovementComponent found on %s"), 
                   *OwnerCharacter->GetName());
        }
        
        if (!AnimInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_MovementController: No AnimInstance found on %s"), 
                   *OwnerCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UAnim_MovementController: Component not attached to ACharacter"));
    }
}

void UAnim_MovementController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementData();
        UpdateBlendSpaceValues();
        
        // Handle landing animation
        if (PreviousState == EAnim_MovementState::Falling && 
            MovementData.MovementState != EAnim_MovementState::Falling &&
            ShouldPlayLandingAnimation())
        {
            PlayMontage(LandingMontage);
        }
        
        PreviousState = MovementData.MovementState;
    }
}

void UAnim_MovementController::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    // Get current velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    
    // Smooth the speed for animation blending
    SmoothSpeed = FMath::FInterpTo(SmoothSpeed, CurrentSpeed, GetWorld()->GetDeltaSeconds(), MovementSmoothingSpeed);
    MovementData.Speed = SmoothSpeed;

    // Calculate movement direction relative to character facing
    if (CurrentSpeed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;
        
        MovementData.Direction = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        MovementData.bIsMoving = true;
    }
    else
    {
        MovementData.Direction = 0.0f;
        MovementData.bIsMoving = false;
    }

    // Update movement flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement state
    CalculateMovementState();

    PreviousSpeed = CurrentSpeed;
}

void UAnim_MovementController::CalculateMovementState()
{
    if (MovementData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            MovementData.MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            MovementData.MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (MovementData.bIsCrouching)
    {
        if (MovementData.bIsMoving)
        {
            MovementData.MovementState = EAnim_MovementState::Crawling;
        }
        else
        {
            MovementData.MovementState = EAnim_MovementState::Crouching;
        }
    }
    else if (MovementData.Speed < WalkSpeedThreshold)
    {
        MovementData.MovementState = EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < RunSpeedThreshold)
    {
        MovementData.MovementState = EAnim_MovementState::Walking;
    }
    else if (MovementData.Speed < SprintSpeedThreshold)
    {
        MovementData.MovementState = EAnim_MovementState::Running;
    }
    else
    {
        MovementData.MovementState = EAnim_MovementState::Sprinting;
    }
}

void UAnim_MovementController::UpdateBlendSpaceValues()
{
    if (!AnimInstance || !MovementBlendSpace)
    {
        return;
    }

    // Update blend space with current movement values
    // This would typically be done in the Animation Blueprint
    // but we can set variables that the ABP can read
}

void UAnim_MovementController::SetMovementState(EAnim_MovementState NewState)
{
    if (MovementData.MovementState != NewState)
    {
        EAnim_MovementState OldState = MovementData.MovementState;
        MovementData.MovementState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Movement state changed from %d to %d"), 
               (int32)OldState, (int32)NewState);
    }
}

bool UAnim_MovementController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return false;
    }

    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate);
    
    if (MontageLength > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Playing animation montage: %s"), *Montage->GetName());
        return true;
    }
    
    return false;
}

void UAnim_MovementController::StopMontage(UAnimMontage* Montage)
{
    if (!AnimInstance)
    {
        return;
    }

    if (Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
        UE_LOG(LogTemp, Log, TEXT("Stopping animation montage: %s"), *Montage->GetName());
    }
    else
    {
        // Stop all montages
        AnimInstance->StopAllMontages(0.2f);
        UE_LOG(LogTemp, Log, TEXT("Stopping all animation montages"));
    }
}

bool UAnim_MovementController::ShouldPlayLandingAnimation() const
{
    // Play landing animation if we fell for a significant distance
    return (PreviousSpeed > 200.0f || MovementComponent->GetLastUpdateVelocity().Z < -500.0f);
}