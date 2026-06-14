#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_BehaviorState::Idle;
    StateTimer = 0.0f;
    CurrentTarget = FVector::ZeroVector;
    CurrentThreat = nullptr;
    HomeLocation = FVector::ZeroVector;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set home location to current position
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }
    
    // Initialize emotional state with random variations
    EmotionalState.Fear = FMath::RandRange(0.1f, 0.3f);
    EmotionalState.Aggression = FMath::RandRange(0.2f, 0.5f);
    EmotionalState.Curiosity = FMath::RandRange(0.3f, 0.7f);
    EmotionalState.Hunger = FMath::RandRange(0.4f, 0.6f);
    EmotionalState.Fatigue = FMath::RandRange(0.1f, 0.2f);
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmotionalState(DeltaTime);
    ExecuteBehavior(DeltaTime);
    StateTimer += DeltaTime;
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Add memory of state change
        if (AActor* Owner = GetOwner())
        {
            AddMemory(Owner->GetActorLocation(), FString::Printf(TEXT("StateChange_%s"), 
                *UEnum::GetValueAsString(NewState)), 0.5f);
        }
    }
}

void UNPC_BehaviorComponent::AddMemory(FVector Location, FString EventType, float Importance)
{
    FNPC_Memory NewMemory;
    NewMemory.Location = Location;
    NewMemory.EventType = EventType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    Memories.Add(NewMemory);
    
    // Keep only the 20 most important/recent memories
    if (Memories.Num() > 20)
    {
        Memories.Sort([](const FNPC_Memory& A, const FNPC_Memory& B) {
            return (A.Importance + A.Timestamp * 0.1f) > (B.Importance + B.Timestamp * 0.1f);
        });
        Memories.SetNum(20);
    }
}

void UNPC_BehaviorComponent::UpdateEmotionalState(float DeltaTime)
{
    // Natural decay of emotions over time
    EmotionalState.Fear = FMath::Max(0.0f, EmotionalState.Fear - DeltaTime * 0.1f);
    EmotionalState.Aggression = FMath::Max(0.0f, EmotionalState.Aggression - DeltaTime * 0.05f);
    EmotionalState.Curiosity = FMath::Clamp(EmotionalState.Curiosity + DeltaTime * 0.02f, 0.0f, 1.0f);
    EmotionalState.Hunger = FMath::Clamp(EmotionalState.Hunger + DeltaTime * 0.03f, 0.0f, 1.0f);
    EmotionalState.Fatigue = FMath::Clamp(EmotionalState.Fatigue + DeltaTime * 0.01f, 0.0f, 1.0f);
    
    // Detect threats and adjust fear/aggression
    AActor* Threat = DetectNearbyThreats();
    if (Threat)
    {
        CurrentThreat = Threat;
        EmotionalState.Fear = FMath::Min(1.0f, EmotionalState.Fear + DeltaTime * 0.5f);
        EmotionalState.Aggression = FMath::Min(1.0f, EmotionalState.Aggression + DeltaTime * 0.3f);
    }
    else
    {
        CurrentThreat = nullptr;
    }
}

AActor* UNPC_BehaviorComponent::DetectNearbyThreats()
{
    if (!GetOwner() || !GetWorld())
        return nullptr;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = DetectionRadius;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
            
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            // Check if this is the player character or another threatening NPC
            if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
            {
                NearestThreat = Actor;
                NearestDistance = Distance;
            }
        }
    }
    
    return NearestThreat;
}

void UNPC_BehaviorComponent::ExecuteBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            ProcessIdleBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrolling:
            ProcessPatrolBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Investigating:
            ProcessInvestigatingBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Fleeing:
            ProcessFleeingBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Attacking:
            ProcessAttackingBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Feeding:
            ProcessFeedingBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Resting:
            ProcessRestingBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Socializing:
            ProcessSocializingBehavior(DeltaTime);
            break;
    }
}

FVector UNPC_BehaviorComponent::GetPatrolTarget()
{
    // Generate a random point within patrol radius from home
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    return HomeLocation + (RandomDirection * RandomDistance);
}

bool UNPC_BehaviorComponent::ShouldFlee()
{
    return EmotionalState.Fear > FleeThreshold && CurrentThreat != nullptr;
}

bool UNPC_BehaviorComponent::ShouldAttack()
{
    return EmotionalState.Aggression > AggressionThreshold && 
           EmotionalState.Fear < 0.5f && 
           CurrentThreat != nullptr;
}

void UNPC_BehaviorComponent::ProcessIdleBehavior(float DeltaTime)
{
    // Check for state transitions
    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
        return;
    }
    
    if (ShouldAttack())
    {
        SetBehaviorState(ENPC_BehaviorState::Attacking);
        return;
    }
    
    // Random state transitions based on emotional state
    if (StateTimer > 5.0f)
    {
        if (EmotionalState.Curiosity > 0.6f && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
        else if (EmotionalState.Hunger > 0.7f)
        {
            SetBehaviorState(ENPC_BehaviorState::Feeding);
        }
        else if (EmotionalState.Fatigue > 0.8f)
        {
            SetBehaviorState(ENPC_BehaviorState::Resting);
        }
    }
}

void UNPC_BehaviorComponent::ProcessPatrolBehavior(float DeltaTime)
{
    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
        return;
    }
    
    if (ShouldAttack())
    {
        SetBehaviorState(ENPC_BehaviorState::Attacking);
        return;
    }
    
    // Set new patrol target if needed
    if (CurrentTarget == FVector::ZeroVector || StateTimer > 10.0f)
    {
        CurrentTarget = GetPatrolTarget();
        StateTimer = 0.0f;
    }
    
    // Move towards target (this would be handled by movement component in practice)
    if (AActor* Owner = GetOwner())
    {
        FVector Direction = (CurrentTarget - Owner->GetActorLocation()).GetSafeNormal();
        // In a real implementation, this would use CharacterMovementComponent
    }
    
    // Return to idle after patrol
    if (StateTimer > 15.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorComponent::ProcessInvestigatingBehavior(float DeltaTime)
{
    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
        return;
    }
    
    // Investigation complete after some time
    if (StateTimer > 8.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorComponent::ProcessFleeingBehavior(float DeltaTime)
{
    if (!CurrentThreat)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
        return;
    }
    
    // Calculate flee direction (away from threat)
    if (AActor* Owner = GetOwner())
    {
        FVector FleeDirection = (Owner->GetActorLocation() - CurrentThreat->GetActorLocation()).GetSafeNormal();
        CurrentTarget = Owner->GetActorLocation() + (FleeDirection * 2000.0f);
        
        // Add memory of threat
        AddMemory(CurrentThreat->GetActorLocation(), TEXT("Threat_Flee"), 0.9f);
    }
    
    // Stop fleeing if far enough or threat is gone
    if (StateTimer > 10.0f || EmotionalState.Fear < 0.3f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorComponent::ProcessAttackingBehavior(float DeltaTime)
{
    if (!CurrentThreat || EmotionalState.Fear > 0.7f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
        return;
    }
    
    // Attack logic would be implemented here
    if (AActor* Owner = GetOwner())
    {
        float DistanceToThreat = FVector::Dist(Owner->GetActorLocation(), CurrentThreat->GetActorLocation());
        if (DistanceToThreat > DetectionRadius * 1.5f)
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
    
    // Add memory of combat
    if (StateTimer > 1.0f)
    {
        AddMemory(CurrentThreat->GetActorLocation(), TEXT("Combat_Engaged"), 0.8f);
    }
}

void UNPC_BehaviorComponent::ProcessFeedingBehavior(float DeltaTime)
{
    // Reduce hunger while feeding
    EmotionalState.Hunger = FMath::Max(0.0f, EmotionalState.Hunger - DeltaTime * 0.2f);
    
    // Feeding complete
    if (EmotionalState.Hunger < 0.3f || StateTimer > 12.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorComponent::ProcessRestingBehavior(float DeltaTime)
{
    // Reduce fatigue while resting
    EmotionalState.Fatigue = FMath::Max(0.0f, EmotionalState.Fatigue - DeltaTime * 0.3f);
    
    // Resting complete
    if (EmotionalState.Fatigue < 0.2f || StateTimer > 8.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorComponent::ProcessSocializingBehavior(float DeltaTime)
{
    // Social interaction reduces stress and increases curiosity
    EmotionalState.Fear = FMath::Max(0.0f, EmotionalState.Fear - DeltaTime * 0.1f);
    EmotionalState.Curiosity = FMath::Min(1.0f, EmotionalState.Curiosity + DeltaTime * 0.05f);
    
    // Socializing complete
    if (StateTimer > 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}