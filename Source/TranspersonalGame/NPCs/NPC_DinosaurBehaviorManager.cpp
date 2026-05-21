#include "NPC_DinosaurBehaviorManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    DayNightCycleTime = 0.0f;
    bIsNocturnal = false;
    SleepStartTime = 22.0f; // 10 PM
    WakeUpTime = 6.0f;      // 6 AM
    
    // Pack behavior defaults
    bIsPackLeader = false;
    PackLeader = nullptr;
    PackCohesionRadius = 1000.0f;
    
    // Internal state
    StateTimer = 0.0f;
    LastThreatCheckTime = 0.0f;
    HungerDecayRate = 1.0f; // Hunger increases by 1 per minute
    CurrentTarget = nullptr;
    PatrolDestination = FVector::ZeroVector;
    bHasPatrolDestination = false;
    
    // Initialize stats based on species
    InitializeSpeciesStats();
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set home territory to current location
    if (AActor* Owner = GetOwner())
    {
        SetHomeTerritory(Owner->GetActorLocation(), DinosaurMemory.TerritoryRadius);
    }
    
    // Initialize behavior tree if available
    if (BehaviorTree)
    {
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
            {
                AIController->RunBehaviorTree(BehaviorTree);
                BlackboardComponent = AIController->GetBlackboardComponent();
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorManager initialized for %s"), 
           *UEnum::GetValueAsString(DinosaurSpecies));
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    
    // Update hunger system
    UpdateHunger(DeltaTime);
    
    // Update daily routine based on time of day
    UpdateDailyRoutine(DayNightCycleTime);
    
    // Detect threats periodically
    if (StateTimer - LastThreatCheckTime > 2.0f) // Check every 2 seconds
    {
        DetectThreats();
        LastThreatCheckTime = StateTimer;
    }
    
    // Process species-specific behavior
    ProcessSpeciesBehavior(DeltaTime);
    
    // Update blackboard with current state
    UpdateBlackboard();
}

void UNPC_DinosaurBehaviorManager::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        ENPC_DinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s: State changed from %s to %s"), 
               *GetOwner()->GetName(),
               *UEnum::GetValueAsString(PreviousState),
               *UEnum::GetValueAsString(NewState));
        
        // Notify pack members if this is a pack leader
        if (bIsPackLeader && (NewState == ENPC_DinosaurBehaviorState::Alert || 
                             NewState == ENPC_DinosaurBehaviorState::Hunting))
        {
            CommunicateWithPack(NewState);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateDailyRoutine(float CurrentTimeOfDay)
{
    DayNightCycleTime = CurrentTimeOfDay;
    
    // Determine if it's sleep time
    bool bShouldSleep = false;
    if (bIsNocturnal)
    {
        // Nocturnal animals sleep during day
        bShouldSleep = (CurrentTimeOfDay > WakeUpTime && CurrentTimeOfDay < SleepStartTime);
    }
    else
    {
        // Diurnal animals sleep at night
        bShouldSleep = (CurrentTimeOfDay > SleepStartTime || CurrentTimeOfDay < WakeUpTime);
    }
    
    // Only change to sleeping if not in combat or alert state
    if (bShouldSleep && CurrentBehaviorState != ENPC_DinosaurBehaviorState::Alert && 
        CurrentBehaviorState != ENPC_DinosaurBehaviorState::Attacking &&
        CurrentBehaviorState != ENPC_DinosaurBehaviorState::Fleeing)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Sleeping);
    }
    else if (!bShouldSleep && CurrentBehaviorState == ENPC_DinosaurBehaviorState::Sleeping)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
    }
}

void UNPC_DinosaurBehaviorManager::DetectThreats()
{
    if (!GetOwner()) return;
    
    AActor* Player = FindNearestPlayer();
    if (Player)
    {
        float DistanceToPlayer = CalculateDistanceToPlayer();
        
        if (DistanceToPlayer <= DinosaurStats.DetectionRange)
        {
            // Update memory with player location
            DinosaurMemory.LastKnownPlayerLocation = Player->GetActorLocation();
            DinosaurMemory.TimeSinceLastPlayerSighting = 0.0f;
            
            // Calculate threat level based on distance and species aggression
            float ThreatLevel = FMath::Clamp((DinosaurStats.DetectionRange - DistanceToPlayer) / DinosaurStats.DetectionRange, 0.0f, 1.0f);
            ThreatLevel *= (DinosaurStats.Aggression / 100.0f);
            
            UpdateMemory(Player, ThreatLevel);
            
            // React based on species and threat level
            if (ThreatLevel > 0.7f && DinosaurStats.Aggression > 50.0f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
                CurrentTarget = Player;
            }
            else if (ThreatLevel > 0.3f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Alert);
            }
        }
    }
    else
    {
        // No player in range, increment time since last sighting
        DinosaurMemory.TimeSinceLastPlayerSighting += 2.0f; // Check interval
        
        // Return to normal behavior if player hasn't been seen for a while
        if (DinosaurMemory.TimeSinceLastPlayerSighting > 30.0f && 
            (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Alert || 
             CurrentBehaviorState == ENPC_DinosaurBehaviorState::Hunting))
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            CurrentTarget = nullptr;
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateMemory(AActor* PerceivedActor, float ThreatLevel)
{
    DinosaurMemory.ThreatLevel = ThreatLevel;
    
    // Add to known threats if threat level is significant
    if (ThreatLevel > 0.5f && !DinosaurMemory.KnownThreats.Contains(PerceivedActor))
    {
        DinosaurMemory.KnownThreats.Add(PerceivedActor);
    }
}

bool UNPC_DinosaurBehaviorManager::IsPlayerInRange(float Range)
{
    return CalculateDistanceToPlayer() <= Range;
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesStats()
{
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            DinosaurStats.MaxHealth = 500.0f;
            DinosaurStats.Health = 500.0f;
            DinosaurStats.Aggression = 85.0f;
            DinosaurStats.Speed = 800.0f;
            DinosaurStats.AttackDamage = 100.0f;
            DinosaurStats.DetectionRange = 4000.0f;
            DinosaurStats.AttackRange = 500.0f;
            DinosaurMemory.TerritoryRadius = 8000.0f;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            DinosaurStats.MaxHealth = 150.0f;
            DinosaurStats.Health = 150.0f;
            DinosaurStats.Aggression = 75.0f;
            DinosaurStats.Speed = 1200.0f;
            DinosaurStats.AttackDamage = 40.0f;
            DinosaurStats.DetectionRange = 3500.0f;
            DinosaurStats.AttackRange = 250.0f;
            DinosaurMemory.TerritoryRadius = 5000.0f;
            bIsNocturnal = false;
            PackCohesionRadius = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            DinosaurStats.MaxHealth = 400.0f;
            DinosaurStats.Health = 400.0f;
            DinosaurStats.Aggression = 30.0f;
            DinosaurStats.Speed = 600.0f;
            DinosaurStats.AttackDamage = 60.0f;
            DinosaurStats.DetectionRange = 2500.0f;
            DinosaurStats.AttackRange = 400.0f;
            DinosaurMemory.TerritoryRadius = 6000.0f;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.MaxHealth = 800.0f;
            DinosaurStats.Health = 800.0f;
            DinosaurStats.Aggression = 10.0f;
            DinosaurStats.Speed = 400.0f;
            DinosaurStats.AttackDamage = 80.0f;
            DinosaurStats.DetectionRange = 2000.0f;
            DinosaurStats.AttackRange = 600.0f;
            DinosaurMemory.TerritoryRadius = 10000.0f;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Ankylosaurus:
            DinosaurStats.MaxHealth = 350.0f;
            DinosaurStats.Health = 350.0f;
            DinosaurStats.Aggression = 20.0f;
            DinosaurStats.Speed = 300.0f;
            DinosaurStats.AttackDamage = 70.0f;
            DinosaurStats.DetectionRange = 2000.0f;
            DinosaurStats.AttackRange = 350.0f;
            DinosaurMemory.TerritoryRadius = 4000.0f;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Parasaurolophus:
            DinosaurStats.MaxHealth = 200.0f;
            DinosaurStats.Health = 200.0f;
            DinosaurStats.Aggression = 15.0f;
            DinosaurStats.Speed = 700.0f;
            DinosaurStats.AttackDamage = 30.0f;
            DinosaurStats.DetectionRange = 3000.0f;
            DinosaurStats.AttackRange = 200.0f;
            DinosaurMemory.TerritoryRadius = 7000.0f;
            bIsNocturnal = false;
            break;
    }
}

void UNPC_DinosaurBehaviorManager::JoinPack(UNPC_DinosaurBehaviorManager* Leader)
{
    if (Leader && Leader != this && DinosaurSpecies == ENPC_DinosaurSpecies::Velociraptor)
    {
        PackLeader = Leader;
        bIsPackLeader = false;
        Leader->PackMembers.AddUnique(this);
        DinosaurMemory.PackMembers.AddUnique(Leader->GetOwner());
        
        UE_LOG(LogTemp, Log, TEXT("Velociraptor %s joined pack led by %s"), 
               *GetOwner()->GetName(), *Leader->GetOwner()->GetName());
    }
}

void UNPC_DinosaurBehaviorManager::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    // If this was a pack leader, disband the pack
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
    
    DinosaurMemory.PackMembers.Empty();
}

void UNPC_DinosaurBehaviorManager::CommunicateWithPack(ENPC_DinosaurBehaviorState AlertState)
{
    for (UNPC_DinosaurBehaviorManager* Member : PackMembers)
    {
        if (Member && Member->CurrentBehaviorState != ENPC_DinosaurBehaviorState::Sleeping)
        {
            Member->SetBehaviorState(AlertState);
            if (CurrentTarget)
            {
                Member->CurrentTarget = CurrentTarget;
                Member->DinosaurMemory.LastKnownPlayerLocation = DinosaurMemory.LastKnownPlayerLocation;
            }
        }
    }
}

FVector UNPC_DinosaurBehaviorManager::GetPackCenterLocation()
{
    if (PackMembers.Num() == 0) return GetOwner()->GetActorLocation();
    
    FVector CenterLocation = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (UNPC_DinosaurBehaviorManager* Member : PackMembers)
    {
        if (Member && Member->GetOwner())
        {
            CenterLocation += Member->GetOwner()->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        CenterLocation /= ValidMembers;
    }
    
    return CenterLocation;
}

void UNPC_DinosaurBehaviorManager::SetHomeTerritory(FVector Location, float Radius)
{
    DinosaurMemory.HomeTerritory = Location;
    DinosaurMemory.TerritoryRadius = Radius;
}

bool UNPC_DinosaurBehaviorManager::IsInHomeTerritory()
{
    if (!GetOwner()) return false;
    
    float DistanceFromHome = FVector::Dist(GetOwner()->GetActorLocation(), DinosaurMemory.HomeTerritory);
    return DistanceFromHome <= DinosaurMemory.TerritoryRadius;
}

FVector UNPC_DinosaurBehaviorManager::GetRandomPatrolPoint()
{
    // Generate random point within territory
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(500.0f, DinosaurMemory.TerritoryRadius * 0.8f);
    FVector PatrolPoint = DinosaurMemory.HomeTerritory + (RandomDirection * RandomDistance);
    
    return PatrolPoint;
}

void UNPC_DinosaurBehaviorManager::AttackTarget(AActor* Target)
{
    if (!Target || !GetOwner()) return;
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    if (DistanceToTarget <= DinosaurStats.AttackRange)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Attacking);
        
        // Apply damage to target if it's the player
        if (APawn* PlayerPawn = Cast<APawn>(Target))
        {
            // Damage will be handled by combat system
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacking player for %f damage"), 
                   *GetOwner()->GetName(), DinosaurStats.AttackDamage);
        }
    }
}

void UNPC_DinosaurBehaviorManager::TakeDamage(float Damage, AActor* DamageSource)
{
    DinosaurStats.Health = FMath::Clamp(DinosaurStats.Health - Damage, 0.0f, DinosaurStats.MaxHealth);
    
    if (DinosaurStats.Health <= 0.0f)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s is fleeing due to low health"), *GetOwner()->GetName());
    }
    else if (DamageSource)
    {
        // Become aggressive toward damage source
        CurrentTarget = DamageSource;
        SetBehaviorState(ENPC_DinosaurBehaviorState::Alert);
        UpdateMemory(DamageSource, 1.0f);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateHunger(float DeltaTime)
{
    DinosaurStats.Hunger = FMath::Clamp(DinosaurStats.Hunger + (HungerDecayRate * DeltaTime / 60.0f), 0.0f, 100.0f);
    
    // If very hungry, prioritize finding food
    if (DinosaurStats.Hunger > 80.0f && CurrentBehaviorState != ENPC_DinosaurBehaviorState::Attacking)
    {
        FindFood();
    }
}

void UNPC_DinosaurBehaviorManager::FindFood()
{
    SetBehaviorState(ENPC_DinosaurBehaviorState::Feeding);
    
    // For carnivores, food might be other dinosaurs or carcasses
    // For herbivores, food is vegetation
    bool bIsCarnivore = (DinosaurSpecies == ENPC_DinosaurSpecies::TRex || 
                        DinosaurSpecies == ENPC_DinosaurSpecies::Velociraptor);
    
    if (bIsCarnivore)
    {
        // Look for prey or carcasses
        UE_LOG(LogTemp, Log, TEXT("Carnivore %s searching for prey"), *GetOwner()->GetName());
    }
    else
    {
        // Look for vegetation
        UE_LOG(LogTemp, Log, TEXT("Herbivore %s searching for vegetation"), *GetOwner()->GetName());
        
        // Reduce hunger when feeding
        DinosaurStats.Hunger = FMath::Clamp(DinosaurStats.Hunger - 20.0f, 0.0f, 100.0f);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateBlackboard()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), DinosaurStats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), DinosaurStats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), DinosaurStats.Aggression);
        BlackboardComponent->SetValueAsVector(TEXT("HomeTerritory"), DinosaurMemory.HomeTerritory);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
        }
        
        if (bHasPatrolDestination)
        {
            BlackboardComponent->SetValueAsVector(TEXT("PatrolDestination"), PatrolDestination);
        }
    }
}

void UNPC_DinosaurBehaviorManager::ProcessSpeciesBehavior(float DeltaTime)
{
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            HandleTRexBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            HandleVelociraptorBehavior(DeltaTime);
            break;
            
        default:
            HandleHerbivoreBehavior(DeltaTime);
            break;
    }
}

AActor* UNPC_DinosaurBehaviorManager::FindNearestPlayer()
{
    if (UWorld* World = GetWorld())
    {
        return UGameplayStatics::GetPlayerPawn(World, 0);
    }
    return nullptr;
}

float UNPC_DinosaurBehaviorManager::CalculateDistanceToPlayer()
{
    AActor* Player = FindNearestPlayer();
    if (Player && GetOwner())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    return FLT_MAX;
}

void UNPC_DinosaurBehaviorManager::HandleTRexBehavior(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            if (StateTimer > 5.0f) // Idle for 5 seconds
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            }
            break;
            
        case ENPC_DinosaurBehaviorState::Patrolling:
            if (!bHasPatrolDestination || StateTimer > 30.0f)
            {
                PatrolDestination = GetRandomPatrolPoint();
                bHasPatrolDestination = true;
                StateTimer = 0.0f;
            }
            break;
            
        case ENPC_DinosaurBehaviorState::Hunting:
            if (CurrentTarget && IsPlayerInRange(DinosaurStats.AttackRange))
            {
                AttackTarget(CurrentTarget);
            }
            break;
    }
}

void UNPC_DinosaurBehaviorManager::HandleVelociraptorBehavior(float DeltaTime)
{
    // Pack coordination behavior
    if (PackLeader && !bIsPackLeader)
    {
        // Follow pack leader's behavior
        if (PackLeader->CurrentBehaviorState == ENPC_DinosaurBehaviorState::Hunting)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
            CurrentTarget = PackLeader->CurrentTarget;
        }
    }
    
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            if (StateTimer > 3.0f) // More active than T-Rex
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            }
            break;
            
        case ENPC_DinosaurBehaviorState::Patrolling:
            if (!bHasPatrolDestination || StateTimer > 20.0f)
            {
                if (bIsPackLeader)
                {
                    PatrolDestination = GetRandomPatrolPoint();
                }
                else if (PackLeader)
                {
                    // Stay near pack center
                    FVector PackCenter = GetPackCenterLocation();
                    FVector RandomOffset = FMath::VRand() * PackCohesionRadius;
                    RandomOffset.Z = 0.0f;
                    PatrolDestination = PackCenter + RandomOffset;
                }
                else
                {
                    PatrolDestination = GetRandomPatrolPoint();
                }
                bHasPatrolDestination = true;
                StateTimer = 0.0f;
            }
            break;
    }
}

void UNPC_DinosaurBehaviorManager::HandleHerbivoreBehavior(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            if (StateTimer > 8.0f) // Herbivores are more leisurely
            {
                if (DinosaurStats.Hunger > 60.0f)
                {
                    SetBehaviorState(ENPC_DinosaurBehaviorState::Feeding);
                }
                else
                {
                    SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
                }
            }
            break;
            
        case ENPC_DinosaurBehaviorState::Feeding:
            if (StateTimer > 10.0f) // Feed for 10 seconds
            {
                DinosaurStats.Hunger = FMath::Clamp(DinosaurStats.Hunger - 30.0f, 0.0f, 100.0f);
                SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
            }
            break;
            
        case ENPC_DinosaurBehaviorState::Alert:
            // Herbivores are more likely to flee than fight
            if (CurrentTarget && IsPlayerInRange(DinosaurStats.DetectionRange * 0.5f))
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);
            }
            else if (StateTimer > 15.0f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
            }
            break;
    }
}