#include "Anim_TribalCharacterShowcase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_TribalCharacterShowcase::UAnim_TribalCharacterShowcase()
{
    // Initialize animation data
    AnimData = FAnim_TribalAnimationData();
    
    // Initialize references
    OwningCharacter = nullptr;
    MovementComponent = nullptr;
    
    // Initialize animation assets to null (will be set in Blueprint or by designer)
    TribalLocomotionBlendSpace = nullptr;
    IdleMontage = nullptr;
    SpearThrowMontage = nullptr;
    ToolCraftingMontage = nullptr;
    GatheringMontage = nullptr;
    ClimbingMontage = nullptr;
    
    // Set default animation control values
    WalkSpeedThreshold = 50.0f;
    RunSpeedThreshold = 300.0f;
    AnimationBlendSpeed = 5.0f;
    
    // Set tribal behavior defaults
    CautiousMovementMultiplier = 0.8f;
    SurvivalStanceIntensity = 1.0f;
    bShowTribalCharacteristics = true;
    
    // Initialize internal state
    PreviousMovementState = EAnim_TribalMovementState::Idle;
    PreviousActionState = EAnim_TribalActionState::None;
    StateTransitionTimer = 0.0f;
    bIsTransitioning = false;
    
    // Initialize animation blending
    CurrentBlendWeight = 0.0f;
    TargetBlendWeight = 0.0f;
    
    // Initialize tribal behavior timers
    CautiousMovementTimer = 0.0f;
    EnvironmentalCheckTimer = 0.0f;
    SurvivalStanceTimer = 0.0f;
}

void UAnim_TribalCharacterShowcase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Get character reference
    OwningCharacter = Cast<ACharacter>(GetOwningActor());
    if (OwningCharacter)
    {
        // Get movement component
        MovementComponent = OwningCharacter->GetCharacterMovement();
        
        UE_LOG(LogTemp, Log, TEXT("Tribal Animation Showcase initialized for character: %s"), 
               *OwningCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Tribal Animation Showcase: No valid character found"));
    }
}

void UAnim_TribalCharacterShowcase::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update movement data
    UpdateMovementData();
    
    // Update animation states
    UpdateAnimationStates();
    
    // Update tribal-specific behavior
    UpdateTribalBehavior();
    
    // Blend animation states
    BlendAnimationStates(DeltaTimeX);
    
    // Update timers
    StateTransitionTimer += DeltaTimeX;
    CautiousMovementTimer += DeltaTimeX;
    EnvironmentalCheckTimer += DeltaTimeX;
    SurvivalStanceTimer += DeltaTimeX;
}

void UAnim_TribalCharacterShowcase::UpdateMovementData()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    AnimData.Speed = Velocity.Size();
    
    // Calculate movement direction relative to character forward
    if (AnimData.Speed > 0.1f)
    {
        FVector ForwardVector = OwningCharacter->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        AnimData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityNormalized)));
        
        // Determine if moving left or right
        FVector RightVector = OwningCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityNormalized);
        if (RightDot < 0.0f)
        {
            AnimData.Direction *= -1.0f;
        }
    }
    else
    {
        AnimData.Direction = 0.0f;
    }
    
    // Update movement state flags
    AnimData.bIsInAir = MovementComponent->IsFalling();
    AnimData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_TribalCharacterShowcase::UpdateAnimationStates()
{
    EAnim_TribalMovementState NewMovementState = AnimData.MovementState;
    
    // Determine movement state based on speed and conditions
    if (AnimData.bIsInAir)
    {
        if (MovementComponent->Velocity.Z > 0.0f)
        {
            NewMovementState = EAnim_TribalMovementState::Jumping;
        }
        else
        {
            NewMovementState = EAnim_TribalMovementState::Falling;
        }
    }
    else if (AnimData.bIsCrouching)
    {
        NewMovementState = EAnim_TribalMovementState::Crouching;
    }
    else if (AnimData.bIsInCombat)
    {
        NewMovementState = EAnim_TribalMovementState::Combat;
    }
    else if (AnimData.ActionState == EAnim_TribalActionState::ToolCrafting)
    {
        NewMovementState = EAnim_TribalMovementState::Crafting;
    }
    else if (AnimData.Speed < WalkSpeedThreshold)
    {
        NewMovementState = EAnim_TribalMovementState::Idle;
    }
    else if (AnimData.Speed < RunSpeedThreshold)
    {
        NewMovementState = EAnim_TribalMovementState::Walking;
    }
    else
    {
        NewMovementState = EAnim_TribalMovementState::Running;
    }
    
    // Apply tribal movement characteristics
    if (bShowTribalCharacteristics)
    {
        ProcessTribalMovement();
    }
    
    // Update movement state if changed
    if (NewMovementState != AnimData.MovementState)
    {
        SetMovementState(NewMovementState);
    }
}

void UAnim_TribalCharacterShowcase::UpdateTribalBehavior()
{
    if (!bShowTribalCharacteristics)
    {
        return;
    }
    
    // Process survival stance
    ProcessSurvivalStance();
    
    // Process environmental awareness
    ProcessEnvironmentalAwareness();
    
    // Update survival stats (simulated)
    if (AnimData.MovementState == EAnim_TribalMovementState::Running)
    {
        AnimData.StaminaLevel = FMath::Max(0.0f, AnimData.StaminaLevel - 0.5f);
    }
    else if (AnimData.MovementState == EAnim_TribalMovementState::Idle)
    {
        AnimData.StaminaLevel = FMath::Min(100.0f, AnimData.StaminaLevel + 0.2f);
    }
}

void UAnim_TribalCharacterShowcase::BlendAnimationStates(float DeltaTime)
{
    // Smooth blend weight transitions
    if (FMath::Abs(CurrentBlendWeight - TargetBlendWeight) > 0.01f)
    {
        CurrentBlendWeight = FMath::FInterpTo(CurrentBlendWeight, TargetBlendWeight, DeltaTime, AnimationBlendSpeed);
    }
    
    // Update transition state
    if (bIsTransitioning && StateTransitionTimer > 0.2f)
    {
        bIsTransitioning = false;
        StateTransitionTimer = 0.0f;
    }
}

void UAnim_TribalCharacterShowcase::ProcessTribalMovement()
{
    // Apply cautious movement characteristics
    if (CautiousMovementTimer > 2.0f && AnimData.MovementState == EAnim_TribalMovementState::Walking)
    {
        // Tribal characters move more cautiously, checking environment
        AnimData.Speed *= CautiousMovementMultiplier;
        CautiousMovementTimer = 0.0f;
    }
}

void UAnim_TribalCharacterShowcase::ProcessSurvivalStance()
{
    // Tribal characters maintain survival awareness
    if (SurvivalStanceTimer > 3.0f)
    {
        // Periodic survival stance adjustments
        SurvivalStanceIntensity = FMath::FRandRange(0.8f, 1.2f);
        SurvivalStanceTimer = 0.0f;
    }
}

void UAnim_TribalCharacterShowcase::ProcessEnvironmentalAwareness()
{
    // Tribal characters frequently check their environment
    if (EnvironmentalCheckTimer > 1.5f)
    {
        // Simulate environmental awareness behavior
        if (AnimData.MovementState == EAnim_TribalMovementState::Idle)
        {
            // Idle characters look around more frequently
            EnvironmentalCheckTimer = 0.0f;
        }
    }
}

// Public Animation Control Functions
void UAnim_TribalCharacterShowcase::PlayTribalAction(EAnim_TribalActionState ActionType)
{
    EAnim_TribalActionState OldState = AnimData.ActionState;
    AnimData.ActionState = ActionType;
    
    UAnimMontage* MontageToPlay = nullptr;
    
    switch (ActionType)
    {
        case EAnim_TribalActionState::SpearThrow:
            MontageToPlay = SpearThrowMontage;
            break;
        case EAnim_TribalActionState::ToolCrafting:
            MontageToPlay = ToolCraftingMontage;
            break;
        case EAnim_TribalActionState::Gathering:
            MontageToPlay = GatheringMontage;
            break;
        case EAnim_TribalActionState::Climbing:
            MontageToPlay = ClimbingMontage;
            break;
        default:
            break;
    }
    
    if (MontageToPlay)
    {
        Montage_Play(MontageToPlay);
        UE_LOG(LogTemp, Log, TEXT("Playing tribal action: %d"), (int32)ActionType);
    }
    
    OnActionStateChanged(OldState, ActionType);
}

void UAnim_TribalCharacterShowcase::StopTribalAction()
{
    EAnim_TribalActionState OldState = AnimData.ActionState;
    AnimData.ActionState = EAnim_TribalActionState::None;
    
    // Stop any playing montages
    if (Montage_IsPlaying(nullptr))
    {
        Montage_Stop(0.2f);
    }
    
    OnActionStateChanged(OldState, EAnim_TribalActionState::None);
}

void UAnim_TribalCharacterShowcase::SetMovementState(EAnim_TribalMovementState NewState)
{
    if (CanTransitionToState(NewState))
    {
        EAnim_TribalMovementState OldState = AnimData.MovementState;
        PreviousMovementState = OldState;
        AnimData.MovementState = NewState;
        
        bIsTransitioning = true;
        StateTransitionTimer = 0.0f;
        
        OnMovementStateChanged(OldState, NewState);
    }
}

void UAnim_TribalCharacterShowcase::SetCombatMode(bool bInCombat)
{
    AnimData.bIsInCombat = bInCombat;
    
    if (bInCombat)
    {
        SetMovementState(EAnim_TribalMovementState::Combat);
    }
}

// Animation State Queries
bool UAnim_TribalCharacterShowcase::IsPlayingTribalAction() const
{
    return AnimData.ActionState != EAnim_TribalActionState::None;
}

EAnim_TribalMovementState UAnim_TribalCharacterShowcase::GetCurrentMovementState() const
{
    return AnimData.MovementState;
}

EAnim_TribalActionState UAnim_TribalCharacterShowcase::GetCurrentActionState() const
{
    return AnimData.ActionState;
}

// Showcase Functions
void UAnim_TribalCharacterShowcase::DemonstrateTribalMovement()
{
    UE_LOG(LogTemp, Log, TEXT("Demonstrating tribal movement patterns"));
    
    // Cycle through movement states for demonstration
    static int32 DemoState = 0;
    
    switch (DemoState % 4)
    {
        case 0:
            SetMovementState(EAnim_TribalMovementState::Idle);
            break;
        case 1:
            SetMovementState(EAnim_TribalMovementState::Walking);
            break;
        case 2:
            SetMovementState(EAnim_TribalMovementState::Running);
            break;
        case 3:
            SetMovementState(EAnim_TribalMovementState::Crouching);
            break;
    }
    
    DemoState++;
}

void UAnim_TribalCharacterShowcase::DemonstrateSurvivalActions()
{
    UE_LOG(LogTemp, Log, TEXT("Demonstrating survival actions"));
    
    // Cycle through survival actions
    static int32 ActionDemo = 0;
    
    switch (ActionDemo % 3)
    {
        case 0:
            PlayTribalAction(EAnim_TribalActionState::ToolCrafting);
            break;
        case 1:
            PlayTribalAction(EAnim_TribalActionState::Gathering);
            break;
        case 2:
            PlayTribalAction(EAnim_TribalActionState::SpearThrow);
            break;
    }
    
    ActionDemo++;
}

void UAnim_TribalCharacterShowcase::DemonstrateHuntingBehavior()
{
    UE_LOG(LogTemp, Log, TEXT("Demonstrating hunting behavior"));
    
    SetCombatMode(true);
    PlayTribalAction(EAnim_TribalActionState::Hunting);
}

void UAnim_TribalCharacterShowcase::ResetToIdleState()
{
    SetMovementState(EAnim_TribalMovementState::Idle);
    StopTribalAction();
    SetCombatMode(false);
    
    UE_LOG(LogTemp, Log, TEXT("Reset to idle state"));
}

// Transition Helpers
bool UAnim_TribalCharacterShowcase::CanTransitionToState(EAnim_TribalMovementState NewState) const
{
    // Allow most transitions, but add some restrictions for realism
    if (bIsTransitioning && StateTransitionTimer < 0.1f)
    {
        return false; // Prevent rapid state changes
    }
    
    // Don't allow direct transition from jumping to running without touching ground
    if (AnimData.MovementState == EAnim_TribalMovementState::Jumping && 
        NewState == EAnim_TribalMovementState::Running && 
        AnimData.bIsInAir)
    {
        return false;
    }
    
    return true;
}

void UAnim_TribalCharacterShowcase::OnMovementStateChanged(EAnim_TribalMovementState OldState, EAnim_TribalMovementState NewState)
{
    UE_LOG(LogTemp, Log, TEXT("Tribal movement state changed from %d to %d"), (int32)OldState, (int32)NewState);
    
    // Adjust blend weights based on new state
    switch (NewState)
    {
        case EAnim_TribalMovementState::Idle:
            TargetBlendWeight = 0.0f;
            break;
        case EAnim_TribalMovementState::Walking:
            TargetBlendWeight = 0.5f;
            break;
        case EAnim_TribalMovementState::Running:
            TargetBlendWeight = 1.0f;
            break;
        default:
            TargetBlendWeight = 0.3f;
            break;
    }
}

void UAnim_TribalCharacterShowcase::OnActionStateChanged(EAnim_TribalActionState OldState, EAnim_TribalActionState NewState)
{
    UE_LOG(LogTemp, Log, TEXT("Tribal action state changed from %d to %d"), (int32)OldState, (int32)NewState);
    
    // Reset action-specific timers and states
    if (NewState == EAnim_TribalActionState::None)
    {
        // Return to normal movement behavior
        bIsTransitioning = false;
    }
}