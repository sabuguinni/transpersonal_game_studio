#include "NPC_AnimationBehaviorBridge.h"
#include "NPC_BaseBehaviorComponent.h"
#include "Animation/Anim_MotionMatchingSystem.h"
#include "Animation/Anim_IKFootPlacement.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

UNPC_AnimationBehaviorBridge::UNPC_AnimationBehaviorBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize animation data
    AnimationData.CurrentState = ENPC_AnimationBehaviorState::Idle;
    AnimationData.PreviousState = ENPC_AnimationBehaviorState::Idle;
    AnimationData.StateTransitionBlendTime = DefaultTransitionTime;
    AnimationData.MovementSpeed = 0.0f;
    AnimationData.MovementDirection = FVector::ZeroVector;
    AnimationData.EmotionalIntensity = 0.5f;
    AnimationData.bIsInCombat = false;
    AnimationData.bIsInPack = false;
    AnimationData.TerrainAdaptation = 0.0f;
}

void UNPC_AnimationBehaviorBridge::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponentReferences();
    
    // Set up initial animation state
    AnimationData.CurrentState = ENPC_AnimationBehaviorState::Idle;
    StateChangeTimer = 0.0f;
    bInStateTransition = false;
}

void UNPC_AnimationBehaviorBridge::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
        
    // Update animation parameters based on current behavior
    UpdateAnimationParameters(DeltaTime);
    
    // Handle state transitions
    if (bInStateTransition)
    {
        HandleStateTransition(DeltaTime);
    }
    
    // Synchronize with behavior component
    SynchronizeWithBehaviorComponent();
    
    // Update motion matching if enabled
    if (bUseMotionMatching && MotionMatchingSystem)
    {
        UpdateMotionMatchingData();
    }
    
    // Update IK foot placement if enabled
    if (bUseIKFootPlacement && IKFootSystem)
    {
        UpdateIKParameters();
    }
}

void UNPC_AnimationBehaviorBridge::UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState NewState, float Intensity)
{
    if (NewState != AnimationData.CurrentState)
    {
        TransitionToState(NewState, AnimationData.StateTransitionBlendTime);
    }
    
    AnimationData.EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UNPC_AnimationBehaviorBridge::SetMotionMatchingParameters(float Speed, FVector Direction, bool bInCombat)
{
    AnimationData.MovementSpeed = Speed;
    AnimationData.MovementDirection = Direction.GetSafeNormal();
    AnimationData.bIsInCombat = bInCombat;
    
    if (MotionMatchingSystem)
    {
        // Update motion matching system with new parameters
        // This would interface with the actual motion matching implementation
    }
}

void UNPC_AnimationBehaviorBridge::UpdateIKFootPlacement(bool bEnable, float TerrainAdaptation)
{
    bUseIKFootPlacement = bEnable;
    AnimationData.TerrainAdaptation = FMath::Clamp(TerrainAdaptation, 0.0f, 1.0f);
    
    if (IKFootSystem)
    {
        // Update IK system parameters
        // This would interface with the actual IK foot placement implementation
    }
}

void UNPC_AnimationBehaviorBridge::TransitionToState(ENPC_AnimationBehaviorState NewState, float BlendTime)
{
    if (NewState == AnimationData.CurrentState)
        return;
        
    AnimationData.PreviousState = AnimationData.CurrentState;
    AnimationData.CurrentState = NewState;
    AnimationData.StateTransitionBlendTime = BlendTime;
    
    StateChangeTimer = 0.0f;
    bInStateTransition = true;
    
    UE_LOG(LogTemp, Log, TEXT("NPC Animation Bridge: Transitioning from %d to %d"), 
           (int32)AnimationData.PreviousState, (int32)AnimationData.CurrentState);
}

void UNPC_AnimationBehaviorBridge::UpdatePackAnimationCoordination(bool bIsPackLeader, int32 PackSize, float PackCohesion)
{
    AnimationData.bIsInPack = (PackSize > 1);
    
    // Adjust animation parameters based on pack role
    if (bIsPackLeader)
    {
        AnimationData.EmotionalIntensity = FMath::Max(AnimationData.EmotionalIntensity, 0.7f);
    }
    
    // Pack cohesion affects movement synchronization
    if (AnimationData.bIsInPack && MotionMatchingSystem)
    {
        // Implement pack movement coordination
        float SyncFactor = PackCohesion * 0.5f;
        // This would be used to synchronize pack member animations
    }
}

void UNPC_AnimationBehaviorBridge::UpdateEmotionalAnimation(float Fear, float Aggression, float Curiosity)
{
    // Calculate overall emotional intensity
    float TotalEmotion = Fear + Aggression + Curiosity;
    AnimationData.EmotionalIntensity = FMath::Clamp(TotalEmotion / 3.0f, 0.0f, 1.0f);
    
    // Determine appropriate animation state based on dominant emotion
    if (Fear > 0.7f)
    {
        UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Fleeing, Fear);
    }
    else if (Aggression > 0.7f)
    {
        UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Aggressive, Aggression);
    }
    else if (Curiosity > 0.6f)
    {
        UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Alert, Curiosity);
    }
}

void UNPC_AnimationBehaviorBridge::UpdateTerritorialAnimation(bool bInTerritory, float TerritorialIntensity)
{
    if (bInTerritory && TerritorialIntensity > 0.5f)
    {
        UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Territorial, TerritorialIntensity);
    }
}

void UNPC_AnimationBehaviorBridge::TriggerCommunicationAnimation(ECommunicationType CommType, float Intensity)
{
    // Trigger specific communication animations based on type
    switch (CommType)
    {
        case ECommunicationType::Roar:
        case ECommunicationType::Growl:
            UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Aggressive, Intensity);
            break;
            
        case ECommunicationType::Chirp:
        case ECommunicationType::Purr:
            UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Socializing, Intensity);
            break;
            
        case ECommunicationType::Warning:
            UpdateAnimationFromBehavior(ENPC_AnimationBehaviorState::Alert, Intensity);
            break;
            
        default:
            break;
    }
}

float UNPC_AnimationBehaviorBridge::GetMovementSpeedNormalized() const
{
    if (MaxMovementSpeed <= 0.0f)
        return 0.0f;
        
    return FMath::Clamp(AnimationData.MovementSpeed / MaxMovementSpeed, 0.0f, 1.0f);
}

void UNPC_AnimationBehaviorBridge::InitializeComponentReferences()
{
    if (AActor* Owner = GetOwner())
    {
        // Find behavior component
        BehaviorComponent = Owner->FindComponentByClass<UNPC_BaseBehaviorComponent>();
        
        // Find animation system components
        MotionMatchingSystem = Owner->FindComponentByClass<UAnim_MotionMatchingSystem>();
        IKFootSystem = Owner->FindComponentByClass<UAnim_IKFootPlacement>();
        
        if (!BehaviorComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("NPC Animation Bridge: No behavior component found on %s"), 
                   *Owner->GetName());
        }
    }
}

void UNPC_AnimationBehaviorBridge::UpdateAnimationParameters(float DeltaTime)
{
    if (!GetOwner())
        return;
        
    // Update movement parameters
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        FVector Velocity = OwnerPawn->GetVelocity();
        AnimationData.MovementSpeed = Velocity.Size();
        AnimationData.MovementDirection = Velocity.GetSafeNormal();
    }
    
    // Update emotional intensity based on current state
    AnimationData.EmotionalIntensity = CalculateEmotionalIntensity();
}

void UNPC_AnimationBehaviorBridge::HandleStateTransition(float DeltaTime)
{
    StateChangeTimer += DeltaTime;
    
    if (StateChangeTimer >= AnimationData.StateTransitionBlendTime)
    {
        bInStateTransition = false;
        StateChangeTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("NPC Animation Bridge: State transition completed to %d"), 
               (int32)AnimationData.CurrentState);
    }
}

void UNPC_AnimationBehaviorBridge::SynchronizeWithBehaviorComponent()
{
    if (!BehaviorComponent)
        return;
        
    // Get current behavior state and map to animation state
    // This would require access to the behavior component's current state
    // For now, we'll implement a basic synchronization
    
    // Example: If behavior component indicates combat, update animation accordingly
    // This would be expanded based on the actual behavior component interface
}

void UNPC_AnimationBehaviorBridge::UpdateMotionMatchingData()
{
    if (!MotionMatchingSystem)
        return;
        
    // Update motion matching system with current animation data
    // This would interface with the actual motion matching implementation
    // Parameters would include movement speed, direction, and current behavior state
}

void UNPC_AnimationBehaviorBridge::UpdateIKParameters()
{
    if (!IKFootSystem)
        return;
        
    // Update IK foot placement system with terrain adaptation data
    // This would interface with the actual IK implementation
    // Parameters would include terrain adaptation factor and current movement state
}

ENPC_AnimationBehaviorState UNPC_AnimationBehaviorBridge::MapBehaviorToAnimationState(EDinosaurBehaviorState BehaviorState)
{
    switch (BehaviorState)
    {
        case EDinosaurBehaviorState::Idle:
            return ENPC_AnimationBehaviorState::Idle;
        case EDinosaurBehaviorState::Patrolling:
            return ENPC_AnimationBehaviorState::Walking;
        case EDinosaurBehaviorState::Hunting:
            return ENPC_AnimationBehaviorState::Hunting;
        case EDinosaurBehaviorState::Feeding:
            return ENPC_AnimationBehaviorState::Feeding;
        case EDinosaurBehaviorState::Sleeping:
            return ENPC_AnimationBehaviorState::Sleeping;
        case EDinosaurBehaviorState::Alert:
            return ENPC_AnimationBehaviorState::Alert;
        case EDinosaurBehaviorState::Fleeing:
            return ENPC_AnimationBehaviorState::Fleeing;
        case EDinosaurBehaviorState::Fighting:
            return ENPC_AnimationBehaviorState::Aggressive;
        default:
            return ENPC_AnimationBehaviorState::Idle;
    }
}

float UNPC_AnimationBehaviorBridge::CalculateEmotionalIntensity()
{
    // Calculate emotional intensity based on current state and environmental factors
    float BaseIntensity = 0.5f;
    
    switch (AnimationData.CurrentState)
    {
        case ENPC_AnimationBehaviorState::Aggressive:
        case ENPC_AnimationBehaviorState::Hunting:
            BaseIntensity = 0.9f;
            break;
        case ENPC_AnimationBehaviorState::Fleeing:
        case ENPC_AnimationBehaviorState::Alert:
            BaseIntensity = 0.8f;
            break;
        case ENPC_AnimationBehaviorState::Running:
            BaseIntensity = 0.7f;
            break;
        case ENPC_AnimationBehaviorState::Walking:
        case ENPC_AnimationBehaviorState::Socializing:
            BaseIntensity = 0.6f;
            break;
        case ENPC_AnimationBehaviorState::Feeding:
            BaseIntensity = 0.4f;
            break;
        case ENPC_AnimationBehaviorState::Sleeping:
            BaseIntensity = 0.1f;
            break;
        default:
            BaseIntensity = 0.5f;
            break;
    }
    
    return FMath::Clamp(BaseIntensity, 0.0f, 1.0f);
}

float UNPC_AnimationBehaviorBridge::CalculateMovementIntensity()
{
    float SpeedNormalized = GetMovementSpeedNormalized();
    float StateMultiplier = 1.0f;
    
    switch (AnimationData.CurrentState)
    {
        case ENPC_AnimationBehaviorState::Fleeing:
        case ENPC_AnimationBehaviorState::Hunting:
            StateMultiplier = 1.2f;
            break;
        case ENPC_AnimationBehaviorState::Running:
            StateMultiplier = 1.1f;
            break;
        case ENPC_AnimationBehaviorState::Walking:
            StateMultiplier = 0.8f;
            break;
        case ENPC_AnimationBehaviorState::Idle:
        case ENPC_AnimationBehaviorState::Sleeping:
            StateMultiplier = 0.1f;
            break;
        default:
            StateMultiplier = 1.0f;
            break;
    }
    
    return FMath::Clamp(SpeedNormalized * StateMultiplier, 0.0f, 1.0f);
}