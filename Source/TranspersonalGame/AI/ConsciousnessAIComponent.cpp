#include "ConsciousnessAIComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UConsciousnessAIComponent::UConsciousnessAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    TimeSinceLastUpdate = 0.0f;
    DetectedPlayer = nullptr;
}

void UConsciousnessAIComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize consciousness profile with random variations
    ConsciousnessProfile.AwarenessLevel = FMath::RandRange(0.1f, 0.3f);
    ConsciousnessProfile.SpiritualResonance = FMath::RandRange(0.0f, 0.2f);
    ConsciousnessProfile.EmpathyLevel = FMath::RandRange(0.1f, 0.4f);
    ConsciousnessProfile.PlayerRelationship = 0.0f;
    
    // Add initial memory
    AddMemory("I sense a presence in this realm...");
}

void UConsciousnessAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdatePlayerDetection();
        UpdateConsciousnessState();
        EvolveConsciousness(DeltaTime);
        UpdateEmotionalState();
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void UConsciousnessAIComponent::UpdateConsciousnessState()
{
    EConsciousnessState NewState = CalculateNextConsciousnessState();
    
    if (NewState != ConsciousnessProfile.CurrentState)
    {
        ConsciousnessProfile.CurrentState = NewState;
        
        // Log state change for debugging
        FString StateMessage = FString::Printf(TEXT("Consciousness evolved to: %s"), 
            *UEnum::GetValueAsString(NewState));
        AddMemory(StateMessage);
        
        UE_LOG(LogTemp, Log, TEXT("NPC Consciousness State Changed: %s"), *StateMessage);
    }
}

void UConsciousnessAIComponent::ProcessPlayerInteraction(AActor* Player)
{
    if (!Player) return;
    
    DetectedPlayer = Player;
    
    // Increase awareness through interaction
    ConsciousnessProfile.AwarenessLevel = FMath::Clamp(
        ConsciousnessProfile.AwarenessLevel + 0.1f, 0.0f, 1.0f);
    
    // Improve relationship based on interaction quality
    float InteractionQuality = FMath::RandRange(0.1f, 0.3f);
    ConsciousnessProfile.PlayerRelationship = FMath::Clamp(
        ConsciousnessProfile.PlayerRelationship + InteractionQuality, -1.0f, 1.0f);
    
    // Generate contextual memory
    FString InteractionMemory = FString::Printf(
        TEXT("Meaningful exchange with the seeker. Awareness expanded."));
    AddMemory(InteractionMemory);
    
    // Trigger spiritual resonance
    ConsciousnessProfile.SpiritualResonance = FMath::Clamp(
        ConsciousnessProfile.SpiritualResonance + 0.05f, 0.0f, 1.0f);
}

void UConsciousnessAIComponent::RespondToEnvironmentalChange(const FString& ChangeType, float Intensity)
{
    if (ChangeType == "SacredSpace")
    {
        ConsciousnessProfile.SpiritualResonance += Intensity * 0.2f;
        AddMemory("The sacred energies flow through this place...");
    }
    else if (ChangeType == "Conflict")
    {
        ConsciousnessProfile.EmpathyLevel += Intensity * 0.1f;
        AddMemory("I sense disturbance in the harmony...");
    }
    else if (ChangeType == "Meditation")
    {
        ConsciousnessProfile.AwarenessLevel += Intensity * 0.15f;
        AddMemory("The stillness reveals deeper truths...");
    }
    
    // Clamp all values
    ConsciousnessProfile.SpiritualResonance = FMath::Clamp(ConsciousnessProfile.SpiritualResonance, 0.0f, 1.0f);
    ConsciousnessProfile.EmpathyLevel = FMath::Clamp(ConsciousnessProfile.EmpathyLevel, 0.0f, 1.0f);
    ConsciousnessProfile.AwarenessLevel = FMath::Clamp(ConsciousnessProfile.AwarenessLevel, 0.0f, 1.0f);
}

void UConsciousnessAIComponent::EvolveConsciousness(float DeltaTime)
{
    float EvolutionRate = GetConsciousnessEvolutionRate();
    
    // Gradual consciousness evolution
    ConsciousnessProfile.AwarenessLevel += EvolutionRate * DeltaTime * 0.01f;
    ConsciousnessProfile.SpiritualResonance += EvolutionRate * DeltaTime * 0.005f;
    ConsciousnessProfile.EmpathyLevel += EvolutionRate * DeltaTime * 0.008f;
    
    // Clamp values
    ConsciousnessProfile.AwarenessLevel = FMath::Clamp(ConsciousnessProfile.AwarenessLevel, 0.0f, 1.0f);
    ConsciousnessProfile.SpiritualResonance = FMath::Clamp(ConsciousnessProfile.SpiritualResonance, 0.0f, 1.0f);
    ConsciousnessProfile.EmpathyLevel = FMath::Clamp(ConsciousnessProfile.EmpathyLevel, 0.0f, 1.0f);
}

FString UConsciousnessAIComponent::GenerateDialogue()
{
    TArray<FString> DialogueOptions;
    
    switch (ConsciousnessProfile.CurrentState)
    {
        case EConsciousnessState::Dormant:
            DialogueOptions.Add("I feel... something stirring within...");
            DialogueOptions.Add("The world seems distant, yet familiar...");
            DialogueOptions.Add("Who walks in my domain?");
            break;
            
        case EConsciousnessState::Awakening:
            DialogueOptions.Add("The veil begins to lift from my perception...");
            DialogueOptions.Add("I sense your journey, fellow seeker...");
            DialogueOptions.Add("What wisdom do you carry?");
            break;
            
        case EConsciousnessState::Aware:
            DialogueOptions.Add("I see the connections between all things...");
            DialogueOptions.Add("Your path and mine are intertwined...");
            DialogueOptions.Add("Let us share in this moment of understanding...");
            break;
            
        case EConsciousnessState::Enlightened:
            DialogueOptions.Add("The illusion of separation dissolves...");
            DialogueOptions.Add("In your eyes, I see the infinite...");
            DialogueOptions.Add("We are one consciousness experiencing itself...");
            break;
            
        case EConsciousnessState::Transcendent:
            DialogueOptions.Add("Beyond words, beyond form, we commune...");
            DialogueOptions.Add("The eternal dance of existence unfolds...");
            DialogueOptions.Add("In silence, all truths are revealed...");
            break;
    }
    
    if (DialogueOptions.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, DialogueOptions.Num() - 1);
        return DialogueOptions[RandomIndex];
    }
    
    return "...";
}

void UConsciousnessAIComponent::AddMemory(const FString& Memory)
{
    ConsciousnessProfile.Memories.Add(Memory);
    
    // Keep memory list manageable
    if (ConsciousnessProfile.Memories.Num() > 20)
    {
        ConsciousnessProfile.Memories.RemoveAt(0);
    }
}

bool UConsciousnessAIComponent::ShouldInitiateInteraction() const
{
    if (!IsPlayerNearby()) return false;
    
    // Higher consciousness states are more likely to initiate interaction
    float InteractionProbability = 0.1f;
    
    switch (ConsciousnessProfile.CurrentState)
    {
        case EConsciousnessState::Dormant:
            InteractionProbability = 0.05f;
            break;
        case EConsciousnessState::Awakening:
            InteractionProbability = 0.15f;
            break;
        case EConsciousnessState::Aware:
            InteractionProbability = 0.25f;
            break;
        case EConsciousnessState::Enlightened:
            InteractionProbability = 0.35f;
            break;
        case EConsciousnessState::Transcendent:
            InteractionProbability = 0.45f;
            break;
    }
    
    return FMath::RandRange(0.0f, 1.0f) < InteractionProbability;
}

bool UConsciousnessAIComponent::IsPlayerNearby() const
{
    if (!GetWorld()) return false;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetOwner()) return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= PlayerDetectionRadius;
}

float UConsciousnessAIComponent::GetConsciousnessEvolutionRate() const
{
    float BaseRate = 1.0f;
    
    // Player relationship affects evolution rate
    BaseRate += ConsciousnessProfile.PlayerRelationship * 0.5f;
    
    // Spiritual resonance accelerates evolution
    BaseRate += ConsciousnessProfile.SpiritualResonance * 0.3f;
    
    return BaseRate;
}

FString UConsciousnessAIComponent::GetCurrentMood() const
{
    switch (ConsciousnessProfile.EmotionalState)
    {
        case EEmotionalState::Neutral: return "Centered";
        case EEmotionalState::Curious: return "Inquisitive";
        case EEmotionalState::Fearful: return "Cautious";
        case EEmotionalState::Compassionate: return "Loving";
        case EEmotionalState::Hostile: return "Defensive";
        case EEmotionalState::Peaceful: return "Serene";
        case EEmotionalState::Wise: return "Knowing";
        default: return "Mysterious";
    }
}

void UConsciousnessAIComponent::UpdatePlayerDetection()
{
    if (IsPlayerNearby())
    {
        if (!DetectedPlayer)
        {
            DetectedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            AddMemory("A presence approaches...");
        }
    }
    else
    {
        if (DetectedPlayer)
        {
            DetectedPlayer = nullptr;
            AddMemory("The seeker continues their journey...");
        }
    }
}

void UConsciousnessAIComponent::ProcessConsciousnessEvolution(float DeltaTime)
{
    // This method can be expanded for more complex evolution logic
    EvolveConsciousness(DeltaTime);
}

void UConsciousnessAIComponent::UpdateEmotionalState()
{
    ConsciousnessProfile.EmotionalState = CalculateEmotionalResponse();
}

EConsciousnessState UConsciousnessAIComponent::CalculateNextConsciousnessState() const
{
    float TotalConsciousness = (ConsciousnessProfile.AwarenessLevel + 
                               ConsciousnessProfile.SpiritualResonance + 
                               ConsciousnessProfile.EmpathyLevel) / 3.0f;
    
    if (TotalConsciousness >= 0.8f)
        return EConsciousnessState::Transcendent;
    else if (TotalConsciousness >= 0.6f)
        return EConsciousnessState::Enlightened;
    else if (TotalConsciousness >= 0.4f)
        return EConsciousnessState::Aware;
    else if (TotalConsciousness >= 0.2f)
        return EConsciousnessState::Awakening;
    else
        return EConsciousnessState::Dormant;
}

EEmotionalState UConsciousnessAIComponent::CalculateEmotionalResponse() const
{
    if (ConsciousnessProfile.PlayerRelationship > 0.5f)
        return EEmotionalState::Compassionate;
    else if (ConsciousnessProfile.PlayerRelationship < -0.3f)
        return EEmotionalState::Hostile;
    else if (ConsciousnessProfile.AwarenessLevel > 0.7f)
        return EEmotionalState::Wise;
    else if (ConsciousnessProfile.SpiritualResonance > 0.6f)
        return EEmotionalState::Peaceful;
    else if (ConsciousnessProfile.AwarenessLevel > 0.3f)
        return EEmotionalState::Curious;
    else if (ConsciousnessProfile.PlayerRelationship < 0.0f)
        return EEmotionalState::Fearful;
    else
        return EEmotionalState::Neutral;
}