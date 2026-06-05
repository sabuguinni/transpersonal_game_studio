#include "NPC_AnimationBehaviorIntegrator.h"
#include "NPC_DinosaurBehaviorIntegrator.h"
#include "Animation/Anim_MotionMatchingComponent.h"
#include "Animation/Anim_IKFootPlacementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_AnimationBehaviorIntegrator::UNPC_AnimationBehaviorIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for animation updates
    
    DefaultBlendTime = 0.5f;
    bEnableMotionMatching = true;
    bEnableIK = true;
    AnimationUpdateRate = 10.0f;
    
    LastAnimationUpdate = 0.0f;
    AnimationBlendTimer = 0.0f;
    bIsTransitioning = false;
    
    // Initialize default transition rules
    TransitionRules.Empty();
    
    // Idle to Movement transitions
    FNPC_AnimationTransitionRule IdleToWalk;
    IdleToWalk.FromState = ENPC_DinosaurBehaviorState::Idle;
    IdleToWalk.ToState = ENPC_DinosaurBehaviorState::Patrolling;
    IdleToWalk.TransitionDuration = 0.3f;
    TransitionRules.Add(IdleToWalk);
    
    // Combat transitions
    FNPC_AnimationTransitionRule PatrolToHunt;
    PatrolToHunt.FromState = ENPC_DinosaurBehaviorState::Patrolling;
    PatrolToHunt.ToState = ENPC_DinosaurBehaviorState::Hunting;
    PatrolToHunt.TransitionDuration = 0.8f;
    TransitionRules.Add(PatrolToHunt);
    
    // Alert transitions
    FNPC_AnimationTransitionRule IdleToAlert;
    IdleToAlert.FromState = ENPC_DinosaurBehaviorState::Idle;
    IdleToAlert.ToState = ENPC_DinosaurBehaviorState::Alert;
    IdleToAlert.TransitionDuration = 0.2f;
    TransitionRules.Add(IdleToAlert);
}

void UNPC_AnimationBehaviorIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    // Set initial animation state
    CurrentAnimationState.CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    CurrentAnimationState.EmotionalState = ENPC_DinosaurEmotionalState::Calm;
}

void UNPC_AnimationBehaviorIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update animation parameters at specified rate
    if (CurrentTime - LastAnimationUpdate >= (1.0f / AnimationUpdateRate))
    {
        UpdateAnimationParameters();
        LastAnimationUpdate = CurrentTime;
    }
    
    // Handle animation blending
    if (bIsTransitioning)
    {
        AnimationBlendTimer -= DeltaTime;
        if (AnimationBlendTimer <= 0.0f)
        {
            bIsTransitioning = false;
            ApplyBehaviorToAnimation();
        }
    }
}

void UNPC_AnimationBehaviorIntegrator::InitializeComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Find behavior integrator component
    BehaviorIntegrator = Owner->FindComponentByClass<UNPC_DinosaurBehaviorIntegrator>();
    
    // Find animation components
    MotionMatchingComponent = Owner->FindComponentByClass<UAnim_MotionMatchingComponent>();
    IKComponent = Owner->FindComponentByClass<UAnim_IKFootPlacementComponent>();
    
    // Find skeletal mesh and animation instance
    USkeletalMeshComponent* SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (SkeletalMesh)
    {
        AnimInstance = SkeletalMesh->GetAnimInstance();
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC_AnimationBehaviorIntegrator: Initialized components for %s"), *Owner->GetName());
}

void UNPC_AnimationBehaviorIntegrator::UpdateBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentAnimationState.CurrentBehaviorState == NewState)
    {
        return;
    }
    
    ENPC_DinosaurBehaviorState PreviousState = CurrentAnimationState.CurrentBehaviorState;
    
    if (CanTransitionToBehavior(NewState))
    {
        FNPC_AnimationTransitionRule* TransitionRule = FindTransitionRule(PreviousState, NewState);
        float BlendTime = TransitionRule ? TransitionRule->TransitionDuration : DefaultBlendTime;
        
        BlendToNewBehavior(NewState, BlendTime);
        
        UE_LOG(LogTemp, Log, TEXT("NPC_AnimationBehaviorIntegrator: Behavior state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

void UNPC_AnimationBehaviorIntegrator::UpdateEmotionalState(ENPC_DinosaurEmotionalState NewEmotionalState)
{
    if (CurrentAnimationState.EmotionalState != NewEmotionalState)
    {
        CurrentAnimationState.EmotionalState = NewEmotionalState;
        PlayEmotionalResponseAnimation(NewEmotionalState);
        
        UE_LOG(LogTemp, Log, TEXT("NPC_AnimationBehaviorIntegrator: Emotional state changed to %d"), 
               (int32)NewEmotionalState);
    }
}

void UNPC_AnimationBehaviorIntegrator::UpdateMovementParameters(float Speed, FVector Direction)
{
    CurrentAnimationState.MovementSpeed = Speed;
    
    // Update motion matching if enabled
    if (bEnableMotionMatching && MotionMatchingComponent)
    {
        // Configure motion matching based on current behavior and movement
        ConfigureMotionMatchingForBehavior(CurrentAnimationState.CurrentBehaviorState);
    }
}

void UNPC_AnimationBehaviorIntegrator::TriggerBehaviorAnimation(ENPC_DinosaurBehaviorState BehaviorState)
{
    switch (BehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Hunting:
            PlayHuntingAnimation();
            break;
        case ENPC_DinosaurBehaviorState::Territorial:
            PlayTerritorialAnimation();
            break;
        case ENPC_DinosaurBehaviorState::PackCoordination:
            PlayPackCoordinationAnimation();
            break;
        default:
            ApplyBehaviorToAnimation();
            break;
    }
}

void UNPC_AnimationBehaviorIntegrator::BlendToNewBehavior(ENPC_DinosaurBehaviorState NewBehavior, float BlendTime)
{
    CurrentAnimationState.CurrentBehaviorState = NewBehavior;
    AnimationBlendTimer = BlendTime;
    bIsTransitioning = true;
    
    // Configure animation systems for new behavior
    if (bEnableMotionMatching)
    {
        ConfigureMotionMatchingForBehavior(NewBehavior);
    }
    
    if (bEnableIK)
    {
        EnableIKForBehavior(NewBehavior);
    }
}

void UNPC_AnimationBehaviorIntegrator::ConfigureMotionMatchingForBehavior(ENPC_DinosaurBehaviorState BehaviorState)
{
    if (!MotionMatchingComponent)
    {
        return;
    }
    
    // Configure motion matching database based on behavior
    switch (BehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Hunting:
            // Use aggressive, predatory motion database
            UE_LOG(LogTemp, Log, TEXT("Configuring motion matching for hunting behavior"));
            break;
        case ENPC_DinosaurBehaviorState::Patrolling:
            // Use calm, exploratory motion database
            UE_LOG(LogTemp, Log, TEXT("Configuring motion matching for patrol behavior"));
            break;
        case ENPC_DinosaurBehaviorState::Alert:
            // Use tense, ready motion database
            UE_LOG(LogTemp, Log, TEXT("Configuring motion matching for alert behavior"));
            break;
        default:
            // Use default motion database
            break;
    }
}

void UNPC_AnimationBehaviorIntegrator::UpdateMotionMatchingDatabase()
{
    if (MotionMatchingComponent)
    {
        // Update motion matching database based on current behavior context
        ConfigureMotionMatchingForBehavior(CurrentAnimationState.CurrentBehaviorState);
    }
}

void UNPC_AnimationBehaviorIntegrator::EnableIKForBehavior(ENPC_DinosaurBehaviorState BehaviorState)
{
    if (!IKComponent)
    {
        return;
    }
    
    // Configure IK settings based on behavior
    switch (BehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Hunting:
            // Enable precise foot placement for stalking
            UE_LOG(LogTemp, Log, TEXT("Enabling IK for hunting behavior - precise foot placement"));
            break;
        case ENPC_DinosaurBehaviorState::Territorial:
            // Enable strong ground contact for dominance display
            UE_LOG(LogTemp, Log, TEXT("Enabling IK for territorial behavior - strong ground contact"));
            break;
        default:
            // Standard IK settings
            break;
    }
}

void UNPC_AnimationBehaviorIntegrator::UpdateIKTargets(const TArray<FVector>& FootTargets)
{
    if (IKComponent && FootTargets.Num() > 0)
    {
        // Update IK foot targets based on terrain and behavior
        for (int32 i = 0; i < FootTargets.Num(); ++i)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Updating IK target %d: %s"), i, *FootTargets[i].ToString());
        }
    }
}

void UNPC_AnimationBehaviorIntegrator::PlayHuntingAnimation()
{
    UE_LOG(LogTemp, Log, TEXT("Playing hunting animation sequence"));
    
    // Trigger hunting-specific animations
    CurrentAnimationState.bIsHunting = true;
    CurrentAnimationState.AggressionLevel = 0.8f;
    
    ApplyBehaviorToAnimation();
}

void UNPC_AnimationBehaviorIntegrator::PlayTerritorialAnimation()
{
    UE_LOG(LogTemp, Log, TEXT("Playing territorial display animation"));
    
    // Trigger territorial display animations
    CurrentAnimationState.AggressionLevel = 0.6f;
    
    ApplyBehaviorToAnimation();
}

void UNPC_AnimationBehaviorIntegrator::PlayPackCoordinationAnimation()
{
    UE_LOG(LogTemp, Log, TEXT("Playing pack coordination animation"));
    
    // Trigger pack coordination gestures
    ApplyBehaviorToAnimation();
}

void UNPC_AnimationBehaviorIntegrator::PlayEmotionalResponseAnimation(ENPC_DinosaurEmotionalState EmotionalState)
{
    switch (EmotionalState)
    {
        case ENPC_DinosaurEmotionalState::Aggressive:
            CurrentAnimationState.AggressionLevel = 1.0f;
            UE_LOG(LogTemp, Log, TEXT("Playing aggressive emotional response"));
            break;
        case ENPC_DinosaurEmotionalState::Fearful:
            CurrentAnimationState.AggressionLevel = 0.1f;
            UE_LOG(LogTemp, Log, TEXT("Playing fearful emotional response"));
            break;
        case ENPC_DinosaurEmotionalState::Curious:
            UE_LOG(LogTemp, Log, TEXT("Playing curious emotional response"));
            break;
        default:
            CurrentAnimationState.AggressionLevel = 0.3f;
            break;
    }
    
    ApplyBehaviorToAnimation();
}

void UNPC_AnimationBehaviorIntegrator::UpdateAnimationParameters()
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Update animation blueprint parameters based on current state
    // This would typically set animation blueprint variables
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Updating animation parameters: Speed=%.2f, Aggression=%.2f"), 
           CurrentAnimationState.MovementSpeed, CurrentAnimationState.AggressionLevel);
}

bool UNPC_AnimationBehaviorIntegrator::CanTransitionToBehavior(ENPC_DinosaurBehaviorState NewBehavior)
{
    // Check if transition is valid based on current state and rules
    FNPC_AnimationTransitionRule* Rule = FindTransitionRule(CurrentAnimationState.CurrentBehaviorState, NewBehavior);
    
    // Allow transition if rule exists or if not currently transitioning
    return (Rule != nullptr) || !bIsTransitioning;
}

FNPC_AnimationTransitionRule* UNPC_AnimationBehaviorIntegrator::FindTransitionRule(ENPC_DinosaurBehaviorState From, ENPC_DinosaurBehaviorState To)
{
    for (FNPC_AnimationTransitionRule& Rule : TransitionRules)
    {
        if (Rule.FromState == From && Rule.ToState == To)
        {
            return &Rule;
        }
    }
    return nullptr;
}

void UNPC_AnimationBehaviorIntegrator::ApplyBehaviorToAnimation()
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Apply current behavior state to animation system
    // This would typically involve setting animation blueprint variables
    // or triggering specific animation montages
    
    UE_LOG(LogTemp, Log, TEXT("Applying behavior state %d to animation system"), 
           (int32)CurrentAnimationState.CurrentBehaviorState);
}