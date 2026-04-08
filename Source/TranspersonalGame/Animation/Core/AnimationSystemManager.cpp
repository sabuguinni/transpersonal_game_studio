#include "AnimationSystemManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateMotionMatchingSetup();
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmotionalBlending(DeltaTime);
}

void UAnimationSystemManager::SetEmotionalState(EEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        
        // Trigger animation state change in Animation Blueprint
        if (AActor* Owner = GetOwner())
        {
            if (USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>())
            {
                if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
                {
                    // Set emotional state variable in Animation Blueprint
                    AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
                }
            }
        }
    }
}

void UAnimationSystemManager::SetPersonality(const FCharacterPersonality& NewPersonality)
{
    Personality = NewPersonality;
    
    // Update target blend values for smooth personality transitions
    TargetBoldnessBlend = Personality.Boldness;
    TargetAggressionBlend = Personality.Aggression;
    TargetCuriosityBlend = Personality.Curiosity;
}

UPoseSearchDatabase* UAnimationSystemManager::GetCurrentMotionDatabase() const
{
    if (!CurrentSetup)
    {
        return nullptr;
    }
    
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Aggressive:
        case EEmotionalState::Hunting:
        case EEmotionalState::Territorial:
            return CurrentSetup->CombatDatabase.LoadSynchronous();
            
        case EEmotionalState::Curious:
        case EEmotionalState::Feeding:
            return CurrentSetup->InteractionDatabase.LoadSynchronous();
            
        default:
            return CurrentSetup->LocomotionDatabase.LoadSynchronous();
    }
}

TArray<FAnimationVariant> UAnimationSystemManager::GetPersonalityMatchedAnimations(const TArray<FAnimationVariant>& Variants) const
{
    TArray<FAnimationVariant> MatchedVariants;
    
    for (const FAnimationVariant& Variant : Variants)
    {
        float MatchScore = CalculatePersonalityMatch(Variant.RequiredPersonality, Variant.PersonalityTolerance);
        
        if (MatchScore > 0.0f)
        {
            FAnimationVariant MatchedVariant = Variant;
            MatchedVariant.Weight *= MatchScore;
            MatchedVariants.Add(MatchedVariant);
        }
    }
    
    return MatchedVariants;
}

float UAnimationSystemManager::CalculatePersonalityMatch(const FCharacterPersonality& RequiredPersonality, float Tolerance) const
{
    float BoldnessDiff = FMath::Abs(Personality.Boldness - RequiredPersonality.Boldness);
    float AggressionDiff = FMath::Abs(Personality.Aggression - RequiredPersonality.Aggression);
    float CuriosityDiff = FMath::Abs(Personality.Curiosity - RequiredPersonality.Curiosity);
    float SocialDiff = FMath::Abs(Personality.SocialTendency - RequiredPersonality.SocialTendency);
    float EnergyDiff = FMath::Abs(Personality.EnergyLevel - RequiredPersonality.EnergyLevel);
    
    float AverageDiff = (BoldnessDiff + AggressionDiff + CuriosityDiff + SocialDiff + EnergyDiff) / 5.0f;
    
    if (AverageDiff <= Tolerance)
    {
        return 1.0f - (AverageDiff / Tolerance);
    }
    
    return 0.0f;
}

void UAnimationSystemManager::UpdateMotionMatchingSetup()
{
    if (!MotionMatchingSetupTable)
    {
        return;
    }
    
    FString ContextString;
    TArray<FName> RowNames = MotionMatchingSetupTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (const FMotionMatchingSetup* Setup = MotionMatchingSetupTable->FindRow<FMotionMatchingSetup>(RowName, ContextString))
        {
            if (Setup->Archetype == CharacterArchetype)
            {
                CurrentSetup = Setup;
                break;
            }
        }
    }
}

void UAnimationSystemManager::UpdateEmotionalBlending(float DeltaTime)
{
    // Smooth blend personality values for natural transitions
    CurrentBoldnessBlend = FMath::FInterpTo(CurrentBoldnessBlend, TargetBoldnessBlend, DeltaTime, PersonalityBlendSpeed);
    CurrentAggressionBlend = FMath::FInterpTo(CurrentAggressionBlend, TargetAggressionBlend, DeltaTime, PersonalityBlendSpeed);
    CurrentCuriosityBlend = FMath::FInterpTo(CurrentCuriosityBlend, TargetCuriosityBlend, DeltaTime, PersonalityBlendSpeed);
}