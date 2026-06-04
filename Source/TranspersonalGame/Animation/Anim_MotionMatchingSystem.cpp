#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching parameters
    MotionMatchingThreshold = 0.1f;
    BlendTime = 0.2f;
    bUseMotionMatching = true;

    // Initialize foot IK settings
    bEnableFootIK = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;

    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    PreviousMotionData = FAnim_MotionData();

    // Initialize tribal animations
    TribalAnimations = FAnim_TribalAnimSet();
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Get component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }

    // Initialize tribal animation sets
    InitializeTribalAnimations();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }

    // Update motion data
    UpdateMotionData(DeltaTime);

    // Perform motion matching if enabled
    if (bUseMotionMatching)
    {
        FindBestMatchingAnimation();
    }

    // Update terrain adaptation
    if (bEnableFootIK)
    {
        PerformFootIK();
    }

    // Update tribal behavior animations
    UpdateTribalBehaviorAnimations();
}

void UAnim_MotionMatchingSystem::UpdateMotionData(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Store previous frame data
    PreviousMotionData = CurrentMotionData;

    // Get movement component
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    // Update velocity and speed
    CurrentMotionData.Velocity = MovementComp->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    CurrentMotionData.bIsMoving = CurrentMotionData.Speed > 5.0f;

    // Calculate acceleration
    if (DeltaTime > 0.0f)
    {
        CurrentMotionData.Acceleration = (CurrentMotionData.Velocity - PreviousMotionData.Velocity) / DeltaTime;
    }

    // Calculate movement direction
    if (CurrentMotionData.bIsMoving)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
    }

    // Update air state
    CurrentMotionData.bIsInAir = MovementComp->IsFalling();

    // Determine movement state
    UpdateMovementParameters(DeltaTime);
}

void UAnim_MotionMatchingSystem::UpdateMovementParameters(float DeltaTime)
{
    EAnim_MovementState NewState = EAnim_MovementState::Idle;

    if (CurrentMotionData.bIsInAir)
    {
        if (CurrentMotionData.Velocity.Z > 0)
        {
            NewState = EAnim_MovementState::Jumping;
        }
        else
        {
            NewState = EAnim_MovementState::Falling;
        }
    }
    else if (CurrentMotionData.bIsMoving)
    {
        if (CurrentMotionData.Speed > 300.0f)
        {
            NewState = EAnim_MovementState::Running;
        }
        else if (CurrentMotionData.Speed > 50.0f)
        {
            NewState = EAnim_MovementState::Walking;
        }
    }

    // Check for crouching
    if (OwnerCharacter && OwnerCharacter->GetCharacterMovement()->IsCrouching())
    {
        NewState = EAnim_MovementState::Crouching;
    }

    // Update state if changed
    if (NewState != CurrentMotionData.MovementState)
    {
        SetMovementState(NewState);
    }
}

void UAnim_MotionMatchingSystem::FindBestMatchingAnimation()
{
    if (!AnimInstance)
    {
        return;
    }

    // Get current tribal animation set
    FAnim_TribalAnimSet* CurrentAnimSet = &TribalAnimations;
    if (RoleAnimationSets.Contains(TribalAnimations.TribalRole))
    {
        CurrentAnimSet = &RoleAnimationSets[TribalAnimations.TribalRole];
    }

    // Select appropriate animation based on movement state
    UAnimMontage* TargetAnimation = nullptr;

    switch (CurrentMotionData.MovementState)
    {
        case EAnim_MovementState::Idle:
            TargetAnimation = CurrentAnimSet->IdleAnimation;
            break;
        case EAnim_MovementState::Walking:
            TargetAnimation = CurrentAnimSet->WalkAnimation;
            break;
        case EAnim_MovementState::Running:
            TargetAnimation = CurrentAnimSet->RunAnimation;
            break;
        case EAnim_MovementState::Crouching:
            // Use idle animation with different parameters for crouching
            TargetAnimation = CurrentAnimSet->IdleAnimation;
            break;
        default:
            TargetAnimation = CurrentAnimSet->IdleAnimation;
            break;
    }

    // Blend to new animation if different
    if (TargetAnimation && AnimInstance->GetCurrentActiveMontage() != TargetAnimation)
    {
        BlendToNewAnimation(TargetAnimation, BlendTime);
    }
}

void UAnim_MotionMatchingSystem::BlendToNewAnimation(UAnimMontage* NewAnimation, float InBlendTime)
{
    if (!AnimInstance || !NewAnimation)
    {
        return;
    }

    // Stop current montage if playing
    if (AnimInstance->IsAnyMontagePlaying())
    {
        AnimInstance->Montage_Stop(InBlendTime);
    }

    // Play new animation
    AnimInstance->Montage_Play(NewAnimation, 1.0f);
}

void UAnim_MotionMatchingSystem::SetTribalRole(EAnim_TribalRole NewRole)
{
    TribalAnimations.TribalRole = NewRole;
    
    // Initialize role-specific animations if not already done
    if (!RoleAnimationSets.Contains(NewRole))
    {
        FAnim_TribalAnimSet NewAnimSet;
        NewAnimSet.TribalRole = NewRole;
        RoleAnimationSets.Add(NewRole, NewAnimSet);
    }

    // Update current behavior animations
    HandleRoleSpecificAnimations();
}

void UAnim_MotionMatchingSystem::PlayTribalGesture(const FString& GestureName)
{
    if (!AnimInstance)
    {
        return;
    }

    // Find gesture animation by name
    FAnim_TribalAnimSet* CurrentAnimSet = &TribalAnimations;
    if (RoleAnimationSets.Contains(TribalAnimations.TribalRole))
    {
        CurrentAnimSet = &RoleAnimationSets[TribalAnimations.TribalRole];
    }

    // Play first available gesture animation (in real implementation, would match by name)
    if (CurrentAnimSet->GestureAnimations.Num() > 0)
    {
        UAnimMontage* GestureAnim = CurrentAnimSet->GestureAnimations[0];
        if (GestureAnim)
        {
            AnimInstance->Montage_Play(GestureAnim, 1.0f);
        }
    }
}

void UAnim_MotionMatchingSystem::PlayWorkAnimation()
{
    FAnim_TribalAnimSet* CurrentAnimSet = &TribalAnimations;
    if (RoleAnimationSets.Contains(TribalAnimations.TribalRole))
    {
        CurrentAnimSet = &RoleAnimationSets[TribalAnimations.TribalRole];
    }

    if (CurrentAnimSet->WorkAnimation)
    {
        BlendToNewAnimation(CurrentAnimSet->WorkAnimation, BlendTime);
    }
}

void UAnim_MotionMatchingSystem::PlayCombatAnimation()
{
    FAnim_TribalAnimSet* CurrentAnimSet = &TribalAnimations;
    if (RoleAnimationSets.Contains(TribalAnimations.TribalRole))
    {
        CurrentAnimSet = &RoleAnimationSets[TribalAnimations.TribalRole];
    }

    if (CurrentAnimSet->CombatAnimation)
    {
        BlendToNewAnimation(CurrentAnimSet->CombatAnimation, BlendTime);
    }
}

void UAnim_MotionMatchingSystem::SetMovementState(EAnim_MovementState NewState)
{
    if (CurrentMotionData.MovementState != NewState)
    {
        CurrentMotionData.MovementState = NewState;
        SmoothTransition(NewState);
    }
}

void UAnim_MotionMatchingSystem::UpdateTerrainAdaptation()
{
    if (bEnableFootIK)
    {
        PerformFootIK();
    }
}

void UAnim_MotionMatchingSystem::EnableFootIK(bool bEnable)
{
    bEnableFootIK = bEnable;
}

void UAnim_MotionMatchingSystem::InitializeTribalAnimations()
{
    // Initialize default tribal animation sets for each role
    TArray<EAnim_TribalRole> TribalRoles = {
        EAnim_TribalRole::Elder,
        EAnim_TribalRole::Hunter,
        EAnim_TribalRole::Gatherer,
        EAnim_TribalRole::Crafter,
        EAnim_TribalRole::Scout,
        EAnim_TribalRole::Warrior,
        EAnim_TribalRole::Shaman
    };

    for (EAnim_TribalRole Role : TribalRoles)
    {
        FAnim_TribalAnimSet AnimSet;
        AnimSet.TribalRole = Role;
        
        // In a real implementation, would load specific animations for each role
        // For now, initialize empty sets that can be populated via Blueprint or data assets
        
        RoleAnimationSets.Add(Role, AnimSet);
    }
}

void UAnim_MotionMatchingSystem::CalculateMotionData(float DeltaTime)
{
    // Advanced motion calculation for better animation matching
    if (!OwnerCharacter)
    {
        return;
    }

    // Calculate velocity-based parameters
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    float VelocityChange = (CurrentVelocity - PreviousMotionData.Velocity).Size();
    
    // Update motion smoothing
    CurrentMotionData.Velocity = FMath::VInterpTo(PreviousMotionData.Velocity, CurrentVelocity, DeltaTime, 10.0f);
}

void UAnim_MotionMatchingSystem::PerformFootIK()
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }

    // Perform line traces for foot IK
    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    FVector TraceStart = ActorLocation;
    FVector TraceEnd = ActorLocation - FVector(0, 0, FootIKTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    // Trace for ground
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    if (bHit)
    {
        // Calculate foot offset for terrain adaptation
        float GroundOffset = HitResult.Location.Z - ActorLocation.Z;
        
        // Apply foot IK offset (would be applied to animation blueprint in real implementation)
        // For now, just log the calculation
        UE_LOG(LogTemp, Log, TEXT("Foot IK Offset: %f"), GroundOffset);
    }
}

float UAnim_MotionMatchingSystem::CalculateAnimationScore(UAnimMontage* Animation)
{
    // Calculate how well an animation matches current motion data
    if (!Animation)
    {
        return 0.0f;
    }

    float Score = 1.0f;

    // Factor in speed matching
    // In real implementation, would compare animation's expected speed with current speed
    
    // Factor in direction matching
    // In real implementation, would compare animation's movement direction
    
    // Factor in acceleration matching
    // In real implementation, would compare animation's acceleration profile

    return Score;
}

void UAnim_MotionMatchingSystem::SmoothTransition(EAnim_MovementState NewState)
{
    // Handle smooth transitions between movement states
    float TransitionTime = BlendTime;

    switch (NewState)
    {
        case EAnim_MovementState::Running:
            TransitionTime = 0.1f; // Quick transition to running
            break;
        case EAnim_MovementState::Jumping:
            TransitionTime = 0.05f; // Very quick transition for jumping
            break;
        case EAnim_MovementState::Landing:
            TransitionTime = 0.15f; // Slightly longer for landing
            break;
        default:
            TransitionTime = BlendTime;
            break;
    }

    // Apply the transition (would be handled by animation blueprint in real implementation)
}

void UAnim_MotionMatchingSystem::UpdateTribalBehaviorAnimations()
{
    // Update animations based on tribal behavior context
    if (!OwnerCharacter)
    {
        return;
    }

    // Check for context-specific animations
    HandleRoleSpecificAnimations();
}

void UAnim_MotionMatchingSystem::HandleRoleSpecificAnimations()
{
    // Handle role-specific animation behaviors
    switch (TribalAnimations.TribalRole)
    {
        case EAnim_TribalRole::Elder:
            // Elders move slower, more deliberate
            break;
        case EAnim_TribalRole::Hunter:
            // Hunters are more alert, ready for action
            break;
        case EAnim_TribalRole::Gatherer:
            // Gatherers have searching, collecting motions
            break;
        case EAnim_TribalRole::Crafter:
            // Crafters have precise, focused movements
            break;
        case EAnim_TribalRole::Scout:
            // Scouts are agile, watchful
            break;
        case EAnim_TribalRole::Warrior:
            // Warriors are strong, combat-ready
            break;
        case EAnim_TribalRole::Shaman:
            // Shamans have mystical, ceremonial movements
            break;
    }
}