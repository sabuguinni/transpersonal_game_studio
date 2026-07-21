#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Initialize default values
    WalkSpeed = 150.0f;
    RunSpeed = 400.0f;
    JumpThreshold = 100.0f;
    AccelerationThreshold = 10.0f;
    bEnableFootIK = true;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation states
    UpdateMovementState(DeltaTime);
    UpdateWeaponState(DeltaTime);
    UpdateSurvivalState(DeltaTime);
    UpdateBlendSpaceParameters(DeltaTime);
    
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementState(float DeltaTime)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get movement data
    const FVector Velocity = OwnerCharacter->GetVelocity();
    const FVector Velocity2D = FVector(Velocity.X, Velocity.Y, 0.0f);
    
    // Update speed
    MovementState.Speed = Velocity2D.Size();
    
    // Update direction
    MovementState.Direction = CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
    
    // Update movement flags
    MovementState.bIsInAir = MovementComponent->IsFalling();
    MovementState.bIsAccelerating = MovementComponent->GetCurrentAcceleration().Size() > AccelerationThreshold;
    MovementState.bIsCrouching = MovementComponent->IsCrouching();
    MovementState.bIsRunning = MovementState.Speed > WalkSpeed && !MovementState.bIsCrouching;
    
    // Update aim rotation
    const FRotator AimRotation = OwnerCharacter->GetBaseAimRotation();
    const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);
    
    MovementState.AimPitch = DeltaRotation.Pitch;
    MovementState.AimYaw = DeltaRotation.Yaw;
}

void UAnim_CharacterAnimInstance::UpdateWeaponState(float DeltaTime)
{
    // TODO: Implement weapon state updates when weapon system is available
    // For now, set default values
    WeaponState.bIsHoldingWeapon = false;
    WeaponState.WeaponType = EWeaponType::None;
    WeaponState.bIsAttacking = false;
    WeaponState.bIsBlocking = false;
    WeaponState.AttackComboIndex = 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateSurvivalState(float DeltaTime)
{
    // TODO: Implement survival state updates when survival system is available
    // For now, set default healthy values
    SurvivalState.HealthPercentage = 1.0f;
    SurvivalState.StaminaPercentage = 1.0f;
    SurvivalState.FearLevel = 0.0f;
    SurvivalState.bIsInjured = false;
    SurvivalState.bIsExhausted = false;
    SurvivalState.bIsTerrorized = false;
}

void UAnim_CharacterAnimInstance::UpdateBlendSpaceParameters(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Calculate blend space coordinates based on movement
    const FVector Velocity = OwnerCharacter->GetVelocity();
    const FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    
    // Transform velocity to local space
    const FVector LocalVelocity = ActorRotation.UnrotateVector(Velocity);
    
    // Normalize to blend space range (-1 to 1)
    const float MaxSpeed = MovementState.bIsRunning ? RunSpeed : WalkSpeed;
    
    if (MaxSpeed > 0.0f)
    {
        BlendSpaceX = FMath::Clamp(LocalVelocity.Y / MaxSpeed, -1.0f, 1.0f);
        BlendSpaceY = FMath::Clamp(LocalVelocity.X / MaxSpeed, -1.0f, 1.0f);
    }
    else
    {
        BlendSpaceX = 0.0f;
        BlendSpaceY = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || MovementState.bIsInAir)
    {
        // Reset IK when in air
        LeftFootIKOffset = 0.0f;
        RightFootIKOffset = 0.0f;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        return;
    }
    
    // Get foot socket locations
    const FVector LeftFootLocation = GetFootLocation(TEXT("foot_l"));
    const FVector RightFootLocation = GetFootLocation(TEXT("foot_r"));
    
    // Perform foot traces
    FRotator LeftFootRotation;
    FRotator RightFootRotation;
    
    const float LeftOffset = PerformFootTrace(LeftFootLocation, LeftFootRotation);
    const float RightOffset = PerformFootTrace(RightFootLocation, RightFootRotation);
    
    // Smooth IK values
    const float InterpSpeed = 15.0f;
    LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, LeftOffset, DeltaTime, InterpSpeed);
    RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, RightOffset, DeltaTime, InterpSpeed);
    
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, LeftFootRotation, DeltaTime, InterpSpeed);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RightFootRotation, DeltaTime, InterpSpeed);
}

float UAnim_CharacterAnimInstance::CalculateDirection(const FVector& Velocity, const FRotator& Rotation) const
{
    if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }
    
    const FVector ForwardVector = Rotation.Vector();
    const FVector VelocityNormalized = Velocity.GetSafeNormal();
    
    const float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
    const float CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized).Z;
    
    return FMath::RadiansToDegrees(FMath::Atan2(CrossProduct, DotProduct));
}

bool UAnim_CharacterAnimInstance::IsMovingOnGround() const
{
    return MovementComponent && MovementComponent->IsMovingOnGround();
}

FVector UAnim_CharacterAnimInstance::GetFootLocation(const FName& SocketName) const
{
    if (!OwnerCharacter || !OwnerCharacter->GetMesh())
    {
        return FVector::ZeroVector;
    }
    
    return OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
}

float UAnim_CharacterAnimInstance::PerformFootTrace(const FVector& FootLocation, FRotator& OutRotation) const
{
    if (!OwnerCharacter)
    {
        OutRotation = FRotator::ZeroRotator;
        return 0.0f;
    }
    
    const UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        OutRotation = FRotator::ZeroRotator;
        return 0.0f;
    }
    
    // Setup trace parameters
    const float TraceDistance = 50.0f;
    const FVector StartLocation = FootLocation + FVector(0.0f, 0.0f, 20.0f);
    const FVector EndLocation = FootLocation - FVector(0.0f, 0.0f, TraceDistance);
    
    // Perform line trace
    FHitResult HitResult;
    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility
    );
    
    if (bHit)
    {
        // Calculate offset
        const float Offset = HitResult.Location.Z - FootLocation.Z;
        
        // Calculate rotation from surface normal
        const FVector UpVector = FVector::UpVector;
        const FVector SurfaceNormal = HitResult.Normal;
        
        const FVector RightVector = FVector::CrossProduct(UpVector, SurfaceNormal).GetSafeNormal();
        const FVector ForwardVector = FVector::CrossProduct(SurfaceNormal, RightVector).GetSafeNormal();
        
        OutRotation = FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
        
        return FMath::Clamp(Offset, -25.0f, 25.0f);
    }
    
    OutRotation = FRotator::ZeroRotator;
    return 0.0f;
}