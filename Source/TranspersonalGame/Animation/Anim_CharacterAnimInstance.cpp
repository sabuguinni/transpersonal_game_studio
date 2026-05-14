#include "Anim_CharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UAnim_CharacterAnimInstance::UAnim_CharacterAnimInstance()
{
    // Set default animation parameters
    WalkSpeed = 150.0f;
    RunSpeed = 400.0f;
    CrouchSpeed = 100.0f;
    TurnRate = 90.0f;
    
    // Enable foot IK by default
    bEnableFootIK = true;
    
    // Initialize blend weights
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    CombatBlend = 0.0f;
    InjuryBlend = 0.0f;
}

void UAnim_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Instance initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Animation Instance could not find owner character"));
    }
}

void UAnim_CharacterAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update all animation data
    UpdateMovementData();
    UpdateMovementState();
    UpdateCombatState();
    UpdateSurvivalData();
    UpdateBlendWeights();
    
    if (bEnableFootIK)
    {
        UpdateFootIK();
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementData()
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Get movement data
    MovementData.Velocity = CharacterMovement->Velocity;
    MovementData.Speed = MovementData.Velocity.Size();
    MovementData.Direction = CalculateDirection(MovementData.Velocity, OwnerCharacter->GetActorRotation());
    MovementData.bIsInAir = CharacterMovement->IsFalling();
    MovementData.bIsCrouching = CharacterMovement->IsCrouching();
    MovementData.Acceleration = CharacterMovement->GetCurrentAcceleration().Size();
    
    // Calculate jump height if in air
    if (MovementData.bIsInAir)
    {
        FVector Location = OwnerCharacter->GetActorLocation();
        FHitResult HitResult;
        FVector Start = Location;
        FVector End = Location - FVector(0, 0, 2000.0f); // Trace down 20m
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic))
        {
            MovementData.JumpHeight = FMath::Abs(HitResult.Location.Z - Location.Z);
        }
    }
    else
    {
        MovementData.JumpHeight = 0.0f;
    }
}

void UAnim_CharacterAnimInstance::UpdateMovementState()
{
    MovementState = DetermineMovementState();
}

void UAnim_CharacterAnimInstance::UpdateCombatState()
{
    CombatState = DetermineCombatState();
}

void UAnim_CharacterAnimInstance::UpdateSurvivalData()
{
    // This would typically get data from a survival component
    // For now, use placeholder values
    SurvivalData.HealthPercent = 1.0f;
    SurvivalData.StaminaPercent = FMath::Clamp(1.0f - (MovementData.Speed / RunSpeed) * 0.5f, 0.0f, 1.0f);
    SurvivalData.FearLevel = 0.0f;
    SurvivalData.bIsInjured = SurvivalData.HealthPercent < 0.5f;
    SurvivalData.bIsExhausted = SurvivalData.StaminaPercent < 0.2f;
}

void UAnim_CharacterAnimInstance::UpdateBlendWeights()
{
    // Calculate blend weights based on speed
    float SpeedRatio = MovementData.Speed / RunSpeed;
    
    // Idle to walk blend
    IdleToWalkBlend = FMath::Clamp(MovementData.Speed / WalkSpeed, 0.0f, 1.0f);
    
    // Walk to run blend
    if (MovementData.Speed > WalkSpeed)
    {
        WalkToRunBlend = FMath::Clamp((MovementData.Speed - WalkSpeed) / (RunSpeed - WalkSpeed), 0.0f, 1.0f);
    }
    else
    {
        WalkToRunBlend = 0.0f;
    }
    
    // Combat blend
    CombatBlend = (CombatState == EAnim_CombatState::Combat) ? 1.0f : 0.0f;
    
    // Injury blend
    InjuryBlend = SurvivalData.bIsInjured ? (1.0f - SurvivalData.HealthPercent) : 0.0f;
}

void UAnim_CharacterAnimInstance::UpdateFootIK()
{
    CalculateFootIK();
}

void UAnim_CharacterAnimInstance::CalculateFootIK()
{
    if (!OwnerCharacter || !bEnableFootIK)
    {
        LeftFootIKOffset = 0.0f;
        RightFootIKOffset = 0.0f;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Get foot bone locations
    FVector LeftFootLocation = MeshComp->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootLocation = MeshComp->GetBoneLocation(TEXT("foot_r"));
    
    // Trace down from each foot to find ground
    FHitResult LeftHit, RightHit;
    FVector TraceStart, TraceEnd;
    float TraceDistance = 50.0f;
    
    // Left foot trace
    TraceStart = LeftFootLocation + FVector(0, 0, 20.0f);
    TraceEnd = LeftFootLocation - FVector(0, 0, TraceDistance);
    
    if (GetWorld()->LineTraceSingleByChannel(LeftHit, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        LeftFootIKOffset = LeftHit.Location.Z - LeftFootLocation.Z;
        LeftFootIKRotation = UKismetMathLibrary::MakeRotFromZ(LeftHit.Normal);
    }
    else
    {
        LeftFootIKOffset = 0.0f;
        LeftFootIKRotation = FRotator::ZeroRotator;
    }
    
    // Right foot trace
    TraceStart = RightFootLocation + FVector(0, 0, 20.0f);
    TraceEnd = RightFootLocation - FVector(0, 0, TraceDistance);
    
    if (GetWorld()->LineTraceSingleByChannel(RightHit, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        RightFootIKOffset = RightHit.Location.Z - RightFootLocation.Z;
        RightFootIKRotation = UKismetMathLibrary::MakeRotFromZ(RightHit.Normal);
    }
    else
    {
        RightFootIKOffset = 0.0f;
        RightFootIKRotation = FRotator::ZeroRotator;
    }
    
    // Clamp IK offsets to reasonable values
    LeftFootIKOffset = FMath::Clamp(LeftFootIKOffset, -20.0f, 20.0f);
    RightFootIKOffset = FMath::Clamp(RightFootIKOffset, -20.0f, 20.0f);
}

void UAnim_CharacterAnimInstance::TriggerJumpAnimation()
{
    // This would trigger a jump montage or set animation state
    UE_LOG(LogTemp, Log, TEXT("Jump animation triggered"));
}

void UAnim_CharacterAnimInstance::TriggerCombatAnimation(bool bEnterCombat)
{
    if (bEnterCombat)
    {
        CombatState = EAnim_CombatState::Combat;
    }
    else
    {
        CombatState = EAnim_CombatState::Peaceful;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat animation triggered: %s"), bEnterCombat ? TEXT("Enter") : TEXT("Exit"));
}

void UAnim_CharacterAnimInstance::TriggerInjuryAnimation(float InjurySeverity)
{
    SurvivalData.bIsInjured = InjurySeverity > 0.0f;
    SurvivalData.HealthPercent = FMath::Clamp(1.0f - InjurySeverity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Injury animation triggered with severity: %f"), InjurySeverity);
}

void UAnim_CharacterAnimInstance::TriggerFearAnimation(float FearIntensity)
{
    SurvivalData.FearLevel = FMath::Clamp(FearIntensity, 0.0f, 1.0f);
    
    if (FearIntensity > 0.5f)
    {
        CombatState = EAnim_CombatState::Alert;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Fear animation triggered with intensity: %f"), FearIntensity);
}

bool UAnim_CharacterAnimInstance::IsMoving() const
{
    return MovementData.Speed > 5.0f;
}

bool UAnim_CharacterAnimInstance::IsRunning() const
{
    return MovementData.Speed > WalkSpeed;
}

bool UAnim_CharacterAnimInstance::IsInCombat() const
{
    return CombatState == EAnim_CombatState::Combat || CombatState == EAnim_CombatState::Alert;
}

float UAnim_CharacterAnimInstance::GetMovementDirection() const
{
    return MovementData.Direction;
}

float UAnim_CharacterAnimInstance::CalculateDirection(const FVector& Velocity, const FRotator& Rotation) const
{
    if (Velocity.SizeSquared() < 0.1f)
    {
        return 0.0f;
    }
    
    FVector ForwardVector = Rotation.Vector();
    FVector NormalizedVelocity = Velocity.GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(ForwardVector, NormalizedVelocity);
    float CrossProduct = FVector::CrossProduct(ForwardVector, NormalizedVelocity).Z;
    
    return FMath::Atan2(CrossProduct, DotProduct) * (180.0f / PI);
}

EAnim_MovementState UAnim_CharacterAnimInstance::DetermineMovementState() const
{
    if (!OwnerCharacter || !CharacterMovement)
    {
        return EAnim_MovementState::Idle;
    }
    
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
    
    if (CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    if (MovementData.Speed < 5.0f)
    {
        return EAnim_MovementState::Idle;
    }
    else if (MovementData.Speed < RunSpeed * 0.7f)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

EAnim_CombatState UAnim_CharacterAnimInstance::DetermineCombatState() const
{
    // This would typically check for nearby threats, player input, etc.
    // For now, return current state or peaceful
    if (SurvivalData.FearLevel > 0.7f)
    {
        return EAnim_CombatState::Alert;
    }
    else if (SurvivalData.HealthPercent <= 0.0f)
    {
        return EAnim_CombatState::Dead;
    }
    else if (SurvivalData.bIsInjured)
    {
        return EAnim_CombatState::Wounded;
    }
    
    return EAnim_CombatState::Peaceful;
}