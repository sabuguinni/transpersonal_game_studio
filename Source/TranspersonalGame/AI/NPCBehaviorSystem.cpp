#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Default values
    Archetype = EDinosaurArchetype::SolitaryHerbivore;
    MemoryDuration = 300.0f; // 5 minutes
    MaxMemoryEntries = 20;
    
    CurrentBehavior = EBehaviorState::Idle;
    CurrentEmotion = EEmotionalState::Calm;
    CurrentStress = 0.0f;
    CurrentHunger = 0.5f;
    CurrentEnergy = 1.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize personality based on archetype if not set
    if (Personality.Aggression == 0.5f && Personality.Curiosity == 0.5f) // Default values
    {
        switch (Archetype)
        {
            case EDinosaurArchetype::SolitaryPredator:
                Personality.Aggression = 0.8f;
                Personality.Territoriality = 0.9f;
                Personality.Fearfulness = 0.2f;
                Personality.Sociability = 0.1f;
                break;
                
            case EDinosaurArchetype::PackHunter:
                Personality.Aggression = 0.7f;
                Personality.Sociability = 0.8f;
                Personality.Intelligence = 0.7f;
                Personality.Territoriality = 0.6f;
                break;
                
            case EDinosaurArchetype::HerdHerbivore:
                Personality.Sociability = 0.9f;
                Personality.Fearfulness = 0.7f;
                Personality.Aggression = 0.2f;
                Personality.Curiosity = 0.4f;
                break;
                
            case EDinosaurArchetype::SolitaryHerbivore:
                Personality.Fearfulness = 0.8f;
                Personality.Curiosity = 0.6f;
                Personality.Aggression = 0.3f;
                Personality.Sociability = 0.2f;
                break;
                
            case EDinosaurArchetype::Scavenger:
                Personality.Curiosity = 0.8f;
                Personality.Intelligence = 0.6f;
                Personality.Fearfulness = 0.6f;
                Personality.Aggression = 0.4f;
                break;
                
            case EDinosaurArchetype::TerritorialPredator:
                Personality.Territoriality = 1.0f;
                Personality.Aggression = 0.9f;
                Personality.Fearfulness = 0.1f;
                Personality.Sociability = 0.1f;
                break;
                
            case EDinosaurArchetype::MigratoryHerbivore:
                Personality.Sociability = 0.8f;
                Personality.Intelligence = 0.5f;
                Personality.Fearfulness = 0.6f;
                Personality.Territoriality = 0.1f;
                break;
                
            case EDinosaurArchetype::AmbushPredator:
                Personality.Intelligence = 0.8f;
                Personality.Aggression = 0.7f;
                Personality.Curiosity = 0.3f;
                Personality.Territoriality = 0.7f;
                break;
        }
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMemory(DeltaTime);
    UpdateNeeds(DeltaTime);
    ProcessDailyRoutines();
    UpdateEmotionalState();
}

void UNPCBehaviorComponent::AddMemoryEntry(AActor* Actor, float Importance, const FGameplayTagContainer& Tags)
{
    if (!Actor)
        return;
        
    FMemoryEntry NewEntry;
    NewEntry.Actor = Actor;
    NewEntry.LastKnownLocation = Actor->GetActorLocation();
    NewEntry.Timestamp = GetWorld()->GetTimeSeconds();
    NewEntry.Importance = Importance;
    NewEntry.AssociatedTags = Tags;
    
    // Check if we already have memory of this actor
    FMemoryEntry* ExistingEntry = GetMemoryOfActor(Actor);
    if (ExistingEntry)
    {
        // Update existing memory
        ExistingEntry->LastKnownLocation = NewEntry.LastKnownLocation;
        ExistingEntry->Timestamp = NewEntry.Timestamp;
        ExistingEntry->Importance = FMath::Max(ExistingEntry->Importance, Importance);
        ExistingEntry->AssociatedTags.AppendTags(Tags);
    }
    else
    {
        // Add new memory entry
        ShortTermMemory.Add(NewEntry);
        
        // Limit memory size
        if (ShortTermMemory.Num() > MaxMemoryEntries)
        {
            // Remove oldest entry
            ShortTermMemory.RemoveAt(0);
        }
    }
}

FMemoryEntry* UNPCBehaviorComponent::GetMemoryOfActor(AActor* Actor)
{
    if (!Actor)
        return nullptr;
        
    // Check short-term memory first
    for (FMemoryEntry& Entry : ShortTermMemory)
    {
        if (Entry.Actor == Actor)
            return &Entry;
    }
    
    // Check long-term memory
    for (FMemoryEntry& Entry : LongTermMemory)
    {
        if (Entry.Actor == Actor)
            return &Entry;
    }
    
    return nullptr;
}

void UNPCBehaviorComponent::SetBehaviorState(EBehaviorState NewState)
{
    if (CurrentBehavior != NewState)
    {
        CurrentBehavior = NewState;
        
        // Update blackboard if we have an AI controller
        if (AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
            }
        }
    }
}

void UNPCBehaviorComponent::SetEmotionalState(EEmotionalState NewEmotion)
{
    if (CurrentEmotion != NewEmotion)
    {
        CurrentEmotion = NewEmotion;
        
        // Update blackboard if we have an AI controller
        if (AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsEnum(TEXT("EmotionalState"), static_cast<uint8>(NewEmotion));
            }
        }
    }
}

FDailyRoutine UNPCBehaviorComponent::GetCurrentRoutine()
{
    float CurrentTime = GetTimeOfDay();
    
    // Find the routine that matches current time
    for (const FDailyRoutine& Routine : DailyRoutines)
    {
        if (CurrentTime >= Routine.StartTime && CurrentTime <= Routine.EndTime)
        {
            return Routine;
        }
    }
    
    // Return default idle routine if none found
    FDailyRoutine DefaultRoutine;
    DefaultRoutine.RoutineBehavior = EBehaviorState::Idle;
    return DefaultRoutine;
}

bool UNPCBehaviorComponent::ShouldReactToPlayer(AActor* Player)
{
    if (!Player)
        return false;
        
    FMemoryEntry* PlayerMemory = GetMemoryOfActor(Player);
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    
    // Base reaction distance depends on archetype and personality
    float BaseReactionDistance = 1000.0f;
    
    switch (Archetype)
    {
        case EDinosaurArchetype::SolitaryPredator:
        case EDinosaurArchetype::TerritorialPredator:
            BaseReactionDistance = 1500.0f;
            break;
        case EDinosaurArchetype::PackHunter:
            BaseReactionDistance = 2000.0f;
            break;
        case EDinosaurArchetype::HerdHerbivore:
        case EDinosaurArchetype::MigratoryHerbivore:
            BaseReactionDistance = 800.0f;
            break;
        case EDinosaurArchetype::SolitaryHerbivore:
            BaseReactionDistance = 600.0f;
            break;
    }
    
    // Modify by personality
    BaseReactionDistance *= (1.0f + Personality.Curiosity * 0.5f);
    BaseReactionDistance *= (1.0f + Personality.Fearfulness * 0.3f);
    
    // If we have memory of negative interactions, increase reaction distance
    if (PlayerMemory && PlayerMemory->AssociatedTags.HasTag(FGameplayTag::RequestGameplayTag("Memory.Negative")))
    {
        BaseReactionDistance *= 1.5f;
    }
    
    return DistanceToPlayer <= BaseReactionDistance;
}

float UNPCBehaviorComponent::CalculateStressLevel()
{
    float StressLevel = 0.0f;
    
    // Base stress from needs
    StressLevel += CurrentHunger * 0.3f;
    StressLevel += (1.0f - CurrentEnergy) * 0.2f;
    
    // Stress from recent negative memories
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (const FMemoryEntry& Memory : ShortTermMemory)
    {
        if (Memory.AssociatedTags.HasTag(FGameplayTag::RequestGameplayTag("Memory.Negative")))
        {
            float TimeSince = CurrentTime - Memory.Timestamp;
            if (TimeSince < 60.0f) // Last minute
            {
                StressLevel += Memory.Importance * (1.0f - TimeSince / 60.0f) * 0.5f;
            }
        }
    }
    
    // Personality modifiers
    StressLevel *= (1.0f + Personality.Fearfulness * 0.5f);
    StressLevel *= (1.0f - Personality.Intelligence * 0.2f); // Smarter animals cope better
    
    return FMath::Clamp(StressLevel, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    // Update hunger (increases over time)
    CurrentHunger += DeltaTime * 0.001f; // Hungry every ~15 minutes
    CurrentHunger = FMath::Clamp(CurrentHunger, 0.0f, 1.0f);
    
    // Update energy (decreases during activity, increases during rest)
    float EnergyChange = -DeltaTime * 0.0005f; // Base energy drain
    
    if (CurrentBehavior == EBehaviorState::Resting)
    {
        EnergyChange = DeltaTime * 0.002f; // Restore energy when resting
    }
    else if (CurrentBehavior == EBehaviorState::Hunting || CurrentBehavior == EBehaviorState::Fleeing)
    {
        EnergyChange = -DeltaTime * 0.003f; // High energy activities
    }
    
    CurrentEnergy += EnergyChange;
    CurrentEnergy = FMath::Clamp(CurrentEnergy, 0.0f, 1.0f);
    
    // Update stress
    CurrentStress = CalculateStressLevel();
}

void UNPCBehaviorComponent::UpdateMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Move old short-term memories to long-term or remove them
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        FMemoryEntry& Memory = ShortTermMemory[i];
        float Age = CurrentTime - Memory.Timestamp;
        
        if (Age > MemoryDuration)
        {
            // Important memories go to long-term storage
            if (Memory.Importance > 0.7f)
            {
                LongTermMemory.Add(Memory);
                
                // Limit long-term memory size
                if (LongTermMemory.Num() > MaxMemoryEntries / 2)
                {
                    LongTermMemory.RemoveAt(0);
                }
            }
            
            ShortTermMemory.RemoveAt(i);
        }
    }
}

void UNPCBehaviorComponent::ProcessDailyRoutines()
{
    FDailyRoutine CurrentRoutine = GetCurrentRoutine();
    
    // If current behavior doesn't match routine and we're not in an emergency state
    if (CurrentBehavior != CurrentRoutine.RoutineBehavior && 
        CurrentBehavior != EBehaviorState::Fleeing && 
        CurrentBehavior != EBehaviorState::Hunting &&
        CurrentStress < 0.7f)
    {
        SetBehaviorState(CurrentRoutine.RoutineBehavior);
    }
}

void UNPCBehaviorComponent::UpdateEmotionalState()
{
    // Determine emotional state based on current conditions
    if (CurrentStress > 0.8f)
    {
        SetEmotionalState(EEmotionalState::Fearful);
    }
    else if (CurrentHunger > 0.8f)
    {
        SetEmotionalState(EEmotionalState::Hungry);
    }
    else if (CurrentEnergy < 0.2f)
    {
        SetEmotionalState(EEmotionalState::Tired);
    }
    else if (CurrentBehavior == EBehaviorState::Hunting || 
             (Personality.Aggression > 0.7f && CurrentStress > 0.3f))
    {
        SetEmotionalState(EEmotionalState::Aggressive);
    }
    else if (Personality.Curiosity > 0.6f && CurrentStress < 0.3f)
    {
        SetEmotionalState(EEmotionalState::Curious);
    }
    else if (CurrentStress < 0.2f && CurrentHunger < 0.3f && CurrentEnergy > 0.7f)
    {
        SetEmotionalState(EEmotionalState::Calm);
    }
    else
    {
        SetEmotionalState(EEmotionalState::Alert);
    }
}

float UNPCBehaviorComponent::GetTimeOfDay()
{
    // This would normally get time from a day/night cycle system
    // For now, return a simple time based on world time
    float WorldTime = GetWorld()->GetTimeSeconds();
    return FMath::Fmod(WorldTime / 1200.0f, 1.0f); // 20-minute day cycle for testing
}