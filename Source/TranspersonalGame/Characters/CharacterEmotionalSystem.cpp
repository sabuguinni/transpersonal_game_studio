#include "CharacterEmotionalSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

UCharacterEmotionalSystem::UCharacterEmotionalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update emotions 10 times per second
    
    // Initialize emotional states
    CurrentEmotionalState.Fear = 0.0f;
    CurrentEmotionalState.Curiosity = 50.0f;
    CurrentEmotionalState.Aggression = 0.0f;
    CurrentEmotionalState.Calmness = 100.0f;
    CurrentEmotionalState.Hunger = 30.0f;
    
    EmotionalDecayRate = 5.0f;
    MaxEmotionalIntensity = 100.0f;
    
    bIsEmotionalSystemActive = true;
}

void UCharacterEmotionalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Start emotional decay timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            EmotionalDecayTimer,
            this,
            &UCharacterEmotionalSystem::ProcessEmotionalDecay,
            1.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Character Emotional System initialized"));
}

void UCharacterEmotionalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsEmotionalSystemActive)
    {
        UpdateEmotionalState(DeltaTime);
        ProcessEmotionalReactions(DeltaTime);
    }
}

void UCharacterEmotionalSystem::TriggerEmotionalResponse(EEmotionalTrigger Trigger, float Intensity)
{
    if (!bIsEmotionalSystemActive) return;
    
    Intensity = FMath::Clamp(Intensity, 0.0f, MaxEmotionalIntensity);
    
    switch (Trigger)
    {
        case EEmotionalTrigger::DinosaurSighting:
            ModifyEmotion(TEXT("Fear"), Intensity * 0.8f);
            ModifyEmotion(TEXT("Curiosity"), Intensity * 0.3f);
            ModifyEmotion(TEXT("Calmness"), -Intensity * 0.6f);
            break;
            
        case EEmotionalTrigger::PredatorNearby:
            ModifyEmotion(TEXT("Fear"), Intensity);
            ModifyEmotion(TEXT("Aggression"), Intensity * 0.4f);
            ModifyEmotion(TEXT("Calmness"), -Intensity);
            break;
            
        case EEmotionalTrigger::SafeAreaReached:
            ModifyEmotion(TEXT("Calmness"), Intensity * 0.7f);
            ModifyEmotion(TEXT("Fear"), -Intensity * 0.5f);
            break;
            
        case EEmotionalTrigger::FoodFound:
            ModifyEmotion(TEXT("Hunger"), -Intensity);
            ModifyEmotion(TEXT("Calmness"), Intensity * 0.3f);
            break;
            
        case EEmotionalTrigger::LowHealth:
            ModifyEmotion(TEXT("Fear"), Intensity * 0.6f);
            ModifyEmotion(TEXT("Aggression"), -Intensity * 0.3f);
            break;
    }
    
    // Broadcast emotional change
    OnEmotionalStateChanged.Broadcast(CurrentEmotionalState);
}

void UCharacterEmotionalSystem::ModifyEmotion(const FString& EmotionName, float Delta)
{
    if (EmotionName == TEXT("Fear"))
    {
        CurrentEmotionalState.Fear = FMath::Clamp(CurrentEmotionalState.Fear + Delta, 0.0f, MaxEmotionalIntensity);
    }
    else if (EmotionName == TEXT("Curiosity"))
    {
        CurrentEmotionalState.Curiosity = FMath::Clamp(CurrentEmotionalState.Curiosity + Delta, 0.0f, MaxEmotionalIntensity);
    }
    else if (EmotionName == TEXT("Aggression"))
    {
        CurrentEmotionalState.Aggression = FMath::Clamp(CurrentEmotionalState.Aggression + Delta, 0.0f, MaxEmotionalIntensity);
    }
    else if (EmotionName == TEXT("Calmness"))
    {
        CurrentEmotionalState.Calmness = FMath::Clamp(CurrentEmotionalState.Calmness + Delta, 0.0f, MaxEmotionalIntensity);
    }
    else if (EmotionName == TEXT("Hunger"))
    {
        CurrentEmotionalState.Hunger = FMath::Clamp(CurrentEmotionalState.Hunger + Delta, 0.0f, MaxEmotionalIntensity);
    }
}

FEmotionalState UCharacterEmotionalSystem::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

float UCharacterEmotionalSystem::GetEmotionIntensity(const FString& EmotionName) const
{
    if (EmotionName == TEXT("Fear")) return CurrentEmotionalState.Fear;
    if (EmotionName == TEXT("Curiosity")) return CurrentEmotionalState.Curiosity;
    if (EmotionName == TEXT("Aggression")) return CurrentEmotionalState.Aggression;
    if (EmotionName == TEXT("Calmness")) return CurrentEmotionalState.Calmness;
    if (EmotionName == TEXT("Hunger")) return CurrentEmotionalState.Hunger;
    
    return 0.0f;
}

void UCharacterEmotionalSystem::SetEmotionalSystemActive(bool bActive)
{
    bIsEmotionalSystemActive = bActive;
    
    if (!bActive && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(EmotionalDecayTimer);
    }
    else if (bActive && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            EmotionalDecayTimer,
            this,
            &UCharacterEmotionalSystem::ProcessEmotionalDecay,
            1.0f,
            true
        );
    }
}

void UCharacterEmotionalSystem::UpdateEmotionalState(float DeltaTime)
{
    // Emotional state interactions - fear reduces curiosity, calmness reduces aggression, etc.
    if (CurrentEmotionalState.Fear > 70.0f)
    {
        CurrentEmotionalState.Curiosity = FMath::Max(0.0f, CurrentEmotionalState.Curiosity - (DeltaTime * 10.0f));
    }
    
    if (CurrentEmotionalState.Calmness > 60.0f)
    {
        CurrentEmotionalState.Aggression = FMath::Max(0.0f, CurrentEmotionalState.Aggression - (DeltaTime * 15.0f));
        CurrentEmotionalState.Fear = FMath::Max(0.0f, CurrentEmotionalState.Fear - (DeltaTime * 8.0f));
    }
    
    // Hunger slowly increases over time
    CurrentEmotionalState.Hunger = FMath::Min(MaxEmotionalIntensity, CurrentEmotionalState.Hunger + (DeltaTime * 2.0f));
}

void UCharacterEmotionalSystem::ProcessEmotionalDecay()
{
    // Gradually return emotions to baseline
    CurrentEmotionalState.Fear = FMath::Max(0.0f, CurrentEmotionalState.Fear - EmotionalDecayRate);
    CurrentEmotionalState.Aggression = FMath::Max(0.0f, CurrentEmotionalState.Aggression - EmotionalDecayRate);
    
    // Curiosity and calmness return to moderate baseline
    if (CurrentEmotionalState.Curiosity < 50.0f)
    {
        CurrentEmotionalState.Curiosity = FMath::Min(50.0f, CurrentEmotionalState.Curiosity + EmotionalDecayRate * 0.5f);
    }
    else if (CurrentEmotionalState.Curiosity > 50.0f)
    {
        CurrentEmotionalState.Curiosity = FMath::Max(50.0f, CurrentEmotionalState.Curiosity - EmotionalDecayRate * 0.3f);
    }
    
    if (CurrentEmotionalState.Calmness < 70.0f)
    {
        CurrentEmotionalState.Calmness = FMath::Min(70.0f, CurrentEmotionalState.Calmness + EmotionalDecayRate * 0.7f);
    }
}

void UCharacterEmotionalSystem::ProcessEmotionalReactions(float DeltaTime)
{
    // Trigger behavioral changes based on emotional state
    if (CurrentEmotionalState.Fear > 80.0f)
    {
        OnHighFearState.Broadcast();
    }
    
    if (CurrentEmotionalState.Aggression > 70.0f)
    {
        OnAggressiveState.Broadcast();
    }
    
    if (CurrentEmotionalState.Hunger > 85.0f)
    {
        OnHighHungerState.Broadcast();
    }
    
    if (CurrentEmotionalState.Calmness > 90.0f && CurrentEmotionalState.Fear < 10.0f)
    {
        OnCalmState.Broadcast();
    }
}