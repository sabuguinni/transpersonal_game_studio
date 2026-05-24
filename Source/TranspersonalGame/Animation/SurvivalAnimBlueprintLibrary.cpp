#include "SurvivalAnimBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

FAnim_LocomotionData USurvivalAnimBlueprintLibrary::CalculateLocomotionData(ACharacter* Character)
{
    FAnim_LocomotionData LocomotionData;
    
    if (!Character)
    {
        return LocomotionData;
    }

    UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
    if (!MovementComponent)
    {
        return LocomotionData;
    }

    // Calculate basic locomotion values
    FVector Velocity = Character->GetVelocity();
    LocomotionData.Speed = Velocity.Size();
    LocomotionData.bIsMoving = LocomotionData.Speed > 1.0f;
    LocomotionData.bIsInAir = MovementComponent->IsFalling();
    LocomotionData.bIsCrouching = MovementComponent->IsCrouching();

    // Calculate movement direction relative to character forward
    if (LocomotionData.bIsMoving)
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        LocomotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // Determine if moving left or right
        FVector RightVector = Character->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            LocomotionData.Direction *= -1.0f;
        }

        LocomotionData.MovementDirection = CalculateMovementDirection(Velocity, Character->GetActorRotation());
    }

    return LocomotionData;
}

EAnim_SurvivalState USurvivalAnimBlueprintLibrary::DetermineSurvivalState(const FAnim_SurvivalStats& Stats, const FAnim_LocomotionData& Locomotion)
{
    // Priority order: Injured > Fearful > Exhausted > Movement states
    
    if (Stats.bIsInjured || Stats.Health < 30.0f)
    {
        return EAnim_SurvivalState::Injured;
    }
    
    if (Stats.bIsFearful || Stats.Fear > 70.0f)
    {
        return EAnim_SurvivalState::Fearful;
    }
    
    if (Stats.bIsExhausted || Stats.Stamina < 20.0f)
    {
        return EAnim_SurvivalState::Exhausted;
    }

    // Movement-based states
    if (Locomotion.bIsInAir)
    {
        return EAnim_SurvivalState::Falling;
    }
    
    if (Locomotion.bIsCrouching)
    {
        return EAnim_SurvivalState::Crouching;
    }
    
    if (Locomotion.bIsMoving)
    {
        if (Locomotion.Speed > 400.0f)
        {
            return EAnim_SurvivalState::Running;
        }
        else
        {
            return EAnim_SurvivalState::Walking;
        }
    }

    return EAnim_SurvivalState::Idle;
}

EAnim_MovementDirection USurvivalAnimBlueprintLibrary::CalculateMovementDirection(const FVector& Velocity, const FRotator& ActorRotation)
{
    if (Velocity.SizeSquared() < 1.0f)
    {
        return EAnim_MovementDirection::Forward;
    }

    FVector VelocityNormalized = Velocity.GetSafeNormal();
    FVector ForwardVector = ActorRotation.Vector();
    FVector RightVector = FRotationMatrix(ActorRotation).GetScaledAxis(EAxis::Y);

    float ForwardDot = FVector::DotProduct(ForwardVector, VelocityNormalized);
    float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);

    // Determine primary direction
    if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
    {
        // Primary movement is forward/backward
        if (ForwardDot > 0.5f)
        {
            if (RightDot > 0.3f)
                return EAnim_MovementDirection::ForwardRight;
            else if (RightDot < -0.3f)
                return EAnim_MovementDirection::ForwardLeft;
            else
                return EAnim_MovementDirection::Forward;
        }
        else
        {
            if (RightDot > 0.3f)
                return EAnim_MovementDirection::BackwardRight;
            else if (RightDot < -0.3f)
                return EAnim_MovementDirection::BackwardLeft;
            else
                return EAnim_MovementDirection::Backward;
        }
    }
    else
    {
        // Primary movement is left/right
        return (RightDot > 0.0f) ? EAnim_MovementDirection::Right : EAnim_MovementDirection::Left;
    }
}

float USurvivalAnimBlueprintLibrary::CalculateSpeedBlendValue(float CurrentSpeed, float WalkSpeed, float RunSpeed)
{
    if (CurrentSpeed <= WalkSpeed)
    {
        return FMath::Clamp(CurrentSpeed / WalkSpeed, 0.0f, 1.0f);
    }
    else
    {
        return FMath::Clamp(1.0f + ((CurrentSpeed - WalkSpeed) / (RunSpeed - WalkSpeed)), 1.0f, 2.0f);
    }
}

float USurvivalAnimBlueprintLibrary::CalculateFearBlendValue(float FearLevel)
{
    return FMath::Clamp(FearLevel / 100.0f, 0.0f, 1.0f);
}

float USurvivalAnimBlueprintLibrary::CalculateInjuryBlendValue(float HealthPercentage)
{
    return FMath::Clamp((100.0f - HealthPercentage) / 100.0f, 0.0f, 1.0f);
}

bool USurvivalAnimBlueprintLibrary::ShouldTransitionToFearState(float FearLevel, float Threshold)
{
    return FearLevel >= Threshold;
}

bool USurvivalAnimBlueprintLibrary::ShouldTransitionToInjuredState(float HealthPercentage, float Threshold)
{
    return HealthPercentage <= Threshold;
}

bool USurvivalAnimBlueprintLibrary::ShouldTransitionToExhaustedState(float StaminaPercentage, float Threshold)
{
    return StaminaPercentage <= Threshold;
}

bool USurvivalAnimBlueprintLibrary::PlaySurvivalMontage(UAnimInstance* AnimInstance, UAnimMontage* Montage, float PlayRate)
{
    if (!AnimInstance || !Montage)
    {
        return false;
    }

    return AnimInstance->Montage_Play(Montage, PlayRate) > 0.0f;
}

void USurvivalAnimBlueprintLibrary::StopSurvivalMontage(UAnimInstance* AnimInstance, UAnimMontage* Montage, float BlendOutTime)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }

    AnimInstance->Montage_Stop(BlendOutTime, Montage);
}

FVector USurvivalAnimBlueprintLibrary::CalculateFootIKOffset(USkeletalMeshComponent* MeshComponent, const FName& BoneName, float TraceDistance)
{
    if (!MeshComponent || !MeshComponent->GetWorld())
    {
        return FVector::ZeroVector;
    }

    // Get bone location in world space
    FVector BoneLocation = MeshComponent->GetBoneLocation(BoneName, EBoneSpaces::WorldSpace);
    
    // Trace downward to find ground
    FVector TraceStart = BoneLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = BoneLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(MeshComponent->GetOwner());
    
    bool bHit = MeshComponent->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calculate offset needed to place foot on ground
        float GroundHeight = HitResult.Location.Z;
        float BoneHeight = BoneLocation.Z;
        float Offset = GroundHeight - BoneHeight;
        
        return FVector(0, 0, FMath::Clamp(Offset, -TraceDistance, TraceDistance));
    }

    return FVector::ZeroVector;
}

FRotator USurvivalAnimBlueprintLibrary::CalculateFootIKRotation(USkeletalMeshComponent* MeshComponent, const FName& BoneName, const FVector& IKOffset)
{
    if (!MeshComponent)
    {
        return FRotator::ZeroRotator;
    }

    // Get current bone rotation
    FRotator CurrentRotation = MeshComponent->GetBoneQuaternion(BoneName, EBoneSpaces::WorldSpace).Rotator();
    
    // Calculate slope-based rotation adjustment
    if (!IKOffset.IsZero())
    {
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Atan2(FMath::Abs(IKOffset.Z), 10.0f));
        SlopeAngle = FMath::Clamp(SlopeAngle, -15.0f, 15.0f);
        
        return FRotator(SlopeAngle, CurrentRotation.Yaw, CurrentRotation.Roll);
    }

    return CurrentRotation;
}

float USurvivalAnimBlueprintLibrary::GetCharacterGroundSpeed(ACharacter* Character)
{
    if (!Character)
    {
        return 0.0f;
    }

    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.0f; // Remove vertical component
    return Velocity.Size();
}

bool USurvivalAnimBlueprintLibrary::IsCharacterOnGround(ACharacter* Character)
{
    if (!Character)
    {
        return false;
    }

    UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
    if (!MovementComponent)
    {
        return false;
    }

    return MovementComponent->IsMovingOnGround();
}

FVector USurvivalAnimBlueprintLibrary::GetCharacterVelocity2D(ACharacter* Character)
{
    if (!Character)
    {
        return FVector::ZeroVector;
    }

    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.0f;
    return Velocity;
}