#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    WalkSpeedThreshold = 150.0f;
    RunSpeedThreshold = 300.0f;
    DirectionDeadZone = 10.0f;
    bEnableFootIK = true;
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    
    LastSpeed = 0.0f;
    LastDirection = 0.0f;
    LastLeanAmount = 0.0f;
    HipOffset = 0.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Instance failed to find owner character"));
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    UpdateMovementValues(DeltaTime);
    
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementValues(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    float CurrentSpeed = Velocity.Size2D();
    
    // Smooth speed changes
    MovementState.Speed = FMath::FInterpTo(LastSpeed, CurrentSpeed, DeltaTime, 8.0f);
    LastSpeed = MovementState.Speed;
    
    // Calculate movement direction relative to character rotation
    if (MovementState.Speed > DirectionDeadZone)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
        
        float CurrentDirection = FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
        MovementState.Direction = FMath::FInterpTo(LastDirection, CurrentDirection, DeltaTime, 10.0f);
        LastDirection = MovementState.Direction;
        
        // Calculate lean amount for turning
        float TargetLean = FMath::Clamp(CrossProduct * MovementState.Speed * 0.01f, -45.0f, 45.0f);
        MovementState.LeanAmount = FMath::FInterpTo(LastLeanAmount, TargetLean, DeltaTime, 5.0f);
        LastLeanAmount = MovementState.LeanAmount;
    }
    else
    {
        MovementState.Direction = FMath::FInterpTo(LastDirection, 0.0f, DeltaTime, 10.0f);
        MovementState.LeanAmount = FMath::FInterpTo(LastLeanAmount, 0.0f, DeltaTime, 5.0f);
        LastDirection = MovementState.Direction;
        LastLeanAmount = MovementState.LeanAmount;
    }
    
    // Update movement state flags
    MovementState.bIsInAir = MovementComponent->IsFalling();
    MovementState.bIsCrouching = MovementComponent->IsCrouching();
    MovementState.bIsAccelerating = MovementComponent->GetCurrentAcceleration().SizeSquared() > 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || MovementState.bIsInAir)
    {
        // Reset IK when in air
        LeftFootIK.IKAlpha = FMath::FInterpTo(LeftFootIK.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        RightFootIK.IKAlpha = FMath::FInterpTo(RightFootIK.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        HipOffset = FMath::FInterpTo(HipOffset, 0.0f, DeltaTime, IKInterpSpeed);
        return;
    }
    
    // Calculate IK for both feet
    LeftFootIK = CalculateFootIK(FName("foot_l"), DeltaTime);
    RightFootIK = CalculateFootIK(FName("foot_r"), DeltaTime);
    
    // Calculate hip offset to keep character level
    float LeftOffset = LeftFootIK.FootLocation.Z;
    float RightOffset = RightFootIK.FootLocation.Z;
    float TargetHipOffset = FMath::Min(LeftOffset, RightOffset);
    
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaTime, IKInterpSpeed);
    
    // Adjust foot positions relative to hip offset
    LeftFootIK.FootLocation.Z -= HipOffset;
    RightFootIK.FootLocation.Z -= HipOffset;
}

FAnim_IKFootData UAnim_CharacterAnimInstance::CalculateFootIK(const FName& SocketName, float DeltaTime)
{
    FAnim_IKFootData FootData;
    
    if (!OwnerCharacter)
    {
        return FootData;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return FootData;
    }
    
    // Get foot socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector FootLocation = GetIKFootLocation(SocketLocation);
    
    // Perform line trace from foot to ground
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerCharacter);
    
    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitResult,
        true
    );
    
    if (bHit)
    {
        // Calculate foot adjustment
        float ImpactDistance = (SocketLocation.Z - HitResult.ImpactPoint.Z);
        FootData.FootLocation = FVector(0, 0, ImpactDistance);
        FootData.FootRotation = GetIKFootRotation(HitResult.ImpactPoint, HitResult.ImpactNormal);
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    }
    else
    {
        // No ground found, disable IK
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
    }
    
    return FootData;
}

FVector UAnim_CharacterAnimInstance::GetIKFootLocation(const FVector& FootLocation)
{
    // Transform foot location to world space for tracing
    if (OwnerCharacter)
    {
        return OwnerCharacter->GetActorTransform().TransformPosition(FootLocation);
    }
    return FootLocation;
}

FRotator UAnim_CharacterAnimInstance::GetIKFootRotation(const FVector& ImpactLocation, const FVector& ImpactNormal)
{
    // Calculate foot rotation to align with ground normal
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(ImpactNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, ImpactNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, ImpactNormal);
}