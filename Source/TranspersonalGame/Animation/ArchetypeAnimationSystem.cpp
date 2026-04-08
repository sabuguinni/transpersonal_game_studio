#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRig/IKRigDefinition.h"
#include "ControlRig/Public/ControlRig.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    PrimaryServiceType = ETickableTickType::Always;
    bTickInEditor = false;
    
    // Initialize default archetype configs
    InitializeDefaultArchetypes();
}

void UArchetypeAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem initialized"));
    
    // Load archetype configurations
    LoadArchetypeConfigurations();
    
    // Initialize gesture system
    InitializeGestureSystem();
    
    // Setup emotional state transitions
    SetupEmotionalTransitions();
}

void UArchetypeAnimationSystem::Deinitialize()
{
    // Cleanup active character states
    ActiveCharacterStates.Empty();
    ArchetypeConfigs.Empty();
    EmotionalTransitionRules.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem deinitialized"));
}

void UArchetypeAnimationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update all active character animation states
    UpdateActiveCharacters(DeltaTime);
    
    // Process emotional state transitions
    ProcessEmotionalTransitions(DeltaTime);
    
    // Update gesture system
    UpdateGestureSystem(DeltaTime);
    
    // Apply adaptive IK
    UpdateAdaptiveIK(DeltaTime);
}

TStatId UArchetypeAnimationSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UArchetypeAnimationSystem, STATGROUP_Tickables);
}

void UArchetypeAnimationSystem::RegisterCharacter(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("Attempted to register null character"));
        return;
    }
    
    // Create animation state for character
    FCharacterAnimationState NewState;
    NewState.CurrentEmotionalState = EEmotionalState::Neutral;
    NewState.CurrentSurvivalState = ESurvivalState::Fresh;
    NewState.CurrentStressLevel = EStressLevel::Calm;
    
    ActiveCharacterStates.Add(Character, NewState);
    CharacterArchetypes.Add(Character, Archetype);
    
    // Apply archetype configuration
    ApplyArchetypeToCharacter(Character, Archetype);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Registered character %s with archetype %d"), 
           *Character->GetName(), (int32)Archetype);
}

void UArchetypeAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (Character)
    {
        ActiveCharacterStates.Remove(Character);
        CharacterArchetypes.Remove(Character);
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Unregistered character %s"), *Character->GetName());
    }
}

void UArchetypeAnimationSystem::UpdateCharacterEmotionalState(ACharacter* Character, EEmotionalState NewState, float Intensity)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& State = ActiveCharacterStates[Character];
    EEmotionalState PreviousState = State.CurrentEmotionalState;
    
    // Apply emotional state change
    State.CurrentEmotionalState = NewState;
    State.EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    State.LastEmotionalChangeTime = GetWorld()->GetTimeSeconds();
    
    // Trigger emotional animation transition
    TriggerEmotionalTransition(Character, PreviousState, NewState, Intensity);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s emotional state changed from %d to %d (intensity: %.2f)"), 
           *Character->GetName(), (int32)PreviousState, (int32)NewState, Intensity);
}

void UArchetypeAnimationSystem::UpdateCharacterSurvivalState(ACharacter* Character, ESurvivalState NewState)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& State = ActiveCharacterStates[Character];
    State.CurrentSurvivalState = NewState;
    
    // Update motion matching database based on survival state
    UpdateMotionMatchingDatabase(Character, NewState);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s survival state changed to %d"), 
           *Character->GetName(), (int32)NewState);
}

void UArchetypeAnimationSystem::TriggerGesture(ACharacter* Character, EGestureType GestureType, float Duration)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    ECharacterArchetype Archetype = CharacterArchetypes[Character];
    const FArchetypeAnimationConfig* Config = GetArchetypeConfig(Archetype);
    
    if (!Config)
    {
        return;
    }
    
    // Find gesture animation
    if (Config->GestureAnimations.Contains(GestureType))
    {
        TSoftObjectPtr<UAnimMontage> GestureMontage = Config->GestureAnimations[GestureType];
        
        if (UAnimMontage* Montage = GestureMontage.LoadSynchronous())
        {
            // Play gesture montage
            if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
            {
                if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
                {
                    float PlayRate = Duration > 0.0f ? Montage->GetPlayLength() / Duration : 1.0f;
                    AnimInstance->Montage_Play(Montage, PlayRate);
                    
                    UE_LOG(LogArchetypeAnimation, Log, TEXT("Triggered gesture %d for character %s"), 
                           (int32)GestureType, *Character->GetName());
                }
            }
        }
    }
}

void UArchetypeAnimationSystem::SetCharacterFatigueLevel(ACharacter* Character, float FatigueLevel)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& State = ActiveCharacterStates[Character];
    State.FatigueLevel = FMath::Clamp(FatigueLevel, 0.0f, 1.0f);
    
    // Update movement characteristics based on fatigue
    UpdateMovementCharacteristics(Character, State);
}

void UArchetypeAnimationSystem::SetCharacterInjuryLevel(ACharacter* Character, float InjuryLevel)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& State = ActiveCharacterStates[Character];
    State.InjuryLevel = FMath::Clamp(InjuryLevel, 0.0f, 1.0f);
    
    // Update movement characteristics based on injury
    UpdateMovementCharacteristics(Character, State);
}

FCharacterAnimationState UArchetypeAnimationSystem::GetCharacterAnimationState(ACharacter* Character) const
{
    if (Character && ActiveCharacterStates.Contains(Character))
    {
        return ActiveCharacterStates[Character];
    }
    
    return FCharacterAnimationState();
}

const FArchetypeAnimationConfig* UArchetypeAnimationSystem::GetArchetypeConfig(ECharacterArchetype Archetype) const
{
    return ArchetypeConfigs.Find(Archetype);
}

void UArchetypeAnimationSystem::InitializeDefaultArchetypes()
{
    // Protagonist Paleontologist
    FArchetypeAnimationConfig PaleontologistConfig;
    PaleontologistConfig.Archetype = ECharacterArchetype::Protagonist_Paleontologist;
    PaleontologistConfig.bUseMotionMatching = true;
    PaleontologistConfig.bUseAdaptiveIK = true;
    PaleontologistConfig.bUseProceduralGestures = true;
    PaleontologistConfig.bUseEmotionalOverrides = true;
    PaleontologistConfig.BlendTime = 0.2f;
    PaleontologistConfig.MotionMatchingWeight = 1.0f;
    PaleontologistConfig.IKWeight = 0.8f;
    PaleontologistConfig.GestureFrequency = 0.7f;
    PaleontologistConfig.EmotionalIntensity = 0.8f;
    
    ArchetypeConfigs.Add(ECharacterArchetype::Protagonist_Paleontologist, PaleontologistConfig);
    
    // Survivor Archetype
    FArchetypeAnimationConfig SurvivorConfig;
    SurvivorConfig.Archetype = ECharacterArchetype::Survivor_Cautious;
    SurvivorConfig.bUseMotionMatching = true;
    SurvivorConfig.bUseAdaptiveIK = true;
    SurvivorConfig.bUseProceduralGestures = false;
    SurvivorConfig.bUseEmotionalOverrides = true;
    SurvivorConfig.BlendTime = 0.15f;
    SurvivorConfig.MotionMatchingWeight = 1.0f;
    SurvivorConfig.IKWeight = 1.0f;
    SurvivorConfig.GestureFrequency = 0.3f;
    SurvivorConfig.EmotionalIntensity = 1.0f;
    
    ArchetypeConfigs.Add(ECharacterArchetype::Survivor_Cautious, SurvivorConfig);
}

void UArchetypeAnimationSystem::LoadArchetypeConfigurations()
{
    // In a full implementation, this would load from data assets or config files
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Loaded %d archetype configurations"), ArchetypeConfigs.Num());
}

void UArchetypeAnimationSystem::InitializeGestureSystem()
{
    // Initialize gesture timing and frequency systems
    GestureRandomStream.Initialize(FMath::Rand());
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Gesture system initialized"));
}

void UArchetypeAnimationSystem::SetupEmotionalTransitions()
{
    // Define emotional state transition rules
    // Fear -> Cautious (natural progression)
    FEmotionalTransitionRule FearToCautious;
    FearToCautious.FromState = EEmotionalState::Fearful;
    FearToCautious.ToState = EEmotionalState::Cautious;
    FearToCautious.TransitionTime = 3.0f;
    FearToCautious.RequiredIntensityThreshold = 0.3f;
    
    EmotionalTransitionRules.Add(FearToCautious);
    
    // Exhausted -> Neutral (recovery)
    FEmotionalTransitionRule ExhaustedToNeutral;
    ExhaustedToNeutral.FromState = EEmotionalState::Exhausted;
    ExhaustedToNeutral.ToState = EEmotionalState::Neutral;
    ExhaustedToNeutral.TransitionTime = 10.0f;
    ExhaustedToNeutral.RequiredIntensityThreshold = 0.5f;
    
    EmotionalTransitionRules.Add(ExhaustedToNeutral);
}

void UArchetypeAnimationSystem::UpdateActiveCharacters(float DeltaTime)
{
    for (auto& CharacterPair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& State = CharacterPair.Value;
        
        if (!Character)
        {
            continue;
        }
        
        // Update movement timing
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            float CurrentSpeed = MovementComp->Velocity.Size();
            
            if (CurrentSpeed < 10.0f) // Essentially stationary
            {
                State.TimeSinceLastMovement += DeltaTime;
            }
            else
            {
                State.TimeSinceLastMovement = 0.0f;
            }
        }
        
        // Update stress level based on environmental factors
        UpdateStressLevel(Character, State, DeltaTime);
        
        // Update procedural gestures
        UpdateProceduralGestures(Character, State, DeltaTime);
    }
}

void UArchetypeAnimationSystem::ProcessEmotionalTransitions(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (auto& CharacterPair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& State = CharacterPair.Value;
        
        if (!Character)
        {
            continue;
        }
        
        // Check for automatic emotional transitions
        for (const FEmotionalTransitionRule& Rule : EmotionalTransitionRules)
        {
            if (State.CurrentEmotionalState == Rule.FromState)
            {
                float TimeSinceChange = CurrentTime - State.LastEmotionalChangeTime;
                
                if (TimeSinceChange >= Rule.TransitionTime && 
                    State.EmotionalIntensity <= Rule.RequiredIntensityThreshold)
                {
                    UpdateCharacterEmotionalState(Character, Rule.ToState, 0.5f);
                    break;
                }
            }
        }
    }
}

void UArchetypeAnimationSystem::UpdateGestureSystem(float DeltaTime)
{
    for (auto& CharacterPair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& State = CharacterPair.Value;
        
        if (!Character)
        {
            continue;
        }
        
        ECharacterArchetype Archetype = CharacterArchetypes[Character];
        const FArchetypeAnimationConfig* Config = GetArchetypeConfig(Archetype);
        
        if (!Config || !Config->bUseProceduralGestures)
        {
            continue;
        }
        
        // Check if it's time for a procedural gesture
        float TimeSinceLastGesture = GetWorld()->GetTimeSeconds() - State.LastGestureTime;
        float GestureInterval = 1.0f / Config->GestureFrequency;
        
        if (TimeSinceLastGesture >= GestureInterval)
        {
            // Randomly trigger appropriate gesture based on current state
            EGestureType GestureToTrigger = SelectAppropriateGesture(Character, State, Archetype);
            
            if (GestureToTrigger != EGestureType::MAX)
            {
                TriggerGesture(Character, GestureToTrigger);
                State.LastGestureTime = GetWorld()->GetTimeSeconds();
            }
        }
    }
}

void UArchetypeAnimationSystem::UpdateAdaptiveIK(float DeltaTime)
{
    // This would integrate with the AdaptiveIKComponent
    // For now, we'll update IK weights based on character states
    
    for (auto& CharacterPair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& State = CharacterPair.Value;
        
        if (!Character)
        {
            continue;
        }
        
        // Adjust IK intensity based on fatigue and injury
        float IKIntensity = 1.0f - (State.FatigueLevel * 0.3f) - (State.InjuryLevel * 0.5f);
        IKIntensity = FMath::Clamp(IKIntensity, 0.2f, 1.0f);
        
        // Apply IK intensity to character (this would interface with IK components)
        ApplyIKIntensity(Character, IKIntensity);
    }
}

void UArchetypeAnimationSystem::ApplyArchetypeToCharacter(ACharacter* Character, ECharacterArchetype Archetype)
{
    const FArchetypeAnimationConfig* Config = GetArchetypeConfig(Archetype);
    
    if (!Config || !Character)
    {
        return;
    }
    
    // Apply motion matching configuration
    if (Config->bUseMotionMatching)
    {
        SetupMotionMatching(Character, *Config);
    }
    
    // Apply IK configuration
    if (Config->bUseAdaptiveIK)
    {
        SetupAdaptiveIK(Character, *Config);
    }
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Applied archetype configuration to character %s"), 
           *Character->GetName());
}

void UArchetypeAnimationSystem::TriggerEmotionalTransition(ACharacter* Character, EEmotionalState FromState, 
                                                          EEmotionalState ToState, float Intensity)
{
    // This would trigger specific animation transitions based on emotional state changes
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Emotional transition for %s: %d -> %d"), 
           *Character->GetName(), (int32)FromState, (int32)ToState);
}

void UArchetypeAnimationSystem::UpdateMotionMatchingDatabase(ACharacter* Character, ESurvivalState SurvivalState)
{
    // This would update the motion matching database based on survival state
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Updated motion matching database for %s (survival state: %d)"), 
           *Character->GetName(), (int32)SurvivalState);
}

void UArchetypeAnimationSystem::UpdateMovementCharacteristics(ACharacter* Character, const FCharacterAnimationState& State)
{
    if (!Character)
    {
        return;
    }
    
    // Calculate movement speed modifier based on fatigue and injury
    float SpeedModifier = 1.0f - (State.FatigueLevel * 0.4f) - (State.InjuryLevel * 0.6f);
    SpeedModifier = FMath::Clamp(SpeedModifier, 0.3f, 1.0f);
    
    // Apply to character movement
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = MovementComp->GetClass()->GetDefaultObject<UCharacterMovementComponent>()->MaxWalkSpeed * SpeedModifier;
    }
}

void UArchetypeAnimationSystem::UpdateStressLevel(ACharacter* Character, FCharacterAnimationState& State, float DeltaTime)
{
    // Simple stress calculation based on environmental factors
    // In a full implementation, this would consider nearby threats, health, etc.
    
    float StressIncrease = 0.0f;
    
    // Increase stress when injured or fatigued
    if (State.InjuryLevel > 0.5f)
    {
        StressIncrease += 0.1f * DeltaTime;
    }
    
    if (State.FatigueLevel > 0.7f)
    {
        StressIncrease += 0.05f * DeltaTime;
    }
    
    // Decrease stress over time when safe
    if (StressIncrease == 0.0f)
    {
        State.FearLevel = FMath::Max(0.0f, State.FearLevel - 0.02f * DeltaTime);
    }
    else
    {
        State.FearLevel = FMath::Min(1.0f, State.FearLevel + StressIncrease);
    }
    
    // Update stress level enum based on fear level
    if (State.FearLevel < 0.2f)
    {
        State.CurrentStressLevel = EStressLevel::Calm;
    }
    else if (State.FearLevel < 0.5f)
    {
        State.CurrentStressLevel = EStressLevel::Nervous;
    }
    else if (State.FearLevel < 0.8f)
    {
        State.CurrentStressLevel = EStressLevel::Stressed;
    }
    else
    {
        State.CurrentStressLevel = EStressLevel::Panicked;
    }
}

void UArchetypeAnimationSystem::UpdateProceduralGestures(ACharacter* Character, FCharacterAnimationState& State, float DeltaTime)
{
    // Update procedural gesture timing and selection
    // This is handled in UpdateGestureSystem
}

EGestureType UArchetypeAnimationSystem::SelectAppropriateGesture(ACharacter* Character, const FCharacterAnimationState& State, ECharacterArchetype Archetype)
{
    // Select gesture based on character archetype and current state
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            if (State.bIsObserving)
            {
                return EGestureType::ObservationPoint;
            }
            else if (State.CurrentEmotionalState == EEmotionalState::Curious)
            {
                return EGestureType::ExamineObject;
            }
            break;
            
        case ECharacterArchetype::Survivor_Cautious:
            if (State.CurrentStressLevel >= EStressLevel::Nervous)
            {
                return EGestureType::CheckSurroundings;
            }
            break;
    }
    
    return EGestureType::MAX; // No gesture
}

void UArchetypeAnimationSystem::SetupMotionMatching(ACharacter* Character, const FArchetypeAnimationConfig& Config)
{
    // Setup motion matching system for character
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Setup motion matching for character %s"), *Character->GetName());
}

void UArchetypeAnimationSystem::SetupAdaptiveIK(ACharacter* Character, const FArchetypeAnimationConfig& Config)
{
    // Setup adaptive IK system for character
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Setup adaptive IK for character %s"), *Character->GetName());
}

void UArchetypeAnimationSystem::ApplyIKIntensity(ACharacter* Character, float Intensity)
{
    // Apply IK intensity to character's IK components
    UE_LOG(LogArchetypeAnimation, Verbose, TEXT("Applied IK intensity %.2f to character %s"), 
           Intensity, *Character->GetName());
}