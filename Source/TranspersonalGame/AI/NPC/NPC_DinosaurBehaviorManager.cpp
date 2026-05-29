#include "NPC_DinosaurBehaviorManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default values
    Species = ENPC_DinosaurSpecies::TRex;
    DetectionRange = 3000.0f;
    AttackRange = 300.0f;
    PatrolRadius = 5000.0f;
    MovementSpeed = 400.0f;
    CurrentState = ENPC_DinosaurState::Idle;
    StateTimer = 0.0f;
    CachedPlayer = nullptr;
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    HomeLocation = GetOwner()->GetActorLocation();
    InitializeSpeciesTraits();
    GeneratePatrolPoints();
    
    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        if (AIController)
        {
            BlackboardComp = AIController->GetBlackboardComponent();
        }
    }
    
    // Cache player reference
    CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    UpdateBehaviorLogic(DeltaTime);
    UpdateBlackboard();
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesTraits()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            DetectionRange = 3000.0f;
            AttackRange = 300.0f;
            PatrolRadius = 5000.0f;
            MovementSpeed = 600.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            DetectionRange = 2500.0f;
            AttackRange = 200.0f;
            PatrolRadius = 3000.0f;
            MovementSpeed = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            DetectionRange = 2000.0f;
            AttackRange = 400.0f;
            PatrolRadius = 2000.0f;
            MovementSpeed = 300.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            DetectionRange = 1500.0f;
            AttackRange = 500.0f;
            PatrolRadius = 4000.0f;
            MovementSpeed = 200.0f;
            break;
            
        case ENPC_DinosaurSpecies::Ankylosaurus:
            DetectionRange = 1800.0f;
            AttackRange = 250.0f;
            PatrolRadius = 1500.0f;
            MovementSpeed = 250.0f;
            break;
            
        case ENPC_DinosaurSpecies::Parasaurolophus:
            DetectionRange = 2200.0f;
            AttackRange = 150.0f;
            PatrolRadius = 6000.0f;
            MovementSpeed = 500.0f;
            break;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBehaviorLogic(float DeltaTime)
{
    if (!CachedPlayer)
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        return;
    }
    
    // Update memory
    Memory.TimeSincePlayerSeen += DeltaTime;
    
    // Check if player is in detection range
    bool bPlayerVisible = CanSeePlayer();
    if (bPlayerVisible)
    {
        Memory.LastPlayerLocation = CachedPlayer->GetActorLocation();
        Memory.TimeSincePlayerSeen = 0.0f;
        ReactToPlayerPresence();
    }
    
    // State-specific behavior
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Patrolling:
            if (StateTimer > 10.0f) // Change patrol point every 10 seconds
            {
                GetNextPatrolPoint();
                StateTimer = 0.0f;
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (Memory.TimeSincePlayerSeen > 15.0f) // Lost player for 15 seconds
            {
                SetDinosaurState(ENPC_DinosaurState::Patrolling);
            }
            break;
            
        case ENPC_DinosaurState::Idle:
            if (StateTimer > 5.0f) // Idle for 5 seconds
            {
                SetDinosaurState(ENPC_DinosaurState::Patrolling);
            }
            break;
            
        case ENPC_DinosaurState::Feeding:
            if (StateTimer > 8.0f) // Feed for 8 seconds
            {
                SetDinosaurState(ENPC_DinosaurState::Idle);
            }
            break;
    }
}

void UNPC_DinosaurBehaviorManager::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s changed state to %d"), 
               *GetOwner()->GetName(), (int32)CurrentState);
    }
}

void UNPC_DinosaurBehaviorManager::UpdatePlayerMemory(const FVector& PlayerLocation, bool bIsHostile)
{
    Memory.LastPlayerLocation = PlayerLocation;
    Memory.bPlayerIsHostile = bIsHostile;
    Memory.TimeSincePlayerSeen = 0.0f;
}

bool UNPC_DinosaurBehaviorManager::CanSeePlayer() const
{
    if (!CachedPlayer)
        return false;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = CachedPlayer->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
    
    return Distance <= DetectionRange;
}

void UNPC_DinosaurBehaviorManager::GeneratePatrolPoints()
{
    Memory.PatrolPoints.Empty();
    
    // Generate 4 patrol points around home location
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (i * 90.0f) * PI / 180.0f;
        FVector Offset = FVector(
            FMath::Cos(Angle) * PatrolRadius,
            FMath::Sin(Angle) * PatrolRadius,
            0.0f
        );
        
        Memory.PatrolPoints.Add(HomeLocation + Offset);
    }
    
    Memory.CurrentPatrolIndex = 0;
}

FVector UNPC_DinosaurBehaviorManager::GetNextPatrolPoint()
{
    if (Memory.PatrolPoints.Num() == 0)
    {
        GeneratePatrolPoints();
    }
    
    FVector NextPoint = Memory.PatrolPoints[Memory.CurrentPatrolIndex];
    Memory.CurrentPatrolIndex = (Memory.CurrentPatrolIndex + 1) % Memory.PatrolPoints.Num();
    
    return NextPoint;
}

void UNPC_DinosaurBehaviorManager::ReactToPlayerPresence()
{
    if (!CachedPlayer)
        return;
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayer->GetActorLocation());
    
    // Species-specific reactions
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
        case ENPC_DinosaurSpecies::Velociraptor:
            // Predators hunt the player
            if (DistanceToPlayer <= AttackRange)
            {
                SetDinosaurState(ENPC_DinosaurState::Territorial);
            }
            else
            {
                SetDinosaurState(ENPC_DinosaurState::Hunting);
            }
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Ankylosaurus:
            // Defensive herbivores
            if (DistanceToPlayer <= AttackRange)
            {
                SetDinosaurState(ENPC_DinosaurState::Territorial);
            }
            else
            {
                SetDinosaurState(ENPC_DinosaurState::Idle);
            }
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Parasaurolophus:
            // Peaceful herbivores flee
            SetDinosaurState(ENPC_DinosaurState::Fleeing);
            break;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBlackboard()
{
    if (!BlackboardComp)
        return;
    
    // Update blackboard with current state
    BlackboardComp->SetValueAsEnum(TEXT("DinosaurState"), (uint8)CurrentState);
    BlackboardComp->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComp->SetValueAsVector(TEXT("LastPlayerLocation"), Memory.LastPlayerLocation);
    BlackboardComp->SetValueAsBool(TEXT("PlayerVisible"), CanSeePlayer());
    BlackboardComp->SetValueAsFloat(TEXT("TimeSincePlayerSeen"), Memory.TimeSincePlayerSeen);
    
    if (Memory.PatrolPoints.Num() > 0)
    {
        BlackboardComp->SetValueAsVector(TEXT("NextPatrolPoint"), GetNextPatrolPoint());
    }
}