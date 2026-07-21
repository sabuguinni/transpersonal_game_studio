#include "Anim_StateManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_StateManager::UAnim_StateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    CurrentStateData = FAnim_StateData();
}

void UAnim_StateManager::BeginPlay()
{
    Super::BeginPlay();
    
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
        UE_LOG(LogTemp, Warning, TEXT("Anim_StateManager: Invalid owner character or movement component"));
    }
}

void UAnim_StateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerCharacter && MovementComponent)
    {
        UpdateMovementData();
        UpdateAimData();
        UpdateMovementState();
    }
}

void UAnim_StateManager::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    EAnim_MovementState NewMovementState = CalculateMovementState();
    
    if (NewMovementState != CurrentStateData.MovementState)
    {
        CurrentStateData.MovementState = NewMovementState;
        
        // Log state changes for debugging
        UE_LOG(LogTemp, Log, TEXT("Animation state changed to: %d"), (int32)NewMovementState);
    }
}

void UAnim_StateManager::SetActionState(EAnim_ActionState NewActionState)
{
    if (CurrentStateData.ActionState != NewActionState)
    {
        CurrentStateData.ActionState = NewActionState;
        UE_LOG(LogTemp, Log, TEXT("Action state changed to: %d"), (int32)NewActionState);
    }
}

void UAnim_StateManager::PlayActionMontage(UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play montage: Invalid AnimInstance or Montage"));
        return;
    }
    
    AnimInstance->Montage_Play(Montage, PlayRate);
    UE_LOG(LogTemp, Log, TEXT("Playing action montage: %s"), *Montage->GetName());
}

void UAnim_StateManager::StopActionMontage(float BlendOutTime)
{
    if (!AnimInstance)
    {
        return;
    }
    
    AnimInstance->Montage_Stop(BlendOutTime);
}

bool UAnim_StateManager::IsPlayingActionMontage() const
{
    if (!AnimInstance)
    {
        return false;
    }
    
    return AnimInstance->IsAnyMontagePlaying();
}

void UAnim_StateManager::UpdateMovementData()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update speed
    FVector Velocity = MovementComponent->Velocity;
    CurrentStateData.Speed = Velocity.Size2D();
    
    // Update direction relative to character forward
    if (CurrentStateData.Speed > 1.0f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        float TargetDirection = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
        
        // Smooth direction changes
        CurrentStateData.Direction = FMath::FInterpTo(
            CurrentStateData.Direction, 
            TargetDirection, 
            GetWorld()->GetDeltaSeconds(), 
            DirectionSmoothingSpeed
        );
    }
    
    // Update air state
    CurrentStateData.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouch state
    CurrentStateData.bIsCrouched = MovementComponent->IsCrouching();
}

void UAnim_StateManager::UpdateAimData()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Get control rotation for aim offset
    FRotator ControlRotation = OwnerCharacter->GetControlRotation();
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    
    // Calculate relative rotation
    FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
    
    // Smooth aim values
    float TargetAimYaw = FMath::Clamp(DeltaRotation.Yaw, -90.0f, 90.0f);
    float TargetAimPitch = FMath::Clamp(DeltaRotation.Pitch, -90.0f, 90.0f);
    
    CurrentStateData.AimYaw = FMath::FInterpTo(
        CurrentStateData.AimYaw, 
        TargetAimYaw, 
        GetWorld()->GetDeltaSeconds(), 
        AimSmoothingSpeed
    );
    
    CurrentStateData.AimPitch = FMath::FInterpTo(
        CurrentStateData.AimPitch, 
        TargetAimPitch, 
        GetWorld()->GetDeltaSeconds(), 
        AimSmoothingSpeed
    );
}

EAnim_MovementState UAnim_StateManager::CalculateMovementState() const
{
    if (!MovementComponent)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check for special movement modes first
    if (MovementComponent->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    if (MovementComponent->IsFalling())
    {
        // Check if we're jumping up or falling down
        if (MovementComponent->Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (MovementComponent->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Ground movement based on speed
    float Speed = CurrentStateData.Speed;
    
    if (Speed < WalkThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (Speed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}