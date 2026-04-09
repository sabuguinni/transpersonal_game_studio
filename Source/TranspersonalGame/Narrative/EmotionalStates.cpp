#include "EmotionalStates.h"
#include "Engine/Engine.h"

bool UEmotionalStateUtilities::CanTransitionTo(EEmotionalState FromState, EEmotionalState ToState)
{
    // Define natural emotional state transitions based on psychological principles
    switch (FromState)
    {
        case EEmotionalState::Calm:
            // From calm, can transition to any state based on external stimuli
            return true;
            
        case EEmotionalState::Anxious:
            // From anxious, can escalate to terror or calm down
            return (ToState == EEmotionalState::Terrified ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Curious ||
                    ToState == EEmotionalState::Determined);
                    
        case EEmotionalState::Terrified:
            // From terrified, usually need to calm down first or become exhausted
            return (ToState == EEmotionalState::Anxious ||
                    ToState == EEmotionalState::Exhausted ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Determined); // Fight or flight response
                    
        case EEmotionalState::Exhausted:
            // From exhausted, need rest to transition to other states
            return (ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Hopeful ||
                    ToState == EEmotionalState::Loneliness);
                    
        case EEmotionalState::Curious:
            // Curiosity can lead to wonder, discovery, or anxiety if danger is perceived
            return (ToState == EEmotionalState::Wonder ||
                    ToState == EEmotionalState::Discovery ||
                    ToState == EEmotionalState::Anxious ||
                    ToState == EEmotionalState::Fear ||
                    ToState == EEmotionalState::Calm);
                    
        case EEmotionalState::Wonder:
            // Wonder can lead to discovery or back to calm
            return (ToState == EEmotionalState::Discovery ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Connection ||
                    ToState == EEmotionalState::Curious);
                    
        case EEmotionalState::Discovery:
            // Discovery brings achievement and hope
            return (ToState == EEmotionalState::Achievement ||
                    ToState == EEmotionalState::Hopeful ||
                    ToState == EEmotionalState::Wonder ||
                    ToState == EEmotionalState::Calm);
                    
        case EEmotionalState::Loneliness:
            // Loneliness can lead to connection or deeper emotional states
            return (ToState == EEmotionalState::Connection ||
                    ToState == EEmotionalState::Anxious ||
                    ToState == EEmotionalState::Hopeful ||
                    ToState == EEmotionalState::Curious);
                    
        case EEmotionalState::Connection:
            // Connection brings peace and empathy
            return (ToState == EEmotionalState::Empathy ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Hopeful ||
                    ToState == EEmotionalState::Wonder);
                    
        case EEmotionalState::Empathy:
            // Empathy can lead to connection or protective determination
            return (ToState == EEmotionalState::Connection ||
                    ToState == EEmotionalState::Determined ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Anxious); // If empathy reveals danger
                    
        case EEmotionalState::Determined:
            // Determination can lead to achievement or exhaustion
            return (ToState == EEmotionalState::Achievement ||
                    ToState == EEmotionalState::Exhausted ||
                    ToState == EEmotionalState::Tension ||
                    ToState == EEmotionalState::Calm);
                    
        case EEmotionalState::Achievement:
            // Achievement brings hope and calm satisfaction
            return (ToState == EEmotionalState::Hopeful ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Wonder ||
                    ToState == EEmotionalState::Determined); // Ready for next challenge
                    
        case EEmotionalState::Hopeful:
            // Hope can lead to determination or back to calm
            return (ToState == EEmotionalState::Determined ||
                    ToState == EEmotionalState::Calm ||
                    ToState == EEmotionalState::Curious ||
                    ToState == EEmotionalState::Anxious); // If hope is threatened
                    
        case EEmotionalState::Fear:
            // Fear can escalate to terror or resolve to anxiety/calm
            return (ToState == EEmotionalState::Terrified ||
                    ToState == EEmotionalState::Anxious ||
                    ToState == EEmotionalState::Determined || // Fight response
                    ToState == EEmotionalState::Calm);
                    
        case EEmotionalState::Tension:
            // Tension can resolve to various states
            return (ToState == EEmotionalState::Fear ||
                    ToState == EEmotionalState::Determined ||
                    ToState == EEmotionalState::Exhausted ||
                    ToState == EEmotionalState::Calm);
                    
        default:
            return false;
    }
}

float UEmotionalStateUtilities::GetTransitionDuration(EEmotionalState FromState, EEmotionalState ToState)
{
    // Base transition time in seconds
    float BaseTime = 2.0f;
    
    // High energy to low energy states take longer
    if (IsHighEnergyState(FromState) && !IsHighEnergyState(ToState))
    {
        BaseTime *= 2.0f;
    }
    
    // Traumatic states (terror, fear) take longer to resolve
    if (FromState == EEmotionalState::Terrified || FromState == EEmotionalState::Fear)
    {
        BaseTime *= 1.5f;
    }
    
    // Positive discoveries happen quickly
    if (ToState == EEmotionalState::Discovery || ToState == EEmotionalState::Wonder)
    {
        BaseTime *= 0.5f;
    }
    
    return BaseTime;
}

TArray<EEmotionalState> UEmotionalStateUtilities::GetNaturalTransitions(EEmotionalState CurrentState)
{
    TArray<EEmotionalState> PossibleTransitions;
    
    // Check all possible states for valid transitions
    for (int32 i = 0; i < (int32)EEmotionalState::Tension + 1; ++i)
    {
        EEmotionalState TestState = (EEmotionalState)i;
        if (CanTransitionTo(CurrentState, TestState))
        {
            PossibleTransitions.Add(TestState);
        }
    }
    
    return PossibleTransitions;
}

bool UEmotionalStateUtilities::IsPositiveState(EEmotionalState State)
{
    switch (State)
    {
        case EEmotionalState::Calm:
        case EEmotionalState::Curious:
        case EEmotionalState::Wonder:
        case EEmotionalState::Discovery:
        case EEmotionalState::Connection:
        case EEmotionalState::Empathy:
        case EEmotionalState::Achievement:
        case EEmotionalState::Hopeful:
            return true;
            
        default:
            return false;
    }
}

bool UEmotionalStateUtilities::IsHighEnergyState(EEmotionalState State)
{
    switch (State)
    {
        case EEmotionalState::Terrified:
        case EEmotionalState::Curious:
        case EEmotionalState::Wonder:
        case EEmotionalState::Discovery:
        case EEmotionalState::Determined:
        case EEmotionalState::Achievement:
        case EEmotionalState::Fear:
        case EEmotionalState::Tension:
            return true;
            
        default:
            return false;
    }
}

float UEmotionalStateUtilities::GetEmotionalWeight(EEmotionalState State)
{
    // Emotional weight affects narrative importance and memory formation
    switch (State)
    {
        case EEmotionalState::Terrified:
        case EEmotionalState::Discovery:
        case EEmotionalState::Achievement:
            return 1.0f; // Maximum impact
            
        case EEmotionalState::Fear:
        case EEmotionalState::Wonder:
        case EEmotionalState::Connection:
        case EEmotionalState::Empathy:
            return 0.8f; // High impact
            
        case EEmotionalState::Anxious:
        case EEmotionalState::Curious:
        case EEmotionalState::Determined:
        case EEmotionalState::Hopeful:
        case EEmotionalState::Tension:
            return 0.6f; // Moderate impact
            
        case EEmotionalState::Loneliness:
        case EEmotionalState::Exhausted:
            return 0.4f; // Lower impact
            
        case EEmotionalState::Calm:
        default:
            return 0.2f; // Baseline
    }
}

FEmotionalContext UEmotionalStateUtilities::CreateEmotionalContext(EEmotionalState State, EEmotionalIntensity Intensity, const FString& Trigger)
{
    FEmotionalContext Context;
    Context.PrimaryState = State;
    Context.Intensity = Intensity;
    Context.Trigger = Trigger;
    Context.Duration = 0.0f;
    Context.bIsTransitioning = false;
    
    return Context;
}

bool UEmotionalStateUtilities::IsEmotionalContextValid(const FEmotionalContext& Context)
{
    // Check if the context makes psychological sense
    if (Context.Duration < 0.0f)
    {
        return false;
    }
    
    // Very high intensity states shouldn't last too long without consequences
    if (Context.Intensity == EEmotionalIntensity::Overwhelming && Context.Duration > 300.0f) // 5 minutes
    {
        return false;
    }
    
    return true;
}

FEmotionalContext UEmotionalStateUtilities::BlendEmotionalContexts(const FEmotionalContext& A, const FEmotionalContext& B, float BlendWeight)
{
    FEmotionalContext BlendedContext;
    
    // Choose primary state based on blend weight and emotional weight
    float WeightA = GetEmotionalWeight(A.PrimaryState) * (1.0f - BlendWeight);
    float WeightB = GetEmotionalWeight(B.PrimaryState) * BlendWeight;
    
    if (WeightA > WeightB)
    {
        BlendedContext.PrimaryState = A.PrimaryState;
        BlendedContext.Intensity = A.Intensity;
    }
    else
    {
        BlendedContext.PrimaryState = B.PrimaryState;
        BlendedContext.Intensity = B.Intensity;
    }
    
    // Blend duration
    BlendedContext.Duration = FMath::Lerp(A.Duration, B.Duration, BlendWeight);
    
    // Combine secondary states
    BlendedContext.SecondaryStates = A.SecondaryStates;
    for (EEmotionalState SecondaryState : B.SecondaryStates)
    {
        BlendedContext.SecondaryStates.AddUnique(SecondaryState);
    }
    
    // Use the more recent trigger
    BlendedContext.Trigger = BlendWeight > 0.5f ? B.Trigger : A.Trigger;
    
    return BlendedContext;
}