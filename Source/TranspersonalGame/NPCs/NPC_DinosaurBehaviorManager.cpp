#include "NPC_DinosaurBehaviorManager.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight perception
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 1600.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Initialize default stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Hunger = 50.0f;
    Stats.Stamina = 100.0f;
    Stats.Aggression = 30.0f;
    Stats.Fear = 0.0f;
    Stats.TerritorialRadius = 2000.0f;
    Stats.DetectionRange = 1500.0f;
    Stats.AttackRange = 300.0f;
    Stats.MovementSpeed = 400.0f;

    // Initialize memory
    Memory.PatrolPoints.Empty();
    Memory.LastKnownPlayerLocation = FVector::ZeroVector;
    Memory.LastPlayerSightTime = 0.0f;
    Memory.HomeLocation = FVector::ZeroVector;
    Memory.KnownThreats.Empty();
    Memory.KnownFood.Empty();
    Memory.LastFeedTime = 0.0f;
    Memory.LastRestTime = 0.0f;

    // Initialize pack behavior
    PackMembers.Empty();
    bIsPackLeader = false;
    PackLeader = nullptr;

    // Initialize time
    CurrentTimeOfDay = 0.0f;
    DayDuration = 1200.0f; // 20 minutes per day

    // Initialize state
    CurrentState = ENPC_DinosaurState::Idle;
    Species = ENPC_DinosaurSpecies::TRex;

    // Internal variables
    StateTimer = 0.0f;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;
    AIControllerRef = nullptr;
    PlayerPawn = nullptr;
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAI();
    InitializeSpeciesTraits();
    
    // Set home location to current location
    Memory.HomeLocation = GetOwner()->GetActorLocation();
    
    // Generate initial patrol points
    for (int32 i = 0; i < 5; i++)
    {
        FVector PatrolPoint = GetRandomPatrolPoint();
        Memory.PatrolPoints.Add(PatrolPoint);
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Behavior Manager initialized for %s"), *GetOwner()->GetName());
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized)
        return;

    // Update time tracking
    LastUpdateTime += DeltaTime;
    StateTimer += DeltaTime;
    UpdateTimeOfDay(DeltaTime);

    // Core updates
    UpdateStats(DeltaTime);
    UpdatePerception(DeltaTime);
    ProcessDailyRoutine(DeltaTime);
    ProcessSpeciesSpecificBehavior(DeltaTime);
    HandlePackCommunication();
    UpdateBlackboard();

    // State management
    if (LastUpdateTime >= 0.5f) // Update every 0.5 seconds
    {
        HandleStateTransition();
        LastUpdateTime = 0.0f;
    }
}

void UNPC_DinosaurBehaviorManager::SetState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurState PreviousState = CurrentState;
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("%s changed state from %d to %d"), 
               *GetOwner()->GetName(), (int32)PreviousState, (int32)NewState);
        
        // Handle state entry logic
        switch (NewState)
        {
            case ENPC_DinosaurState::Patrolling:
                PatrolArea();
                break;
            case ENPC_DinosaurState::Hunting:
                PlayDinosaurSound("Roar");
                break;
            case ENPC_DinosaurState::Fleeing:
                PlayDinosaurSound("Fear");
                break;
            case ENPC_DinosaurState::Sleeping:
                ReturnToHome();
                break;
            default:
                break;
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateStats(float DeltaTime)
{
    // Update hunger over time
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (DeltaTime * 2.0f));
    
    // Update stamina regeneration
    if (CurrentState == ENPC_DinosaurState::Idle || CurrentState == ENPC_DinosaurState::Sleeping)
    {
        Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + (DeltaTime * 10.0f));
    }
    else
    {
        Stats.Stamina = FMath::Max(0.0f, Stats.Stamina - (DeltaTime * 5.0f));
    }
    
    // Update fear decay
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - (DeltaTime * 3.0f));
    
    // Health regeneration when well-fed and rested
    if (Stats.Hunger > 70.0f && Stats.Stamina > 80.0f && Stats.Health < Stats.MaxHealth)
    {
        Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + (DeltaTime * 1.0f));
    }
}

void UNPC_DinosaurBehaviorManager::ProcessDailyRoutine(float DeltaTime)
{
    float TimeOfDayNormalized = CurrentTimeOfDay / DayDuration;
    
    // Morning (0.0 - 0.3): Active hunting/feeding
    if (TimeOfDayNormalized >= 0.0f && TimeOfDayNormalized < 0.3f)
    {
        if (Stats.Hunger < 40.0f && CurrentState != ENPC_DinosaurState::Hunting)
        {
            SetState(ENPC_DinosaurState::Hunting);
        }
    }
    // Midday (0.3 - 0.7): Patrolling/territorial
    else if (TimeOfDayNormalized >= 0.3f && TimeOfDayNormalized < 0.7f)
    {
        if (CurrentState == ENPC_DinosaurState::Idle)
        {
            SetState(ENPC_DinosaurState::Patrolling);
        }
    }
    // Evening (0.7 - 0.9): Return to territory
    else if (TimeOfDayNormalized >= 0.7f && TimeOfDayNormalized < 0.9f)
    {
        if (FVector::Dist(GetOwner()->GetActorLocation(), Memory.HomeLocation) > Stats.TerritorialRadius)
        {
            ReturnToHome();
        }
    }
    // Night (0.9 - 1.0): Sleep/rest
    else
    {
        if (CurrentState != ENPC_DinosaurState::Sleeping && Stats.Stamina < 30.0f)
        {
            SetState(ENPC_DinosaurState::Sleeping);
        }
    }
}

bool UNPC_DinosaurBehaviorManager::CanSeePlayer()
{
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
    
    if (!PlayerPawn)
        return false;
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    
    if (DistanceToPlayer > Stats.DetectionRange)
        return false;
    
    // Line of sight check
    FHitResult HitResult;
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = PlayerPawn->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility
    );
    
    return !bHit || HitResult.GetActor() == PlayerPawn;
}

AActor* UNPC_DinosaurBehaviorManager::FindNearestThreat()
{
    AActor* NearestThreat = nullptr;
    float NearestDistance = Stats.DetectionRange;
    
    for (AActor* Threat : Memory.KnownThreats)
    {
        if (!IsValid(Threat))
            continue;
            
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Threat->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestThreat = Threat;
        }
    }
    
    return NearestThreat;
}

AActor* UNPC_DinosaurBehaviorManager::FindNearestFood()
{
    AActor* NearestFood = nullptr;
    float NearestDistance = Stats.DetectionRange;
    
    for (AActor* Food : Memory.KnownFood)
    {
        if (!IsValid(Food))
            continue;
            
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Food->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestFood = Food;
        }
    }
    
    return NearestFood;
}

void UNPC_DinosaurBehaviorManager::UpdateMemory(AActor* Actor, bool bIsThreat)
{
    if (!IsValid(Actor))
        return;
    
    if (bIsThreat)
    {
        Memory.KnownThreats.AddUnique(Actor);
        if (Actor == PlayerPawn)
        {
            Memory.LastKnownPlayerLocation = Actor->GetActorLocation();
            Memory.LastPlayerSightTime = GetWorld()->GetTimeSeconds();
        }
    }
    else
    {
        Memory.KnownFood.AddUnique(Actor);
    }
}

void UNPC_DinosaurBehaviorManager::MoveToLocation(FVector TargetLocation)
{
    if (!AIControllerRef)
        return;
    
    // Use AI controller to move to location
    AIControllerRef->MoveToLocation(TargetLocation, Stats.AttackRange);
}

void UNPC_DinosaurBehaviorManager::PatrolArea()
{
    if (Memory.PatrolPoints.Num() == 0)
        return;
    
    // Find nearest patrol point
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector NearestPatrolPoint = Memory.PatrolPoints[0];
    float NearestDistance = FVector::Dist(CurrentLocation, NearestPatrolPoint);
    
    for (const FVector& PatrolPoint : Memory.PatrolPoints)
    {
        float Distance = FVector::Dist(CurrentLocation, PatrolPoint);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestPatrolPoint = PatrolPoint;
        }
    }
    
    MoveToLocation(NearestPatrolPoint);
}

void UNPC_DinosaurBehaviorManager::ReturnToHome()
{
    MoveToLocation(Memory.HomeLocation);
}

void UNPC_DinosaurBehaviorManager::AttackTarget(AActor* Target)
{
    if (!IsValid(Target) || !IsInAttackRange(Target))
        return;
    
    // Apply damage to target
    if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
    {
        float Damage = 25.0f; // Base damage
        
        // Species-specific damage modifiers
        switch (Species)
        {
            case ENPC_DinosaurSpecies::TRex:
                Damage *= 3.0f;
                break;
            case ENPC_DinosaurSpecies::Velociraptor:
                Damage *= 1.5f;
                break;
            case ENPC_DinosaurSpecies::Triceratops:
                Damage *= 2.0f;
                break;
            default:
                break;
        }
        
        // Apply damage (would need damage system implementation)
        UE_LOG(LogTemp, Warning, TEXT("%s attacks %s for %f damage"), 
               *GetOwner()->GetName(), *Target->GetName(), Damage);
        
        PlayDinosaurSound("Attack");
    }
}

void UNPC_DinosaurBehaviorManager::FleeFromThreat(AActor* Threat)
{
    if (!IsValid(Threat))
        return;
    
    FVector ThreatLocation = Threat->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector FleeDirection = (CurrentLocation - ThreatLocation).GetSafeNormal();
    FVector FleeLocation = CurrentLocation + (FleeDirection * 2000.0f);
    
    MoveToLocation(FleeLocation);
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + 20.0f);
}

bool UNPC_DinosaurBehaviorManager::IsInAttackRange(AActor* Target)
{
    if (!IsValid(Target))
        return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= Stats.AttackRange;
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesTraits()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.MaxHealth = 300.0f;
            Stats.Health = 300.0f;
            Stats.Aggression = 80.0f;
            Stats.AttackRange = 400.0f;
            Stats.MovementSpeed = 600.0f;
            Stats.DetectionRange = 2000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            Stats.MaxHealth = 80.0f;
            Stats.Health = 80.0f;
            Stats.Aggression = 90.0f;
            Stats.AttackRange = 200.0f;
            Stats.MovementSpeed = 800.0f;
            Stats.DetectionRange = 1800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            Stats.MaxHealth = 250.0f;
            Stats.Health = 250.0f;
            Stats.Aggression = 40.0f;
            Stats.AttackRange = 300.0f;
            Stats.MovementSpeed = 400.0f;
            Stats.DetectionRange = 1200.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.MaxHealth = 500.0f;
            Stats.Health = 500.0f;
            Stats.Aggression = 10.0f;
            Stats.AttackRange = 500.0f;
            Stats.MovementSpeed = 200.0f;
            Stats.DetectionRange = 1000.0f;
            break;
            
        default:
            // Default stats already set in constructor
            break;
    }
}

void UNPC_DinosaurBehaviorManager::ApplySpeciesBehavior()
{
    // Species-specific behavior logic
    switch (Species)
    {
        case ENPC_DinosaurSpecies::Velociraptor:
            // Pack hunting behavior
            if (PackMembers.Num() > 0 && CanSeePlayer())
            {
                CoordinatePackHunt(PlayerPawn);
            }
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            // Peaceful herbivore - flee from threats
            if (AActor* Threat = FindNearestThreat())
            {
                SetState(ENPC_DinosaurState::Fleeing);
                FleeFromThreat(Threat);
            }
            break;
            
        case ENPC_DinosaurSpecies::TRex:
            // Apex predator - aggressive territorial behavior
            if (Stats.Aggression > 60.0f && CanSeePlayer())
            {
                SetState(ENPC_DinosaurState::Hunting);
            }
            break;
            
        default:
            break;
    }
}

float UNPC_DinosaurBehaviorManager::GetDistanceToPlayer()
{
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
    
    if (!PlayerPawn)
        return -1.0f;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
}

bool UNPC_DinosaurBehaviorManager::IsNightTime()
{
    float TimeOfDayNormalized = CurrentTimeOfDay / DayDuration;
    return TimeOfDayNormalized >= 0.9f || TimeOfDayNormalized < 0.1f;
}

FVector UNPC_DinosaurBehaviorManager::GetRandomPatrolPoint()
{
    FVector BaseLocation = Memory.HomeLocation;
    float Radius = Stats.TerritorialRadius * 0.8f; // Stay within 80% of territorial radius
    
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(Radius * 0.3f, Radius);
    
    FVector RandomPoint = BaseLocation + FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return RandomPoint;
}

void UNPC_DinosaurBehaviorManager::PlayDinosaurSound(const FString& SoundType)
{
    // Log sound for now - would integrate with audio system
    UE_LOG(LogTemp, Log, TEXT("%s plays %s sound"), *GetOwner()->GetName(), *SoundType);
}

void UNPC_DinosaurBehaviorManager::JoinPack(UNPC_DinosaurBehaviorManager* Leader)
{
    if (!Leader || Leader == this)
        return;
    
    PackLeader = Leader;
    Leader->PackMembers.AddUnique(this);
    bIsPackLeader = false;
    
    UE_LOG(LogTemp, Log, TEXT("%s joined pack led by %s"), 
           *GetOwner()->GetName(), *Leader->GetOwner()->GetName());
}

void UNPC_DinosaurBehaviorManager::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    // Disband pack if leader leaves
    if (bIsPackLeader)
    {
        for (UNPC_DinosaurBehaviorManager* Member : PackMembers)
        {
            if (Member)
            {
                Member->PackLeader = nullptr;
            }
        }
        PackMembers.Empty();
        bIsPackLeader = false;
    }
}

void UNPC_DinosaurBehaviorManager::CoordinatePackHunt(AActor* Target)
{
    if (!bIsPackLeader || !IsValid(Target))
        return;
    
    // Coordinate pack attack
    for (UNPC_DinosaurBehaviorManager* Member : PackMembers)
    {
        if (Member && Member->CurrentState != ENPC_DinosaurState::Hunting)
        {
            Member->SetState(ENPC_DinosaurState::Hunting);
            Member->UpdateMemory(Target, true);
        }
    }
}

void UNPC_DinosaurBehaviorManager::InitializeAI()
{
    AIControllerRef = Cast<AAIController>(GetOwner()->GetInstigatorController());
    if (!AIControllerRef)
    {
        // Try to get AI controller from pawn
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            AIControllerRef = Cast<AAIController>(OwnerPawn->GetController());
        }
    }
    
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void UNPC_DinosaurBehaviorManager::UpdatePerception(float DeltaTime)
{
    if (!AIPerceptionComponent)
        return;
    
    // Check for player visibility
    if (CanSeePlayer())
    {
        UpdateMemory(PlayerPawn, true);
        
        // React based on species and stats
        float PlayerDistance = GetDistanceToPlayer();
        if (PlayerDistance < Stats.AttackRange && Stats.Aggression > 50.0f)
        {
            SetState(ENPC_DinosaurState::Hunting);
        }
        else if (Stats.Fear > 30.0f)
        {
            SetState(ENPC_DinosaurState::Fleeing);
        }
    }
}

void UNPC_DinosaurBehaviorManager::HandleStateTransition()
{
    // State transition logic based on current conditions
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            if (Stats.Hunger < 30.0f)
            {
                SetState(ENPC_DinosaurState::Hunting);
            }
            else if (StateTimer > 10.0f) // Idle for 10 seconds
            {
                SetState(ENPC_DinosaurState::Patrolling);
            }
            break;
            
        case ENPC_DinosaurState::Patrolling:
            if (CanSeePlayer() && Stats.Aggression > 40.0f)
            {
                SetState(ENPC_DinosaurState::Hunting);
            }
            else if (StateTimer > 30.0f) // Patrol for 30 seconds
            {
                SetState(ENPC_DinosaurState::Idle);
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (Stats.Fear > 50.0f)
            {
                SetState(ENPC_DinosaurState::Fleeing);
            }
            else if (!CanSeePlayer() && StateTimer > 15.0f)
            {
                SetState(ENPC_DinosaurState::Patrolling);
            }
            break;
            
        case ENPC_DinosaurState::Fleeing:
            if (Stats.Fear < 20.0f && StateTimer > 10.0f)
            {
                SetState(ENPC_DinosaurState::Idle);
            }
            break;
            
        case ENPC_DinosaurState::Sleeping:
            if (Stats.Stamina > 80.0f || !IsNightTime())
            {
                SetState(ENPC_DinosaurState::Idle);
            }
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;
    
    // Update blackboard values for behavior tree
    BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
    BlackboardComponent->SetValueAsFloat(TEXT("Health"), Stats.Health);
    BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), Stats.Hunger);
    BlackboardComponent->SetValueAsFloat(TEXT("Fear"), Stats.Fear);
    BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), Memory.HomeLocation);
    
    if (PlayerPawn)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("PlayerDistance"), GetDistanceToPlayer());
    }
}

void UNPC_DinosaurBehaviorManager::ProcessSpeciesSpecificBehavior(float DeltaTime)
{
    ApplySpeciesBehavior();
}

void UNPC_DinosaurBehaviorManager::HandlePackCommunication()
{
    // Pack communication logic
    if (Species == ENPC_DinosaurSpecies::Velociraptor && PackMembers.Num() > 0)
    {
        // Share threat information with pack
        if (Memory.KnownThreats.Num() > 0)
        {
            for (UNPC_DinosaurBehaviorManager* Member : PackMembers)
            {
                if (Member)
                {
                    for (AActor* Threat : Memory.KnownThreats)
                    {
                        Member->UpdateMemory(Threat, true);
                    }
                }
            }
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateTimeOfDay(float DeltaTime)
{
    CurrentTimeOfDay += DeltaTime;
    if (CurrentTimeOfDay >= DayDuration)
    {
        CurrentTimeOfDay = 0.0f;
    }
}