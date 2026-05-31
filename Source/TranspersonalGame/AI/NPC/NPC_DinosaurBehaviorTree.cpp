#include "NPC_DinosaurBehaviorTree.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    // Initialize default behavior data
    BehaviorData.Species = ENPC_DinosaurSpecies::TRex;
    BehaviorData.CurrentState = ENPC_DinosaurState::Idle;
    BehaviorData.PatrolRadius = 5000.0f;
    BehaviorData.DetectionRange = 3000.0f;
    BehaviorData.AttackRange = 300.0f;
    BehaviorData.MovementSpeed = 600.0f;
    BehaviorData.Aggression = 0.7f;
    BehaviorData.Hunger = 0.5f;
    BehaviorData.Stamina = 1.0f;
    BehaviorData.bIsPackHunter = false;
    BehaviorData.PatrolCenter = FVector::ZeroVector;
    BehaviorData.CurrentTarget = nullptr;
    BehaviorData.LastPlayerSightTime = 0.0f;
    BehaviorData.TerritorialRadius = 2000.0f;
}

void UNPC_DinosaurBehaviorTree::InitializeBehaviorForSpecies(ENPC_DinosaurSpecies Species, const FVector& SpawnLocation)
{
    BehaviorData.Species = Species;
    BehaviorData.PatrolCenter = SpawnLocation;
    
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            SetupTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            SetupVelociraptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Ankylosaurus:
        case ENPC_DinosaurSpecies::Parasaurolophus:
            SetupHerbivoreBehavior();
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur behavior initialized for species: %d at location: %s"), 
           (int32)Species, *SpawnLocation.ToString());
}

void UNPC_DinosaurBehaviorTree::SetupTRexBehavior()
{
    BehaviorData.PatrolRadius = 8000.0f;
    BehaviorData.DetectionRange = 4000.0f;
    BehaviorData.AttackRange = 500.0f;
    BehaviorData.MovementSpeed = 800.0f;
    BehaviorData.Aggression = 0.9f;
    BehaviorData.bIsPackHunter = false;
    BehaviorData.TerritorialRadius = 3000.0f;
    BehaviorData.CurrentState = ENPC_DinosaurState::Patrol;
}

void UNPC_DinosaurBehaviorTree::SetupVelociraptorBehavior()
{
    BehaviorData.PatrolRadius = 6000.0f;
    BehaviorData.DetectionRange = 3500.0f;
    BehaviorData.AttackRange = 200.0f;
    BehaviorData.MovementSpeed = 1200.0f;
    BehaviorData.Aggression = 0.8f;
    BehaviorData.bIsPackHunter = true;
    BehaviorData.TerritorialRadius = 2500.0f;
    BehaviorData.CurrentState = ENPC_DinosaurState::Patrol;
}

void UNPC_DinosaurBehaviorTree::SetupHerbivoreBehavior()
{
    BehaviorData.PatrolRadius = 4000.0f;
    BehaviorData.DetectionRange = 2000.0f;
    BehaviorData.AttackRange = 400.0f;
    BehaviorData.MovementSpeed = 400.0f;
    BehaviorData.Aggression = 0.3f;
    BehaviorData.bIsPackHunter = false;
    BehaviorData.TerritorialRadius = 1500.0f;
    BehaviorData.CurrentState = ENPC_DinosaurState::Eat;
}

void UNPC_DinosaurBehaviorTree::UpdateBehaviorState(ENPC_DinosaurState NewState)
{
    if (BehaviorData.CurrentState != NewState)
    {
        ENPC_DinosaurState PreviousState = BehaviorData.CurrentState;
        BehaviorData.CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur state changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);
    }
}

bool UNPC_DinosaurBehaviorTree::ShouldChasePlayer(AActor* PlayerActor, float Distance)
{
    if (!PlayerActor)
    {
        return false;
    }
    
    // Check if player is within detection range
    if (Distance > BehaviorData.DetectionRange)
    {
        return false;
    }
    
    // Update last sight time
    BehaviorData.LastPlayerSightTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Aggression and hunger influence chase decision
    float ChaseThreshold = (1.0f - BehaviorData.Aggression) + (BehaviorData.Hunger * 0.5f);
    
    return Distance <= (BehaviorData.DetectionRange * ChaseThreshold);
}

bool UNPC_DinosaurBehaviorTree::ShouldAttackTarget(AActor* Target, float Distance)
{
    if (!Target || Distance > BehaviorData.AttackRange)
    {
        return false;
    }
    
    // Check stamina for attack
    if (BehaviorData.Stamina < 0.3f)
    {
        return false;
    }
    
    return true;
}

FVector UNPC_DinosaurBehaviorTree::GetNextPatrolPoint()
{
    // Generate random point within patrol radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(BehaviorData.PatrolRadius * 0.3f, BehaviorData.PatrolRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return BehaviorData.PatrolCenter + RandomOffset;
}

void UNPC_DinosaurBehaviorTree::SetTerritorialBehavior(const FVector& Center, float Radius)
{
    BehaviorData.PatrolCenter = Center;
    BehaviorData.TerritorialRadius = Radius;
    BehaviorData.CurrentState = ENPC_DinosaurState::Territorial;
    
    UE_LOG(LogTemp, Warning, TEXT("Territorial behavior set at %s with radius %f"), 
           *Center.ToString(), Radius);
}

float UNPC_DinosaurBehaviorTree::GetSpeciesAggression(ENPC_DinosaurSpecies Species)
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            return 0.9f;
        case ENPC_DinosaurSpecies::Velociraptor:
            return 0.8f;
        case ENPC_DinosaurSpecies::Ankylosaurus:
            return 0.4f;
        case ENPC_DinosaurSpecies::Triceratops:
            return 0.3f;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            return 0.1f;
        case ENPC_DinosaurSpecies::Parasaurolophus:
            return 0.2f;
        default:
            return 0.5f;
    }
}

float UNPC_DinosaurBehaviorTree::GetSpeciesDetectionRange(ENPC_DinosaurSpecies Species)
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            return 4000.0f;
        case ENPC_DinosaurSpecies::Velociraptor:
            return 3500.0f;
        case ENPC_DinosaurSpecies::Triceratops:
            return 2500.0f;
        case ENPC_DinosaurSpecies::Ankylosaurus:
            return 2000.0f;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            return 3000.0f;
        case ENPC_DinosaurSpecies::Parasaurolophus:
            return 2800.0f;
        default:
            return 2500.0f;
    }
}

bool UNPC_DinosaurBehaviorTree::IsNocturnal(ENPC_DinosaurSpecies Species)
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            return true; // T-Rex hunts at night
        case ENPC_DinosaurSpecies::Velociraptor:
            return true; // Pack hunters active at night
        default:
            return false; // Most herbivores are diurnal
    }
}

void UNPC_DinosaurBehaviorTree::SetupTRexBehavior()
{
    // T-Rex specific behavior setup
    BehaviorData.PatrolRadius = 8000.0f;
    BehaviorData.DetectionRange = 4000.0f;
    BehaviorData.AttackRange = 500.0f;
    BehaviorData.MovementSpeed = 800.0f;
    BehaviorData.Aggression = 0.9f;
    BehaviorData.bIsPackHunter = false;
    BehaviorData.TerritorialRadius = 3000.0f;
}

void UNPC_DinosaurBehaviorTree::SetupVelociraptorBehavior()
{
    // Velociraptor pack behavior
    BehaviorData.PatrolRadius = 6000.0f;
    BehaviorData.DetectionRange = 3500.0f;
    BehaviorData.AttackRange = 200.0f;
    BehaviorData.MovementSpeed = 1200.0f;
    BehaviorData.Aggression = 0.8f;
    BehaviorData.bIsPackHunter = true;
    BehaviorData.TerritorialRadius = 2500.0f;
}

void UNPC_DinosaurBehaviorTree::SetupHerbivoreBehavior()
{
    // Herbivore defensive behavior
    BehaviorData.PatrolRadius = 4000.0f;
    BehaviorData.DetectionRange = 2000.0f;
    BehaviorData.AttackRange = 400.0f;
    BehaviorData.MovementSpeed = 400.0f;
    BehaviorData.Aggression = 0.3f;
    BehaviorData.bIsPackHunter = false;
    BehaviorData.TerritorialRadius = 1500.0f;
}

float UNPC_DinosaurBehaviorTree::GetRandomPatrolDelay()
{
    return FMath::RandRange(3.0f, 8.0f);
}

bool UNPC_DinosaurBehaviorTree::IsPlayerInTerritory(AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        return false;
    }
    
    float Distance = FVector::Dist(PlayerActor->GetActorLocation(), BehaviorData.PatrolCenter);
    return Distance <= BehaviorData.TerritorialRadius;
}