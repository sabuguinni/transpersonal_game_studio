#include "AnimationSystemCore.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

UAnimationSystemCore::UAnimationSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Generate unique personality seed based on spawn location and time
    PersonalitySeed = FMath::Rand();
}

void UAnimationSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate individual variations for this character instance
    GenerateIndividualVariations();
    
    // Initialize motion matching settings based on character type
    InitializeMotionMatchingForArchetype();
}

void UAnimationSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAnimationParameters(DeltaTime);
    HandleEmotionalTransitions(DeltaTime);
}

void UAnimationSystemCore::SetEmotionalState(EEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        PreviousEmotionalState = CurrentEmotionalState;
        CurrentEmotionalState = NewState;
        StateTransitionTimer = 0.0f;
        
        // Trigger Blueprint event for animation blueprint to respond
        OnEmotionalStateChanged(PreviousEmotionalState, CurrentEmotionalState);
        
        UE_LOG(LogTemp, Log, TEXT("Character emotional state changed from %d to %d"), 
               (int32)PreviousEmotionalState, (int32)CurrentEmotionalState);
    }
}

void UAnimationSystemCore::UpdateFearLevel(float FearLevel)
{
    CurrentFearLevel = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    
    // Automatically adjust emotional state based on fear level
    if (CurrentFearLevel > 0.8f)
    {
        SetEmotionalState(EEmotionalState::Panicked);
    }
    else if (CurrentFearLevel > 0.5f)
    {
        SetEmotionalState(EEmotionalState::Fearful);
    }
    else if (CurrentFearLevel > 0.2f)
    {
        SetEmotionalState(EEmotionalState::Alert);
    }
    else
    {
        SetEmotionalState(EEmotionalState::Calm);
    }
}

float UAnimationSystemCore::GetCurrentBlendTime() const
{
    float BaseBlendTime = MotionMatchingConfig.DefaultBlendTime;
    
    // Adjust blend time based on emotional state
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Panicked:
        case EEmotionalState::Fearful:
            BaseBlendTime = MotionMatchingConfig.EmergencyBlendTime;
            break;
            
        case EEmotionalState::Calm:
            BaseBlendTime = MotionMatchingConfig.RelaxedBlendTime;
            break;
            
        default:
            BaseBlendTime = MotionMatchingConfig.DefaultBlendTime;
            break;
    }
    
    // Apply personality influence
    float PersonalityInfluence = CalculatePersonalityInfluence();
    return BaseBlendTime * PersonalityInfluence;
}

void UAnimationSystemCore::GenerateIndividualVariations()
{
    // Use consistent random stream for this character
    FRandomStream RandomStream(PersonalitySeed);
    
    // Generate personality variations
    Personality.Nervousness = FMath::Clamp(RandomStream.FRandRange(0.0f, 1.0f), 0.0f, 1.0f);
    Personality.Confidence = FMath::Clamp(1.0f - Personality.Nervousness + RandomStream.FRandRange(-0.2f, 0.2f), 0.0f, 1.0f);
    Personality.Curiosity = RandomStream.FRandRange(0.2f, 0.8f);
    
    // Adjust aggressiveness based on character type
    switch (CharacterType)
    {
        case ECharacterArchetype::Paleontologist:
            Personality.Aggressiveness = RandomStream.FRandRange(0.1f, 0.3f);
            break;
            
        case ECharacterArchetype::SmallHerbivore:
            Personality.Aggressiveness = RandomStream.FRandRange(0.0f, 0.2f);
            break;
            
        case ECharacterArchetype::LargeHerbivore:
            Personality.Aggressiveness = RandomStream.FRandRange(0.2f, 0.5f);
            break;
            
        case ECharacterArchetype::SmallCarnivore:
            Personality.Aggressiveness = RandomStream.FRandRange(0.6f, 0.9f);
            break;
            
        case ECharacterArchetype::LargeCarnivore:
            Personality.Aggressiveness = RandomStream.FRandRange(0.7f, 1.0f);
            break;
            
        default:
            Personality.Aggressiveness = RandomStream.FRandRange(0.3f, 0.7f);
            break;
    }
    
    // Generate physical variations
    Personality.MovementSpeedMultiplier = RandomStream.FRandRange(0.85f, 1.15f);
    Personality.AnimationSpeedVariation = RandomStream.FRandRange(0.9f, 1.1f);
    Personality.LimbLengthVariation = RandomStream.FRandRange(0.95f, 1.05f);
    Personality.PostureVariation = RandomStream.FRandRange(0.98f, 1.02f);
    
    UE_LOG(LogTemp, Log, TEXT("Generated personality for %s: Nervousness=%.2f, Confidence=%.2f, Aggressiveness=%.2f"), 
           *UEnum::GetValueAsString(CharacterType), 
           Personality.Nervousness, 
           Personality.Confidence, 
           Personality.Aggressiveness);
}

void UAnimationSystemCore::UpdateAnimationParameters(float DeltaTime)
{
    // Update state transition timer
    StateTransitionTimer += DeltaTime;
    
    // Apply personality-based animation modifications
    if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
        {
            // Set animation parameters that can be read by Animation Blueprints
            AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
            
            // Expose personality traits to animation blueprint
            // These would be connected to variables in the Animation Blueprint
        }
    }
}

void UAnimationSystemCore::HandleEmotionalTransitions(float DeltaTime)
{
    // Implement gradual emotional state transitions
    // Fear levels naturally decay over time if no threats are present
    if (CurrentFearLevel > 0.0f)
    {
        float FearDecayRate = 0.1f; // Adjust based on character personality
        FearDecayRate *= (1.0f + Personality.Nervousness); // Nervous characters stay afraid longer
        
        CurrentFearLevel = FMath::Max(0.0f, CurrentFearLevel - (FearDecayRate * DeltaTime));
        
        // Update emotional state based on new fear level
        UpdateFearLevel(CurrentFearLevel);
    }
}

float UAnimationSystemCore::CalculatePersonalityInfluence() const
{
    // Calculate how personality affects animation timing
    float Influence = 1.0f;
    
    // Nervous characters move more quickly/jerkily
    Influence *= (1.0f + (Personality.Nervousness * 0.3f));
    
    // Confident characters move more deliberately
    Influence *= (1.0f - (Personality.Confidence * 0.2f));
    
    return FMath::Clamp(Influence, 0.5f, 2.0f);
}

void UAnimationSystemCore::InitializeMotionMatchingForArchetype()
{
    // Set default motion matching settings based on character archetype
    switch (CharacterType)
    {
        case ECharacterArchetype::Paleontologist:
            MotionMatchingConfig.DefaultBlendTime = 0.25f;
            MotionMatchingConfig.EmergencyBlendTime = 0.1f;
            MotionMatchingConfig.RelaxedBlendTime = 0.4f;
            bEnableHandIK = true; // For tool interactions
            break;
            
        case ECharacterArchetype::SmallHerbivore:
            MotionMatchingConfig.DefaultBlendTime = 0.15f;
            MotionMatchingConfig.EmergencyBlendTime = 0.05f; // Very quick fear responses
            MotionMatchingConfig.RelaxedBlendTime = 0.3f;
            break;
            
        case ECharacterArchetype::LargeCarnivore:
            MotionMatchingConfig.DefaultBlendTime = 0.3f;
            MotionMatchingConfig.EmergencyBlendTime = 0.2f; // Slower but more powerful
            MotionMatchingConfig.RelaxedBlendTime = 0.6f;
            break;
            
        default:
            // Keep default values
            break;
    }
}