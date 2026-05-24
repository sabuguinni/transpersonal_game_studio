#include "Anim_CharacterAnimationBlueprint.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_CharacterAnimationBlueprint::UAnim_CharacterAnimationBlueprint()
{
    // Initialize default values
    MovementSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    SprintSpeedThreshold = 600.0f;
    DirectionChangeThreshold = 45.0f;
    StateTransitionTime = 0.2f;
    CurrentStateTime = 0.0f;
    
    LastSpeed = 0.0f;
    LastState = EAnim_CharacterState::Idle;
    LastVelocity = FVector::ZeroVector;
    
    // Initialize animation data
    AnimationData = FAnim_CharacterAnimationData();
}

void UAnim_CharacterAnimationBlueprint::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwnerCharacter = Cast<ATranspersonalCharacter>(GetOwningActor());
    
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        UE_LOG(LogTemp, Log, TEXT("Animation Blueprint initialized for character: %s"), *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get TranspersonalCharacter reference in Animation Blueprint"));
    }
}

void UAnim_CharacterAnimationBlueprint::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update current state time
    CurrentStateTime += DeltaTimeX;
    
    // Update all animation data
    UpdateMovementData();
    UpdatePhysicsData();
    UpdateSurvivalData();
    UpdateEnvironmentalData();
    
    // Update animation state based on data
    UpdateAnimationState();
    
    // Update movement-specific animations
    UpdateMovementAnimation();
    
    // Update survival animations
    UpdateSurvivalAnimations();
    
    // Update environmental animations
    UpdateEnvironmentalAnimations();
}

void UAnim_CharacterAnimationBlueprint::UpdateAnimationState()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    EAnim_CharacterState NewState = AnimationData.CurrentState;
    
    // Determine new state based on current conditions
    if (AnimationData.HealthPercentage <= 0.0f)
    {
        NewState = EAnim_CharacterState::Dead;
    }
    else if (AnimationData.bIsInCombat)
    {
        NewState = EAnim_CharacterState::Combat;
    }
    else if (AnimationData.bIsInAir && !AnimationData.bIsGrounded)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            NewState = EAnim_CharacterState::Jumping;
        }
        else
        {
            NewState = EAnim_CharacterState::Falling;
        }
    }
    else if (AnimationData.Speed > SprintSpeedThreshold)
    {
        NewState = EAnim_CharacterState::Sprinting;
    }
    else if (AnimationData.Speed > RunSpeedThreshold)
    {
        NewState = EAnim_CharacterState::Running;
    }
    else if (AnimationData.Speed > MovementSpeedThreshold)
    {
        NewState = EAnim_CharacterState::Walking;
    }
    else
    {
        NewState = EAnim_CharacterState::Idle;
    }
    
    // Handle state transitions
    if (NewState != AnimationData.CurrentState)
    {
        if (CanTransitionToState(NewState))
        {
            EAnim_CharacterState OldState = AnimationData.CurrentState;
            AnimationData.CurrentState = NewState;
            HandleStateTransition(OldState, NewState);
        }
    }
}

void UAnim_CharacterAnimationBlueprint::SetCharacterState(EAnim_CharacterState NewState)
{
    if (CanTransitionToState(NewState))
    {
        EAnim_CharacterState OldState = AnimationData.CurrentState;
        AnimationData.CurrentState = NewState;
        HandleStateTransition(OldState, NewState);
        CurrentStateTime = 0.0f;
    }
}

void UAnim_CharacterAnimationBlueprint::UpdateMovementAnimation()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Calculate movement direction for blend space
    CalculateMovementDirection();
    
    // Update speed for blend space
    AnimationData.Speed = MovementComponent->Velocity.Size();
    
    // Calculate direction relative to character facing
    FVector Velocity = MovementComponent->Velocity;
    if (!Velocity.IsNearlyZero())
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        AnimationData.Direction = FMath::Atan2(FVector::DotProduct(Velocity, OwnerCharacter->GetActorRightVector()),
                                              FVector::DotProduct(Velocity, ForwardVector));
        AnimationData.Direction = FMath::RadiansToDegrees(AnimationData.Direction);
    }
    else
    {
        AnimationData.Direction = 0.0f;
    }
}

void UAnim_CharacterAnimationBlueprint::CalculateMovementDirection()
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    FVector Velocity = MovementComponent->Velocity;
    if (Velocity.IsNearlyZero())
    {
        AnimationData.MovementDirection = EAnim_MovementDirection::Forward;
        return;
    }
    
    // Get character's forward and right vectors
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    // Calculate dot products
    float ForwardDot = FVector::DotProduct(Velocity.GetSafeNormal(), ForwardVector);
    float RightDot = FVector::DotProduct(Velocity.GetSafeNormal(), RightVector);
    
    // Determine direction based on dot products
    if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
    {
        if (ForwardDot > 0.0f)
        {
            AnimationData.MovementDirection = (RightDot > 0.3f) ? EAnim_MovementDirection::ForwardRight :
                                            (RightDot < -0.3f) ? EAnim_MovementDirection::ForwardLeft :
                                            EAnim_MovementDirection::Forward;
        }
        else
        {
            AnimationData.MovementDirection = (RightDot > 0.3f) ? EAnim_MovementDirection::BackwardRight :
                                            (RightDot < -0.3f) ? EAnim_MovementDirection::BackwardLeft :
                                            EAnim_MovementDirection::Backward;
        }
    }
    else
    {
        AnimationData.MovementDirection = (RightDot > 0.0f) ? EAnim_MovementDirection::Right : EAnim_MovementDirection::Left;
    }
}

void UAnim_CharacterAnimationBlueprint::UpdateSurvivalAnimations()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update survival-related animation parameters
    // This would typically read from the character's survival stats
    // For now, we'll use placeholder logic
    
    // Health affects animation speed and posture
    if (AnimationData.HealthPercentage < 30.0f)
    {
        // Character is injured - slower animations, hunched posture
        if (AnimationData.CurrentState != EAnim_CharacterState::Dead && 
            AnimationData.CurrentState != EAnim_CharacterState::Injured)
        {
            SetCharacterState(EAnim_CharacterState::Injured);
        }
    }
    
    // Stamina affects movement speed
    if (AnimationData.StaminaPercentage < 20.0f)
    {
        // Force slower movement when exhausted
        if (AnimationData.CurrentState == EAnim_CharacterState::Running ||
            AnimationData.CurrentState == EAnim_CharacterState::Sprinting)
        {
            SetCharacterState(EAnim_CharacterState::Walking);
        }
    }
    
    // Fear affects posture and movement
    if (AnimationData.FearLevel > 70.0f)
    {
        // Tense, alert posture
        // This would modify the animation blend weights
    }
}

void UAnim_CharacterAnimationBlueprint::PlaySurvivalAction(ESurvivalActionType ActionType)
{
    if (SurvivalMontages.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No survival montages configured"));
        return;
    }
    
    // Map action type to montage index
    int32 MontageIndex = static_cast<int32>(ActionType);
    if (SurvivalMontages.IsValidIndex(MontageIndex) && SurvivalMontages[MontageIndex])
    {
        Montage_Play(SurvivalMontages[MontageIndex]);
        SetCharacterState(EAnim_CharacterState::Crafting); // or appropriate state
    }
}

void UAnim_CharacterAnimationBlueprint::EnterCombatMode()
{
    AnimationData.bIsInCombat = true;
    SetCharacterState(EAnim_CharacterState::Combat);
    
    if (CombatMontage)
    {
        Montage_Play(CombatMontage);
    }
}

void UAnim_CharacterAnimationBlueprint::ExitCombatMode()
{
    AnimationData.bIsInCombat = false;
    
    if (CombatMontage && Montage_IsPlaying(CombatMontage))
    {
        Montage_Stop(0.2f, CombatMontage);
    }
    
    // Return to appropriate movement state
    UpdateAnimationState();
}

void UAnim_CharacterAnimationBlueprint::PlayAttackAnimation(int32 AttackIndex)
{
    if (CombatMontage)
    {
        FString SectionName = FString::Printf(TEXT("Attack_%d"), AttackIndex);
        Montage_JumpToSection(FName(*SectionName), CombatMontage);
    }
}

void UAnim_CharacterAnimationBlueprint::UpdateEnvironmentalAnimations()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update terrain adaptation
    AdaptToTerrain();
    
    // Update environmental factors
    // This would read from environmental systems
}

void UAnim_CharacterAnimationBlueprint::AdaptToTerrain()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Perform line trace to detect terrain slope
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        FVector SurfaceNormal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector));
        AnimationData.TerrainSlope = FMath::RadiansToDegrees(SlopeAngle);
        
        // Determine if terrain is uneven
        AnimationData.bIsOnUnevenTerrain = AnimationData.TerrainSlope > 15.0f;
    }
}

void UAnim_CharacterAnimationBlueprint::PlayMontageByName(const FString& MontageName)
{
    // This would typically look up montages by name from a data table or asset registry
    UE_LOG(LogTemp, Log, TEXT("Attempting to play montage: %s"), *MontageName);
}

void UAnim_CharacterAnimationBlueprint::StopAllMontages()
{
    Montage_Stop(0.2f);
}

void UAnim_CharacterAnimationBlueprint::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    FVector CurrentVelocity = MovementComponent->Velocity;
    AnimationData.Speed = CurrentVelocity.Size();
    
    // Store for next frame comparison
    LastVelocity = CurrentVelocity;
    LastSpeed = AnimationData.Speed;
}

void UAnim_CharacterAnimationBlueprint::UpdatePhysicsData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    AnimationData.bIsInAir = MovementComponent->IsFalling();
    AnimationData.bIsGrounded = !AnimationData.bIsInAir;
}

void UAnim_CharacterAnimationBlueprint::UpdateSurvivalData()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // This would typically read from the character's survival component
    // For now, using placeholder values
    AnimationData.HealthPercentage = 100.0f; // Would read from character
    AnimationData.StaminaPercentage = 100.0f; // Would read from character
    AnimationData.FearLevel = 0.0f; // Would read from character
}

void UAnim_CharacterAnimationBlueprint::UpdateEnvironmentalData()
{
    // Update environmental factors that affect animation
    // This would read from weather, time of day, etc.
}

bool UAnim_CharacterAnimationBlueprint::CanTransitionToState(EAnim_CharacterState NewState) const
{
    // Define transition rules
    switch (AnimationData.CurrentState)
    {
        case EAnim_CharacterState::Dead:
            return false; // Dead state is final
            
        case EAnim_CharacterState::Jumping:
            // Can only transition to falling or landing
            return (NewState == EAnim_CharacterState::Falling || 
                   NewState == EAnim_CharacterState::Idle ||
                   NewState == EAnim_CharacterState::Walking);
            
        case EAnim_CharacterState::Falling:
            // Can transition to any grounded state
            return (NewState != EAnim_CharacterState::Jumping);
            
        default:
            return true; // Most states can transition freely
    }
}

void UAnim_CharacterAnimationBlueprint::HandleStateTransition(EAnim_CharacterState OldState, EAnim_CharacterState NewState)
{
    UE_LOG(LogTemp, Log, TEXT("Animation state transition: %d -> %d"), 
           static_cast<int32>(OldState), static_cast<int32>(NewState));
    
    CurrentStateTime = 0.0f;
    LastState = OldState;
    
    // Handle specific transition logic
    switch (NewState)
    {
        case EAnim_CharacterState::Jumping:
            if (JumpMontage)
            {
                Montage_Play(JumpMontage);
            }
            break;
            
        case EAnim_CharacterState::Combat:
            if (CombatMontage)
            {
                Montage_Play(CombatMontage);
            }
            break;
            
        case EAnim_CharacterState::Dead:
            // Play death animation
            StopAllMontages();
            break;
            
        default:
            // Handle other transitions
            break;
    }
}