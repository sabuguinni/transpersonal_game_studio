#include "AnimationSystemManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize personality traits based on archetype
    switch (AnimationProfile.Archetype)
    {
        case ECharacterArchetype::Protagonist:
            AnimationProfile.PersonalityTraits.Add(TEXT("Caution"), 0.8f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Curiosity"), 0.7f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Determination"), 0.6f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Nervousness"), 0.5f);
            break;
            
        case ECharacterArchetype::DinosaurHerbivore:
            AnimationProfile.PersonalityTraits.Add(TEXT("Alertness"), 0.9f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Gentleness"), 0.8f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Skittishness"), 0.7f);
            break;
            
        case ECharacterArchetype::DinosaurCarnivore:
            AnimationProfile.PersonalityTraits.Add(TEXT("Confidence"), 0.8f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Predatory_Focus"), 0.9f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Territorial"), 0.7f);
            break;
            
        case ECharacterArchetype::DinosaurPredator:
            AnimationProfile.PersonalityTraits.Add(TEXT("Stealth"), 0.9f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Patience"), 0.8f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Explosive_Power"), 0.9f);
            break;
            
        case ECharacterArchetype::DinosaurAmbush:
            AnimationProfile.PersonalityTraits.Add(TEXT("Stillness"), 0.95f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Sudden_Movement"), 0.9f);
            AnimationProfile.PersonalityTraits.Add(TEXT("Camouflage_Behavior"), 0.8f);
            break;
    }
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessEmotionalTransitions(DeltaTime);
    UpdateAnimationParameters();
}

void UAnimationSystemManager::UpdateEmotionalState(EEmotionalState NewState, float Intensity)
{
    if (AnimationProfile.CurrentEmotionalState != NewState)
    {
        AnimationProfile.CurrentEmotionalState = NewState;
        EmotionalStateIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        StateTransitionTimer = 0.0f;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Animation State Changed: %d with intensity %f"), 
               (int32)NewState, EmotionalStateIntensity);
    }
}

void UAnimationSystemManager::ApplyPersonalityModifier(const FString& TraitName, float Value)
{
    AnimationProfile.PersonalityTraits.Add(TraitName, FMath::Clamp(Value, 0.0f, 1.0f));
}

float UAnimationSystemManager::GetMovementStyleMultiplier() const
{
    float BaseMultiplier = 1.0f;
    
    // Apply emotional state modifiers
    switch (AnimationProfile.CurrentEmotionalState)
    {
        case EEmotionalState::Fearful:
            BaseMultiplier *= 0.7f; // Slower, more careful movement
            break;
        case EEmotionalState::Panicked:
            BaseMultiplier *= 1.3f; // Faster but less controlled
            break;
        case EEmotionalState::Alert:
            BaseMultiplier *= 0.9f; // Slightly slower, more deliberate
            break;
        case EEmotionalState::Aggressive:
            BaseMultiplier *= 1.1f; // More forceful movement
            break;
    }
    
    // Apply personality trait modifiers
    if (const float* CautionValue = AnimationProfile.PersonalityTraits.Find(TEXT("Caution")))
    {
        BaseMultiplier *= FMath::Lerp(1.2f, 0.8f, *CautionValue);
    }
    
    if (const float* ConfidenceValue = AnimationProfile.PersonalityTraits.Find(TEXT("Confidence")))
    {
        BaseMultiplier *= FMath::Lerp(0.9f, 1.1f, *ConfidenceValue);
    }
    
    return BaseMultiplier;
}

bool UAnimationSystemManager::ShouldUseCautiousMovement() const
{
    // High caution characters always use cautious movement
    if (const float* CautionValue = AnimationProfile.PersonalityTraits.Find(TEXT("Caution")))
    {
        if (*CautionValue > 0.7f) return true;
    }
    
    // Fearful or alert states trigger cautious movement
    return AnimationProfile.CurrentEmotionalState == EEmotionalState::Fearful ||
           AnimationProfile.CurrentEmotionalState == EEmotionalState::Alert;
}

void UAnimationSystemManager::UpdateAnimationParameters()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                // Set animation parameters that can be read by Animation Blueprints
                AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
                
                // Update custom animation variables
                float MovementMultiplier = GetMovementStyleMultiplier();
                bool bCautiousMovement = ShouldUseCautiousMovement();
                
                // These would be exposed in the Animation Blueprint
                // AnimInstance->SetFloatValue(TEXT("MovementStyleMultiplier"), MovementMultiplier);
                // AnimInstance->SetBoolValue(TEXT("bUseCautiousMovement"), bCautiousMovement);
            }
        }
    }
}

void UAnimationSystemManager::ProcessEmotionalTransitions(float DeltaTime)
{
    StateTransitionTimer += DeltaTime;
    
    // Gradually reduce emotional intensity over time
    if (StateTransitionTimer > 2.0f) // 2 seconds before starting to calm down
    {
        EmotionalStateIntensity = FMath::FInterpTo(EmotionalStateIntensity, 0.3f, DeltaTime, 0.5f);
        
        // Return to calm state if intensity is low enough
        if (EmotionalStateIntensity < 0.4f && AnimationProfile.CurrentEmotionalState != EEmotionalState::Calm)
        {
            AnimationProfile.CurrentEmotionalState = EEmotionalState::Calm;
            UE_LOG(LogTemp, Log, TEXT("Character returned to calm state"));
        }
    }
}