#include "NPC_DinosaurBehaviorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorComponent::UNPC_DinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default stats
    Stats.Health = 100.0f;
    Stats.MaxHealth = 100.0f;
    Stats.Hunger = 50.0f;
    Stats.Thirst = 50.0f;
    Stats.Stamina = 100.0f;
    Stats.Fear = 0.0f;
    Stats.Aggression = 30.0f;
    Stats.TerritorialRadius = 2000.0f;
    
    // Initialize default behavior settings
    BehaviorSettings.PatrolRadius = 1500.0f;
    BehaviorSettings.DetectionRange = 1200.0f;
    BehaviorSettings.AttackRange = 300.0f;
    BehaviorSettings.FleeRange = 800.0f;
    BehaviorSettings.WalkSpeed = 150.0f;
    BehaviorSettings.RunSpeed = 400.0f;
    BehaviorSettings.AttackDamage = 25.0f;
    BehaviorSettings.bIsPackHunter = false;
    BehaviorSettings.PackSize = 3;
}

void UNPC_DinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set patrol center to current location
    PatrolCenter = GetOwner()->GetActorLocation();
    CurrentDestination = PatrolCenter;
    
    // Initialize species-specific behavior
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            InitializeTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Raptor:
            InitializeRaptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
        case ENPC_DinosaurSpecies::Brachiosaurus:
        case ENPC_DinosaurSpecies::Stegosaurus:
        case ENPC_DinosaurSpecies::Ankylosaurus:
        case ENPC_DinosaurSpecies::Parasaurolophus:
            InitializeHerbivoreBehavior();
            break;
        default:
            break;
    }
    
    // Start with idle behavior
    SetBehaviorState(ENPC_DinosaurState::Idle);
}

void UNPC_DinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
    
    // Update behavior state timer
    StateTimer += DeltaTime;
    
    // Update stats (hunger, thirst, etc.)
    UpdateStats(DeltaTime);
    
    // Execute current behavior
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_DinosaurState::Hunting:
            UpdateHuntingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurState::Feeding:
            UpdateFeedingBehavior(DeltaTime);
            break;
        default:
            break;
    }
    
    // Scan for targets and threats periodically
    if (FMath::Fmod(StateTimer, 1.0f) < 0.1f) // Every second
    {
        ScanForTargets();
        ScanForThreats();
    }
}

void UNPC_DinosaurBehaviorComponent::SetDinosaurSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    
    // Reinitialize behavior for new species
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            InitializeTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Raptor:
            InitializeRaptorBehavior();
            break;
        default:
            InitializeHerbivoreBehavior();
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::SetBehaviorState(ENPC_DinosaurState NewState)
{
    if (CurrentState == NewState)
        return;
    
    // Exit current state
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Hunting:
            CurrentTarget = nullptr;
            break;
        default:
            break;
    }
    
    // Enter new state
    CurrentState = NewState;
    StateTimer = 0.0f;
    
    switch (NewState)
    {
        case ENPC_DinosaurState::Patrolling:
            CurrentDestination = GetRandomPatrolPoint();
            break;
        case ENPC_DinosaurState::Idle:
            CurrentDestination = GetOwner()->GetActorLocation();
            break;
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::StartPatrolling()
{
    SetBehaviorState(ENPC_DinosaurState::Patrolling);
}

void UNPC_DinosaurBehaviorComponent::StartHunting(AActor* Target)
{
    if (!Target)
        return;
    
    CurrentTarget = Target;
    SetBehaviorState(ENPC_DinosaurState::Hunting);
    PlayRoarSound();
}

void UNPC_DinosaurBehaviorComponent::StartFleeing(AActor* ThreatSource)
{
    if (!ThreatSource)
        return;
    
    // Calculate flee direction (away from threat)
    FVector FleeDirection = GetOwner()->GetActorLocation() - ThreatSource->GetActorLocation();
    FleeDirection.Normalize();
    
    CurrentDestination = GetOwner()->GetActorLocation() + (FleeDirection * BehaviorSettings.FleeRange);
    SetBehaviorState(ENPC_DinosaurState::Fleeing);
}

void UNPC_DinosaurBehaviorComponent::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);
    Stats.Fear = FMath::Min(100.0f, Stats.Fear + (DamageAmount * 2.0f));
    
    PlayHurtSound();
    
    if (Stats.Health <= 0.0f)
    {
        // Dinosaur died
        SetBehaviorState(ENPC_DinosaurState::Idle);
        GetOwner()->SetActorHiddenInGame(true);
        SetComponentTickEnabled(false);
        return;
    }
    
    // Decide whether to flee or fight based on fear and aggression
    float FleeThreshold = (Stats.Fear * 2.0f) - Stats.Aggression;
    if (FleeThreshold > 50.0f && DamageSource)
    {
        StartFleeing(DamageSource);
    }
    else if (DamageSource && Stats.Aggression > 40.0f)
    {
        StartHunting(DamageSource);
    }
}

bool UNPC_DinosaurBehaviorComponent::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
        return false;
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    // Simple line trace to check visibility
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstacle hit
}

float UNPC_DinosaurBehaviorComponent::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
        return FLT_MAX;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

void UNPC_DinosaurBehaviorComponent::JoinPack(UNPC_DinosaurBehaviorComponent* NewPackLeader)
{
    if (!NewPackLeader || NewPackLeader == this)
        return;
    
    // Leave current pack if any
    LeavePack();
    
    // Join new pack
    PackLeader = NewPackLeader;
    PackLeader->PackMembers.AddUnique(this);
}

void UNPC_DinosaurBehaviorComponent::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    // Disband pack if this was the leader
    for (auto Member : PackMembers)
    {
        if (Member)
        {
            Member->PackLeader = nullptr;
        }
    }
    PackMembers.Empty();
}

void UNPC_DinosaurBehaviorComponent::InitializeTRexBehavior()
{
    Stats.MaxHealth = 200.0f;
    Stats.Health = 200.0f;
    Stats.Aggression = 80.0f;
    Stats.TerritorialRadius = 3000.0f;
    
    BehaviorSettings.DetectionRange = 2000.0f;
    BehaviorSettings.AttackRange = 400.0f;
    BehaviorSettings.WalkSpeed = 200.0f;
    BehaviorSettings.RunSpeed = 600.0f;
    BehaviorSettings.AttackDamage = 50.0f;
    BehaviorSettings.bIsPackHunter = false;
}

void UNPC_DinosaurBehaviorComponent::InitializeRaptorBehavior()
{
    Stats.MaxHealth = 80.0f;
    Stats.Health = 80.0f;
    Stats.Aggression = 70.0f;
    Stats.TerritorialRadius = 1500.0f;
    
    BehaviorSettings.DetectionRange = 1500.0f;
    BehaviorSettings.AttackRange = 250.0f;
    BehaviorSettings.WalkSpeed = 250.0f;
    BehaviorSettings.RunSpeed = 800.0f;
    BehaviorSettings.AttackDamage = 30.0f;
    BehaviorSettings.bIsPackHunter = true;
    BehaviorSettings.PackSize = 4;
}

void UNPC_DinosaurBehaviorComponent::InitializeHerbivoreBehavior()
{
    Stats.MaxHealth = 150.0f;
    Stats.Health = 150.0f;
    Stats.Aggression = 20.0f;
    Stats.TerritorialRadius = 1000.0f;
    
    BehaviorSettings.DetectionRange = 800.0f;
    BehaviorSettings.AttackRange = 200.0f;
    BehaviorSettings.WalkSpeed = 120.0f;
    BehaviorSettings.RunSpeed = 350.0f;
    BehaviorSettings.AttackDamage = 15.0f;
    BehaviorSettings.bIsPackHunter = false;
}

void UNPC_DinosaurBehaviorComponent::UpdateIdleBehavior(float DeltaTime)
{
    // After 3-8 seconds of idle, start patrolling
    if (StateTimer > FMath::RandRange(3.0f, 8.0f))
    {
        StartPatrolling();
    }
}

void UNPC_DinosaurBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    if (!GetOwner())
        return;
    
    float DistanceToDestination = FVector::Dist(GetOwner()->GetActorLocation(), CurrentDestination);
    
    // If reached destination, pick a new patrol point
    if (DistanceToDestination < 100.0f)
    {
        CurrentDestination = GetRandomPatrolPoint();
    }
    else
    {
        // Move toward destination
        MoveToLocation(CurrentDestination, BehaviorSettings.WalkSpeed);
    }
    
    // Random chance to return to idle
    if (StateTimer > 20.0f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        SetBehaviorState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !GetOwner())
    {
        SetBehaviorState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
    
    // If target is too far, give up hunt
    if (DistanceToTarget > BehaviorSettings.DetectionRange * 1.5f)
    {
        CurrentTarget = nullptr;
        SetBehaviorState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    // If close enough, attack
    if (DistanceToTarget <= BehaviorSettings.AttackRange)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastAttackTime > 2.0f) // Attack every 2 seconds
        {
            // Deal damage to target
            if (CurrentTarget->IsA<APawn>())
            {
                // Apply damage logic here
                PlayAttackSound();
            }
            LastAttackTime = CurrentTime;
        }
    }
    else
    {
        // Chase target
        MoveToLocation(CurrentTarget->GetActorLocation(), BehaviorSettings.RunSpeed);
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateFleeingBehavior(float DeltaTime)
{
    if (!GetOwner())
        return;
    
    float DistanceToDestination = FVector::Dist(GetOwner()->GetActorLocation(), CurrentDestination);
    
    if (DistanceToDestination < 200.0f || StateTimer > 10.0f)
    {
        // Reached safe distance or fled long enough
        Stats.Fear = FMath::Max(0.0f, Stats.Fear - 20.0f);
        SetBehaviorState(ENPC_DinosaurState::Idle);
    }
    else
    {
        MoveToLocation(CurrentDestination, BehaviorSettings.RunSpeed);
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateFeedingBehavior(float DeltaTime)
{
    // Feeding restores hunger and health
    Stats.Hunger = FMath::Min(100.0f, Stats.Hunger + (20.0f * DeltaTime));
    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + (5.0f * DeltaTime));
    
    // Feed for 5-10 seconds
    if (StateTimer > FMath::RandRange(5.0f, 10.0f))
    {
        SetBehaviorState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorComponent::ScanForTargets()
{
    if (CurrentState == ENPC_DinosaurState::Fleeing || !GetOwner())
        return;
    
    // Carnivores hunt for prey
    if (Species == ENPC_DinosaurSpecies::TRex || Species == ENPC_DinosaurSpecies::Raptor)
    {
        AActor* NearestPlayer = FindNearestPlayer();
        if (NearestPlayer && GetDistanceToTarget(NearestPlayer) <= BehaviorSettings.DetectionRange)
        {
            if (CanSeeTarget(NearestPlayer))
            {
                StartHunting(NearestPlayer);
            }
        }
    }
}

void UNPC_DinosaurBehaviorComponent::ScanForThreats()
{
    if (!GetOwner())
        return;
    
    AActor* NearestThreat = FindNearestThreat();
    if (NearestThreat && GetDistanceToTarget(NearestThreat) <= BehaviorSettings.FleeRange)
    {
        if (Stats.Fear > Stats.Aggression)
        {
            StartFleeing(NearestThreat);
        }
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateStats(float DeltaTime)
{
    // Gradually decrease fear over time
    Stats.Fear = FMath::Max(0.0f, Stats.Fear - (5.0f * DeltaTime));
    
    // Hunger and thirst increase over time
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (2.0f * DeltaTime));
    Stats.Thirst = FMath::Max(0.0f, Stats.Thirst - (3.0f * DeltaTime));
    
    // Low hunger/thirst affects behavior
    if (Stats.Hunger < 20.0f || Stats.Thirst < 20.0f)
    {
        if (CurrentState != ENPC_DinosaurState::Feeding && FMath::RandRange(0.0f, 1.0f) < 0.1f)
        {
            SetBehaviorState(ENPC_DinosaurState::Feeding);
        }
    }
}

void UNPC_DinosaurBehaviorComponent::MoveToLocation(const FVector& Location, float Speed)
{
    if (!GetOwner())
        return;
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
    {
        FVector Direction = (Location - GetOwner()->GetActorLocation()).GetSafeNormal();
        OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = Speed;
        OwnerCharacter->AddMovementInput(Direction, 1.0f);
    }
}

AActor* UNPC_DinosaurBehaviorComponent::FindNearestPlayer() const
{
    if (!GetWorld())
        return nullptr;
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    return PlayerPawn;
}

AActor* UNPC_DinosaurBehaviorComponent::FindNearestPrey() const
{
    // Implementation for finding prey (other dinosaurs, etc.)
    return nullptr;
}

AActor* UNPC_DinosaurBehaviorComponent::FindNearestThreat() const
{
    // Implementation for finding threats
    return FindNearestPlayer(); // For now, player is the main threat
}

bool UNPC_DinosaurBehaviorComponent::IsLocationReachable(const FVector& Location) const
{
    // Simple check - could be enhanced with navigation system
    return true;
}

FVector UNPC_DinosaurBehaviorComponent::GetRandomPatrolPoint() const
{
    if (!GetOwner())
        return FVector::ZeroVector;
    
    // Generate random point within patrol radius
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(200.0f, BehaviorSettings.PatrolRadius);
    return PatrolCenter + (RandomDirection * RandomDistance);
}

void UNPC_DinosaurBehaviorComponent::PlayRoarSound()
{
    // Implementation for playing roar sound
    UE_LOG(LogTemp, Log, TEXT("Dinosaur roars!"));
}

void UNPC_DinosaurBehaviorComponent::PlayAttackSound()
{
    // Implementation for playing attack sound
    UE_LOG(LogTemp, Log, TEXT("Dinosaur attacks!"));
}

void UNPC_DinosaurBehaviorComponent::PlayHurtSound()
{
    // Implementation for playing hurt sound
    UE_LOG(LogTemp, Log, TEXT("Dinosaur is hurt!"));
}