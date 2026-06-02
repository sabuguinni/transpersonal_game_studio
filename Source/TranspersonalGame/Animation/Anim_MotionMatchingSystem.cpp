#include "Anim_MotionMatchingSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS update rate
    
    // Initialize motion matching parameters
    MotionMatchingThreshold = 0.8f;
    StateTransitionTime = 0.3f;
    bEnableMotionMatching = true;
    
    // Initialize internal state
    PreviousState = EAnim_MotionState::Idle;
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
    LastUpdateTime = 0.0f;
    UpdateFrequency = 0.016f;
    bNeedsUpdate = true;
    
    // Initialize motion data
    CurrentMotionData = FAnim_MotionData();
    
    // Initialize blend space config
    BlendSpaceConfig = FAnim_BlendSpaceConfig();
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache character references
    CacheCharacterReferences();
    
    // Initialize animation assets
    InitializeAnimationAssets();
    
    UE_LOG(LogTemp, Warning, TEXT("Motion Matching System initialized for %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableMotionMatching || !OwnerCharacter)
    {
        return;
    }
    
    // Update at specified frequency
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateMotionData(DeltaTime);
        CalculateMotionState();
        UpdateBlendSpaces();
        HandleStateTransitions();
        
        LastUpdateTime = 0.0f;
    }
    
    // Update transition timer
    if (bIsTransitioning)
    {
        StateTransitionTimer += DeltaTime;
        if (StateTransitionTimer >= StateTransitionTime)
        {
            bIsTransitioning = false;
            StateTransitionTimer = 0.0f;
        }
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    // Update velocity and speed
    CurrentMotionData.Velocity = MovementComp->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (CurrentMotionData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityNormalized = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            CurrentMotionData.Direction *= -1.0f;
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }
    
    // Update air state
    CurrentMotionData.bIsInAir = MovementComp->IsFalling();
    
    // Update crouching state
    CurrentMotionData.bIsCrouching = MovementComp->IsCrouching();
}

void UAnim_MotionMatchingSystem::CalculateMotionState()
{
    EAnim_MotionState NewState = CurrentMotionData.CurrentState;
    
    // Determine state based on motion data
    if (CurrentMotionData.bIsInAir)
    {
        NewState = EAnim_MotionState::Jumping;
    }
    else if (CurrentMotionData.bIsCrouching)
    {
        NewState = EAnim_MotionState::Crouching;
    }
    else if (CurrentMotionData.Speed > 400.0f) // Running threshold
    {
        NewState = EAnim_MotionState::Running;
    }
    else if (CurrentMotionData.Speed > 50.0f) // Walking threshold
    {
        NewState = EAnim_MotionState::Walking;
    }
    else
    {
        NewState = EAnim_MotionState::Idle;
    }
    
    // Check if state should transition
    if (NewState != CurrentMotionData.CurrentState && ShouldTransitionToState(NewState))
    {
        SetMotionState(NewState);
    }
}

void UAnim_MotionMatchingSystem::SetMotionState(EAnim_MotionState NewState)
{
    if (NewState == CurrentMotionData.CurrentState)
    {
        return;
    }
    
    PreviousState = CurrentMotionData.CurrentState;
    CurrentMotionData.CurrentState = NewState;
    
    // Start transition
    bIsTransitioning = true;
    StateTransitionTimer = 0.0f;
    
    // Play appropriate montage if available
    if (StateMontages.Contains(NewState))
    {
        PlayMontage(NewState);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Motion state changed from %d to %d"), 
           (int32)PreviousState, (int32)NewState);
}

void UAnim_MotionMatchingSystem::UpdateBlendSpaces()
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Set blend space values based on current motion
    SetBlendSpaceValues(CurrentMotionData.Speed, CurrentMotionData.Direction);
}

void UAnim_MotionMatchingSystem::SetBlendSpaceValues(float Speed, float Direction)
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Normalize speed for blend space (0-600 units/sec -> 0-1)
    float NormalizedSpeed = FMath::Clamp(Speed / 600.0f, 0.0f, 1.0f);
    
    // Normalize direction (-180 to 180 degrees -> -1 to 1)
    float NormalizedDirection = FMath::Clamp(Direction / 180.0f, -1.0f, 1.0f);
    
    // Set animation blueprint variables (these would be exposed in the AnimBP)
    // AnimInstance->SetVariableValue(TEXT("Speed"), NormalizedSpeed);
    // AnimInstance->SetVariableValue(TEXT("Direction"), NormalizedDirection);
}

void UAnim_MotionMatchingSystem::PlayMontage(EAnim_MotionState State, float PlayRate)
{
    UAnimMontage* Montage = GetMontageForState(State);
    if (Montage && AnimInstance)
    {
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void UAnim_MotionMatchingSystem::StopMontage(float BlendOutTime)
{
    if (AnimInstance)
    {
        AnimInstance->Montage_Stop(BlendOutTime);
    }
}

void UAnim_MotionMatchingSystem::SetTribalRole(EAnim_TribalRole NewRole)
{
    CurrentMotionData.TribalRole = NewRole;
    
    // Update idle animation based on role
    UAnimSequence* RoleIdleAnim = GetIdleAnimationForRole(NewRole);
    if (RoleIdleAnim && AnimInstance)
    {
        // Set role-specific idle animation
        // This would typically be handled in the Animation Blueprint
    }
}

void UAnim_MotionMatchingSystem::PlayTribalGesture(EAnim_TribalRole Role)
{
    // Play role-specific gesture animation
    switch (Role)
    {
        case EAnim_TribalRole::Hunter:
            // Play hunting gesture (bow draw, spear throw, etc.)
            break;
        case EAnim_TribalRole::Gatherer:
            // Play gathering gesture (picking, collecting, etc.)
            break;
        case EAnim_TribalRole::Crafter:
            // Play crafting gesture (hammering, carving, etc.)
            break;
        case EAnim_TribalRole::Elder:
            // Play wisdom gesture (pointing, teaching, etc.)
            break;
        default:
            break;
    }
}

void UAnim_MotionMatchingSystem::PlayWorkAnimation(EAnim_TribalRole Role)
{
    // Play extended work animation based on role
    EAnim_MotionState WorkState = EAnim_MotionState::Crafting;
    
    switch (Role)
    {
        case EAnim_TribalRole::Hunter:
            WorkState = EAnim_MotionState::Combat;
            break;
        case EAnim_TribalRole::Gatherer:
            WorkState = EAnim_MotionState::Gathering;
            break;
        case EAnim_TribalRole::Crafter:
            WorkState = EAnim_MotionState::Crafting;
            break;
        default:
            WorkState = EAnim_MotionState::Idle;
            break;
    }
    
    SetMotionState(WorkState);
}

void UAnim_MotionMatchingSystem::PlayCommunicationGesture()
{
    // Play generic communication gesture
    // This could be waving, pointing, or tribal sign language
    PlayMontage(EAnim_MotionState::Idle, 1.0f);
}

void UAnim_MotionMatchingSystem::EnableMotionMatching(bool bEnable)
{
    bEnableMotionMatching = bEnable;
    
    if (!bEnable)
    {
        // Reset to idle state when disabled
        SetMotionState(EAnim_MotionState::Idle);
    }
}

EAnim_MotionState UAnim_MotionMatchingSystem::GetCurrentMotionState() const
{
    return CurrentMotionData.CurrentState;
}

FAnim_MotionData UAnim_MotionMatchingSystem::GetMotionData() const
{
    return CurrentMotionData;
}

void UAnim_MotionMatchingSystem::HandleStateTransitions()
{
    // Handle smooth transitions between states
    if (bIsTransitioning)
    {
        float TransitionAlpha = StateTransitionTimer / StateTransitionTime;
        TransitionAlpha = FMath::Clamp(TransitionAlpha, 0.0f, 1.0f);
        
        // Apply transition logic here
        // This would typically involve blending between animations
    }
}

bool UAnim_MotionMatchingSystem::ShouldTransitionToState(EAnim_MotionState NewState)
{
    // Prevent rapid state changes
    if (bIsTransitioning)
    {
        return false;
    }
    
    // State-specific transition rules
    switch (CurrentMotionData.CurrentState)
    {
        case EAnim_MotionState::Jumping:
            // Only allow transition out of jumping when landing
            return !CurrentMotionData.bIsInAir;
            
        case EAnim_MotionState::Combat:
            // Combat state has priority, harder to interrupt
            return NewState == EAnim_MotionState::Idle || NewState == EAnim_MotionState::Running;
            
        default:
            return true;
    }
}

float UAnim_MotionMatchingSystem::CalculateMotionScore(const FAnim_MotionData& TargetMotion)
{
    float Score = 0.0f;
    
    // Calculate similarity score between current and target motion
    float SpeedDiff = FMath::Abs(CurrentMotionData.Speed - TargetMotion.Speed);
    float DirectionDiff = FMath::Abs(CurrentMotionData.Direction - TargetMotion.Direction);
    
    // Normalize differences and calculate score
    Score = 1.0f - ((SpeedDiff / 600.0f) + (DirectionDiff / 180.0f)) * 0.5f;
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

void UAnim_MotionMatchingSystem::CacheCharacterReferences()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        
        if (SkeletalMeshComponent)
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
    }
}

void UAnim_MotionMatchingSystem::InitializeAnimationAssets()
{
    // Initialize default animation assets
    // In a real implementation, these would be loaded from content browser
    
    // Clear existing mappings
    StateMontages.Empty();
    RoleIdleAnimations.Empty();
    
    // Note: Asset loading would typically happen here
    // For now, we'll leave these empty and they can be set in Blueprint
    
    UE_LOG(LogTemp, Warning, TEXT("Animation assets initialized (assets should be set in Blueprint)"));
}

UAnimMontage* UAnim_MotionMatchingSystem::GetMontageForState(EAnim_MotionState State)
{
    if (StateMontages.Contains(State))
    {
        return StateMontages[State];
    }
    
    return nullptr;
}

UAnimSequence* UAnim_MotionMatchingSystem::GetIdleAnimationForRole(EAnim_TribalRole Role)
{
    if (RoleIdleAnimations.Contains(Role))
    {
        return RoleIdleAnimations[Role];
    }
    
    return nullptr;
}