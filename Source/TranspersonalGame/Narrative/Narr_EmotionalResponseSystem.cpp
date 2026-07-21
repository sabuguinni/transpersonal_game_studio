#include "Narr_EmotionalResponseSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UNarr_EmotionalResponseSystem::UNarr_EmotionalResponseSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentEmotionalState = ENarr_EmotionalState::Calm;
    CurrentIntensity = 0.5f;
    EmotionalDecayRate = 0.1f;
    bIsEmotionallyActive = true;
    EmotionalThreshold = 0.3f;
    EmotionalTimer = 0.0f;

    // Initialize default emotional responses
    FNarr_EmotionalResponse CalmResponse;
    CalmResponse.EmotionalState = ENarr_EmotionalState::Calm;
    CalmResponse.TriggerEvent = TEXT("Default");
    CalmResponse.DialogueResponse = TEXT("The day is peaceful. I feel at ease.");
    EmotionalResponses.Add(CalmResponse);

    FNarr_EmotionalResponse FearResponse;
    FearResponse.EmotionalState = ENarr_EmotionalState::Fear;
    FearResponse.TriggerEvent = TEXT("DinosaurSighting");
    FearResponse.DialogueResponse = TEXT("By the spirits! A great beast approaches! We must flee!");
    EmotionalResponses.Add(FearResponse);

    FNarr_EmotionalResponse AlertResponse;
    AlertResponse.EmotionalState = ENarr_EmotionalState::Alert;
    AlertResponse.TriggerEvent = TEXT("StrangerApproach");
    AlertResponse.DialogueResponse = TEXT("Who goes there? State your purpose in our lands.");
    EmotionalResponses.Add(AlertResponse);

    FNarr_EmotionalResponse CuriosityResponse;
    CuriosityResponse.EmotionalState = ENarr_EmotionalState::Curiosity;
    CuriosityResponse.TriggerEvent = TEXT("NewDiscovery");
    CuriosityResponse.DialogueResponse = TEXT("Fascinating... I have never seen such a thing before.");
    EmotionalResponses.Add(CuriosityResponse);
}

void UNarr_EmotionalResponseSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bIsEmotionallyActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Emotional Response System initialized for %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UNarr_EmotionalResponseSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsEmotionallyActive)
    {
        UpdateEmotionalDecay(DeltaTime);
    }
}

void UNarr_EmotionalResponseSystem::TriggerEmotionalResponse(const FString& EventName, ENarr_EmotionalState NewState, float Intensity)
{
    if (!bIsEmotionallyActive)
    {
        return;
    }

    // Find specific response for this event
    FNarr_EmotionalResponse* Response = FindEmotionalResponse(EventName);
    
    if (Response)
    {
        CurrentEmotionalState = Response->EmotionalState;
        CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        EmotionalTimer = Response->Duration;

        UE_LOG(LogTemp, Log, TEXT("Emotional response triggered: %s - State: %d, Intensity: %f"), 
               *EventName, (int32)CurrentEmotionalState, CurrentIntensity);

        // Broadcast emotional change if needed
        if (GetOwner())
        {
            UE_LOG(LogTemp, Log, TEXT("%s emotional state changed to %d with intensity %f"), 
                   *GetOwner()->GetName(), (int32)CurrentEmotionalState, CurrentIntensity);
        }
    }
    else
    {
        // Use provided state if no specific response found
        CurrentEmotionalState = NewState;
        CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        EmotionalTimer = 5.0f; // Default duration
    }
}

ENarr_EmotionalState UNarr_EmotionalResponseSystem::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

float UNarr_EmotionalResponseSystem::GetCurrentIntensity() const
{
    return CurrentIntensity;
}

FString UNarr_EmotionalResponseSystem::GetEmotionalDialogue() const
{
    FNarr_EmotionalResponse* Response = nullptr;
    
    // Find response matching current emotional state
    for (int32 i = 0; i < EmotionalResponses.Num(); i++)
    {
        if (EmotionalResponses[i].EmotionalState == CurrentEmotionalState)
        {
            Response = const_cast<FNarr_EmotionalResponse*>(&EmotionalResponses[i]);
            break;
        }
    }

    if (Response && !Response->DialogueResponse.IsEmpty())
    {
        return Response->DialogueResponse;
    }

    // Default responses based on emotional state
    switch (CurrentEmotionalState)
    {
        case ENarr_EmotionalState::Fear:
            return TEXT("I sense great danger nearby...");
        case ENarr_EmotionalState::Alert:
            return TEXT("Something is not right here.");
        case ENarr_EmotionalState::Anger:
            return TEXT("This cannot be tolerated!");
        case ENarr_EmotionalState::Curiosity:
            return TEXT("How interesting...");
        case ENarr_EmotionalState::Sadness:
            return TEXT("These are dark times indeed.");
        case ENarr_EmotionalState::Relief:
            return TEXT("Thank the spirits, we are safe.");
        case ENarr_EmotionalState::Excitement:
            return TEXT("This is wonderful news!");
        default:
            return TEXT("The day continues as always.");
    }
}

void UNarr_EmotionalResponseSystem::SetEmotionalState(ENarr_EmotionalState NewState, float Intensity)
{
    CurrentEmotionalState = NewState;
    CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    EmotionalTimer = 5.0f;
}

void UNarr_EmotionalResponseSystem::ResetEmotionalState()
{
    CurrentEmotionalState = ENarr_EmotionalState::Calm;
    CurrentIntensity = 0.5f;
    EmotionalTimer = 0.0f;
}

bool UNarr_EmotionalResponseSystem::IsEmotionallyActive() const
{
    return bIsEmotionallyActive && CurrentIntensity > EmotionalThreshold;
}

void UNarr_EmotionalResponseSystem::UpdateEmotionalDecay(float DeltaTime)
{
    if (EmotionalTimer > 0.0f)
    {
        EmotionalTimer -= DeltaTime;
        
        if (EmotionalTimer <= 0.0f)
        {
            // Gradually decay emotional intensity
            CurrentIntensity = FMath::Max(0.0f, CurrentIntensity - (EmotionalDecayRate * DeltaTime));
            
            // Return to calm state when intensity is very low
            if (CurrentIntensity < EmotionalThreshold)
            {
                CurrentEmotionalState = ENarr_EmotionalState::Calm;
                CurrentIntensity = 0.5f;
            }
        }
    }
}

FNarr_EmotionalResponse* UNarr_EmotionalResponseSystem::FindEmotionalResponse(const FString& EventName)
{
    for (int32 i = 0; i < EmotionalResponses.Num(); i++)
    {
        if (EmotionalResponses[i].TriggerEvent.Equals(EventName, ESearchCase::IgnoreCase))
        {
            return &EmotionalResponses[i];
        }
    }
    return nullptr;
}