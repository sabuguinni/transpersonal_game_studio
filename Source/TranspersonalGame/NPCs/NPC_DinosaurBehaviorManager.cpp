#include "NPC_DinosaurBehaviorManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

UNPC_DinosaurBehaviorComponent::UNPC_DinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default stats based on species
    Stats.Health = 100.0f;
    Stats.Hunger = 50.0f;
    Stats.Aggression = 30.0f;
    Stats.Fear = 0.0f;
    Stats.Stamina = 100.0f;
    Stats.TerritorialRadius = 5000.0f;
}

void UNPC_DinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set species-specific stats
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.Health = 200.0f;
            Stats.Aggression = 80.0f;
            DetectionRadius = 4000.0f;
            AttackRange = 800.0f;
            MovementSpeed = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            Stats.Health = 60.0f;
            Stats.Aggression = 90.0f;
            DetectionRadius = 3500.0f;
            AttackRange = 400.0f;
            MovementSpeed = 1200.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            Stats.Health = 150.0f;
            Stats.Aggression = 40.0f;
            DetectionRadius = 2500.0f;
            AttackRange = 600.0f;
            MovementSpeed = 500.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.Health = 300.0f;
            Stats.Aggression = 10.0f;
            DetectionRadius = 2000.0f;
            AttackRange = 1000.0f;
            MovementSpeed = 300.0f;
            break;
    }
    
    // Initialize home territory
    Memory.HomeTerritory = GetOwner()->GetActorLocation();
    
    // Start behavior update timer
    GetWorld()->GetTimerManager().SetTimer(BehaviorUpdateTimer, this, 
        &UNPC_DinosaurBehaviorComponent::UpdateBehavior, 1.0f, true);
    
    // Start with patrol state
    SetBehaviorState(ENPC_DinosaurState::Patrolling);
}

void UNPC_DinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateStats(DeltaTime);
    
    // Update memory timers
    Memory.TimeSincePlayerSeen += DeltaTime;
}

void UNPC_DinosaurBehaviorComponent::SetBehaviorState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Log state change
        FString StateName = UEnum::GetValueAsString(NewState);
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s changed state to %s"), 
            *GetOwner()->GetName(), *StateName);
    }
}

bool UNPC_DinosaurBehaviorComponent::CanSeePlayer()
{
    AActor* Player = FindNearestPlayer();
    if (!Player) return false;
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    
    if (DistanceToPlayer <= DetectionRadius)
    {
        // Simple line of sight check
        FHitResult HitResult;
        FVector Start = GetOwner()->GetActorLocation();
        FVector End = Player->GetActorLocation();
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
        
        if (!bHit || HitResult.GetActor() == Player)
        {
            Memory.LastKnownPlayerLocation = Player->GetActorLocation();
            Memory.TimeSincePlayerSeen = 0.0f;
            return true;
        }
    }
    
    return false;
}

void UNPC_DinosaurBehaviorComponent::StartHunting(AActor* Target)
{
    CurrentTarget = Target;
    SetBehaviorState(ENPC_DinosaurState::Hunting);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s started hunting %s"), 
        *GetOwner()->GetName(), Target ? *Target->GetName() : TEXT("Unknown"));
}

void UNPC_DinosaurBehaviorComponent::StartPatrolling()
{
    SetBehaviorState(ENPC_DinosaurState::Patrolling);
    
    // Generate patrol points around home territory
    if (Memory.PatrolPoints.Num() == 0)
    {
        FVector HomeLocation = Memory.HomeTerritory;
        float PatrolRadius = Stats.TerritorialRadius * 0.5f;
        
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) * PI / 180.0f;
            FVector PatrolPoint = HomeLocation + FVector(
                FMath::Cos(Angle) * PatrolRadius,
                FMath::Sin(Angle) * PatrolRadius,
                0.0f
            );
            Memory.PatrolPoints.Add(PatrolPoint);
        }
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateStats(float DeltaTime)
{
    // Hunger increases over time
    Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + DeltaTime * 0.5f);
    
    // Stamina regeneration
    if (CurrentState != ENPC_DinosaurState::Hunting)
    {
        Stats.Stamina = FMath::Min(100.0f, Stats.Stamina + DeltaTime * 10.0f);
    }
    
    // Fear decay
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - DeltaTime * 5.0f);
}

void UNPC_DinosaurBehaviorComponent::UpdateBehavior()
{
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            ProcessIdleState();
            break;
            
        case ENPC_DinosaurState::Patrolling:
            ProcessPatrolState();
            break;
            
        case ENPC_DinosaurState::Hunting:
            ProcessHuntingState();
            break;
            
        case ENPC_DinosaurState::Fleeing:
            ProcessFleeingState();
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::ProcessIdleState()
{
    // Check for player
    if (CanSeePlayer() && Stats.Aggression > 50.0f)
    {
        StartHunting(FindNearestPlayer());
        return;
    }
    
    // Random chance to start patrolling
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        StartPatrolling();
    }
}

void UNPC_DinosaurBehaviorComponent::ProcessPatrolState()
{
    // Check for player
    if (CanSeePlayer() && Stats.Aggression > 30.0f)
    {
        StartHunting(FindNearestPlayer());
        return;
    }
    
    // Move to next patrol point
    if (Memory.PatrolPoints.Num() > 0)
    {
        static int32 CurrentPatrolIndex = 0;
        FVector TargetPoint = Memory.PatrolPoints[CurrentPatrolIndex];
        
        float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), TargetPoint);
        
        if (DistanceToTarget < 500.0f)
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % Memory.PatrolPoints.Num();
        }
        else
        {
            MoveToLocation(TargetPoint);
        }
    }
}

void UNPC_DinosaurBehaviorComponent::ProcessHuntingState()
{
    if (!CurrentTarget)
    {
        SetBehaviorState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // Attack if in range
    if (DistanceToTarget <= AttackRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacking target!"), *GetOwner()->GetName());
        
        // Apply damage logic here
        Stats.Stamina -= 20.0f;
        
        if (Stats.Stamina <= 0.0f)
        {
            SetBehaviorState(ENPC_DinosaurState::Idle);
        }
    }
    else if (DistanceToTarget <= DetectionRadius * 2.0f)
    {
        // Chase target
        MoveToLocation(CurrentTarget->GetActorLocation());
    }
    else
    {
        // Lost target
        CurrentTarget = nullptr;
        SetBehaviorState(ENPC_DinosaurState::Patrolling);
    }
}

void UNPC_DinosaurBehaviorComponent::ProcessFleeingState()
{
    // Move away from threats
    if (Memory.KnownThreats.Num() > 0)
    {
        FVector FleeDirection = FVector::ZeroVector;
        
        for (AActor* Threat : Memory.KnownThreats)
        {
            if (Threat)
            {
                FVector DirectionFromThreat = GetOwner()->GetActorLocation() - Threat->GetActorLocation();
                DirectionFromThreat.Normalize();
                FleeDirection += DirectionFromThreat;
            }
        }
        
        FleeDirection.Normalize();
        FVector FleeLocation = GetOwner()->GetActorLocation() + FleeDirection * 2000.0f;
        MoveToLocation(FleeLocation);
    }
    
    // Return to normal behavior after some time
    if (Stats.Fear < 20.0f)
    {
        SetBehaviorState(ENPC_DinosaurState::Patrolling);
    }
}

AActor* UNPC_DinosaurBehaviorComponent::FindNearestPlayer()
{
    return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void UNPC_DinosaurBehaviorComponent::MoveToLocation(FVector TargetLocation)
{
    // Simple movement - in a real implementation, this would use AI movement components
    FVector Direction = (TargetLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetOwner()->GetActorLocation() + Direction * MovementSpeed * GetWorld()->GetDeltaSeconds();
    
    GetOwner()->SetActorLocation(NewLocation);
}

// Behavior Manager Implementation
ANPC_DinosaurBehaviorManager::ANPC_DinosaurBehaviorManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f;
}

void ANPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all dinosaurs in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("DinosaurAI"), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        RegisterDinosaur(Actor);
    }
    
    // Start global behavior update timer
    GetWorld()->GetTimerManager().SetTimer(GlobalUpdateTimer, this, 
        &ANPC_DinosaurBehaviorManager::UpdateGlobalBehavior, 5.0f, true);
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorManager initialized with %d dinosaurs"), 
        ManagedDinosaurs.Num());
}

void ANPC_DinosaurBehaviorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update dinosaur interactions
    UpdateDinosaurInteractions();
}

void ANPC_DinosaurBehaviorManager::RegisterDinosaur(AActor* DinosaurActor)
{
    if (DinosaurActor && !ManagedDinosaurs.Contains(DinosaurActor))
    {
        ManagedDinosaurs.Add(DinosaurActor);
        
        UE_LOG(LogTemp, Warning, TEXT("Registered dinosaur: %s"), *DinosaurActor->GetName());
    }
}

void ANPC_DinosaurBehaviorManager::UnregisterDinosaur(AActor* DinosaurActor)
{
    ManagedDinosaurs.Remove(DinosaurActor);
}

void ANPC_DinosaurBehaviorManager::UpdateGlobalBehavior()
{
    // Clean up null references
    ManagedDinosaurs.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    // Process pack behavior
    if (bEnablePackBehavior)
    {
        ProcessPackBehavior();
    }
    
    // Process territorial behavior
    ProcessTerritorialBehavior();
}

TArray<AActor*> ANPC_DinosaurBehaviorManager::GetDinosaursInRadius(FVector Center, float Radius)
{
    TArray<AActor*> NearbyDinosaurs;
    
    for (AActor* Dinosaur : ManagedDinosaurs)
    {
        if (Dinosaur)
        {
            float Distance = FVector::Dist(Dinosaur->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                NearbyDinosaurs.Add(Dinosaur);
            }
        }
    }
    
    return NearbyDinosaurs;
}

void ANPC_DinosaurBehaviorManager::TriggerTerritorialResponse(FVector Location, float Radius)
{
    TArray<AActor*> AffectedDinosaurs = GetDinosaursInRadius(Location, Radius);
    
    for (AActor* Dinosaur : AffectedDinosaurs)
    {
        UNPC_DinosaurBehaviorComponent* BehaviorComp = Dinosaur->FindComponentByClass<UNPC_DinosaurBehaviorComponent>();
        if (BehaviorComp)
        {
            BehaviorComp->Stats.Aggression += 20.0f;
            BehaviorComp->SetBehaviorState(ENPC_DinosaurState::Territorial);
        }
    }
}

void ANPC_DinosaurBehaviorManager::ProcessPackBehavior()
{
    // Group raptors for pack hunting
    TArray<AActor*> Raptors;
    
    for (AActor* Dinosaur : ManagedDinosaurs)
    {
        UNPC_DinosaurBehaviorComponent* BehaviorComp = Dinosaur->FindComponentByClass<UNPC_DinosaurBehaviorComponent>();
        if (BehaviorComp && BehaviorComp->Species == ENPC_DinosaurSpecies::Velociraptor)
        {
            Raptors.Add(Dinosaur);
        }
    }
    
    // Coordinate raptor pack behavior
    if (Raptors.Num() >= 2)
    {
        for (AActor* Raptor : Raptors)
        {
            UNPC_DinosaurBehaviorComponent* BehaviorComp = Raptor->FindComponentByClass<UNPC_DinosaurBehaviorComponent>();
            if (BehaviorComp)
            {
                BehaviorComp->Stats.Aggression += 10.0f; // Pack bonus
            }
        }
    }
}

void ANPC_DinosaurBehaviorManager::ProcessTerritorialBehavior()
{
    // Check for territorial conflicts between dinosaurs
    for (int32 i = 0; i < ManagedDinosaurs.Num(); i++)
    {
        for (int32 j = i + 1; j < ManagedDinosaurs.Num(); j++)
        {
            AActor* DinoA = ManagedDinosaurs[i];
            AActor* DinoB = ManagedDinosaurs[j];
            
            if (!DinoA || !DinoB) continue;
            
            float Distance = FVector::Dist(DinoA->GetActorLocation(), DinoB->GetActorLocation());
            
            UNPC_DinosaurBehaviorComponent* BehaviorA = DinoA->FindComponentByClass<UNPC_DinosaurBehaviorComponent>();
            UNPC_DinosaurBehaviorComponent* BehaviorB = DinoB->FindComponentByClass<UNPC_DinosaurBehaviorComponent>();
            
            if (BehaviorA && BehaviorB)
            {
                float TerritorialDistance = FMath::Min(BehaviorA->Stats.TerritorialRadius, BehaviorB->Stats.TerritorialRadius);
                
                if (Distance < TerritorialDistance)
                {
                    // Territorial conflict
                    BehaviorA->Stats.Aggression += 5.0f;
                    BehaviorB->Stats.Aggression += 5.0f;
                }
            }
        }
    }
}

void ANPC_DinosaurBehaviorManager::UpdateDinosaurInteractions()
{
    // Update interactions between dinosaurs and environment
    for (AActor* Dinosaur : ManagedDinosaurs)
    {
        if (!Dinosaur) continue;
        
        UNPC_DinosaurBehaviorComponent* BehaviorComp = Dinosaur->FindComponentByClass<UNPC_DinosaurBehaviorComponent>();
        if (BehaviorComp)
        {
            // Apply global aggression multiplier
            BehaviorComp->Stats.Aggression *= GlobalAggressionMultiplier;
            BehaviorComp->Stats.Aggression = FMath::Clamp(BehaviorComp->Stats.Aggression, 0.0f, 100.0f);
        }
    }
}