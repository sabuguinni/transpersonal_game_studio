#include "Anim_CharacterAnimController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimController::UAnim_CharacterAnimController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize IK settings
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    LeftFootSocketName = TEXT("foot_l");
    RightFootSocketName = TEXT("foot_r");
    
    // Initialize component references
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MeshComponent = nullptr;
    AnimInstance = nullptr;
}

void UAnim_CharacterAnimController::BeginPlay()
{
    Super::BeginPlay();
    
    CacheComponentReferences();
    
    // Initialize animation data
    AnimData = FAnim_CharacterAnimData();
    AnimData.MovementState = EAnim_MovementState::Idle;
    AnimData.CombatState = EAnim_CombatState::None;
    AnimData.EmotionalState = EAnim_EmotionalState::Calm;
}

void UAnim_CharacterAnimController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsValidForAnimation())
    {
        return;
    }
    
    UpdateMovementData(DeltaTime);
    UpdateCombatData(DeltaTime);
    UpdateEmotionalData(DeltaTime);
    
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_CharacterAnimController::CacheComponentReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MeshComponent = OwnerCharacter->GetMesh();
        if (MeshComponent)
        {
            AnimInstance = MeshComponent->GetAnimInstance();
        }
    }
}

bool UAnim_CharacterAnimController::IsValidForAnimation() const
{
    return OwnerCharacter && MovementComponent && MeshComponent && AnimInstance;
}

void UAnim_CharacterAnimController::UpdateMovementData(float DeltaTime)
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Update speed
    FVector Velocity = MovementComponent->Velocity;
    AnimData.Speed = Velocity.Size2D();
    
    // Update direction
    if (AnimData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        AnimData.Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
    }
    else
    {
        AnimData.Direction = 0.0f;
    }
    
    // Update air state
    AnimData.bIsInAir = MovementComponent->IsFalling();
    
    // Update crouching state
    AnimData.bIsCrouching = MovementComponent->IsCrouching();
    
    // Update movement state based on speed and conditions
    if (AnimData.bIsInAir)
    {
        if (Velocity.Z > 0.0f)
        {
            AnimData.MovementState = EAnim_MovementState::Jumping;
        }
        else
        {
            AnimData.MovementState = EAnim_MovementState::Falling;
        }
    }
    else if (AnimData.bIsCrouching)
    {
        AnimData.MovementState = EAnim_MovementState::Crouching;
    }
    else if (MovementComponent->IsSwimming())
    {
        AnimData.MovementState = EAnim_MovementState::Swimming;
    }
    else if (AnimData.Speed < 1.0f)
    {
        AnimData.MovementState = EAnim_MovementState::Idle;
    }
    else if (AnimData.Speed < 300.0f)
    {
        AnimData.MovementState = EAnim_MovementState::Walking;
    }
    else if (AnimData.Speed < 500.0f)
    {
        AnimData.MovementState = EAnim_MovementState::Running;
    }
    else
    {
        AnimData.MovementState = EAnim_MovementState::Sprinting;
    }
}

void UAnim_CharacterAnimController::UpdateCombatData(float DeltaTime)
{
    // This would be updated based on combat system integration
    // For now, maintain current combat state
    
    // Example logic for combat state transitions
    if (AnimData.CombatState == EAnim_CombatState::Attacking)
    {
        // Check if attack animation is finished
        if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
        {
            AnimData.CombatState = EAnim_CombatState::Ready;
        }
    }
}

void UAnim_CharacterAnimController::UpdateEmotionalData(float DeltaTime)
{
    // This would be updated based on survival/fear system integration
    // For now, maintain current emotional state
    
    // Example logic for emotional state transitions
    if (AnimData.EmotionalState == EAnim_EmotionalState::Fearful)
    {
        // Gradually return to calm state if no threats
        // This would integrate with the fear system
    }
}

void UAnim_CharacterAnimController::UpdateFootIK(float DeltaTime)
{
    if (!MeshComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Perform foot traces
    FVector LeftFootTarget = PerformFootTrace(LeftFootSocketName, FootIKTraceDistance);
    FVector RightFootTarget = PerformFootTrace(RightFootSocketName, FootIKTraceDistance);
    
    // Interpolate to smooth IK positions
    AnimData.LeftFootIKLocation = FMath::VInterpTo(
        AnimData.LeftFootIKLocation,
        LeftFootTarget,
        DeltaTime,
        FootIKInterpSpeed
    );
    
    AnimData.RightFootIKLocation = FMath::VInterpTo(
        AnimData.RightFootIKLocation,
        RightFootTarget,
        DeltaTime,
        FootIKInterpSpeed
    );
    
    // Calculate pelvis offset to keep character grounded
    float LeftOffset = AnimData.LeftFootIKLocation.Z;
    float RightOffset = AnimData.RightFootIKLocation.Z;
    float TargetPelvisOffset = FMath::Min(LeftOffset, RightOffset);
    
    AnimData.PelvisOffset = FMath::FInterpTo(
        AnimData.PelvisOffset,
        TargetPelvisOffset,
        DeltaTime,
        FootIKInterpSpeed
    );
}

FVector UAnim_CharacterAnimController::PerformFootTrace(const FName& SocketName, float TraceDistance)
{
    if (!MeshComponent || !OwnerCharacter)
    {
        return FVector::ZeroVector;
    }
    
    // Get socket location
    FVector SocketLocation = MeshComponent->GetSocketLocation(SocketName);
    
    // Perform line trace downward
    FVector StartLocation = SocketLocation + FVector(0, 0, 20.0f);
    FVector EndLocation = SocketLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate offset from original socket position
        float Offset = HitResult.Location.Z - SocketLocation.Z;
        return FVector(0, 0, Offset);
    }
    
    return FVector::ZeroVector;
}

void UAnim_CharacterAnimController::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_CharacterAnimController::StopMontage(UAnimMontage* Montage)
{
    if (AnimInstance && Montage)
    {
        AnimInstance->Montage_Stop(0.2f, Montage);
    }
}

void UAnim_CharacterAnimController::SetMovementState(EAnim_MovementState NewState)
{
    AnimData.MovementState = NewState;
}

void UAnim_CharacterAnimController::SetCombatState(EAnim_CombatState NewState)
{
    AnimData.CombatState = NewState;
    
    // Trigger appropriate montages based on combat state
    switch (NewState)
    {
        case EAnim_CombatState::Attacking:
            if (AttackMontage)
            {
                PlayMontage(AttackMontage);
            }
            break;
        case EAnim_CombatState::Blocking:
            if (BlockMontage)
            {
                PlayMontage(BlockMontage);
            }
            break;
        case EAnim_CombatState::Dodging:
            if (DodgeMontage)
            {
                PlayMontage(DodgeMontage);
            }
            break;
        default:
            break;
    }
}

void UAnim_CharacterAnimController::SetEmotionalState(EAnim_EmotionalState NewState)
{
    AnimData.EmotionalState = NewState;
}