#include "NPC_DinosaurBehaviorManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSpeciesDefaults();
    SetupAIReferences();
    
    // Initialize territory center to current location
    if (GetOwner())
    {
        Memory.TerritoryCenter = GetOwner()->GetActorLocation();
    }
    
    // Set initial state
    SetDinosaurState(ENPC_DinosaurState::Idle);
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
    {
        return;
    }
    
    // Update core systems
    UpdateStats(DeltaTime);
    ProcessPerception(DeltaTime);
    ExecuteCurrentBehavior(DeltaTime);
    
    // Update timing
    TimeSinceLastStateChange += DeltaTime;
    TimeSinceLastStatsUpdate += DeltaTime;
    Memory.TimeSinceLastPlayerSighting += DeltaTime;
    
    // Handle state transitions
    HandleStateTransitions();
    
    // Update blackboard for AI
    UpdateBlackboardValues();
}

void UNPC_DinosaurBehaviorManager::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        TimeSinceLastStateChange = 0.0f;
        
        // Log state change for debugging
        if (GetOwner())
        {
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to %d"), 
                *GetOwner()->GetName(), (int32)NewState);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateStats(float DeltaTime)
{
    // Update hunger and thirst over time
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (DeltaTime * 0.5f)); // Lose 0.5 hunger per second
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - (DeltaTime * 0.8f)); // Lose 0.8 thirst per second
    
    // Regenerate stamina when not in combat
    if (CurrentState != ENPC_DinosaurState::Hunting && CurrentState != ENPC_DinosaurState::Fleeing)
    {
        Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + (DeltaTime * 10.0f));
    }
    
    // Health decreases if starving or dehydrated
    if (Stats.Hunger <= 0.0f || Stats.Thirst <= 0.0f)
    {
        Stats.Health = FMath::Max(0.0f, Stats.Health - (DeltaTime * 5.0f));
    }
    
    // Reduce fear over time
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - (DeltaTime * 2.0f));
    
    // Check for death
    if (Stats.Health <= 0.0f && CurrentState != ENPC_DinosaurState::Dead)
    {
        SetDinosaurState(ENPC_DinosaurState::Dead);
    }
}

void UNPC_DinosaurBehaviorManager::ProcessPerception(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    
    if (Player)
    {
        float DistanceToPlayer = GetDistanceToPlayer();
        
        // Update memory if player is detected
        if (DistanceToPlayer <= Stats.DetectionRange)
        {
            Memory.LastKnownPlayerLocation = Player->GetActorLocation();
            Memory.TimeSinceLastPlayerSighting = 0.0f;
            
            // Add player as threat if aggressive species or if player is too close
            if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Velociraptor)
            {
                if (!Memory.bPlayerIsKnownThreat)
                {
                    Memory.bPlayerIsKnownThreat = true;
                    AddThreatToMemory(Player);
                }
            }
        }
    }
    
    // Clean up old threats
    for (int32 i = Memory.KnownThreats.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(Memory.KnownThreats[i]))
        {
            Memory.KnownThreats.RemoveAt(i);
        }
    }
}

void UNPC_DinosaurBehaviorManager::ExecuteCurrentBehavior(float DeltaTime)
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            ExecuteTRexBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            ExecuteVelociraptorBehavior(DeltaTime);
            break;
            
        default:
            ExecuteHerbivoreBehavior(DeltaTime);
            break;
    }
}

void UNPC_DinosaurBehaviorManager::ExecuteTRexBehavior(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    float DistanceToPlayer = GetDistanceToPlayer();
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime) // 10% chance per second to start patrolling
            {
                TransitionToPatrolling();
            }
            break;
            
        case ENPC_DinosaurState::Patrolling:
            if (Player && DistanceToPlayer <= Stats.DetectionRange)
            {
                TransitionToHunting(Player);
            }
            else
            {
                UpdateMovement(DeltaTime);
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (!Player || DistanceToPlayer > Stats.DetectionRange * 2.0f)
            {
                TransitionToPatrolling();
            }
            else if (DistanceToPlayer <= Stats.AttackRange)
            {
                // Attack behavior would be handled by combat system
                Stats.Aggression = FMath::Min(100.0f, Stats.Aggression + DeltaTime * 10.0f);
            }
            break;
            
        case ENPC_DinosaurState::Feeding:
            Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + DeltaTime * 20.0f);
            if (Stats.Hunger >= 80.0f)
            {
                TransitionToIdle();
            }
            break;
            
        case ENPC_DinosaurState::Dead:
            // Do nothing when dead
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorManager::ExecuteVelociraptorBehavior(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    float DistanceToPlayer = GetDistanceToPlayer();
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            if (FMath::RandRange(0.0f, 1.0f) < 0.15f * DeltaTime) // 15% chance per second (more active than T-Rex)
            {
                TransitionToPatrolling();
            }
            break;
            
        case ENPC_DinosaurState::Patrolling:
            if (Player && DistanceToPlayer <= Stats.DetectionRange * 1.2f) // Better detection than T-Rex
            {
                TransitionToHunting(Player);
            }
            else
            {
                UpdateMovement(DeltaTime);
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (!Player || DistanceToPlayer > Stats.DetectionRange * 2.5f)
            {
                TransitionToPatrolling();
            }
            else if (DistanceToPlayer <= Stats.AttackRange * 1.5f) // Larger attack range
            {
                Stats.Aggression = FMath::Min(100.0f, Stats.Aggression + DeltaTime * 15.0f);
            }
            break;
            
        case ENPC_DinosaurState::Feeding:
            Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + DeltaTime * 15.0f);
            if (Stats.Hunger >= 70.0f)
            {
                TransitionToIdle();
            }
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorManager::ExecuteHerbivoreBehavior(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    float DistanceToPlayer = GetDistanceToPlayer();
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f * DeltaTime) // 5% chance per second (less active)
            {
                TransitionToPatrolling();
            }
            else if (Stats.Hunger < 30.0f)
            {
                TransitionToFeeding();
            }
            break;
            
        case ENPC_DinosaurState::Patrolling:
            if (Player && DistanceToPlayer <= Stats.DetectionRange * 0.8f) // Shorter detection range
            {
                Stats.Fear = FMath::Min(100.0f, Stats.Fear + DeltaTime * 20.0f);
                if (Stats.Fear > 50.0f)
                {
                    TransitionToFleeing(Player);
                }
            }
            else
            {
                UpdateMovement(DeltaTime);
            }
            break;
            
        case ENPC_DinosaurState::Fleeing:
            if (!Player || DistanceToPlayer > Stats.DetectionRange * 3.0f)
            {
                TransitionToIdle();
            }
            break;
            
        case ENPC_DinosaurState::Feeding:
            Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + DeltaTime * 25.0f);
            if (Stats.Hunger >= 90.0f)
            {
                TransitionToIdle();
            }
            break;
            
        default:
            break;
    }
}

bool UNPC_DinosaurBehaviorManager::IsPlayerInRange(float Range) const
{
    float Distance = GetDistanceToPlayer();
    return Distance > 0.0f && Distance <= Range;
}

AActor* UNPC_DinosaurBehaviorManager::FindNearestPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        return UGameplayStatics::GetPlayerPawn(World, 0);
    }
    return nullptr;
}

float UNPC_DinosaurBehaviorManager::GetDistanceToPlayer() const
{
    AActor* Player = FindNearestPlayer();
    if (Player && GetOwner())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    return -1.0f;
}

void UNPC_DinosaurBehaviorManager::SetPatrolPoints(const TArray<FVector>& Points)
{
    Memory.PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}

void UNPC_DinosaurBehaviorManager::AddThreatToMemory(AActor* Threat)
{
    if (Threat && !Memory.KnownThreats.Contains(Threat))
    {
        Memory.KnownThreats.Add(Threat);
    }
}

void UNPC_DinosaurBehaviorManager::RemoveThreatFromMemory(AActor* Threat)
{
    Memory.KnownThreats.Remove(Threat);
    if (Threat == FindNearestPlayer())
    {
        Memory.bPlayerIsKnownThreat = false;
    }
}

void UNPC_DinosaurBehaviorManager::TransitionToIdle()
{
    SetDinosaurState(ENPC_DinosaurState::Idle);
    CurrentTarget = nullptr;
}

void UNPC_DinosaurBehaviorManager::TransitionToPatrolling()
{
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
    
    // Generate random patrol points if none exist
    if (Memory.PatrolPoints.Num() == 0)
    {
        TArray<FVector> PatrolPoints;
        FVector Center = Memory.TerritoryCenter;
        
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) + FMath::RandRange(-30.0f, 30.0f);
            float Distance = FMath::RandRange(Stats.TerritorialRadius * 0.3f, Stats.TerritorialRadius * 0.8f);
            
            FVector Point = Center + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
                0.0f
            );
            
            PatrolPoints.Add(Point);
        }
        
        SetPatrolPoints(PatrolPoints);
    }
}

void UNPC_DinosaurBehaviorManager::TransitionToHunting(AActor* Target)
{
    SetDinosaurState(ENPC_DinosaurState::Hunting);
    CurrentTarget = Target;
    Stats.Aggression = FMath::Min(100.0f, Stats.Aggression + 20.0f);
}

void UNPC_DinosaurBehaviorManager::TransitionToFleeing(AActor* Threat)
{
    SetDinosaurState(ENPC_DinosaurState::Fleeing);
    CurrentTarget = Threat;
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + 30.0f);
}

void UNPC_DinosaurBehaviorManager::TransitionToFeeding()
{
    SetDinosaurState(ENPC_DinosaurState::Feeding);
    CurrentTarget = nullptr;
}

bool UNPC_DinosaurBehaviorManager::IsInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, Memory.TerritoryCenter);
    return Distance <= Stats.TerritorialRadius;
}

FVector UNPC_DinosaurBehaviorManager::GetRandomPatrolPoint() const
{
    if (Memory.PatrolPoints.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Memory.PatrolPoints.Num() - 1);
        return Memory.PatrolPoints[RandomIndex];
    }
    
    // Generate random point in territory
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(0.0f, Stats.TerritorialRadius);
    
    return Memory.TerritoryCenter + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
}

void UNPC_DinosaurBehaviorManager::UpdateBlackboardValues()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), Stats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), Stats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Fear"), Stats.Fear);
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), Stats.Aggression);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
        
        BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), Memory.TerritoryCenter);
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), Memory.LastKnownPlayerLocation);
    }
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesDefaults()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.Health = 200.0f;
            Stats.Aggression = 80.0f;
            Stats.TerritorialRadius = 8000.0f;
            Stats.DetectionRange = 4000.0f;
            Stats.AttackRange = 400.0f;
            PatrolSpeed = 300.0f;
            ChaseSpeed = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            Stats.Health = 80.0f;
            Stats.Aggression = 90.0f;
            Stats.TerritorialRadius = 5000.0f;
            Stats.DetectionRange = 3500.0f;
            Stats.AttackRange = 200.0f;
            PatrolSpeed = 400.0f;
            ChaseSpeed = 1000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            Stats.Health = 300.0f;
            Stats.Aggression = 30.0f;
            Stats.TerritorialRadius = 6000.0f;
            Stats.DetectionRange = 2000.0f;
            Stats.AttackRange = 300.0f;
            PatrolSpeed = 150.0f;
            ChaseSpeed = 400.0f;
            FleeSpeed = 600.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.Health = 500.0f;
            Stats.Aggression = 10.0f;
            Stats.TerritorialRadius = 10000.0f;
            Stats.DetectionRange = 1500.0f;
            Stats.AttackRange = 500.0f;
            PatrolSpeed = 100.0f;
            ChaseSpeed = 200.0f;
            FleeSpeed = 300.0f;
            break;
            
        default:
            // Default herbivore stats
            Stats.Health = 150.0f;
            Stats.Aggression = 20.0f;
            Stats.TerritorialRadius = 4000.0f;
            Stats.DetectionRange = 2500.0f;
            Stats.AttackRange = 250.0f;
            PatrolSpeed = 200.0f;
            ChaseSpeed = 500.0f;
            FleeSpeed = 700.0f;
            break;
    }
}

void UNPC_DinosaurBehaviorManager::SetupAIReferences()
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        DinosaurAIController = Cast<AAIController>(OwnerPawn->GetController());
        
        if (DinosaurAIController)
        {
            BlackboardComponent = DinosaurAIController->GetBlackboardComponent();
            BehaviorTreeComponent = DinosaurAIController->GetBehaviorTreeComponent();
        }
        
        PawnSensingComponent = OwnerPawn->FindComponentByClass<UPawnSensingComponent>();
    }
}

bool UNPC_DinosaurBehaviorManager::ShouldChangeState() const
{
    return TimeSinceLastStateChange >= StateChangeInterval;
}

void UNPC_DinosaurBehaviorManager::HandleStateTransitions()
{
    // Handle automatic state transitions based on conditions
    if (Stats.Health <= 0.0f && CurrentState != ENPC_DinosaurState::Dead)
    {
        SetDinosaurState(ENPC_DinosaurState::Dead);
        return;
    }
    
    // Handle hunger-driven feeding for herbivores
    if (Species != ENPC_DinosaurSpecies::TRex && Species != ENPC_DinosaurSpecies::Velociraptor)
    {
        if (Stats.Hunger < 20.0f && CurrentState != ENPC_DinosaurState::Feeding)
        {
            TransitionToFeeding();
            return;
        }
    }
    
    // Handle fear-driven fleeing
    if (Stats.Fear > 70.0f && CurrentState != ENPC_DinosaurState::Fleeing)
    {
        AActor* Player = FindNearestPlayer();
        if (Player)
        {
            TransitionToFleeing(Player);
            return;
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateMovement(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector TargetLocation = FVector::ZeroVector;
    bool bHasTarget = false;
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Patrolling:
            if (Memory.PatrolPoints.Num() > 0)
            {
                TargetLocation = Memory.PatrolPoints[CurrentPatrolIndex];
                bHasTarget = true;
                
                // Check if reached patrol point
                float DistanceToPatrol = FVector::Dist(GetOwner()->GetActorLocation(), TargetLocation);
                if (DistanceToPatrol < 200.0f)
                {
                    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % Memory.PatrolPoints.Num();
                }
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (CurrentTarget)
            {
                TargetLocation = CurrentTarget->GetActorLocation();
                bHasTarget = true;
            }
            break;
            
        case ENPC_DinosaurState::Fleeing:
            if (CurrentTarget)
            {
                // Move away from threat
                FVector FleeDirection = GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation();
                FleeDirection.Normalize();
                TargetLocation = GetOwner()->GetActorLocation() + (FleeDirection * 2000.0f);
                bHasTarget = true;
            }
            break;
    }
    
    // Update blackboard with movement target
    if (BlackboardComponent && bHasTarget)
    {
        BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), TargetLocation);
    }
}

void UNPC_DinosaurBehaviorManager::UpdatePerceptionData()
{
    // This would integrate with UE5's perception system
    // For now, we use simple distance checks
}