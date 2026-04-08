#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance

    // Initialize with random personality
    Personality = FNPCPersonality();
    
    // Set initial needs
    Hunger = FMath::RandRange(0.0f, 0.3f);
    Thirst = FMath::RandRange(0.0f, 0.3f);
    Energy = FMath::RandRange(0.7f, 1.0f);
    Social = FMath::RandRange(0.3f, 0.7f);
    Safety = 1.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate initial daily routine if none provided
    if (DailyRoutine.Num() == 0)
    {
        GenerateRandomRoutine();
    }
    
    // Set initial behavior state
    CurrentState = GetCurrentRoutineActivity();
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update needs periodically
    UpdateNeeds(DeltaTime);
    
    // Check if we should change behavior based on current situation
    ENPCBehaviorState NewBehavior = DecideNextBehavior();
    if (NewBehavior != CurrentState)
    {
        SetBehaviorState(NewBehavior);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPCBehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        ENPCBehaviorState PreviousState = CurrentState;
        CurrentState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed behavior from %d to %d"), 
               *GetOwner()->GetName(), 
               (int32)PreviousState, 
               (int32)CurrentState);
    }
}

void UNPCBehaviorComponent::AddMemory(AActor* Actor, FVector Location, float EmotionalWeight, const FString& EventType)
{
    if (!Actor) return;
    
    FNPCMemoryEntry NewMemory;
    NewMemory.RememberedActor = Actor;
    NewMemory.Location = Location;
    NewMemory.EmotionalWeight = EmotionalWeight;
    NewMemory.EventType = EventType;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    
    // Remove old memories if we have too many
    if (Memories.Num() >= 50)
    {
        // Remove oldest memory
        float OldestTime = FLT_MAX;
        int32 OldestIndex = 0;
        for (int32 i = 0; i < Memories.Num(); i++)
        {
            if (Memories[i].Timestamp < OldestTime)
            {
                OldestTime = Memories[i].Timestamp;
                OldestIndex = i;
            }
        }
        Memories.RemoveAt(OldestIndex);
    }
    
    Memories.Add(NewMemory);
}

FNPCMemoryEntry UNPCBehaviorComponent::GetMemoryOfActor(AActor* Actor)
{
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.RememberedActor == Actor)
        {
            return Memory;
        }
    }
    return FNPCMemoryEntry(); // Return empty memory if not found
}

ENPCBehaviorState UNPCBehaviorComponent::GetCurrentRoutineActivity()
{
    float CurrentTime = GetTimeOfDay();
    
    // Find the current routine activity
    for (const FDailyRoutineEntry& Entry : DailyRoutine)
    {
        float EndTime = Entry.StartTime + Entry.Duration;
        if (EndTime > 1.0f) EndTime -= 1.0f; // Handle wrap-around
        
        if (Entry.StartTime <= EndTime)
        {
            // Normal case: activity doesn't wrap around midnight
            if (CurrentTime >= Entry.StartTime && CurrentTime < EndTime)
            {
                return Entry.Activity;
            }
        }
        else
        {
            // Wrap-around case: activity crosses midnight
            if (CurrentTime >= Entry.StartTime || CurrentTime < EndTime)
            {
                return Entry.Activity;
            }
        }
    }
    
    return ENPCBehaviorState::Idle; // Default fallback
}

void UNPCBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    LastNeedsUpdate += DeltaTime;
    if (LastNeedsUpdate < NeedsUpdateInterval) return;
    
    LastNeedsUpdate = 0.0f;
    
    // Increase basic needs over time
    Hunger = FMath::Clamp(Hunger + 0.01f, 0.0f, 1.0f);
    Thirst = FMath::Clamp(Thirst + 0.015f, 0.0f, 1.0f); // Thirst increases faster
    Energy = FMath::Clamp(Energy - 0.005f, 0.0f, 1.0f); // Energy decreases slowly
    
    // Social need changes based on personality
    float SocialChange = (Personality.Sociability > 0.5f) ? 0.01f : -0.005f;
    Social = FMath::Clamp(Social + SocialChange, 0.0f, 1.0f);
    
    // Safety decreases if we remember dangerous events
    float DangerousMemories = 0.0f;
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.EmotionalWeight < -0.5f && 
            GetWorld()->GetTimeSeconds() - Memory.Timestamp < 300.0f) // Last 5 minutes
        {
            DangerousMemories += 0.1f;
        }
    }
    Safety = FMath::Clamp(1.0f - DangerousMemories, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::ProcessDomestication(AActor* Human, float InteractionQuality, float DeltaTime)
{
    if (!bCanBeDomesticated || !Human) return;
    
    // Domestication is influenced by personality and interaction quality
    float PersonalityModifier = (1.0f - Personality.Caution) * Personality.Intelligence;
    float TrustChange = InteractionQuality * PersonalityModifier * DeltaTime * 0.01f;
    
    TrustLevel = FMath::Clamp(TrustLevel + TrustChange, 0.0f, 1.0f);
    
    // Set trusted human when trust level is high enough
    if (TrustLevel > 0.3f && !TrustedHuman)
    {
        TrustedHuman = Human;
        AddMemory(Human, Human->GetActorLocation(), 0.8f, TEXT("Trusted Human"));
    }
}

ENPCBehaviorState UNPCBehaviorComponent::DecideNextBehavior()
{
    // Calculate priorities for different behaviors
    TMap<ENPCBehaviorState, float> BehaviorPriorities;
    
    BehaviorPriorities.Add(ENPCBehaviorState::Foraging, CalculateBehaviorPriority(ENPCBehaviorState::Foraging));
    BehaviorPriorities.Add(ENPCBehaviorState::Drinking, CalculateBehaviorPriority(ENPCBehaviorState::Drinking));
    BehaviorPriorities.Add(ENPCBehaviorState::Resting, CalculateBehaviorPriority(ENPCBehaviorState::Resting));
    BehaviorPriorities.Add(ENPCBehaviorState::Socializing, CalculateBehaviorPriority(ENPCBehaviorState::Socializing));
    BehaviorPriorities.Add(ENPCBehaviorState::Patrolling, CalculateBehaviorPriority(ENPCBehaviorState::Patrolling));
    BehaviorPriorities.Add(ENPCBehaviorState::Fleeing, CalculateBehaviorPriority(ENPCBehaviorState::Fleeing));
    
    // Add current routine activity with bonus
    ENPCBehaviorState RoutineActivity = GetCurrentRoutineActivity();
    float RoutinePriority = BehaviorPriorities.Contains(RoutineActivity) ? 
                           BehaviorPriorities[RoutineActivity] + 0.2f : 0.2f;
    BehaviorPriorities.Add(RoutineActivity, RoutinePriority);
    
    // Find highest priority behavior
    ENPCBehaviorState BestBehavior = ENPCBehaviorState::Idle;
    float HighestPriority = 0.0f;
    
    for (const auto& Pair : BehaviorPriorities)
    {
        if (Pair.Value > HighestPriority)
        {
            HighestPriority = Pair.Value;
            BestBehavior = Pair.Key;
        }
    }
    
    return BestBehavior;
}

float UNPCBehaviorComponent::CalculateBehaviorPriority(ENPCBehaviorState Behavior)
{
    float Priority = 0.0f;
    
    switch (Behavior)
    {
        case ENPCBehaviorState::Foraging:
            Priority = Hunger * 2.0f; // High priority when hungry
            break;
            
        case ENPCBehaviorState::Drinking:
            Priority = Thirst * 2.5f; // Higher priority than food
            break;
            
        case ENPCBehaviorState::Resting:
            Priority = (1.0f - Energy) * 1.5f;
            break;
            
        case ENPCBehaviorState::Socializing:
            Priority = (1.0f - Social) * Personality.Sociability;
            break;
            
        case ENPCBehaviorState::Fleeing:
            Priority = (1.0f - Safety) * 3.0f; // Highest priority when in danger
            break;
            
        case ENPCBehaviorState::Patrolling:
            Priority = Personality.Territoriality * 0.5f;
            break;
            
        default:
            Priority = 0.1f;
            break;
    }
    
    return FMath::Clamp(Priority, 0.0f, 3.0f);
}

void UNPCBehaviorComponent::GenerateRandomRoutine()
{
    DailyRoutine.Empty();
    
    // Create a basic routine based on personality
    if (Personality.Sociability > 0.6f)
    {
        // Social creature - more socializing time
        DailyRoutine.Add({0.1f, 0.2f, ENPCBehaviorState::Foraging, FVector::ZeroVector, 1.0f});
        DailyRoutine.Add({0.3f, 0.15f, ENPCBehaviorState::Socializing, FVector::ZeroVector, 1.0f});
        DailyRoutine.Add({0.5f, 0.1f, ENPCBehaviorState::Drinking, FVector::ZeroVector, 1.0f});
        DailyRoutine.Add({0.7f, 0.2f, ENPCBehaviorState::Resting, FVector::ZeroVector, 1.0f});
    }
    else
    {
        // Solitary creature - more patrolling and foraging
        DailyRoutine.Add({0.1f, 0.25f, ENPCBehaviorState::Foraging, FVector::ZeroVector, 1.0f});
        DailyRoutine.Add({0.4f, 0.1f, ENPCBehaviorState::Drinking, FVector::ZeroVector, 1.0f});
        DailyRoutine.Add({0.6f, 0.2f, ENPCBehaviorState::Patrolling, FVector::ZeroVector, 1.0f});
        DailyRoutine.Add({0.8f, 0.15f, ENPCBehaviorState::Resting, FVector::ZeroVector, 1.0f});
    }
}

float UNPCBehaviorComponent::GetTimeOfDay()
{
    // Get time of day as fraction (0.0 = dawn, 0.5 = noon, 1.0 = midnight)
    if (UWorld* World = GetWorld())
    {
        float GameTime = World->GetTimeSeconds();
        float DayLength = 1200.0f; // 20 minutes per day cycle
        return FMath::Fmod(GameTime / DayLength, 1.0f);
    }
    return 0.0f;
}