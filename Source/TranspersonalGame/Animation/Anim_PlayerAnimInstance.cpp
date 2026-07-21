#include "Anim_PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UAnim_PlayerAnimInstance::UAnim_PlayerAnimInstance()
{
    // Initialize default values
    MovementState = EAnim_PlayerMovementState::Idle;
    CombatState = EAnim_PlayerCombatState::None;
    EmotionalState = EAnim_PlayerEmotionalState::Calm;
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsInAir = false;
    bIsCrouching = false;
    Pitch = 0.0f;
    Yaw = 0.0f;
    Roll = 0.0f;
    
    bIsArmed = false;
    bIsAttacking = false;
    bIsBlocking = false;
    
    HealthPercentage = 1.0f;
    StaminaPercentage = 1.0f;
    FearLevel = 0.0f;
    HungerLevel = 0.0f;
    ThirstLevel = 0.0f;
    
    LeftFootIKOffset = 0.0f;
    RightFootIKOffset = 0.0f;
    LeftFootIKRotation = FRotator::ZeroRotator;
    RightFootIKRotation = FRotator::ZeroRotator;
    PelvisIKOffset = 0.0f;
    
    IdleToWalkBlend = 0.0f;
    WalkToRunBlend = 0.0f;
    CombatBlend = 0.0f;
    EmotionalBlend = 0.0f;
    
    OwnerCharacter = nullptr;
    MovementComponent = nullptr;
}

void UAnim_PlayerAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Cache character and movement component references
    OwnerCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UAnim_PlayerAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update all animation states
    UpdateMovementState();
    UpdateCombatState();
    UpdateSurvivalState();
    UpdateIKFootPlacement();
    UpdateAnimationBlending();
}

void UAnim_PlayerAnimInstance::UpdateMovementState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = OwnerCharacter->GetVelocity();
    Speed = Velocity.Size2D();
    
    // Calculate direction relative to actor forward
    if (Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        
        FVector NormalizedVelocity = Velocity.GetSafeNormal2D();
        
        float ForwardDot = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        float RightDot = FVector::DotProduct(RightVector, NormalizedVelocity);
        
        Direction = FMath::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
    else
    {
        Direction = 0.0f;
    }
    
    // Update movement state based on speed and conditions
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    
    if (bIsInAir)
    {
        MovementState = MovementComponent->Velocity.Z > 0 ? 
            EAnim_PlayerMovementState::Jumping : EAnim_PlayerMovementState::Falling;
    }
    else if (bIsCrouching)
    {
        MovementState = EAnim_PlayerMovementState::Crouching;
    }
    else if (Speed < 10.0f)
    {
        MovementState = EAnim_PlayerMovementState::Idle;
    }
    else if (Speed < WALK_SPEED_THRESHOLD)
    {
        MovementState = EAnim_PlayerMovementState::Walking;
    }
    else if (Speed < RUN_SPEED_THRESHOLD)
    {
        MovementState = EAnim_PlayerMovementState::Running;
    }
    
    // Update rotation values
    FRotator ActorRotation = OwnerCharacter->GetActorRotation();
    Pitch = ActorRotation.Pitch;
    Yaw = ActorRotation.Yaw;
    Roll = ActorRotation.Roll;
}

void UAnim_PlayerAnimInstance::UpdateCombatState()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Check for combat state transitions
    if (ShouldEnterCombatState() && CombatState == EAnim_PlayerCombatState::None)
    {
        CombatState = EAnim_PlayerCombatState::Armed;
        bIsArmed = true;
    }
    else if (ShouldExitCombatState() && CombatState != EAnim_PlayerCombatState::None)
    {
        CombatState = EAnim_PlayerCombatState::None;
        bIsArmed = false;
        bIsAttacking = false;
        bIsBlocking = false;
    }
    
    // Update combat flags based on input or game state
    // This would typically be driven by player input or AI decisions
    // For now, we'll use placeholder logic
    
    if (CombatState != EAnim_PlayerCombatState::None)
    {
        // Example: Check for attack input or AI attack decision
        // bIsAttacking = SomeInputCheck();
        // bIsBlocking = SomeOtherInputCheck();
        
        if (bIsAttacking)
        {
            CombatState = EAnim_PlayerCombatState::Attacking;
        }
        else if (bIsBlocking)
        {
            CombatState = EAnim_PlayerCombatState::Blocking;
        }
        else
        {
            CombatState = EAnim_PlayerCombatState::Armed;
        }
    }
}

void UAnim_PlayerAnimInstance::UpdateSurvivalState()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update survival stats - these would typically come from a survival component
    // For now, we'll use placeholder values that could be driven by game systems
    
    // Example: Get values from a survival component
    // if (USurvivalComponent* SurvivalComp = OwnerCharacter->FindComponentByClass<USurvivalComponent>())
    // {
    //     HealthPercentage = SurvivalComp->GetHealthPercentage();
    //     StaminaPercentage = SurvivalComp->GetStaminaPercentage();
    //     FearLevel = SurvivalComp->GetFearLevel();
    //     HungerLevel = SurvivalComp->GetHungerLevel();
    //     ThirstLevel = SurvivalComp->GetThirstLevel();
    // }
    
    // Calculate emotional state based on survival conditions
    EmotionalState = CalculateEmotionalState();
}

void UAnim_PlayerAnimInstance::UpdateIKFootPlacement()
{
    if (!OwnerCharacter || bIsInAir)
    {
        // Reset IK when in air
        LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, 0.0f, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
        RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, 0.0f, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
        PelvisIKOffset = FMath::FInterpTo(PelvisIKOffset, 0.0f, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
        return;
    }
    
    // Perform foot traces and update IK offsets
    float LeftFootOffset = 0.0f;
    float RightFootOffset = 0.0f;
    FRotator LeftFootRot = FRotator::ZeroRotator;
    FRotator RightFootRot = FRotator::ZeroRotator;
    
    PerformFootTrace(TEXT("foot_l"), LeftFootOffset, LeftFootRot);
    PerformFootTrace(TEXT("foot_r"), RightFootOffset, RightFootRot);
    
    // Smooth interpolation to target values
    LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, LeftFootOffset, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
    RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, RightFootOffset, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
    LeftFootIKRotation = FMath::RInterpTo(LeftFootIKRotation, LeftFootRot, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
    RightFootIKRotation = FMath::RInterpTo(RightFootIKRotation, RightFootRot, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
    
    // Update pelvis offset
    UpdatePelvisIK();
}

void UAnim_PlayerAnimInstance::UpdateAnimationBlending()
{
    float DeltaTime = GetDeltaSeconds();
    
    // Update movement blending
    float TargetIdleToWalk = (Speed > 10.0f && Speed < WALK_SPEED_THRESHOLD) ? Speed / WALK_SPEED_THRESHOLD : 0.0f;
    float TargetWalkToRun = (Speed > WALK_SPEED_THRESHOLD) ? 
        FMath::Clamp((Speed - WALK_SPEED_THRESHOLD) / (RUN_SPEED_THRESHOLD - WALK_SPEED_THRESHOLD), 0.0f, 1.0f) : 0.0f;
    
    IdleToWalkBlend = FMath::FInterpTo(IdleToWalkBlend, TargetIdleToWalk, DeltaTime, 8.0f);
    WalkToRunBlend = FMath::FInterpTo(WalkToRunBlend, TargetWalkToRun, DeltaTime, 8.0f);
    
    // Update combat blending
    float TargetCombatBlend = (CombatState != EAnim_PlayerCombatState::None) ? 1.0f : 0.0f;
    CombatBlend = FMath::FInterpTo(CombatBlend, TargetCombatBlend, DeltaTime, COMBAT_TRANSITION_SPEED);
    
    // Update emotional blending
    float TargetEmotionalBlend = 0.0f;
    switch (EmotionalState)
    {
        case EAnim_PlayerEmotionalState::Afraid:
        case EAnim_PlayerEmotionalState::Aggressive:
        case EAnim_PlayerEmotionalState::Exhausted:
        case EAnim_PlayerEmotionalState::Injured:
            TargetEmotionalBlend = 1.0f;
            break;
        case EAnim_PlayerEmotionalState::Alert:
            TargetEmotionalBlend = 0.5f;
            break;
        default:
            TargetEmotionalBlend = 0.0f;
            break;
    }
    
    EmotionalBlend = FMath::FInterpTo(EmotionalBlend, TargetEmotionalBlend, DeltaTime, EMOTIONAL_TRANSITION_SPEED);
}

float UAnim_PlayerAnimInstance::PerformFootTrace(FName SocketName, float& FootIKOffset, FRotator& FootIKRotation)
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return 0.0f;
    }
    
    // Get socket location
    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector StartLocation = SocketLocation;
    FVector EndLocation = StartLocation - FVector(0, 0, IK_TRACE_DISTANCE);
    
    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate IK offset
        float Distance = (StartLocation - HitResult.Location).Z;
        FootIKOffset = Distance - IK_TRACE_DISTANCE;
        
        // Calculate foot rotation based on surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
        
        FootIKRotation = FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
        
        return FootIKOffset;
    }
    
    FootIKOffset = 0.0f;
    FootIKRotation = FRotator::ZeroRotator;
    return 0.0f;
}

void UAnim_PlayerAnimInstance::UpdatePelvisIK()
{
    // Calculate pelvis offset based on foot IK offsets
    float TargetPelvisOffset = FMath::Min(LeftFootIKOffset, RightFootIKOffset);
    if (TargetPelvisOffset < 0.0f)
    {
        TargetPelvisOffset = FMath::Abs(TargetPelvisOffset) * 0.5f;
    }
    else
    {
        TargetPelvisOffset = 0.0f;
    }
    
    PelvisIKOffset = FMath::FInterpTo(PelvisIKOffset, TargetPelvisOffset, GetDeltaSeconds(), IK_INTERPOLATION_SPEED);
}

bool UAnim_PlayerAnimInstance::ShouldEnterCombatState() const
{
    // Placeholder logic - this would typically check for:
    // - Nearby enemies
    // - Player input (weapon drawn)
    // - Threat detection
    return false;
}

bool UAnim_PlayerAnimInstance::ShouldExitCombatState() const
{
    // Placeholder logic - this would typically check for:
    // - No nearby enemies
    // - Player input (weapon sheathed)
    // - Timeout after last threat
    return false;
}

EAnim_PlayerEmotionalState UAnim_PlayerAnimInstance::CalculateEmotionalState() const
{
    // Calculate emotional state based on survival conditions
    if (HealthPercentage < 0.3f)
    {
        return EAnim_PlayerEmotionalState::Injured;
    }
    
    if (StaminaPercentage < 0.2f)
    {
        return EAnim_PlayerEmotionalState::Exhausted;
    }
    
    if (FearLevel > 0.7f)
    {
        return EAnim_PlayerEmotionalState::Afraid;
    }
    
    if (FearLevel > 0.3f || HungerLevel > 0.6f || ThirstLevel > 0.6f)
    {
        return EAnim_PlayerEmotionalState::Alert;
    }
    
    // Check for aggressive state (could be based on combat readiness, recent combat, etc.)
    if (CombatState != EAnim_PlayerCombatState::None && FearLevel < 0.3f)
    {
        return EAnim_PlayerEmotionalState::Aggressive;
    }
    
    return EAnim_PlayerEmotionalState::Calm;
}

float UAnim_PlayerAnimInstance::CalculateAnimationSpeedMultiplier() const
{
    float SpeedMultiplier = 1.0f;
    
    // Apply survival-based speed modifiers
    SpeedMultiplier *= GetStaminaSpeedModifier();
    SpeedMultiplier *= GetHealthSpeedModifier();
    SpeedMultiplier *= GetFearSpeedModifier();
    
    return FMath::Clamp(SpeedMultiplier, 0.3f, 2.0f);
}

float UAnim_PlayerAnimInstance::GetStaminaSpeedModifier() const
{
    // Reduce animation speed when stamina is low
    if (StaminaPercentage < 0.3f)
    {
        return FMath::Lerp(0.6f, 1.0f, StaminaPercentage / 0.3f);
    }
    return 1.0f;
}

float UAnim_PlayerAnimInstance::GetHealthSpeedModifier() const
{
    // Reduce animation speed when health is low
    if (HealthPercentage < 0.5f)
    {
        return FMath::Lerp(0.7f, 1.0f, HealthPercentage / 0.5f);
    }
    return 1.0f;
}

float UAnim_PlayerAnimInstance::GetFearSpeedModifier() const
{
    // Increase animation speed when afraid (adrenaline effect)
    if (FearLevel > 0.5f)
    {
        return FMath::Lerp(1.0f, 1.3f, (FearLevel - 0.5f) / 0.5f);
    }
    return 1.0f;
}