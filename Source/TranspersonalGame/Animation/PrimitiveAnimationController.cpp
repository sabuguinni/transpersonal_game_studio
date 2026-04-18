#include "PrimitiveAnimationController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UPrimitiveAnimationController::UPrimitiveAnimationController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentState = EAnim_MovementState::Idle;
    WalkSpeedThreshold = 100.0f;
    RunSpeedThreshold = 300.0f;
    BlendSpeed = 5.0f;
    
    // Initialize blend data
    BlendData.Speed = 0.0f;
    BlendData.Direction = 0.0f;
    BlendData.bIsInAir = false;
    BlendData.bIsCrouching = false;
    BlendData.AimPitch = 0.0f;
    BlendData.AimYaw = 0.0f;
}

void UPrimitiveAnimationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComp = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("PrimitiveAnimationController: Initialized for character %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrimitiveAnimationController: Owner is not a Character!"));
    }
}

void UPrimitiveAnimationController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Get movement data from character
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Calculate current speed and movement state
    FVector Velocity = MovementComp->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    bool bIsInAir = MovementComp->IsFalling();
    bool bIsCrouching = MovementComp->IsCrouching();
    
    // Update movement state
    UpdateMovementState(CurrentSpeed, bIsInAir, bIsCrouching);
    
    // Update blend data with smooth interpolation
    UpdateBlendData(DeltaTime);
}

void UPrimitiveAnimationController::UpdateMovementState(float Speed, bool bIsInAir, bool bIsCrouching)
{
    // Store target values for smooth blending
    TargetSpeed = Speed;
    
    // Determine movement state based on conditions
    DetermineMovementState(Speed, bIsInAir, bIsCrouching);
    
    // Update immediate boolean states
    BlendData.bIsInAir = bIsInAir;
    BlendData.bIsCrouching = bIsCrouching;
}

void UPrimitiveAnimationController::DetermineMovementState(float Speed, bool bIsInAir, bool bIsCrouching)
{
    EAnim_MovementState NewState = CurrentState;
    
    if (bIsInAir)
    {
        // In air states
        if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
        {
            float VerticalVelocity = OwnerCharacter->GetCharacterMovement()->Velocity.Z;
            if (VerticalVelocity > 0.0f)
            {
                NewState = EAnim_MovementState::Jumping;
            }
            else
            {
                NewState = EAnim_MovementState::Falling;
            }
        }
    }
    else if (bIsCrouching)
    {
        NewState = EAnim_MovementState::Crouching;
    }
    else
    {
        // Ground movement states
        if (Speed < 10.0f)
        {
            NewState = EAnim_MovementState::Idle;
        }
        else if (Speed < RunSpeedThreshold)
        {
            NewState = EAnim_MovementState::Walking;
        }
        else
        {
            NewState = EAnim_MovementState::Running;
        }
    }
    
    // Log state changes
    if (NewState != CurrentState)
    {
        UE_LOG(LogTemp, Log, TEXT("Animation State Change: %d -> %d (Speed: %.1f)"), 
               (int32)CurrentState, (int32)NewState, Speed);
        CurrentState = NewState;
    }
}

void UPrimitiveAnimationController::UpdateBlendData(float DeltaTime)
{
    // Smooth interpolation of blend values
    BlendData.Speed = FMath::FInterpTo(BlendData.Speed, TargetSpeed, DeltaTime, BlendSpeed);
    BlendData.Direction = FMath::FInterpTo(BlendData.Direction, TargetDirection, DeltaTime, BlendSpeed);
    BlendData.AimPitch = FMath::FInterpTo(BlendData.AimPitch, TargetAimPitch, DeltaTime, BlendSpeed);
    BlendData.AimYaw = FMath::FInterpTo(BlendData.AimYaw, TargetAimYaw, DeltaTime, BlendSpeed);
    
    // Calculate movement direction relative to character rotation
    if (OwnerCharacter && TargetSpeed > 10.0f)
    {
        FVector Velocity = OwnerCharacter->GetCharacterMovement()->Velocity;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        // Project velocity onto character's local axes
        float ForwardComponent = FVector::DotProduct(Velocity.GetSafeNormal2D(), ForwardVector);
        float RightComponent = FVector::DotProduct(Velocity.GetSafeNormal2D(), RightVector);
        
        // Convert to angle (-180 to 180)
        TargetDirection = FMath::Atan2(RightComponent, ForwardComponent) * (180.0f / PI);
    }
    else
    {
        TargetDirection = 0.0f;
    }
}

void UPrimitiveAnimationController::PlayJumpAnimation()
{
    if (JumpMontage)
    {
        PlayMontageIfValid(JumpMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing jump animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Jump montage not assigned!"));
    }
}

void UPrimitiveAnimationController::PlayClimbAnimation()
{
    if (ClimbMontage)
    {
        PlayMontageIfValid(ClimbMontage);
        CurrentState = EAnim_MovementState::Climbing;
        UE_LOG(LogTemp, Log, TEXT("Playing climb animation"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Climb montage not assigned!"));
    }
}

void UPrimitiveAnimationController::SetAimOffset(float Pitch, float Yaw)
{
    TargetAimPitch = FMath::Clamp(Pitch, -90.0f, 90.0f);
    TargetAimYaw = FMath::Clamp(Yaw, -180.0f, 180.0f);
}

void UPrimitiveAnimationController::PlayMontageIfValid(UAnimMontage* Montage)
{
    if (!Montage || !SkeletalMeshComp)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(Montage);
    }
}