#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRigDefinition.h"
#include "ControlRig.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentEmotionalState = EEmotionalState::Neutral;
    CurrentSurvivalState = ESurvivalState::Fresh;
    CurrentStressLevel = EStressLevel::Calm;
    
    FatigueLevel = 0.0f;
    InjuryLevel = 0.0f;
    FearLevel = 0.0f;
    ConfidenceLevel = 1.0f;
    
    LastGestureTime = 0.0f;
    TimeSinceLastMovement = 0.0f;
    
    bIsInDanger = false;
    bIsHiding = false;
    bIsObserving = false;
    
    // Animation blending parameters
    BlendTimeMultiplier = 1.0f;
    MotionMatchingWeight = 1.0f;
    IKWeight = 1.0f;
    GestureFrequency = 0.5f;
    EmotionalIntensity = 0.7f;
    
    // Update frequencies
    EmotionalUpdateFrequency = 0.2f; // 5 times per second
    GestureUpdateFrequency = 0.1f;   // 10 times per second
    
    EmotionalUpdateTimer = 0.0f;
    GestureUpdateTimer = 0.0f;
}

void UArchetypeAnimationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("ArchetypeAnimationSystem requires a Character owner"));
        return;
    }
    
    // Get movement component
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("Character must have a CharacterMovementComponent"));
        return;
    }
    
    // Get skeletal mesh component
    SkeletalMeshComponent = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("Character must have a SkeletalMeshComponent"));
        return;
    }
    
    // Initialize archetype configuration
    InitializeArchetypeConfiguration();
    
    // Setup Motion Matching
    SetupMotionMatching();
    
    // Setup IK systems
    SetupIKSystems();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem initialized for character: %s"), 
           *OwnerCharacter->GetName());
}

void UArchetypeAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update timers
    EmotionalUpdateTimer += DeltaTime;
    GestureUpdateTimer += DeltaTime;
    TimeSinceLastMovement += DeltaTime;
    
    // Update movement tracking
    UpdateMovementTracking(DeltaTime);
    
    // Update emotional state periodically
    if (EmotionalUpdateTimer >= EmotionalUpdateFrequency)
    {
        UpdateEmotionalState(DeltaTime);
        EmotionalUpdateTimer = 0.0f;
    }
    
    // Update gesture system
    if (GestureUpdateTimer >= GestureUpdateFrequency)
    {
        UpdateGestureSystem(DeltaTime);
        GestureUpdateTimer = 0.0f;
    }
    
    // Update Motion Matching database selection
    UpdateMotionMatchingSelection();
    
    // Update IK weights based on current state
    UpdateIKWeights(DeltaTime);
    
    // Apply archetype-specific modifications
    ApplyArchetypeModifications(DeltaTime);
}

void UArchetypeAnimationSystem::InitializeArchetypeConfiguration()
{
    // Load archetype configuration based on character type
    if (ArchetypeConfigs.Contains(CurrentArchetype))
    {
        CurrentConfig = ArchetypeConfigs[CurrentArchetype];
    }
    else
    {
        // Create default configuration
        CurrentConfig = FArchetypeAnimationConfig();
        CurrentConfig.Archetype = CurrentArchetype;
        
        UE_LOG(LogArchetypeAnimation, Warning, 
               TEXT("No configuration found for archetype %d, using defaults"), 
               (int32)CurrentArchetype);
    }
    
    // Apply body language settings
    ApplyBodyLanguageConfiguration();
}

void UArchetypeAnimationSystem::SetupMotionMatching()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No AnimInstance found for Motion Matching setup"));
        return;
    }
    
    // Setup Motion Matching databases for different emotional states
    SetupEmotionalDatabases();
    
    // Setup Motion Matching databases for different survival states
    SetupSurvivalDatabases();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Motion Matching setup completed"));
}

void UArchetypeAnimationSystem::SetupIKSystems()
{
    // Setup Adaptive IK if enabled
    if (CurrentConfig.bUseAdaptiveIK && CurrentConfig.IKRigDefinition.IsValid())
    {
        // Initialize IK Rig
        LoadedIKRig = CurrentConfig.IKRigDefinition.LoadSynchronous();
        if (LoadedIKRig)
        {
            UE_LOG(LogArchetypeAnimation, Log, TEXT("IK Rig loaded successfully"));
        }
    }
    
    // Setup foot IK for terrain adaptation
    SetupFootIK();
    
    // Setup look-at IK for threat awareness
    SetupLookAtIK();
}

void UArchetypeAnimationSystem::UpdateMovementTracking(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector CurrentVelocity = MovementComponent->Velocity;
    float CurrentSpeed = CurrentVelocity.Size();
    
    // Update movement state
    bool bWasMoving = bIsMoving;
    bIsMoving = CurrentSpeed > MovementThreshold;
    
    if (bIsMoving)
    {
        TimeSinceLastMovement = 0.0f;
        
        // Calculate movement direction
        if (CurrentSpeed > 0.1f)
        {
            FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
            FVector NormalizedVelocity = CurrentVelocity.GetSafeNormal();
            MovementDirection = FVector::DotProduct(ForwardVector, NormalizedVelocity);
        }
    }
    
    // Store for next frame
    LastVelocity = CurrentVelocity;
    LastSpeed = CurrentSpeed;
}

void UArchetypeAnimationSystem::UpdateEmotionalState(float DeltaTime)
{
    // Calculate stress based on environmental factors
    float EnvironmentalStress = CalculateEnvironmentalStress();
    
    // Update fear level based on nearby threats
    UpdateFearLevel(DeltaTime);
    
    // Update fatigue based on activity
    UpdateFatigueLevel(DeltaTime);
    
    // Determine new emotional state based on all factors
    EEmotionalState NewEmotionalState = DetermineEmotionalState();
    
    if (NewEmotionalState != CurrentEmotionalState)
    {
        // Trigger emotional state change
        OnEmotionalStateChanged(CurrentEmotionalState, NewEmotionalState);
        CurrentEmotionalState = NewEmotionalState;
    }
    
    // Update stress level
    EStressLevel NewStressLevel = DetermineStressLevel(EnvironmentalStress);
    if (NewStressLevel != CurrentStressLevel)
    {
        CurrentStressLevel = NewStressLevel;
    }
}

void UArchetypeAnimationSystem::UpdateGestureSystem(float DeltaTime)
{
    if (!CurrentConfig.bUseProceduralGestures)
    {
        return;
    }
    
    // Check if it's time for a new gesture
    float TimeSinceLastGesture = GetWorld()->GetTimeSeconds() - LastGestureTime;
    float GestureInterval = 1.0f / CurrentConfig.GestureFrequency;
    
    if (TimeSinceLastGesture >= GestureInterval)
    {
        // Determine appropriate gesture based on current state
        EGestureType AppropriateGesture = DetermineAppropriateGesture();
        
        if (AppropriateGesture != EGestureType::MAX)
        {
            PlayGesture(AppropriateGesture);
            LastGestureTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UArchetypeAnimationSystem::UpdateMotionMatchingSelection()
{
    UPoseSearchDatabase* NewDatabase = SelectAppropriateDatabase();
    
    if (NewDatabase != CurrentActiveDatabase)
    {
        CurrentActiveDatabase = NewDatabase;
        
        // Notify animation system of database change
        if (OnDatabaseChanged.IsBound())
        {
            OnDatabaseChanged.Broadcast(NewDatabase);
        }
    }
}

void UArchetypeAnimationSystem::UpdateIKWeights(float DeltaTime)
{
    // Update foot IK weight based on movement state
    float TargetFootIKWeight = bIsMoving ? 1.0f : 0.8f;
    CurrentFootIKWeight = FMath::FInterpTo(CurrentFootIKWeight, TargetFootIKWeight, DeltaTime, 2.0f);
    
    // Update look-at IK weight based on threat awareness
    float TargetLookAtWeight = bIsInDanger ? 1.0f : 0.3f;
    CurrentLookAtWeight = FMath::FInterpTo(CurrentLookAtWeight, TargetLookAtWeight, DeltaTime, 3.0f);
}

void UArchetypeAnimationSystem::ApplyArchetypeModifications(float DeltaTime)
{
    // Apply archetype-specific movement modifications
    ApplyMovementModifications();
    
    // Apply emotional intensity modifications
    ApplyEmotionalModifications();
    
    // Apply survival state modifications
    ApplySurvivalModifications();
}

float UArchetypeAnimationSystem::CalculateEnvironmentalStress()
{
    float Stress = 0.0f;
    
    // Add stress based on time of day (night is more stressful)
    // Add stress based on weather conditions
    // Add stress based on nearby predators
    // Add stress based on resource scarcity
    
    return FMath::Clamp(Stress, 0.0f, 1.0f);
}

void UArchetypeAnimationSystem::UpdateFearLevel(float DeltaTime)
{
    // Implement fear calculation based on:
    // - Nearby predators
    // - Loud noises
    // - Sudden movements
    // - Environmental dangers
    
    float TargetFear = 0.0f; // Calculate based on threats
    FearLevel = FMath::FInterpTo(FearLevel, TargetFear, DeltaTime, 1.0f);
}

void UArchetypeAnimationSystem::UpdateFatigueLevel(float DeltaTime)
{
    // Increase fatigue based on activity
    if (bIsMoving)
    {
        float FatigueIncrease = LastSpeed * DeltaTime * 0.001f; // Very gradual
        FatigueLevel = FMath::Clamp(FatigueLevel + FatigueIncrease, 0.0f, 1.0f);
    }
    else
    {
        // Recover when resting
        float FatigueDecrease = DeltaTime * 0.01f;
        FatigueLevel = FMath::Clamp(FatigueLevel - FatigueDecrease, 0.0f, 1.0f);
    }
}

EEmotionalState UArchetypeAnimationSystem::DetermineEmotionalState()
{
    // Determine emotional state based on current conditions
    if (FearLevel > 0.7f)
    {
        return EEmotionalState::Fearful;
    }
    else if (FatigueLevel > 0.8f)
    {
        return EEmotionalState::Exhausted;
    }
    else if (bIsObserving)
    {
        return EEmotionalState::Curious;
    }
    else if (ConfidenceLevel > 0.8f)
    {
        return EEmotionalState::Confident;
    }
    else if (TimeSinceLastMovement > 30.0f)
    {
        return EEmotionalState::Cautious;
    }
    
    return EEmotionalState::Neutral;
}

EStressLevel UArchetypeAnimationSystem::DetermineStressLevel(float EnvironmentalStress)
{
    float TotalStress = EnvironmentalStress + FearLevel * 0.5f + FatigueLevel * 0.3f;
    
    if (TotalStress > 0.8f)
        return EStressLevel::Panicked;
    else if (TotalStress > 0.6f)
        return EStressLevel::Tense;
    else if (TotalStress > 0.4f)
        return EStressLevel::Alert;
    else if (TotalStress > 0.2f)
        return EStressLevel::Aware;
    else
        return EStressLevel::Calm;
}

EGestureType UArchetypeAnimationSystem::DetermineAppropriateGesture()
{
    // Determine gesture based on archetype and current state
    switch (CurrentArchetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            if (bIsObserving)
                return EGestureType::ObservationPoint;
            else if (CurrentEmotionalState == EEmotionalState::Curious)
                return EGestureType::ExamineObject;
            break;
            
        case ECharacterArchetype::Survivor_Veteran:
            if (bIsInDanger)
                return EGestureType::SignalDanger;
            else if (CurrentStressLevel >= EStressLevel::Alert)
                return EGestureType::CheckSurroundings;
            break;
            
        default:
            break;
    }
    
    return EGestureType::MAX; // No gesture
}

void UArchetypeAnimationSystem::PlayGesture(EGestureType GestureType)
{
    if (!CurrentConfig.GestureAnimations.Contains(GestureType))
    {
        return;
    }
    
    UAnimMontage* GestureMontage = CurrentConfig.GestureAnimations[GestureType].LoadSynchronous();
    if (GestureMontage && SkeletalMeshComponent)
    {
        UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(GestureMontage);
            
            UE_LOG(LogArchetypeAnimation, Log, TEXT("Playing gesture: %d"), (int32)GestureType);
        }
    }
}

UPoseSearchDatabase* UArchetypeAnimationSystem::SelectAppropriateDatabase()
{
    // Priority: Emotional state > Survival state > Default
    
    if (CurrentConfig.EmotionalDatabases.Contains(CurrentEmotionalState))
    {
        return CurrentConfig.EmotionalDatabases[CurrentEmotionalState].LoadSynchronous();
    }
    
    if (CurrentConfig.SurvivalDatabases.Contains(CurrentSurvivalState))
    {
        return CurrentConfig.SurvivalDatabases[CurrentSurvivalState].LoadSynchronous();
    }
    
    // Fallback to default database
    return DefaultMotionDatabase.LoadSynchronous();
}

void UArchetypeAnimationSystem::SetupEmotionalDatabases()
{
    // This would be configured in Blueprint or data assets
    // For now, we'll just log that setup is needed
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Emotional databases setup - configure in Blueprint"));
}

void UArchetypeAnimationSystem::SetupSurvivalDatabases()
{
    // This would be configured in Blueprint or data assets
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Survival databases setup - configure in Blueprint"));
}

void UArchetypeAnimationSystem::SetupFootIK()
{
    // Initialize foot IK system
    bFootIKEnabled = CurrentConfig.bUseAdaptiveIK;
    CurrentFootIKWeight = 0.0f;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Foot IK setup completed. Enabled: %s"), 
           bFootIKEnabled ? TEXT("true") : TEXT("false"));
}

void UArchetypeAnimationSystem::SetupLookAtIK()
{
    // Initialize look-at IK system
    bLookAtIKEnabled = true;
    CurrentLookAtWeight = 0.0f;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Look-at IK setup completed"));
}

void UArchetypeAnimationSystem::ApplyBodyLanguageConfiguration()
{
    // Apply body language settings from current configuration
    const FArchetypeBodyLanguage& BodyLanguage = CurrentConfig.BodyLanguage;
    
    // These would affect animation parameters
    BlendTimeMultiplier = BodyLanguage.PostureWeight;
    GestureFrequency = BodyLanguage.GestureFrequency;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Body language configuration applied"));
}

void UArchetypeAnimationSystem::ApplyMovementModifications()
{
    // Apply archetype-specific movement modifications
}

void UArchetypeAnimationSystem::ApplyEmotionalModifications()
{
    // Apply emotional state modifications to animation
}

void UArchetypeAnimationSystem::ApplySurvivalModifications()
{
    // Apply survival state modifications to animation
}

void UArchetypeAnimationSystem::OnEmotionalStateChanged(EEmotionalState OldState, EEmotionalState NewState)
{
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Emotional state changed from %d to %d"), 
           (int32)OldState, (int32)NewState);
    
    // Broadcast the change
    if (OnEmotionalStateChangedDelegate.IsBound())
    {
        OnEmotionalStateChangedDelegate.Broadcast(OldState, NewState);
    }
}

void UArchetypeAnimationSystem::SetArchetype(ECharacterArchetype NewArchetype)
{
    if (CurrentArchetype != NewArchetype)
    {
        CurrentArchetype = NewArchetype;
        InitializeArchetypeConfiguration();
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype changed to: %d"), (int32)NewArchetype);
    }
}

void UArchetypeAnimationSystem::SetEmotionalState(EEmotionalState NewState, float Intensity)
{
    if (CurrentEmotionalState != NewState)
    {
        OnEmotionalStateChanged(CurrentEmotionalState, NewState);
        CurrentEmotionalState = NewState;
    }
    
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UArchetypeAnimationSystem::SetSurvivalState(ESurvivalState NewState)
{
    if (CurrentSurvivalState != NewState)
    {
        CurrentSurvivalState = NewState;
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Survival state changed to: %d"), (int32)NewState);
    }
}

void UArchetypeAnimationSystem::SetDangerState(bool bInDanger)
{
    if (bIsInDanger != bInDanger)
    {
        bIsInDanger = bInDanger;
        
        if (bInDanger)
        {
            // Increase stress and fear
            FearLevel = FMath::Clamp(FearLevel + 0.3f, 0.0f, 1.0f);
        }
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Danger state changed to: %s"), 
               bInDanger ? TEXT("true") : TEXT("false"));
    }
}

float UArchetypeAnimationSystem::GetCurrentBlendTime() const
{
    return CurrentConfig.BlendTime * BlendTimeMultiplier;
}

float UArchetypeAnimationSystem::GetMotionMatchingWeight() const
{
    return CurrentConfig.MotionMatchingWeight * MotionMatchingWeight;
}

float UArchetypeAnimationSystem::GetIKWeight() const
{
    return CurrentConfig.IKWeight * IKWeight;
}