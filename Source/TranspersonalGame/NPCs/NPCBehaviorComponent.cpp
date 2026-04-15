#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    BehaviorData = FNPC_BehaviorData();
    NPCName = TEXT("Unnamed NPC");
    TribeID = 1;
    Age = FMath::RandRange(18, 60);
    StateChangeInterval = FMath::RandRange(5.0f, 15.0f);
    LastStateChangeTime = 0.0f;
    DecisionThreshold = 0.5f;
    
    InitializeDefaultStates();
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Randomize initial personality traits
    BehaviorData.Aggression = FMath::RandRange(0.1f, 0.9f);
    BehaviorData.Sociability = FMath::RandRange(0.2f, 0.9f);
    BehaviorData.Curiosity = FMath::RandRange(0.1f, 0.8f);
    BehaviorData.WorkEthic = FMath::RandRange(0.3f, 0.9f);
    
    // Set random initial mood
    BehaviorData.CurrentMood = FMath::RandRange(0.3f, 0.8f);
    
    // Choose initial behavior state
    BehaviorData.CurrentState = ChooseNewState();
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s initialized with personality: Aggression=%.2f, Sociability=%.2f"), 
           *NPCName, BehaviorData.Aggression, BehaviorData.Sociability);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
    
    // Update NPC needs and behavior
    UpdateNeeds(DeltaTime);
    UpdateBehaviorLogic(DeltaTime);
    
    // Check for state changes
    if (ShouldChangeState())
    {
        ENPC_BehaviorState NewState = ChooseNewState();
        if (NewState != BehaviorData.CurrentState)
        {
            SetBehaviorState(NewState);
        }
    }
}

void UNPCBehaviorComponent::InitializeDefaultStates()
{
    AvailableStates.Empty();
    AvailableStates.Add(ENPC_BehaviorState::Idle);
    AvailableStates.Add(ENPC_BehaviorState::Working);
    AvailableStates.Add(ENPC_BehaviorState::Socializing);
    AvailableStates.Add(ENPC_BehaviorState::Eating);
    AvailableStates.Add(ENPC_BehaviorState::Patrolling);
    AvailableStates.Add(ENPC_BehaviorState::Gathering);
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState != BehaviorData.CurrentState)
    {
        ENPC_BehaviorState PreviousState = BehaviorData.CurrentState;
        BehaviorData.CurrentState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed state from %d to %d"), 
               *NPCName, (int32)PreviousState, (int32)NewState);
        
        // Trigger state-specific behaviors
        switch (NewState)
        {
            case ENPC_BehaviorState::Working:
                BehaviorData.Energy -= 0.1f;
                UpdateMood(0.05f); // Slight mood boost from productivity
                break;
                
            case ENPC_BehaviorState::Socializing:
                UpdateMood(0.15f); // Mood boost from social interaction
                break;
                
            case ENPC_BehaviorState::Eating:
                BehaviorData.Hunger = FMath::Max(0.0f, BehaviorData.Hunger - 0.5f);
                BehaviorData.Energy += 0.2f;
                break;
                
            case ENPC_BehaviorState::Sleeping:
                BehaviorData.Energy = 1.0f;
                UpdateMood(0.1f);
                break;
                
            case ENPC_BehaviorState::Fleeing:
                BehaviorData.Fear += 0.3f;
                UpdateMood(-0.2f);
                break;
        }
    }
}

ENPC_BehaviorState UNPCBehaviorComponent::GetCurrentBehaviorState() const
{
    return BehaviorData.CurrentState;
}

void UNPCBehaviorComponent::UpdateMood(float MoodChange)
{
    BehaviorData.CurrentMood = FMath::Clamp(BehaviorData.CurrentMood + MoodChange, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    // Gradually increase hunger
    BehaviorData.Hunger += DeltaTime * 0.01f; // Hungry every ~100 seconds
    BehaviorData.Hunger = FMath::Clamp(BehaviorData.Hunger, 0.0f, 1.0f);
    
    // Gradually decrease energy
    BehaviorData.Energy -= DeltaTime * 0.005f; // Tired every ~200 seconds
    BehaviorData.Energy = FMath::Clamp(BehaviorData.Energy, 0.0f, 1.0f);
    
    // Gradually decrease fear
    BehaviorData.Fear = FMath::Max(0.0f, BehaviorData.Fear - DeltaTime * 0.1f);
    
    // Mood affected by needs
    if (BehaviorData.Hunger > 0.7f)
    {
        UpdateMood(-DeltaTime * 0.05f); // Hungry = grumpy
    }
    
    if (BehaviorData.Energy < 0.3f)
    {
        UpdateMood(-DeltaTime * 0.03f); // Tired = grumpy
    }
}

bool UNPCBehaviorComponent::ShouldChangeState() const
{
    float TimeSinceLastChange = GetWorld()->GetTimeSeconds() - LastStateChangeTime;
    
    // Force state change if too much time has passed
    if (TimeSinceLastChange > StateChangeInterval)
    {
        return true;
    }
    
    // Emergency state changes
    if (BehaviorData.Fear > 0.5f && BehaviorData.CurrentState != ENPC_BehaviorState::Fleeing)
    {
        return true;
    }
    
    if (BehaviorData.Hunger > 0.8f && BehaviorData.CurrentState != ENPC_BehaviorState::Eating)
    {
        return true;
    }
    
    if (BehaviorData.Energy < 0.2f && BehaviorData.CurrentState != ENPC_BehaviorState::Sleeping)
    {
        return true;
    }
    
    return false;
}

ENPC_BehaviorState UNPCBehaviorComponent::ChooseNewState()
{
    // Emergency states first
    if (BehaviorData.Fear > 0.5f)
    {
        return ENPC_BehaviorState::Fleeing;
    }
    
    if (BehaviorData.Hunger > 0.8f)
    {
        return ENPC_BehaviorState::Eating;
    }
    
    if (BehaviorData.Energy < 0.2f)
    {
        return ENPC_BehaviorState::Sleeping;
    }
    
    // Calculate desirability for each available state
    ENPC_BehaviorState BestState = ENPC_BehaviorState::Idle;
    float BestDesirability = 0.0f;
    
    for (ENPC_BehaviorState State : AvailableStates)
    {
        float Desirability = CalculateStateDesirability(State);
        if (Desirability > BestDesirability)
        {
            BestDesirability = Desirability;
            BestState = State;
        }
    }
    
    return BestState;
}

float UNPCBehaviorComponent::CalculateStateDesirability(ENPC_BehaviorState State) const
{
    float Desirability = 0.5f; // Base desirability
    
    switch (State)
    {
        case ENPC_BehaviorState::Working:
            Desirability += BehaviorData.WorkEthic * 0.5f;
            Desirability -= BehaviorData.Hunger * 0.3f;
            Desirability -= (1.0f - BehaviorData.Energy) * 0.4f;
            break;
            
        case ENPC_BehaviorState::Socializing:
            Desirability += BehaviorData.Sociability * 0.6f;
            Desirability += (1.0f - BehaviorData.CurrentMood) * 0.3f; // More social when sad
            break;
            
        case ENPC_BehaviorState::Gathering:
            Desirability += BehaviorData.WorkEthic * 0.4f;
            Desirability += BehaviorData.Hunger * 0.5f; // More likely when hungry
            break;
            
        case ENPC_BehaviorState::Patrolling:
            Desirability += BehaviorData.Aggression * 0.3f;
            Desirability += BehaviorData.Curiosity * 0.2f;
            break;
            
        case ENPC_BehaviorState::Idle:
            Desirability += (1.0f - BehaviorData.WorkEthic) * 0.4f;
            Desirability += (1.0f - BehaviorData.Energy) * 0.2f;
            break;
    }
    
    // Add some randomness
    Desirability += FMath::RandRange(-0.2f, 0.2f);
    
    return FMath::Clamp(Desirability, 0.0f, 1.0f);
}

void UNPCBehaviorComponent::UpdateBehaviorLogic(float DeltaTime)
{
    // State-specific updates
    switch (BehaviorData.CurrentState)
    {
        case ENPC_BehaviorState::Working:
            BehaviorData.Energy -= DeltaTime * 0.02f;
            if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // Rare chance
            {
                UpdateMood(0.05f); // Small satisfaction from work
            }
            break;
            
        case ENPC_BehaviorState::Socializing:
            if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per update
            {
                UpdateMood(0.02f); // Gradual mood improvement
            }
            break;
            
        case ENPC_BehaviorState::Gathering:
            BehaviorData.Energy -= DeltaTime * 0.015f;
            if (FMath::RandRange(0.0f, 1.0f) < 0.005f) // Chance to find food
            {
                BehaviorData.Hunger = FMath::Max(0.0f, BehaviorData.Hunger - 0.1f);
            }
            break;
    }
}

void UNPCBehaviorComponent::ReactToPlayer(AActor* Player, float Distance)
{
    if (!Player)
        return;
    
    // Reaction based on personality and distance
    if (Distance < 500.0f) // Close proximity
    {
        if (BehaviorData.Personality == ENPC_Personality::Aggressive && BehaviorData.Aggression > 0.6f)
        {
            BehaviorData.Fear -= 0.1f; // Aggressive NPCs are less afraid
            UpdateMood(-0.05f); // But they don't like strangers
        }
        else if (BehaviorData.Personality == ENPC_Personality::Cautious)
        {
            BehaviorData.Fear += 0.2f;
            UpdateMood(-0.1f);
        }
        else if (BehaviorData.Personality == ENPC_Personality::Friendly && BehaviorData.Sociability > 0.5f)
        {
            UpdateMood(0.1f); // Happy to see someone new
            BehaviorData.Fear = FMath::Max(0.0f, BehaviorData.Fear - 0.05f);
        }
        else if (BehaviorData.Personality == ENPC_Personality::Curious)
        {
            UpdateMood(0.05f); // Interested in the stranger
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s reacted to player at distance %.1f"), *NPCName, Distance);
}

void UNPCBehaviorComponent::ReactToThreat(AActor* Threat)
{
    if (!Threat)
        return;
    
    // Immediate fear response
    BehaviorData.Fear += 0.5f;
    BehaviorData.Fear = FMath::Clamp(BehaviorData.Fear, 0.0f, 1.0f);
    
    // Mood impact
    UpdateMood(-0.3f);
    
    // Force flee state if not already fleeing
    if (BehaviorData.CurrentState != ENPC_BehaviorState::Fleeing)
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NPC %s is fleeing from threat!"), *NPCName);
}