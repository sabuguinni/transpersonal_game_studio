#include "Narr_CharacterPersonalitySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNarr_CharacterPersonalitySystem::UNarr_CharacterPersonalitySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f;
    
    // Initialize default personality traits
    PersonalityTraits.Add(ENarr_PersonalityTrait::Cautious, 0.7f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Analytical, 0.8f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Protective, 0.6f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Curious, 0.5f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Authoritative, 0.4f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Empathetic, 0.6f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Pragmatic, 0.9f);
    PersonalityTraits.Add(ENarr_PersonalityTrait::Optimistic, 0.3f);
    
    CurrentEmotionalState = ENarr_EmotionalState::Calm;
    EmotionalIntensity = 0.5f;
    BaseStressLevel = 0.3f;
    CurrentStressLevel = BaseStressLevel;
    
    PersonalityUpdateInterval = 5.0f;
    EmotionalDecayRate = 0.1f;
    StressRecoveryRate = 0.05f;
}

void UNarr_CharacterPersonalitySystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            PersonalityUpdateTimer,
            this,
            &UNarr_CharacterPersonalitySystem::UpdatePersonalityState,
            PersonalityUpdateInterval,
            true
        );
    }
    
    InitializeCharacterPersonality();
}

void UNarr_CharacterPersonalitySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmotionalState(DeltaTime);
    UpdateStressLevel(DeltaTime);
    ProcessPersonalityInfluences(DeltaTime);
}

void UNarr_CharacterPersonalitySystem::InitializeCharacterPersonality()
{
    // Set character-specific personality based on character type
    if (CharacterType == ENarr_CharacterType::Scientist)
    {
        PersonalityTraits[ENarr_PersonalityTrait::Analytical] = 0.9f;
        PersonalityTraits[ENarr_PersonalityTrait::Curious] = 0.8f;
        PersonalityTraits[ENarr_PersonalityTrait::Cautious] = 0.7f;
        PersonalityTraits[ENarr_PersonalityTrait::Pragmatic] = 0.8f;
    }
    else if (CharacterType == ENarr_CharacterType::MilitaryOfficer)
    {
        PersonalityTraits[ENarr_PersonalityTrait::Authoritative] = 0.9f;
        PersonalityTraits[ENarr_PersonalityTrait::Protective] = 0.8f;
        PersonalityTraits[ENarr_PersonalityTrait::Pragmatic] = 0.9f;
        PersonalityTraits[ENarr_PersonalityTrait::Cautious] = 0.8f;
    }
    else if (CharacterType == ENarr_CharacterType::FieldResearcher)
    {
        PersonalityTraits[ENarr_PersonalityTrait::Curious] = 0.9f;
        PersonalityTraits[ENarr_PersonalityTrait::Analytical] = 0.7f;
        PersonalityTraits[ENarr_PersonalityTrait::Optimistic] = 0.6f;
        PersonalityTraits[ENarr_PersonalityTrait::Empathetic] = 0.7f;
    }
    else if (CharacterType == ENarr_CharacterType::Veterinarian)
    {
        PersonalityTraits[ENarr_PersonalityTrait::Empathetic] = 0.9f;
        PersonalityTraits[ENarr_PersonalityTrait::Protective] = 0.8f;
        PersonalityTraits[ENarr_PersonalityTrait::Analytical] = 0.7f;
        PersonalityTraits[ENarr_PersonalityTrait::Cautious] = 0.8f;
    }
    
    OnPersonalityInitialized.Broadcast(CharacterType, PersonalityTraits);
}

void UNarr_CharacterPersonalitySystem::UpdatePersonalityState()
{
    // Adjust personality traits based on recent experiences
    for (auto& Trait : PersonalityTraits)
    {
        float CurrentValue = Trait.Value;
        
        // Stress affects different traits differently
        if (CurrentStressLevel > 0.7f)
        {
            if (Trait.Key == ENarr_PersonalityTrait::Cautious)
            {
                Trait.Value = FMath::Min(1.0f, CurrentValue + 0.1f);
            }
            else if (Trait.Key == ENarr_PersonalityTrait::Optimistic)
            {
                Trait.Value = FMath::Max(0.0f, CurrentValue - 0.1f);
            }
        }
        
        // Emotional state influences personality expression
        if (CurrentEmotionalState == ENarr_EmotionalState::Excited)
        {
            if (Trait.Key == ENarr_PersonalityTrait::Curious)
            {
                Trait.Value = FMath::Min(1.0f, CurrentValue + 0.05f);
            }
        }
        else if (CurrentEmotionalState == ENarr_EmotionalState::Fearful)
        {
            if (Trait.Key == ENarr_PersonalityTrait::Protective)
            {
                Trait.Value = FMath::Min(1.0f, CurrentValue + 0.1f);
            }
        }
    }
    
    OnPersonalityUpdated.Broadcast(PersonalityTraits, CurrentEmotionalState);
}

void UNarr_CharacterPersonalitySystem::UpdateEmotionalState(float DeltaTime)
{
    // Gradually return to calm state if no external influences
    if (EmotionalIntensity > 0.1f)
    {
        EmotionalIntensity = FMath::Max(0.0f, EmotionalIntensity - (EmotionalDecayRate * DeltaTime));
    }
    else
    {
        CurrentEmotionalState = ENarr_EmotionalState::Calm;
        EmotionalIntensity = 0.1f;
    }
}

void UNarr_CharacterPersonalitySystem::UpdateStressLevel(float DeltaTime)
{
    // Gradually recover from stress
    if (CurrentStressLevel > BaseStressLevel)
    {
        CurrentStressLevel = FMath::Max(BaseStressLevel, CurrentStressLevel - (StressRecoveryRate * DeltaTime));
    }
}

void UNarr_CharacterPersonalitySystem::ProcessPersonalityInfluences(float DeltaTime)
{
    // Process environmental and situational influences on personality
    if (GetWorld())
    {
        // Check for nearby threats or interesting discoveries
        // This would integrate with other game systems like dinosaur AI or quest system
        
        // Example: High stress environment makes characters more cautious
        if (IsInDangerousArea())
        {
            ModifyStressLevel(0.1f);
            TriggerEmotionalResponse(ENarr_EmotionalState::Fearful, 0.6f);
        }
        
        // Example: Successful research discoveries make characters more optimistic
        if (HasRecentDiscovery())
        {
            TriggerEmotionalResponse(ENarr_EmotionalState::Excited, 0.7f);
            ModifyPersonalityTrait(ENarr_PersonalityTrait::Optimistic, 0.05f);
        }
    }
}

void UNarr_CharacterPersonalitySystem::TriggerEmotionalResponse(ENarr_EmotionalState NewState, float Intensity)
{
    CurrentEmotionalState = NewState;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    OnEmotionalStateChanged.Broadcast(CurrentEmotionalState, EmotionalIntensity);
    
    // Trigger appropriate voice lines or animations based on emotional state
    PlayEmotionalVoiceLine(NewState, Intensity);
}

void UNarr_CharacterPersonalitySystem::ModifyPersonalityTrait(ENarr_PersonalityTrait Trait, float Delta)
{
    if (PersonalityTraits.Contains(Trait))
    {
        float NewValue = FMath::Clamp(PersonalityTraits[Trait] + Delta, 0.0f, 1.0f);
        PersonalityTraits[Trait] = NewValue;
        
        OnPersonalityTraitChanged.Broadcast(Trait, NewValue);
    }
}

void UNarr_CharacterPersonalitySystem::ModifyStressLevel(float Delta)
{
    CurrentStressLevel = FMath::Clamp(CurrentStressLevel + Delta, 0.0f, 1.0f);
    OnStressLevelChanged.Broadcast(CurrentStressLevel);
}

float UNarr_CharacterPersonalitySystem::GetPersonalityTrait(ENarr_PersonalityTrait Trait) const
{
    if (PersonalityTraits.Contains(Trait))
    {
        return PersonalityTraits[Trait];
    }
    return 0.0f;
}

FString UNarr_CharacterPersonalitySystem::GetPersonalityDescription() const
{
    FString Description = TEXT("Personality Profile: ");
    
    for (const auto& Trait : PersonalityTraits)
    {
        FString TraitName = GetPersonalityTraitName(Trait.Key);
        float Value = Trait.Value;
        
        if (Value > 0.7f)
        {
            Description += FString::Printf(TEXT("High %s, "), *TraitName);
        }
        else if (Value > 0.4f)
        {
            Description += FString::Printf(TEXT("Moderate %s, "), *TraitName);
        }
    }
    
    Description += FString::Printf(TEXT("Current State: %s"), *GetEmotionalStateName(CurrentEmotionalState));
    
    return Description;
}

FString UNarr_CharacterPersonalitySystem::GetPersonalityTraitName(ENarr_PersonalityTrait Trait) const
{
    switch (Trait)
    {
        case ENarr_PersonalityTrait::Cautious: return TEXT("Cautious");
        case ENarr_PersonalityTrait::Analytical: return TEXT("Analytical");
        case ENarr_PersonalityTrait::Protective: return TEXT("Protective");
        case ENarr_PersonalityTrait::Curious: return TEXT("Curious");
        case ENarr_PersonalityTrait::Authoritative: return TEXT("Authoritative");
        case ENarr_PersonalityTrait::Empathetic: return TEXT("Empathetic");
        case ENarr_PersonalityTrait::Pragmatic: return TEXT("Pragmatic");
        case ENarr_PersonalityTrait::Optimistic: return TEXT("Optimistic");
        default: return TEXT("Unknown");
    }
}

FString UNarr_CharacterPersonalitySystem::GetEmotionalStateName(ENarr_EmotionalState State) const
{
    switch (State)
    {
        case ENarr_EmotionalState::Calm: return TEXT("Calm");
        case ENarr_EmotionalState::Excited: return TEXT("Excited");
        case ENarr_EmotionalState::Fearful: return TEXT("Fearful");
        case ENarr_EmotionalState::Angry: return TEXT("Angry");
        case ENarr_EmotionalState::Sad: return TEXT("Sad");
        case ENarr_EmotionalState::Surprised: return TEXT("Surprised");
        case ENarr_EmotionalState::Focused: return TEXT("Focused");
        case ENarr_EmotionalState::Stressed: return TEXT("Stressed");
        default: return TEXT("Unknown");
    }
}

bool UNarr_CharacterPersonalitySystem::IsInDangerousArea() const
{
    // This would integrate with the game's danger detection system
    // For now, return false as placeholder
    return false;
}

bool UNarr_CharacterPersonalitySystem::HasRecentDiscovery() const
{
    // This would integrate with the quest/discovery system
    // For now, return false as placeholder
    return false;
}

void UNarr_CharacterPersonalitySystem::PlayEmotionalVoiceLine(ENarr_EmotionalState State, float Intensity)
{
    // This would integrate with the voice system to play appropriate audio
    // Based on character type and emotional state
    
    if (AActor* Owner = GetOwner())
    {
        UE_LOG(LogTemp, Log, TEXT("Character %s expressing %s emotion with intensity %f"), 
               *Owner->GetName(), *GetEmotionalStateName(State), Intensity);
    }
}