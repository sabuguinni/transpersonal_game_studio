#include "Anim_PlayerLocomotionSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_PlayerLocomotionSystem::UAnim_PlayerLocomotionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 400.0f;
    DirectionChangeSmoothing = 5.0f;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    AnimInstance = nullptr;
    
    PreviousState = EAnim_LocomotionState::Idle;
    StateChangeTime = 0.0f;
    SmoothedDirection = 0.0f;
    
    JumpMontage = nullptr;
    LandingMontage = nullptr;
    LocomotionBlendSpace = nullptr;
}

void UAnim_PlayerLocomotionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner character and components
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
    
    if (!OwnerCharacter || !MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_PlayerLocomotionSystem: Failed to initialize - missing character or movement component"));
    }
}

void UAnim_PlayerLocomotionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateLocomotionData();
        StateChangeTime += DeltaTime;
    }
}

void UAnim_PlayerLocomotionSystem::UpdateLocomotionData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Calculate current speed
    FVector Velocity = MovementComponent->Velocity;
    LocomotionData.Speed = Velocity.Size2D();
    
    // Calculate movement direction relative to character forward
    LocomotionData.Direction = CalculateMovementDirection();
    
    // Update air state
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouching state
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate and update locomotion state
    EAnim_LocomotionState NewState = CalculateLocomotionState();
    if (NewState != LocomotionData.CurrentState)
    {
        HandleStateTransition(NewState);
    }
}

EAnim_LocomotionState UAnim_PlayerLocomotionSystem::CalculateLocomotionState() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return EAnim_LocomotionState::Idle;
    }
    
    // Check if in air first
    if (LocomotionData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            return EAnim_LocomotionState::Jumping;
        }
        else
        {
            return EAnim_LocomotionState::Falling;
        }
    }
    
    // Check if crouching
    if (LocomotionData.bIsCrouching)
    {
        return EAnim_LocomotionState::Crouching;
    }
    
    // Determine ground movement state based on speed
    if (LocomotionData.Speed < 10.0f) // Small threshold to avoid jitter
    {
        return EAnim_LocomotionState::Idle;
    }
    else if (LocomotionData.Speed < RunSpeedThreshold)
    {
        return EAnim_LocomotionState::Walking;
    }
    else
    {
        return EAnim_LocomotionState::Running;
    }
}

void UAnim_PlayerLocomotionSystem::HandleStateTransition(EAnim_LocomotionState NewState)
{
    PreviousState = LocomotionData.CurrentState;
    LocomotionData.CurrentState = NewState;
    StateChangeTime = 0.0f;
    
    // Handle specific state transitions
    if (PreviousState == EAnim_LocomotionState::Falling && NewState != EAnim_LocomotionState::Falling)
    {
        // Just landed
        PlayLandingMontage();
    }
}

float UAnim_PlayerLocomotionSystem::CalculateMovementDirection() const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return 0.0f;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.Size2D() < 10.0f)
    {
        return SmoothedDirection; // Maintain last direction when not moving
    }
    
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    // Normalize velocity to 2D
    FVector Velocity2D = FVector(Velocity.X, Velocity.Y, 0.0f).GetSafeNormal();
    
    // Calculate angle relative to forward direction
    float ForwardDot = FVector::DotProduct(Velocity2D, ForwardVector);
    float RightDot = FVector::DotProduct(Velocity2D, RightVector);
    
    float Direction = UKismetMathLibrary::DegAtan2(RightDot, ForwardDot);
    
    // Smooth direction changes
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    SmoothedDirection = UKismetMathLibrary::FInterpTo(SmoothedDirection, Direction, DeltaTime, DirectionChangeSmoothing);
    
    return SmoothedDirection;
}

EAnim_LocomotionState UAnim_PlayerLocomotionSystem::GetCurrentLocomotionState() const
{
    return LocomotionData.CurrentState;
}

FAnim_LocomotionData UAnim_PlayerLocomotionSystem::GetLocomotionData() const
{
    return LocomotionData;
}

void UAnim_PlayerLocomotionSystem::PlayJumpMontage()
{
    if (AnimInstance && JumpMontage)
    {
        AnimInstance->Montage_Play(JumpMontage);
    }
}

void UAnim_PlayerLocomotionSystem::PlayLandingMontage()
{
    if (AnimInstance && LandingMontage)
    {
        AnimInstance->Montage_Play(LandingMontage);
    }
}

void UAnim_PlayerLocomotionSystem::StopAllMontages()
{
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.2f);
    }
}