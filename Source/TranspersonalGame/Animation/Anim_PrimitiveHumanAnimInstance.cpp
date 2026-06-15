#include "Anim_PrimitiveHumanAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_PrimitiveHumanAnimInstance::UAnim_PrimitiveHumanAnimInstance()
{
    // Initialize default values
    CurrentMovementState = EAnim_MovementState::Idle;
    CurrentCombatState = EAnim_CombatState::Unarmed;
    CurrentSurvivalAction = EAnim_SurvivalAction::None;
    
    // Initialize blend values
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    CombatBlend = 0.0f;
    SurvivalActionBlend = 0.0f;
    
    // Initialize survival stats
    FatigueLevel = 0.0f;
    HealthPercentage = 1.0f;
    FearLevel = 0.0f;
    bIsInjured = false;
    
    // IK settings
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 15.0f;
    
    // Blend speeds
    MovementBlendSpeed = 5.0f;
    CombatBlendSpeed = 8.0f;
    SurvivalBlendSpeed = 3.0f;
}

void UAnim_PrimitiveHumanAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        CharacterMovement = OwningCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Primitive Human Animation Instance initialized for: %s"), 
               *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get owning character in animation instance"));
    }
}

void UAnim_PrimitiveHumanAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    
    if (!OwningCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update all animation systems
    UpdateMovementData(DeltaTime);
    UpdateSurvivalStats();
    UpdateFootIK(DeltaTime);
    UpdateBlendValues(DeltaTime);
    UpdateStateTransitions();
}

void UAnim_PrimitiveHumanAnimInstance::UpdateMovementData(float DeltaTime)
{
    if (!CharacterMovement)
    {
        return;
    }
    
    // Get movement velocity
    MovementData.Velocity = CharacterMovement->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (MovementData.Speed > 1.0f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = MovementData.Velocity.GetSafeNormal();
        MovementData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwningCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            MovementData.Direction *= -1.0f;
        }
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update air state
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    
    // Calculate ground distance for landing prediction
    if (MovementData.bIsInAir)
    {
        FVector StartLocation = OwningCharacter->GetActorLocation();
        FVector EndLocation = StartLocation - FVector(0, 0, 1000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwningCharacter);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, 
                                                ECC_WorldStatic, QueryParams))
        {
            MovementData.GroundDistance = HitResult.Distance;
        }
        else
        {
            MovementData.GroundDistance = 1000.0f;
        }
    }
    else
    {
        MovementData.GroundDistance = 0.0f;
    }
    
    // Update movement state
    CurrentMovementState = DetermineMovementState();
}

EAnim_MovementState UAnim_PrimitiveHumanAnimInstance::DetermineMovementState()
{
    if (MovementData.bIsInAir)
    {
        if (MovementData.Velocity.Z > 0)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    if (MovementData.bIsCrouching)
    {
        return EAnim_MovementState::Crouching;
    }
    
    if (MovementData.Speed < 1.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < 300.0f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_PrimitiveHumanAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!OwningCharacter || MovementData.bIsInAir)
    {
        // Gradually disable IK when in air
        IKFootData.IKAlpha = FMath::FInterpTo(IKFootData.IKAlpha, 0.0f, DeltaTime, IKInterpSpeed);
        return;
    }
    
    // Enable IK when on ground
    IKFootData.IKAlpha = FMath::FInterpTo(IKFootData.IKAlpha, 1.0f, DeltaTime, IKInterpSpeed);
    
    // Get foot bone locations (approximate)
    FVector CharacterLocation = OwningCharacter->GetActorLocation();
    FVector LeftFootLocation = CharacterLocation + OwningCharacter->GetActorRightVector() * -15.0f;
    FVector RightFootLocation = CharacterLocation + OwningCharacter->GetActorRightVector() * 15.0f;
    
    // Calculate foot offsets and rotations
    CalculateFootOffset(LeftFootLocation, IKFootData.LeftFootOffset, IKFootData.LeftFootRotation, true);
    CalculateFootOffset(RightFootLocation, IKFootData.RightFootOffset, IKFootData.RightFootRotation, false);
    
    // Calculate pelvis offset (average of foot offsets)
    float TargetPelvisOffset = (IKFootData.LeftFootOffset.Z + IKFootData.RightFootOffset.Z) * 0.5f;
    IKFootData.PelvisOffset = FMath::FInterpTo(IKFootData.PelvisOffset, TargetPelvisOffset, DeltaTime, IKInterpSpeed);
    
    // Adjust foot offsets relative to pelvis
    IKFootData.LeftFootOffset.Z -= IKFootData.PelvisOffset;
    IKFootData.RightFootOffset.Z -= IKFootData.PelvisOffset;
}

void UAnim_PrimitiveHumanAnimInstance::CalculateFootOffset(FVector FootLocation, FVector& OutOffset, FRotator& OutRotation, bool bIsLeftFoot)
{
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Calculate offset
        float OffsetZ = HitResult.ImpactPoint.Z - FootLocation.Z;
        OutOffset = FVector(0, 0, OffsetZ);
        
        // Calculate rotation based on surface normal
        FVector SurfaceNormal = HitResult.ImpactNormal;
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        OutRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
        
        // Convert to relative rotation
        FRotator CharacterRotation = OwningCharacter->GetActorRotation();
        OutRotation = UKismetMathLibrary::NormalizedDeltaRotator(OutRotation, CharacterRotation);
    }
    else
    {
        OutOffset = FVector::ZeroVector;
        OutRotation = FRotator::ZeroRotator;
    }
}

FVector UAnim_PrimitiveHumanAnimInstance::TraceForGround(FVector FootLocation, float TraceDistance)
{
    FVector TraceStart = FootLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.ImpactPoint;
    }
    
    return FootLocation;
}

void UAnim_PrimitiveHumanAnimInstance::UpdateBlendValues(float DeltaTime)
{
    // Update movement blends
    float TargetIdleToWalk = (CurrentMovementState == EAnim_MovementState::Walking) ? 1.0f : 0.0f;
    IdleToWalkBlend = FMath::FInterpTo(IdleToWalkBlend, TargetIdleToWalk, DeltaTime, MovementBlendSpeed);
    
    float TargetWalkToRun = (CurrentMovementState == EAnim_MovementState::Running) ? 1.0f : 0.0f;
    WalkToRunBlend = FMath::FInterpTo(WalkToRunBlend, TargetWalkToRun, DeltaTime, MovementBlendSpeed);
    
    // Update combat blend
    float TargetCombatBlend = (CurrentCombatState != EAnim_CombatState::Unarmed) ? 1.0f : 0.0f;
    CombatBlend = FMath::FInterpTo(CombatBlend, TargetCombatBlend, DeltaTime, CombatBlendSpeed);
    
    // Update survival action blend
    float TargetSurvivalBlend = (CurrentSurvivalAction != EAnim_SurvivalAction::None) ? 1.0f : 0.0f;
    SurvivalActionBlend = FMath::FInterpTo(SurvivalActionBlend, TargetSurvivalBlend, DeltaTime, SurvivalBlendSpeed);
}

void UAnim_PrimitiveHumanAnimInstance::UpdateStateTransitions()
{
    // Handle automatic state transitions based on conditions
    
    // Exit survival actions if moving fast
    if (CurrentSurvivalAction != EAnim_SurvivalAction::None && MovementData.Speed > 100.0f)
    {
        SetSurvivalAction(EAnim_SurvivalAction::None);
    }
    
    // Auto-exit combat state if no threat nearby (simplified logic)
    if (CurrentCombatState != EAnim_CombatState::Unarmed && FearLevel < 0.1f)
    {
        // Could add timer here for more realistic combat exit
    }
}

void UAnim_PrimitiveHumanAnimInstance::UpdateSurvivalStats()
{
    // This would integrate with the survival system from other agents
    // For now, use placeholder values that could be set by external systems
    
    // Fatigue affects animation speed
    float MovementIntensity = MovementData.Speed / 600.0f; // Normalize to 0-1
    FatigueLevel = FMath::Clamp(FatigueLevel + MovementIntensity * 0.001f, 0.0f, 1.0f);
    
    // Health affects posture
    if (HealthPercentage < 0.3f)
    {
        bIsInjured = true;
    }
    else if (HealthPercentage > 0.7f)
    {
        bIsInjured = false;
    }
}

float UAnim_PrimitiveHumanAnimInstance::GetAnimationSpeedModifier() const
{
    float SpeedMod = 1.0f;
    
    // Reduce speed when fatigued
    SpeedMod *= FMath::Lerp(1.0f, 0.6f, FatigueLevel);
    
    // Reduce speed when injured
    if (bIsInjured)
    {
        SpeedMod *= 0.8f;
    }
    
    // Reduce speed when afraid
    SpeedMod *= FMath::Lerp(1.0f, 1.2f, FearLevel); // Fear can make you move faster
    
    return FMath::Clamp(SpeedMod, 0.3f, 1.5f);
}

void UAnim_PrimitiveHumanAnimInstance::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMovementState != NewState)
    {
        CurrentMovementState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Movement state changed to: %d"), (int32)NewState);
    }
}

void UAnim_PrimitiveHumanAnimInstance::SetCombatState(EAnim_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        UE_LOG(LogTemp, Log, TEXT("Combat state changed to: %d"), (int32)NewState);
    }
}

void UAnim_PrimitiveHumanAnimInstance::SetSurvivalAction(EAnim_SurvivalAction NewAction)
{
    if (CurrentSurvivalAction != NewAction)
    {
        CurrentSurvivalAction = NewAction;
        UE_LOG(LogTemp, Log, TEXT("Survival action changed to: %d"), (int32)NewAction);
    }
}

void UAnim_PrimitiveHumanAnimInstance::TriggerAttackAnimation()
{
    // This would trigger a montage or state machine transition
    UE_LOG(LogTemp, Log, TEXT("Attack animation triggered"));
    
    // Set temporary combat state
    SetCombatState(EAnim_CombatState::Attacking);
    
    // Could use a timer to reset state after animation completes
}

void UAnim_PrimitiveHumanAnimInstance::TriggerBlockAnimation()
{
    UE_LOG(LogTemp, Log, TEXT("Block animation triggered"));
    SetCombatState(EAnim_CombatState::Blocking);
}

void UAnim_PrimitiveHumanAnimInstance::TriggerDodgeAnimation()
{
    UE_LOG(LogTemp, Log, TEXT("Dodge animation triggered"));
    SetCombatState(EAnim_CombatState::Dodging);
}

void UAnim_PrimitiveHumanAnimInstance::SmoothBlendValue(float& CurrentValue, float TargetValue, float BlendSpeed, float DeltaTime)
{
    CurrentValue = FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, BlendSpeed);
}