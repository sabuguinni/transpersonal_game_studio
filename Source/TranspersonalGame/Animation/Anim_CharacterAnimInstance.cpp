#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    bEnableFootIK = true;
    HipOffset = 0.0f;
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    bIsMoving = false;
    
    CurrentState = TEXT("Idle");
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    MotionMatchingSystem = nullptr;
    CurrentActionMontage = nullptr;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        MotionMatchingSystem = OwnerCharacter->FindComponentByClass<UAnim_MotionMatchingSystem>();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateLocomotionVariables();
    UpdateMotionMatchingData();
    
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaSeconds);
    }
}

void UAnim_CharacterAnimInstance::UpdateLocomotionVariables()
{
    if (!MovementComponent)
    {
        return;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();
    bIsMoving = Speed > 3.0f;
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Calculate movement direction
    if (Speed > 0.0f && OwnerCharacter)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = Velocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
        Direction = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
        
        // Determine if moving left or right
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityDirection);
        if (CrossProduct.Z < 0)
        {
            Direction *= -1.0f;
        }
    }
    else
    {
        Direction = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::UpdateMotionMatchingData()
{
    if (MotionMatchingSystem)
    {
        MotionData = MotionMatchingSystem->CurrentMotionData;
        CurrentState = MotionMatchingSystem->CurrentAnimationState;
    }
}

void UAnim_CharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Reset IK when in air
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
        HipOffset = FMath::FInterpTo(HipOffset, 0.0f, DeltaSeconds, IKInterpSpeed);
        return;
    }
    
    // Calculate foot IK for both feet
    LeftFootIK = CalculateFootIK(TEXT("foot_l"), DeltaSeconds);
    RightFootIK = CalculateFootIK(TEXT("foot_r"), DeltaSeconds);
    
    // Calculate hip offset to keep character grounded
    float LeftFootOffset = LeftFootIK.FootLocation.Z;
    float RightFootOffset = RightFootIK.FootLocation.Z;
    float TargetHipOffset = FMath::Min(LeftFootOffset, RightFootOffset);
    
    if (TargetHipOffset < 0.0f)
    {
        HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaSeconds, IKInterpSpeed);
        
        // Adjust foot locations relative to hip
        LeftFootIK.FootLocation.Z -= HipOffset;
        RightFootIK.FootLocation.Z -= HipOffset;
    }
    else
    {
        HipOffset = FMath::FInterpTo(HipOffset, 0.0f, DeltaSeconds, IKInterpSpeed);
    }
}

FAnim_IKFootData UAnim_CharacterAnimInstance::CalculateFootIK(FName SocketName, float DeltaSeconds)
{
    FAnim_IKFootData FootData;
    
    if (!OwnerCharacter)
    {
        return FootData;
    }
    
    USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
    if (!Mesh)
    {
        return FootData;
    }
    
    // Get foot socket location
    FVector SocketLocation = Mesh->GetSocketLocation(SocketName);
    
    // Perform trace downward from foot
    float ImpactZ = 0.0f;
    FVector TraceStart = SocketLocation;
    FVector TraceEnd = TraceStart - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        ImpactZ = HitResult.Location.Z;
        
        // Calculate foot offset
        float FootOffset = ImpactZ - SocketLocation.Z;
        FootData.FootLocation = FVector(0, 0, FootOffset);
        
        // Calculate foot rotation based on surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        float PitchAngle = FMath::RadiansToDegrees(FMath::Asin(FVector::DotProduct(SurfaceNormal, ForwardVector)));
        float RollAngle = FMath::RadiansToDegrees(FMath::Asin(FVector::DotProduct(SurfaceNormal, RightVector)));
        
        FootData.FootRotation = FRotator(-PitchAngle, 0, RollAngle);
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, DeltaSeconds, IKInterpSpeed);
        
        // Calculate knee target for pole vector
        FootData.KneeTargetZ = FMath::Abs(FootOffset) * 0.5f;
    }
    else
    {
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaSeconds, IKInterpSpeed);
    }
    
    return FootData;
}

void UAnim_CharacterAnimInstance::PlayActionMontage(FName ActionName, float PlayRate)
{
    if (!MotionMatchingSystem)
    {
        return;
    }
    
    UAnimMontage* MontageToPlay = MotionMatchingSystem->GetActionMontage(ActionName);
    if (MontageToPlay)
    {
        CurrentActionMontage = MontageToPlay;
        Montage_Play(MontageToPlay, PlayRate);
        
        UE_LOG(LogTemp, Log, TEXT("Playing Action Montage: %s"), *ActionName.ToString());
    }
}

void UAnim_CharacterAnimInstance::StopActionMontage(float BlendOutTime)
{
    if (CurrentActionMontage && Montage_IsPlaying(CurrentActionMontage))
    {
        Montage_Stop(BlendOutTime, CurrentActionMontage);
        CurrentActionMontage = nullptr;
    }
}

FVector UAnim_CharacterAnimInstance::PerformFootTrace(const FVector& FootLocation, float& OutImpactZ) const
{
    if (!GetWorld())
    {
        OutImpactZ = FootLocation.Z;
        return FVector::ZeroVector;
    }
    
    FVector TraceStart = FootLocation + FVector(0, 0, 20);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    if (OwnerCharacter)
    {
        QueryParams.AddIgnoredActor(OwnerCharacter);
    }
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        OutImpactZ = HitResult.Location.Z;
        return HitResult.Normal;
    }
    
    OutImpactZ = FootLocation.Z;
    return FVector::UpVector;
}