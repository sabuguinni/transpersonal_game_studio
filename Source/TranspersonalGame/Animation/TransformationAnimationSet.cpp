#include "TransformationAnimationSet.h"
#include "Engine/Engine.h"

UTransformationAnimationSet::UTransformationAnimationSet()
{
    InitializeDefaultAnimationSets();
}

void UTransformationAnimationSet::InitializeDefaultAnimationSets()
{
    // Initialize state animation sets with empty structures
    StateAnimations.Add(EConsciousnessState::Neutral, FStateAnimationSet());
    StateAnimations.Add(EConsciousnessState::Meditative, FStateAnimationSet());
    StateAnimations.Add(EConsciousnessState::Transcendent, FStateAnimationSet());
    StateAnimations.Add(EConsciousnessState::Shadow, FStateAnimationSet());
    StateAnimations.Add(EConsciousnessState::Integrated, FStateAnimationSet());
    StateAnimations.Add(EConsciousnessState::Transforming, FStateAnimationSet());

    // Initialize common transformation sequences
    FTransformationSequence NeutralToMeditative;
    NeutralToMeditative.FromState = EConsciousnessState::Neutral;
    NeutralToMeditative.ToState = EConsciousnessState::Meditative;
    NeutralToMeditative.TransformationDuration = 2.0f;
    TransformationSequences.Add(NeutralToMeditative);

    FTransformationSequence MeditativeToTranscendent;
    MeditativeToTranscendent.FromState = EConsciousnessState::Meditative;
    MeditativeToTranscendent.ToState = EConsciousnessState::Transcendent;
    MeditativeToTranscendent.TransformationDuration = 3.0f;
    TransformationSequences.Add(MeditativeToTranscendent);

    FTransformationSequence NeutralToShadow;
    NeutralToShadow.FromState = EConsciousnessState::Neutral;
    NeutralToShadow.ToState = EConsciousnessState::Shadow;
    NeutralToShadow.TransformationDuration = 1.5f;
    TransformationSequences.Add(NeutralToShadow);

    FTransformationSequence ShadowToIntegrated;
    ShadowToIntegrated.FromState = EConsciousnessState::Shadow;
    ShadowToIntegrated.ToState = EConsciousnessState::Integrated;
    ShadowToIntegrated.TransformationDuration = 4.0f;
    TransformationSequences.Add(ShadowToIntegrated);

    FTransformationSequence IntegratedToTranscendent;
    IntegratedToTranscendent.FromState = EConsciousnessState::Integrated;
    IntegratedToTranscendent.ToState = EConsciousnessState::Transcendent;
    IntegratedToTranscendent.TransformationDuration = 2.5f;
    TransformationSequences.Add(IntegratedToTranscendent);
}

FStateAnimationSet UTransformationAnimationSet::GetAnimationSetForState(EConsciousnessState State)
{
    if (StateAnimations.Contains(State))
    {
        return StateAnimations[State];
    }
    
    // Return universal animations as fallback
    return UniversalAnimations;
}

UAnimMontage* UTransformationAnimationSet::GetTransformationMontage(EConsciousnessState FromState, EConsciousnessState ToState)
{
    for (const FTransformationSequence& Sequence : TransformationSequences)
    {
        if (Sequence.FromState == FromState && Sequence.ToState == ToState)
        {
            return Sequence.TransformationMontage;
        }
    }
    
    return nullptr;
}

float UTransformationAnimationSet::GetTransformationDuration(EConsciousnessState FromState, EConsciousnessState ToState)
{
    for (const FTransformationSequence& Sequence : TransformationSequences)
    {
        if (Sequence.FromState == FromState && Sequence.ToState == ToState)
        {
            return Sequence.TransformationDuration;
        }
    }
    
    // Default duration if no specific sequence found
    return 2.0f;
}

TArray<UAnimMontage*> UTransformationAnimationSet::GetGesturesForState(EConsciousnessState State)
{
    if (StateAnimations.Contains(State))
    {
        return StateAnimations[State].GestureMontages;
    }
    
    // Return universal gestures as fallback
    return UniversalAnimations.GestureMontages;
}