#include "NPC_DinosaurBehaviorManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Math/UnrealMathUtility.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize default values
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    CurrentState = ENPC_DinosaurState::Idle;
    DetectionRange = 3000.0f;
    AttackRange = 300.0f;
    FleeRange = 1500.0f;
    MovementSpeed = 400.0f;
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 50.0f;
    Aggression = 30.0f;

    // Pack behavior defaults
    bIsPackHunter = false;
    PackData = FNPC_PackBehaviorData();

    // Territory defaults
    bIsTerritorial = true;
    TerritoryData = FNPC_TerritoryData();

    // AI components
    BehaviorTreeAsset = nullptr;
    DinosaurAIController = nullptr;
    BlackboardComponent = nullptr;

    // Perception setup
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    if (PawnSensingComponent)
    {
        PawnSensingComponent->SightRadius = DetectionRange;
        PawnSensingComponent->HearingThreshold = 1200.0f;
        PawnSensingComponent->LOSHearingThreshold = 1500.0f;
        PawnSensingComponent->bOnlySensePlayers = false;
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &UNPC_DinosaurBehaviorManager::OnSeePawn);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &UNPC_DinosaurBehaviorManager::OnHearNoise);
    }

    // Internal state
    CurrentTarget = nullptr;
    LastSeenTargetTime = 0.0f;
    StateChangeTime = 0.0f;
    LastKnownTargetLocation = FVector::ZeroVector;
    IdleTimer = 0.0f;
    HungerTimer = 0.0f;
    bInitialized = false;
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: BeginPlay started"));
    
    // Initialize AI controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        DinosaurAIController = Cast<AAIController>(OwnerPawn->GetController());
        if (DinosaurAIController && DinosaurAIController->GetBlackboardComponent())
        {
            BlackboardComponent = DinosaurAIController->GetBlackboardComponent();
        }
    }

    // Initialize behavior based on species
    InitializeDinosaurBehavior();
    
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Initialized for species %d"), (int32)DinosaurSpecies);
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bInitialized)
    {
        return;
    }

    UpdateBehaviorTick(DeltaTime);
    UpdateHunger(DeltaTime);
    UpdateAggression(DeltaTime);

    // Update pack behavior if pack hunter
    if (bIsPackHunter)
    {
        UpdatePackBehavior();
    }

    // Update territory behavior if territorial
    if (bIsTerritorial)
    {
        PatrolTerritory();
    }
}

void UNPC_DinosaurBehaviorManager::InitializeDinosaurBehavior()
{
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Initializing behavior for species %d"), (int32)DinosaurSpecies);

    // Setup species-specific behaviors
    switch (DinosaurSpecies)
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
        case ENPC_DinosaurSpecies::Pachycephalo:
        case ENPC_DinosaurSpecies::Protoceratops:
        case ENPC_DinosaurSpecies::Tsintaosaurus:
            SetupHerbivoreBehavior();
            break;
        default:
            SetupTRexBehavior();
            break;
    }

    // Setup territory if territorial
    if (bIsTerritorial && GetOwner())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        SetupTerritory(OwnerLocation, TerritoryData.TerritoryRadius);
    }

    bInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Initialization complete"));
}

void UNPC_DinosaurBehaviorManager::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorManager: State change from %d to %d"), (int32)CurrentState, (int32)NewState);
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();

        // Update blackboard if available
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), (uint8)CurrentState);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBehaviorTick(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            HandleIdleBehavior();
            break;
        case ENPC_DinosaurState::Patrolling:
            HandlePatrolBehavior();
            break;
        case ENPC_DinosaurState::Hunting:
            HandleHuntingBehavior();
            break;
        case ENPC_DinosaurState::Chasing:
            HandleChasingBehavior();
            break;
        case ENPC_DinosaurState::Attacking:
            HandleAttackingBehavior();
            break;
        case ENPC_DinosaurState::Fleeing:
            HandleFleeingBehavior();
            break;
        case ENPC_DinosaurState::Feeding:
            // Handle feeding behavior
            break;
        case ENPC_DinosaurState::Sleeping:
            // Handle sleeping behavior
            break;
        case ENPC_DinosaurState::Territorial:
            // Handle territorial defense
            break;
    }
}

bool UNPC_DinosaurBehaviorManager::CanSeeTarget(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return false;
    }

    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        TargetLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit;
}

float UNPC_DinosaurBehaviorManager::GetDistanceToTarget(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return -1.0f;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

void UNPC_DinosaurBehaviorManager::SetupTRexBehavior()
{
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Setting up T-Rex behavior"));
    
    // T-Rex is a solitary apex predator
    bIsPackHunter = false;
    bIsTerritorial = true;
    
    DetectionRange = 4000.0f;
    AttackRange = 500.0f;
    MovementSpeed = 600.0f;
    MaxHealth = 300.0f;
    Health = MaxHealth;
    Aggression = 80.0f;
    
    TerritoryData.TerritoryRadius = 8000.0f;
    
    if (PawnSensingComponent)
    {
        PawnSensingComponent->SightRadius = DetectionRange;
    }
    
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

void UNPC_DinosaurBehaviorManager::SetupVelociraptorBehavior()
{
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Setting up Velociraptor behavior"));
    
    // Velociraptors are pack hunters
    bIsPackHunter = true;
    bIsTerritorial = false;
    
    DetectionRange = 2500.0f;
    AttackRange = 200.0f;
    MovementSpeed = 800.0f;
    MaxHealth = 80.0f;
    Health = MaxHealth;
    Aggression = 90.0f;
    
    PackData.PackRadius = 1500.0f;
    
    if (PawnSensingComponent)
    {
        PawnSensingComponent->SightRadius = DetectionRange;
    }
    
    SetDinosaurState(ENPC_DinosaurState::Hunting);
}

void UNPC_DinosaurBehaviorManager::SetupHerbivoreBehavior()
{
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Setting up Herbivore behavior"));
    
    // Herbivores are generally peaceful but can defend themselves
    bIsPackHunter = false;
    bIsTerritorial = false;
    
    DetectionRange = 2000.0f;
    AttackRange = 300.0f;
    FleeRange = 2500.0f;
    MovementSpeed = 300.0f;
    MaxHealth = 150.0f;
    Health = MaxHealth;
    Aggression = 20.0f;
    
    if (PawnSensingComponent)
    {
        PawnSensingComponent->SightRadius = DetectionRange;
    }
    
    SetDinosaurState(ENPC_DinosaurState::Feeding);
}

void UNPC_DinosaurBehaviorManager::OnSeePawn(APawn* Pawn)
{
    if (!Pawn || Pawn == GetOwner())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorManager: Saw pawn %s"), *Pawn->GetName());

    // Check if it's a player
    if (Pawn->IsA<ACharacter>() && Pawn->IsPlayerControlled())
    {
        CurrentTarget = Pawn;
        LastSeenTargetTime = GetWorld()->GetTimeSeconds();
        LastKnownTargetLocation = Pawn->GetActorLocation();

        // React based on species and current state
        float DistanceToPlayer = GetDistanceToTarget(Pawn);
        
        if (DinosaurSpecies == ENPC_DinosaurSpecies::TRex || DinosaurSpecies == ENPC_DinosaurSpecies::Velociraptor)
        {
            if (DistanceToPlayer <= AttackRange)
            {
                SetDinosaurState(ENPC_DinosaurState::Attacking);
            }
            else if (DistanceToPlayer <= DetectionRange)
            {
                SetDinosaurState(ENPC_DinosaurState::Chasing);
            }
        }
        else // Herbivores
        {
            if (DistanceToPlayer <= FleeRange)
            {
                SetDinosaurState(ENPC_DinosaurState::Fleeing);
            }
        }

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
            BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
        }
    }
}

void UNPC_DinosaurBehaviorManager::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (!NoiseInstigator || NoiseInstigator == GetOwner())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorManager: Heard noise from %s"), *NoiseInstigator->GetName());

    // React to noise based on species
    if (Volume > 0.5f && (DinosaurSpecies == ENPC_DinosaurSpecies::TRex || DinosaurSpecies == ENPC_DinosaurSpecies::Velociraptor))
    {
        LastKnownTargetLocation = Location;
        if (CurrentState == ENPC_DinosaurState::Idle || CurrentState == ENPC_DinosaurState::Patrolling)
        {
            SetDinosaurState(ENPC_DinosaurState::Hunting);
        }
    }
}

void UNPC_DinosaurBehaviorManager::HandleIdleBehavior()
{
    IdleTimer += GetWorld()->GetDeltaSeconds();
    
    // After some idle time, start patrolling or hunting
    if (IdleTimer > FMath::RandRange(3.0f, 8.0f))
    {
        IdleTimer = 0.0f;
        
        if (bIsTerritorial)
        {
            SetDinosaurState(ENPC_DinosaurState::Patrolling);
        }
        else if (bIsPackHunter)
        {
            SetDinosaurState(ENPC_DinosaurState::Hunting);
        }
        else
        {
            SetDinosaurState(ENPC_DinosaurState::Feeding);
        }
    }
}

void UNPC_DinosaurBehaviorManager::HandlePatrolBehavior()
{
    if (bIsTerritorial && TerritoryData.PatrolPoints.Num() > 0)
    {
        // Move towards current patrol point
        FVector CurrentPatrolPoint = TerritoryData.PatrolPoints[TerritoryData.CurrentPatrolIndex];
        MoveToLocation(CurrentPatrolPoint);
        
        // Check if reached patrol point
        if (GetOwner() && FVector::Dist(GetOwner()->GetActorLocation(), CurrentPatrolPoint) < 200.0f)
        {
            TerritoryData.CurrentPatrolIndex = (TerritoryData.CurrentPatrolIndex + 1) % TerritoryData.PatrolPoints.Num();
        }
    }
}

void UNPC_DinosaurBehaviorManager::HandleHuntingBehavior()
{
    // Look for prey or investigate last known location
    if (CurrentTarget && CanSeeTarget(CurrentTarget))
    {
        SetDinosaurState(ENPC_DinosaurState::Chasing);
    }
    else if (LastKnownTargetLocation != FVector::ZeroVector)
    {
        MoveToLocation(LastKnownTargetLocation);
        
        // If reached last known location and no target found, return to patrolling
        if (GetOwner() && FVector::Dist(GetOwner()->GetActorLocation(), LastKnownTargetLocation) < 300.0f)
        {
            LastKnownTargetLocation = FVector::ZeroVector;
            SetDinosaurState(ENPC_DinosaurState::Patrolling);
        }
    }
}

void UNPC_DinosaurBehaviorManager::HandleChasingBehavior()
{
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
        
        if (DistanceToTarget <= AttackRange)
        {
            SetDinosaurState(ENPC_DinosaurState::Attacking);
        }
        else if (DistanceToTarget > DetectionRange * 1.5f)
        {
            // Lost target, return to hunting
            SetDinosaurState(ENPC_DinosaurState::Hunting);
        }
        else
        {
            // Continue chasing
            MoveToLocation(CurrentTarget->GetActorLocation());
            LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        }
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Hunting);
    }
}

void UNPC_DinosaurBehaviorManager::HandleAttackingBehavior()
{
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
        
        if (DistanceToTarget > AttackRange * 1.2f)
        {
            // Target moved away, chase again
            SetDinosaurState(ENPC_DinosaurState::Chasing);
        }
        else
        {
            // Perform attack (this would trigger animation and damage)
            UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Attacking target!"));
            
            // After attack, brief pause then continue chasing
            GetWorld()->GetTimerManager().SetTimer(
                FTimerHandle(),
                [this]() { SetDinosaurState(ENPC_DinosaurState::Chasing); },
                2.0f,
                false
            );
        }
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Hunting);
    }
}

void UNPC_DinosaurBehaviorManager::HandleFleeingBehavior()
{
    if (CurrentTarget)
    {
        // Move away from threat
        FVector FleeDirection = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector FleeLocation = GetOwner()->GetActorLocation() + (FleeDirection * 2000.0f);
        MoveToLocation(FleeLocation);
        
        // Stop fleeing after some time or distance
        float DistanceToThreat = GetDistanceToTarget(CurrentTarget);
        if (DistanceToThreat > FleeRange * 2.0f)
        {
            CurrentTarget = nullptr;
            SetDinosaurState(ENPC_DinosaurState::Feeding);
        }
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Feeding);
    }
}

AActor* UNPC_DinosaurBehaviorManager::FindNearestPlayer()
{
    if (!GetWorld())
    {
        return nullptr;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        return PlayerController->GetPawn();
    }
    
    return nullptr;
}

void UNPC_DinosaurBehaviorManager::MoveToLocation(FVector TargetLocation)
{
    if (DinosaurAIController && GetOwner())
    {
        DinosaurAIController->MoveToLocation(TargetLocation, 100.0f);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateHunger(float DeltaTime)
{
    HungerTimer += DeltaTime;
    
    // Increase hunger over time
    if (HungerTimer > 60.0f) // Every minute
    {
        Hunger = FMath::Clamp(Hunger + 5.0f, 0.0f, 100.0f);
        HungerTimer = 0.0f;
        
        // High hunger increases aggression
        if (Hunger > 80.0f)
        {
            Aggression = FMath::Clamp(Aggression + 10.0f, 0.0f, 100.0f);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateAggression(float DeltaTime)
{
    // Aggression slowly decreases over time when not in combat
    if (CurrentState != ENPC_DinosaurState::Attacking && CurrentState != ENPC_DinosaurState::Chasing)
    {
        Aggression = FMath::Clamp(Aggression - (DeltaTime * 2.0f), 0.0f, 100.0f);
    }
}

void UNPC_DinosaurBehaviorManager::SetupTerritory(FVector Center, float Radius)
{
    TerritoryData.TerritoryCenter = Center;
    TerritoryData.TerritoryRadius = Radius;
    
    // Generate patrol points around the territory
    TerritoryData.PatrolPoints.Empty();
    int32 NumPatrolPoints = 6;
    
    for (int32 i = 0; i < NumPatrolPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumPatrolPoints;
        float PatrolRadius = Radius * 0.7f; // Patrol inside territory boundary
        
        FVector PatrolPoint = Center + FVector(
            FMath::Cos(Angle) * PatrolRadius,
            FMath::Sin(Angle) * PatrolRadius,
            0.0f
        );
        
        TerritoryData.PatrolPoints.Add(PatrolPoint);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorManager: Territory setup with %d patrol points"), TerritoryData.PatrolPoints.Num());
}

void UNPC_DinosaurBehaviorManager::PatrolTerritory()
{
    if (!bIsTerritorial || TerritoryData.PatrolPoints.Num() == 0)
    {
        return;
    }
    
    // This is handled in HandlePatrolBehavior when state is Patrolling
}

bool UNPC_DinosaurBehaviorManager::IsInTerritory(FVector Location)
{
    if (!bIsTerritorial)
    {
        return true;
    }
    
    float DistanceFromCenter = FVector::Dist(Location, TerritoryData.TerritoryCenter);
    return DistanceFromCenter <= TerritoryData.TerritoryRadius;
}

// Pack behavior functions (stubs for now)
void UNPC_DinosaurBehaviorManager::JoinPack(UNPC_DinosaurBehaviorManager* PackLeaderComponent) {}
void UNPC_DinosaurBehaviorManager::LeavePack() {}
void UNPC_DinosaurBehaviorManager::UpdatePackBehavior() {}
void UNPC_DinosaurBehaviorManager::CoordinatePackHunt(AActor* Target) {}
void UNPC_DinosaurBehaviorManager::DefendTerritory(AActor* Intruder) {}
AActor* UNPC_DinosaurBehaviorManager::FindNearestPrey() { return nullptr; }
bool UNPC_DinosaurBehaviorManager::IsPlayerInRange(float Range) { return false; }