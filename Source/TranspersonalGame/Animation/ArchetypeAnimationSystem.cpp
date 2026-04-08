#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IK/IKRigDefinition.h"
#include "ControlRig/Public/ControlRig.h"
#include "Characters/CharacterArchetypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    // Set this subsystem to tick
    bTickEnabled = true;
    
    // Initialize default values
    DefaultBlendTime = 0.2f;
    EmotionalTransitionTime = 0.5f;
    GestureBlendTime = 0.3f;
    IKBlendTime = 0.1f;
    
    bUseGlobalMotionMatching = true;
    bUseAdaptiveIK = true;
    bUseProceduralGestures = true;
    bUseEmotionalOverrides = true;
    
    MaxActiveCharacters = 50;
    UpdateFrequency = 60.0f;
    LODUpdateDistance = 5000.0f;
}

void UArchetypeAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem: Initializing..."));
    
    // Initialize archetype configurations
    InitializeArchetypeConfigurations();
    
    // Setup default emotional state mappings
    InitializeEmotionalStateMappings();
    
    // Initialize gesture library
    InitializeGestureLibrary();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem: Initialized successfully"));
}

void UArchetypeAnimationSystem::Deinitialize()
{
    // Clean up all tracked characters
    TrackedCharacters.Empty();
    ArchetypeConfigurations.Empty();
    EmotionalStateMappings.Empty();
    GestureLibrary.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem: Deinitialized"));
}

bool UArchetypeAnimationSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UArchetypeAnimationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update all tracked characters
    UpdateTrackedCharacters(DeltaTime);
    
    // Process emotional state transitions
    ProcessEmotionalTransitions(DeltaTime);
    
    // Update procedural gestures
    UpdateProceduralGestures(DeltaTime);
    
    // Optimize performance based on distance
    OptimizePerformance(DeltaTime);
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
    
    if (TrackedCharacters.Contains(Character))
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("Character %s is already registered"), *Character->GetName());
        return;
    }
    
    // Create animation state for this character
    FCharacterAnimationState NewState;
    NewState.CurrentEmotionalState = EEmotionalState::Neutral;
    NewState.CurrentSurvivalState = ESurvivalState::Fresh;
    NewState.CurrentStressLevel = EStressLevel::Calm;
    
    TrackedCharacters.Add(Character, NewState);
    CharacterArchetypes.Add(Character, Archetype);
    
    // Apply archetype configuration
    ApplyArchetypeConfiguration(Character, Archetype);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Registered character %s with archetype %d"), 
           *Character->GetName(), (int32)Archetype);
}

void UArchetypeAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    TrackedCharacters.Remove(Character);
    CharacterArchetypes.Remove(Character);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Unregistered character %s"), *Character->GetName());
}

void UArchetypeAnimationSystem::UpdateEmotionalState(ACharacter* Character, EEmotionalState NewState, float Intensity)
{
    if (!Character || !TrackedCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = TrackedCharacters[Character];
    
    // Only update if state actually changed
    if (AnimState.CurrentEmotionalState != NewState)
    {
        AnimState.PreviousEmotionalState = AnimState.CurrentEmotionalState;
        AnimState.CurrentEmotionalState = NewState;
        AnimState.EmotionalTransitionTime = 0.0f;
        AnimState.EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        
        // Trigger emotional animation transition
        TriggerEmotionalTransition(Character, NewState, Intensity);
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s emotional state changed to %d with intensity %.2f"), 
               *Character->GetName(), (int32)NewState, Intensity);
    }
}

void UArchetypeAnimationSystem::TriggerGesture(ACharacter* Character, EGestureType GestureType, float Intensity)
{
    if (!Character || !TrackedCharacters.Contains(Character))
    {
        return;
    }
    
    ECharacterArchetype Archetype = CharacterArchetypes.FindRef(Character);
    const FArchetypeAnimationConfig* Config = ArchetypeConfigurations.Find(Archetype);
    
    if (!Config)
    {
        return;
    }
    
    // Find gesture animation
    const TSoftObjectPtr<UAnimMontage>* GestureAnim = Config->GestureAnimations.Find(GestureType);
    if (!GestureAnim || !GestureAnim->IsValid())
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No gesture animation found for type %d"), (int32)GestureType);
        return;
    }
    
    // Play gesture montage
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        UAnimMontage* Montage = GestureAnim->LoadSynchronous();
        if (Montage)
        {
            float PlayRate = FMath::Lerp(0.8f, 1.2f, Intensity);
            AnimInstance->Montage_Play(Montage, PlayRate);
            
            // Update gesture tracking
            FCharacterAnimationState& AnimState = TrackedCharacters[Character];
            AnimState.LastGestureTime = GetWorld()->GetTimeSeconds();
            AnimState.LastGestureType = GestureType;
            
            UE_LOG(LogArchetypeAnimation, Log, TEXT("Triggered gesture %d for character %s"), 
                   (int32)GestureType, *Character->GetName());
        }
    }
}

bool UArchetypeAnimationSystem::CanTriggerGesture(ACharacter* Character, EGestureType GestureType) const
{
    if (!Character || !TrackedCharacters.Contains(Character))
    {
        return false;
    }
    
    const FCharacterAnimationState& AnimState = TrackedCharacters[Character];
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastGesture = CurrentTime - AnimState.LastGestureTime;
    
    // Minimum time between gestures based on archetype
    ECharacterArchetype Archetype = CharacterArchetypes.FindRef(Character);
    const FArchetypeAnimationConfig* Config = ArchetypeConfigurations.Find(Archetype);
    
    if (Config)
    {
        float MinGestureInterval = 1.0f / FMath::Max(Config->GestureFrequency, 0.1f);
        return TimeSinceLastGesture >= MinGestureInterval;
    }
    
    return TimeSinceLastGesture >= 2.0f; // Default minimum interval
}

FCharacterAnimationState UArchetypeAnimationSystem::GetCharacterAnimationState(ACharacter* Character) const
{
    if (Character && TrackedCharacters.Contains(Character))
    {
        return TrackedCharacters[Character];
    }
    
    return FCharacterAnimationState();
}

void UArchetypeAnimationSystem::SetArchetypeConfiguration(ECharacterArchetype Archetype, const FArchetypeAnimationConfig& Config)
{
    ArchetypeConfigurations.Add(Archetype, Config);
    
    // Apply to all characters of this archetype
    for (const auto& CharacterPair : CharacterArchetypes)
    {
        if (CharacterPair.Value == Archetype)
        {
            ApplyArchetypeConfiguration(CharacterPair.Key, Archetype);
        }
    }
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Updated configuration for archetype %d"), (int32)Archetype);
}

UPoseSearchDatabase* UArchetypeAnimationSystem::GetMotionDatabaseForState(ACharacter* Character, EEmotionalState EmotionalState, ESurvivalState SurvivalState) const
{
    if (!Character)
    {
        return nullptr;
    }
    
    ECharacterArchetype Archetype = CharacterArchetypes.FindRef(Character);
    const FArchetypeAnimationConfig* Config = ArchetypeConfigurations.Find(Archetype);
    
    if (!Config)
    {
        return nullptr;
    }
    
    // Try emotional state first
    const TSoftObjectPtr<UPoseSearchDatabase>* EmotionalDB = Config->EmotionalDatabases.Find(EmotionalState);
    if (EmotionalDB && EmotionalDB->IsValid())
    {
        return EmotionalDB->LoadSynchronous();
    }
    
    // Fall back to survival state
    const TSoftObjectPtr<UPoseSearchDatabase>* SurvivalDB = Config->SurvivalDatabases.Find(SurvivalState);
    if (SurvivalDB && SurvivalDB->IsValid())
    {
        return SurvivalDB->LoadSynchronous();
    }
    
    return nullptr;
}

void UArchetypeAnimationSystem::InitializeArchetypeConfigurations()
{
    // Initialize default configurations for each archetype
    
    // Paleontologist configuration
    FArchetypeAnimationConfig PaleontologistConfig;
    PaleontologistConfig.Archetype = ECharacterArchetype::Protagonist_Paleontologist;
    PaleontologistConfig.BodyLanguage.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    PaleontologistConfig.BodyLanguage.MovementPersonality = EMovementPersonality::Cautious;
    PaleontologistConfig.BodyLanguage.GestureFrequency = 0.7f;
    PaleontologistConfig.BodyLanguage.EyeContactLevel = 0.8f;
    PaleontologistConfig.GestureFrequency = 0.6f;
    PaleontologistConfig.EmotionalIntensity = 0.8f;
    
    ArchetypeConfigurations.Add(ECharacterArchetype::Protagonist_Paleontologist, PaleontologistConfig);
    
    // Add more archetype configurations as needed
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Initialized archetype configurations"));
}

void UArchetypeAnimationSystem::InitializeEmotionalStateMappings()
{
    // Map emotional states to animation parameters
    EmotionalStateMappings.Add(EEmotionalState::Neutral, FEmotionalAnimationParams{0.0f, 0.0f, 0.0f});
    EmotionalStateMappings.Add(EEmotionalState::Curious, FEmotionalAnimationParams{0.2f, 0.1f, 0.3f});
    EmotionalStateMappings.Add(EEmotionalState::Fearful, FEmotionalAnimationParams{0.8f, 0.7f, 0.9f});
    EmotionalStateMappings.Add(EEmotionalState::Determined, FEmotionalAnimationParams{0.1f, 0.9f, 0.2f});
    EmotionalStateMappings.Add(EEmotionalState::Exhausted, FEmotionalAnimationParams{0.3f, 0.2f, 0.8f});
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Initialized emotional state mappings"));
}

void UArchetypeAnimationSystem::InitializeGestureLibrary()
{
    // Initialize gesture library with default gestures
    // This would typically be loaded from data assets
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Initialized gesture library"));
}

void UArchetypeAnimationSystem::UpdateTrackedCharacters(float DeltaTime)
{
    for (auto& CharacterPair : TrackedCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& AnimState = CharacterPair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Update time-based values
        AnimState.EmotionalTransitionTime += DeltaTime;
        AnimState.TimeSinceLastMovement += DeltaTime;
        
        // Update movement-based values
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            FVector CurrentVelocity = MovementComp->Velocity;
            float CurrentSpeed = CurrentVelocity.Size();
            
            if (CurrentSpeed > 10.0f) // Moving threshold
            {
                AnimState.TimeSinceLastMovement = 0.0f;
            }
            
            // Update stress level based on movement
            UpdateStressLevel(Character, AnimState, DeltaTime);
        }
        
        // Update fatigue and injury effects
        UpdateConditionEffects(Character, AnimState, DeltaTime);
    }
}

void UArchetypeAnimationSystem::ProcessEmotionalTransitions(float DeltaTime)
{
    for (auto& CharacterPair : TrackedCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& AnimState = CharacterPair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Process ongoing emotional transitions
        if (AnimState.EmotionalTransitionTime < EmotionalTransitionTime)
        {
            float TransitionAlpha = AnimState.EmotionalTransitionTime / EmotionalTransitionTime;
            BlendEmotionalStates(Character, AnimState, TransitionAlpha);
        }
    }
}

void UArchetypeAnimationSystem::UpdateProceduralGestures(float DeltaTime)
{
    if (!bUseProceduralGestures)
    {
        return;
    }
    
    for (auto& CharacterPair : TrackedCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterAnimationState& AnimState = CharacterPair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Check if character should perform procedural gestures
        if (ShouldPerformProceduralGesture(Character, AnimState))
        {
            EGestureType GestureType = SelectProceduralGesture(Character, AnimState);
            if (GestureType != EGestureType::MAX)
            {
                TriggerGesture(Character, GestureType, 0.5f);
            }
        }
    }
}

void UArchetypeAnimationSystem::OptimizePerformance(float DeltaTime)
{
    // Implement LOD system for distant characters
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            for (auto& CharacterPair : TrackedCharacters)
            {
                ACharacter* Character = CharacterPair.Key;
                if (IsValid(Character))
                {
                    float Distance = FVector::Dist(PlayerLocation, Character->GetActorLocation());
                    
                    // Reduce update frequency for distant characters
                    if (Distance > LODUpdateDistance)
                    {
                        // Skip some updates for distant characters
                        continue;
                    }
                }
            }
        }
    }
}

void UArchetypeAnimationSystem::ApplyArchetypeConfiguration(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!Character)
    {
        return;
    }
    
    const FArchetypeAnimationConfig* Config = ArchetypeConfigurations.Find(Archetype);
    if (!Config)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No configuration found for archetype %d"), (int32)Archetype);
        return;
    }
    
    // Apply configuration to character's animation system
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        // Set archetype-specific parameters
        // This would typically involve setting animation blueprint variables
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Applied archetype configuration to character %s"), 
               *Character->GetName());
    }
}

void UArchetypeAnimationSystem::TriggerEmotionalTransition(ACharacter* Character, EEmotionalState NewState, float Intensity)
{
    if (!Character)
    {
        return;
    }
    
    // Get emotional animation parameters
    const FEmotionalAnimationParams* Params = EmotionalStateMappings.Find(NewState);
    if (!Params)
    {
        return;
    }
    
    // Apply emotional state to animation system
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        // Set emotional parameters in animation blueprint
        // This would typically involve calling animation blueprint functions
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Triggered emotional transition for character %s to state %d"), 
               *Character->GetName(), (int32)NewState);
    }
}

void UArchetypeAnimationSystem::UpdateStressLevel(ACharacter* Character, FCharacterAnimationState& AnimState, float DeltaTime)
{
    // Update stress level based on various factors
    float TargetStress = 0.0f;
    
    // Factor in fear level
    TargetStress += AnimState.FearLevel * 0.8f;
    
    // Factor in fatigue
    TargetStress += AnimState.FatigueLevel * 0.3f;
    
    // Factor in injury
    TargetStress += AnimState.InjuryLevel * 0.5f;
    
    // Factor in danger state
    if (AnimState.bIsInDanger)
    {
        TargetStress += 0.7f;
    }
    
    // Smooth transition to target stress
    float CurrentStressFloat = (float)AnimState.CurrentStressLevel;
    float NewStressFloat = FMath::FInterpTo(CurrentStressFloat, TargetStress * 4.0f, DeltaTime, 2.0f);
    
    EStressLevel NewStressLevel = (EStressLevel)FMath::Clamp((int32)NewStressFloat, 0, (int32)EStressLevel::MAX - 1);
    
    if (NewStressLevel != AnimState.CurrentStressLevel)
    {
        AnimState.CurrentStressLevel = NewStressLevel;
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s stress level changed to %d"), 
               *Character->GetName(), (int32)NewStressLevel);
    }
}

void UArchetypeAnimationSystem::UpdateConditionEffects(ACharacter* Character, FCharacterAnimationState& AnimState, float DeltaTime)
{
    // Update fatigue effects on animation
    if (AnimState.FatigueLevel > 0.5f)
    {
        // Increase blend times for tired characters
        // Reduce gesture frequency
        // Add slight tremor to movements
    }
    
    // Update injury effects on animation
    if (AnimState.InjuryLevel > 0.3f)
    {
        // Modify posture
        // Add limping or favoring
        // Reduce movement speed
    }
}

void UArchetypeAnimationSystem::BlendEmotionalStates(ACharacter* Character, FCharacterAnimationState& AnimState, float TransitionAlpha)
{
    // Blend between previous and current emotional states
    const FEmotionalAnimationParams* PrevParams = EmotionalStateMappings.Find(AnimState.PreviousEmotionalState);
    const FEmotionalAnimationParams* CurrentParams = EmotionalStateMappings.Find(AnimState.CurrentEmotionalState);
    
    if (PrevParams && CurrentParams)
    {
        // Interpolate emotional parameters
        float BlendedTension = FMath::Lerp(PrevParams->TensionLevel, CurrentParams->TensionLevel, TransitionAlpha);
        float BlendedEnergy = FMath::Lerp(PrevParams->EnergyLevel, CurrentParams->EnergyLevel, TransitionAlpha);
        float BlendedFocus = FMath::Lerp(PrevParams->FocusLevel, CurrentParams->FocusLevel, TransitionAlpha);
        
        // Apply blended parameters to animation system
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            // Set blended emotional parameters
        }
    }
}

bool UArchetypeAnimationSystem::ShouldPerformProceduralGesture(ACharacter* Character, const FCharacterAnimationState& AnimState) const
{
    // Check if enough time has passed since last gesture
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastGesture = CurrentTime - AnimState.LastGestureTime;
    
    ECharacterArchetype Archetype = CharacterArchetypes.FindRef(Character);
    const FArchetypeAnimationConfig* Config = ArchetypeConfigurations.Find(Archetype);
    
    if (Config)
    {
        float GestureInterval = 1.0f / FMath::Max(Config->GestureFrequency, 0.1f);
        return TimeSinceLastGesture >= GestureInterval;
    }
    
    return TimeSinceLastGesture >= 3.0f; // Default interval
}

EGestureType UArchetypeAnimationSystem::SelectProceduralGesture(ACharacter* Character, const FCharacterAnimationState& AnimState) const
{
    ECharacterArchetype Archetype = CharacterArchetypes.FindRef(Character);
    
    // Select gesture based on archetype and current state
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            if (AnimState.bIsObserving)
            {
                return EGestureType::ObservationPoint;
            }
            else if (AnimState.CurrentEmotionalState == EEmotionalState::Curious)
            {
                return EGestureType::ExamineObject;
            }
            break;
            
        default:
            break;
    }
    
    // Default gesture based on emotional state
    switch (AnimState.CurrentEmotionalState)
    {
        case EEmotionalState::Fearful:
            return EGestureType::CheckSurroundings;
        case EEmotionalState::Cautious:
            return EGestureType::ListenCarefully;
        default:
            break;
    }
    
    return EGestureType::MAX; // No gesture
}