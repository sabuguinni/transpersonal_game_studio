#include "NPC_BehaviorStates.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UNPC_BehaviorStateComponent::UNPC_BehaviorStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default state
    CurrentState = ENPC_BehaviorState::Idle;
    PreviousState = ENPC_BehaviorState::Idle;
    
    // Default parameters
    StateChangeDelay = 2.0f;
    MemoryDecayRate = 0.1f;
    MaxMemories = 10;
    
    TimeInCurrentState = 0.0f;
    LastStateChangeTime = 0.0f;
}

void UNPC_BehaviorStateComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with default state
    OnStateEnter(CurrentState);
}

void UNPC_BehaviorStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeInCurrentState += DeltaTime;
    
    // Update current state logic
    UpdateCurrentState(DeltaTime);
    
    // Update memory system
    UpdateMemoryStrength(DeltaTime);
}

void UNPC_BehaviorStateComponent::ChangeState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }
    
    // Check if transition is allowed
    if (!CanTransitionTo(NewState))
    {
        return;
    }
    
    // Check state change delay
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateChangeTime < StateChangeDelay)
    {
        return;
    }
    
    // Perform state transition
    ENPC_BehaviorState OldState = CurrentState;
    OnStateExit(OldState);
    
    PreviousState = CurrentState;
    CurrentState = NewState;
    TimeInCurrentState = 0.0f;
    LastStateChangeTime = CurrentTime;
    
    OnStateEnter(NewState);
    
    UE_LOG(LogTemp, Log, TEXT("NPC State changed from %d to %d"), (int32)OldState, (int32)NewState);
}

bool UNPC_BehaviorStateComponent::CanTransitionTo(ENPC_BehaviorState TargetState)
{
    // Check if there's a valid transition defined
    for (const FNPC_StateTransition& Transition : StateTransitions)
    {
        if (Transition.FromState == CurrentState && Transition.ToState == TargetState)
        {
            return true;
        }
    }
    
    // Allow transitions to idle and flee from any state (emergency states)
    if (TargetState == ENPC_BehaviorState::Idle || TargetState == ENPC_BehaviorState::Flee)
    {
        return true;
    }
    
    return false;
}

void UNPC_BehaviorStateComponent::AddMemory(AActor* Actor, const FVector& Location, bool bHostile)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if we already have a memory of this actor
    FNPC_Memory* ExistingMemory = GetMemoryOfActor(Actor);
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->LastKnownLocation = Location;
        ExistingMemory->MemoryStrength = 1.0f;
        ExistingMemory->TimeLastSeen = GetWorld()->GetTimeSeconds();
        ExistingMemory->bWasHostile = bHostile;
        return;
    }
    
    // Remove oldest memory if at capacity
    if (Memories.Num() >= MaxMemories)
    {
        float OldestTime = FLT_MAX;
        int32 OldestIndex = 0;
        
        for (int32 i = 0; i < Memories.Num(); i++)
        {
            if (Memories[i].TimeLastSeen < OldestTime)
            {
                OldestTime = Memories[i].TimeLastSeen;
                OldestIndex = i;
            }
        }
        
        Memories.RemoveAt(OldestIndex);
    }
    
    // Add new memory
    FNPC_Memory NewMemory;
    NewMemory.RememberedActor = Actor;
    NewMemory.LastKnownLocation = Location;
    NewMemory.MemoryStrength = 1.0f;
    NewMemory.TimeLastSeen = GetWorld()->GetTimeSeconds();
    NewMemory.bWasHostile = bHostile;
    
    Memories.Add(NewMemory);
}

FNPC_Memory* UNPC_BehaviorStateComponent::GetMemoryOfActor(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }
    
    for (FNPC_Memory& Memory : Memories)
    {
        if (Memory.RememberedActor == Actor)
        {
            return &Memory;
        }
    }
    
    return nullptr;
}

void UNPC_BehaviorStateComponent::ForgetActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        if (Memories[i].RememberedActor == Actor)
        {
            Memories.RemoveAt(i);
            break;
        }
    }
}

void UNPC_BehaviorStateComponent::UpdateMemoryStrength(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay memory strength over time
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        FNPC_Memory& Memory = Memories[i];
        
        // Calculate time since last seen
        float TimeSinceLastSeen = CurrentTime - Memory.TimeLastSeen;
        
        // Decay memory strength
        Memory.MemoryStrength -= MemoryDecayRate * DeltaTime * (TimeSinceLastSeen / 60.0f);
        
        // Remove memories that have decayed too much
        if (Memory.MemoryStrength <= 0.0f || !IsValid(Memory.RememberedActor))
        {
            Memories.RemoveAt(i);
        }
    }
}

void UNPC_BehaviorStateComponent::OnStateEnter(ENPC_BehaviorState NewState)
{
    switch (NewState)
    {
        case ENPC_BehaviorState::Idle:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Idle state"));
            break;
            
        case ENPC_BehaviorState::Patrol:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Patrol state"));
            break;
            
        case ENPC_BehaviorState::Hunt:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Hunt state"));
            break;
            
        case ENPC_BehaviorState::Chase:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Chase state"));
            break;
            
        case ENPC_BehaviorState::Attack:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Attack state"));
            break;
            
        case ENPC_BehaviorState::Flee:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Flee state"));
            break;
            
        case ENPC_BehaviorState::Feed:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Feed state"));
            break;
            
        case ENPC_BehaviorState::Sleep:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Sleep state"));
            break;
            
        case ENPC_BehaviorState::Territorial:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Territorial state"));
            break;
            
        case ENPC_BehaviorState::Pack:
            UE_LOG(LogTemp, Log, TEXT("NPC entering Pack Behavior state"));
            break;
    }
}

void UNPC_BehaviorStateComponent::OnStateExit(ENPC_BehaviorState OldState)
{
    switch (OldState)
    {
        case ENPC_BehaviorState::Attack:
            // Reset attack cooldowns or cleanup
            break;
            
        case ENPC_BehaviorState::Chase:
            // Clear chase target
            break;
            
        case ENPC_BehaviorState::Flee:
            // Reset fear levels
            break;
            
        default:
            break;
    }
}

void UNPC_BehaviorStateComponent::UpdateCurrentState(float DeltaTime)
{
    // State-specific update logic
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            // Check for nearby threats or prey
            if (TimeInCurrentState > 5.0f)
            {
                // Transition to patrol after being idle for 5 seconds
                ChangeState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        case ENPC_BehaviorState::Patrol:
            // Check for targets or threats
            break;
            
        case ENPC_BehaviorState::Hunt:
            // Update hunting behavior
            break;
            
        case ENPC_BehaviorState::Chase:
            // Update chase behavior
            break;
            
        case ENPC_BehaviorState::Attack:
            // Update attack behavior
            break;
            
        case ENPC_BehaviorState::Flee:
            // Update flee behavior
            if (TimeInCurrentState > 10.0f)
            {
                // Return to idle after fleeing for 10 seconds
                ChangeState(ENPC_BehaviorState::Idle);
            }
            break;
            
        case ENPC_BehaviorState::Feed:
            // Update feeding behavior
            break;
            
        case ENPC_BehaviorState::Sleep:
            // Update sleep behavior
            break;
            
        case ENPC_BehaviorState::Territorial:
            // Update territorial behavior
            break;
            
        case ENPC_BehaviorState::Pack:
            // Update pack behavior
            break;
    }
}