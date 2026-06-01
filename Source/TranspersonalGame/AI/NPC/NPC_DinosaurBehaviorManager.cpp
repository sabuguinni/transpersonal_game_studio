#include "NPC_DinosaurBehaviorManager.h"
#include "NPC_DinosaurAI.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize behavior state
    BehaviorState.CurrentBehavior = ENPC_DinosaurBehaviorType::Idle;
    BehaviorState.PatrolCenter = FVector::ZeroVector;
    BehaviorState.PatrolRadius = 5000.0f;
    BehaviorState.TargetActor = nullptr;
    BehaviorState.LastBehaviorChangeTime = 0.0f;
    BehaviorState.AggressionLevel = 0.0f;
    BehaviorState.bIsInCombat = false;
    BehaviorState.bIsHunting = false;
    
    // Initialize memory
    Memory.SeenActors.Empty();
    Memory.InterestingLocations.Empty();
    Memory.LastPlayerSightingTime = 0.0f;
    Memory.ThreatLevel = 0.0f;
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get AI controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        OwnerAI = Cast<ANPC_DinosaurAI>(OwnerPawn->GetController());
        if (OwnerAI)
        {
            BehaviorTreeComponent = OwnerAI->GetBehaviorTreeComponent();
            BlackboardComponent = OwnerAI->GetBlackboardComponent();
        }
    }
    
    // Set initial patrol center to current location
    if (GetOwner())
    {
        BehaviorState.PatrolCenter = GetOwner()->GetActorLocation();
    }
    
    // Initialize species-specific behavior
    InitializeSpeciesBehavior(DinosaurSpecies);
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Update behavior logic
    UpdateBehaviorLogic(DeltaTime);
    
    // Update species-specific logic
    UpdateSpeciesLogic(DeltaTime);
    
    // Update threat assessment
    UpdateThreatAssessment();
    
    // Cleanup old memories
    CleanupMemory();
}

void UNPC_DinosaurBehaviorManager::SetBehaviorState(ENPC_DinosaurBehaviorType NewBehavior)
{
    if (BehaviorState.CurrentBehavior != NewBehavior)
    {
        BehaviorState.CurrentBehavior = NewBehavior;
        BehaviorState.LastBehaviorChangeTime = GetWorld()->GetTimeSeconds();
        
        // Update blackboard if available
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewBehavior));
        }
    }
}

ENPC_DinosaurBehaviorType UNPC_DinosaurBehaviorManager::GetCurrentBehavior() const
{
    return BehaviorState.CurrentBehavior;
}

void UNPC_DinosaurBehaviorManager::UpdateBehaviorLogic(float DeltaTime)
{
    switch (BehaviorState.CurrentBehavior)
    {
        case ENPC_DinosaurBehaviorType::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurBehaviorType::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurBehaviorType::Chase:
            UpdateChaseBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurBehaviorType::Attack:
            UpdateAttackBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurBehaviorType::Flee:
            UpdateFleeBehavior(DeltaTime);
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorManager::SetPatrolArea(FVector Center, float Radius)
{
    BehaviorState.PatrolCenter = Center;
    BehaviorState.PatrolRadius = Radius;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolCenter"), Center);
        BlackboardComponent->SetValueAsFloat(TEXT("PatrolRadius"), Radius);
    }
}

FVector UNPC_DinosaurBehaviorManager::GetNextPatrolPoint()
{
    return GetRandomPatrolPoint();
}

bool UNPC_DinosaurBehaviorManager::IsInPatrolArea(FVector Location) const
{
    float Distance = FVector::Dist(Location, BehaviorState.PatrolCenter);
    return Distance <= BehaviorState.PatrolRadius;
}

void UNPC_DinosaurBehaviorManager::SetTarget(AActor* NewTarget)
{
    if (BehaviorState.TargetActor != NewTarget)
    {
        BehaviorState.TargetActor = NewTarget;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        }
        
        if (NewTarget)
        {
            RememberActor(NewTarget);
            BehaviorState.LastKnownPlayerLocation = NewTarget->GetActorLocation();
        }
    }
}

AActor* UNPC_DinosaurBehaviorManager::GetCurrentTarget() const
{
    return BehaviorState.TargetActor;
}

void UNPC_DinosaurBehaviorManager::UpdateThreatAssessment()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Find player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
    {
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerCharacter->GetActorLocation());
    
    // Check if player is in sight range
    if (DistanceToPlayer <= SightRange && CanSeeActor(PlayerCharacter))
    {
        Memory.LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
        Memory.LastPlayerLocation = PlayerCharacter->GetActorLocation();
        
        // Determine behavior based on distance and species
        if (DistanceToPlayer <= AttackRange && BehaviorState.CurrentBehavior != ENPC_DinosaurBehaviorType::Attack)
        {
            SetTarget(PlayerCharacter);
            SetBehaviorState(ENPC_DinosaurBehaviorType::Attack);
        }
        else if (DistanceToPlayer <= ChaseRange && BehaviorState.CurrentBehavior != ENPC_DinosaurBehaviorType::Chase)
        {
            SetTarget(PlayerCharacter);
            SetBehaviorState(ENPC_DinosaurBehaviorType::Chase);
        }
    }
    else if (BehaviorState.CurrentBehavior == ENPC_DinosaurBehaviorType::Chase || 
             BehaviorState.CurrentBehavior == ENPC_DinosaurBehaviorType::Attack)
    {
        // Lost sight of player, return to patrol
        float TimeSinceLastSighting = GetWorld()->GetTimeSeconds() - Memory.LastPlayerSightingTime;
        if (TimeSinceLastSighting > 5.0f)
        {
            SetTarget(nullptr);
            SetBehaviorState(ENPC_DinosaurBehaviorType::Patrol);
        }
    }
}

bool UNPC_DinosaurBehaviorManager::CanSeeActor(AActor* Actor) const
{
    if (!GetOwner() || !Actor)
    {
        return false;
    }
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = Actor->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Actor);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstruction
}

void UNPC_DinosaurBehaviorManager::RememberActor(AActor* Actor)
{
    if (Actor && !Memory.SeenActors.Contains(Actor))
    {
        Memory.SeenActors.Add(Actor);
    }
}

void UNPC_DinosaurBehaviorManager::RememberLocation(FVector Location)
{
    Memory.InterestingLocations.Add(Location);
}

void UNPC_DinosaurBehaviorManager::ForgetActor(AActor* Actor)
{
    Memory.SeenActors.Remove(Actor);
}

bool UNPC_DinosaurBehaviorManager::HasSeenActor(AActor* Actor) const
{
    return Memory.SeenActors.Contains(Actor);
}

void UNPC_DinosaurBehaviorManager::EnterCombatMode()
{
    BehaviorState.bIsInCombat = true;
    BehaviorState.AggressionLevel = 1.0f;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), true);
    }
}

void UNPC_DinosaurBehaviorManager::ExitCombatMode()
{
    BehaviorState.bIsInCombat = false;
    BehaviorState.AggressionLevel = 0.0f;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), false);
    }
}

bool UNPC_DinosaurBehaviorManager::IsInCombat() const
{
    return BehaviorState.bIsInCombat;
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesBehavior(ENPC_DinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            SightRange = 4000.0f;
            AttackRange = 400.0f;
            ChaseRange = 6000.0f;
            PatrolSpeed = 150.0f;
            ChaseSpeed = 800.0f;
            BehaviorState.PatrolRadius = 8000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            SightRange = 3000.0f;
            AttackRange = 200.0f;
            ChaseRange = 4000.0f;
            PatrolSpeed = 300.0f;
            ChaseSpeed = 1000.0f;
            BehaviorState.PatrolRadius = 5000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            SightRange = 2000.0f;
            AttackRange = 300.0f;
            ChaseRange = 3000.0f;
            PatrolSpeed = 100.0f;
            ChaseSpeed = 400.0f;
            BehaviorState.PatrolRadius = 4000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            SightRange = 1500.0f;
            AttackRange = 500.0f;
            ChaseRange = 2000.0f;
            PatrolSpeed = 80.0f;
            ChaseSpeed = 200.0f;
            BehaviorState.PatrolRadius = 6000.0f;
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateSpeciesLogic(float DeltaTime)
{
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            UpdateTRexBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            UpdateVelociraptorBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            UpdateTriceratopsBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            UpdateBrachiosaurusBehavior(DeltaTime);
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateIdleBehavior(float DeltaTime)
{
    float TimeSinceLastChange = GetWorld()->GetTimeSeconds() - BehaviorState.LastBehaviorChangeTime;
    
    if (TimeSinceLastChange > BehaviorChangeInterval)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorType::Patrol);
    }
}

void UNPC_DinosaurBehaviorManager::UpdatePatrolBehavior(float DeltaTime)
{
    // Continue patrolling until target is found
    if (BlackboardComponent)
    {
        FVector NextPatrolPoint = GetNextPatrolPoint();
        BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), NextPatrolPoint);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateChaseBehavior(float DeltaTime)
{
    if (BehaviorState.TargetActor)
    {
        float Distance = GetDistanceToTarget();
        
        if (Distance <= AttackRange)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorType::Attack);
        }
        else if (Distance > ChaseRange)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorType::Patrol);
        }
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), BehaviorState.TargetActor->GetActorLocation());
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateAttackBehavior(float DeltaTime)
{
    if (BehaviorState.TargetActor)
    {
        float Distance = GetDistanceToTarget();
        
        if (Distance > AttackRange)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorType::Chase);
        }
    }
    else
    {
        SetBehaviorState(ENPC_DinosaurBehaviorType::Patrol);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateFleeBehavior(float DeltaTime)
{
    // Flee behavior - move away from threat
    if (BehaviorState.TargetActor)
    {
        FVector FleeDirection = GetOwner()->GetActorLocation() - BehaviorState.TargetActor->GetActorLocation();
        FleeDirection.Normalize();
        FVector FleeLocation = GetOwner()->GetActorLocation() + FleeDirection * 2000.0f;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("MoveToLocation"), FleeLocation);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateTRexBehavior(float DeltaTime)
{
    // T-Rex specific behavior - aggressive hunter
    BehaviorState.AggressionLevel = FMath::Min(BehaviorState.AggressionLevel + DeltaTime * 0.1f, 1.0f);
}

void UNPC_DinosaurBehaviorManager::UpdateVelociraptorBehavior(float DeltaTime)
{
    // Velociraptor specific behavior - pack hunter, quick and agile
    BehaviorState.AggressionLevel = FMath::Min(BehaviorState.AggressionLevel + DeltaTime * 0.2f, 1.0f);
}

void UNPC_DinosaurBehaviorManager::UpdateTriceratopsBehavior(float DeltaTime)
{
    // Triceratops specific behavior - defensive herbivore
    if (BehaviorState.TargetActor)
    {
        BehaviorState.AggressionLevel = FMath::Min(BehaviorState.AggressionLevel + DeltaTime * 0.05f, 0.6f);
    }
    else
    {
        BehaviorState.AggressionLevel = FMath::Max(BehaviorState.AggressionLevel - DeltaTime * 0.1f, 0.0f);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBrachiosaurusBehavior(float DeltaTime)
{
    // Brachiosaurus specific behavior - peaceful herbivore, rarely aggressive
    BehaviorState.AggressionLevel = FMath::Max(BehaviorState.AggressionLevel - DeltaTime * 0.05f, 0.0f);
}

float UNPC_DinosaurBehaviorManager::GetDistanceToTarget() const
{
    if (!GetOwner() || !BehaviorState.TargetActor)
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), BehaviorState.TargetActor->GetActorLocation());
}

FVector UNPC_DinosaurBehaviorManager::GetRandomPatrolPoint() const
{
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    );
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(BehaviorState.PatrolRadius * 0.3f, BehaviorState.PatrolRadius);
    
    return BehaviorState.PatrolCenter + RandomDirection * RandomDistance;
}

bool UNPC_DinosaurBehaviorManager::ShouldChangeTarget(AActor* NewTarget) const
{
    if (!BehaviorState.TargetActor)
    {
        return true;
    }
    
    if (!NewTarget)
    {
        return false;
    }
    
    // Prefer closer targets
    float CurrentDistance = GetDistanceToTarget();
    float NewDistance = FVector::Dist(GetOwner()->GetActorLocation(), NewTarget->GetActorLocation());
    
    return NewDistance < CurrentDistance * 0.8f;
}

void UNPC_DinosaurBehaviorManager::CleanupMemory()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old actor memories
    Memory.SeenActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    // Limit memory size
    if (Memory.SeenActors.Num() > 20)
    {
        Memory.SeenActors.RemoveAt(0, Memory.SeenActors.Num() - 20);
    }
    
    if (Memory.InterestingLocations.Num() > 10)
    {
        Memory.InterestingLocations.RemoveAt(0, Memory.InterestingLocations.Num() - 10);
    }
}