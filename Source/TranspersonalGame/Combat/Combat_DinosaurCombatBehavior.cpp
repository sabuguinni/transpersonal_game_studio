#include "Combat_DinosaurCombatBehavior.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

UCombat_DinosaurCombatBehavior::UCombat_DinosaurCombatBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
}

void UCombat_DinosaurCombatBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize species-specific stats
    InitializeSpeciesStats();
    
    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        DinosaurAIController = Cast<AAIController>(OwnerPawn->GetController());
        if (DinosaurAIController && DinosaurAIController->GetBlackboardComponent())
        {
            BlackboardComponent = DinosaurAIController->GetBlackboardComponent();
        }
    }
    
    // Set initial state
    SetCombatState(ECombat_DinosaurCombatState::Idle);
}

void UCombat_DinosaurCombatBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateCombatBehavior(DeltaTime);
}

void UCombat_DinosaurCombatBehavior::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        StateChangeTimer = 0.0f;
        
        // Update blackboard if available
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(FName("CombatState"), static_cast<uint8>(NewState));
        }
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed combat state to %d"), 
               *GetOwner()->GetName(), static_cast<int32>(NewState));
    }
}

void UCombat_DinosaurCombatBehavior::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent && CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetActor"), CurrentTarget);
    }
    
    UpdateTargetDistance();
}

bool UCombat_DinosaurCombatBehavior::CanAttackTarget() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return false;
    }
    
    // Check if target is in attack range
    if (DistanceToTarget > CombatStats.AttackRange)
    {
        return false;
    }
    
    // Check attack cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return false;
    }
    
    return true;
}

void UCombat_DinosaurCombatBehavior::PerformAttack()
{
    if (!CanAttackTarget())
    {
        return;
    }
    
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Apply damage to target if it has a combat component
    if (UCombat_DinosaurCombatBehavior* TargetCombat = CurrentTarget->FindComponentByClass<UCombat_DinosaurCombatBehavior>())
    {
        TargetCombat->TakeDamage(CombatStats.AttackDamage, GetOwner());
    }
    
    // Visual/Audio feedback would go here
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacked %s for %f damage"), 
           *GetOwner()->GetName(), *CurrentTarget->GetName(), CombatStats.AttackDamage);
    
    // Set attack state
    SetCombatState(ECombat_DinosaurCombatState::Attacking);
}

void UCombat_DinosaurCombatBehavior::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    CombatStats.Health = FMath::Max(0.0f, CombatStats.Health - DamageAmount);
    
    // Set damage source as target if we don't have one
    if (!CurrentTarget && DamageSource)
    {
        SetTarget(DamageSource);
    }
    
    // Determine reaction based on health and species
    if (ShouldFlee())
    {
        SetCombatState(ECombat_DinosaurCombatState::Fleeing);
    }
    else if (ShouldAttack())
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s took %f damage, health now %f"), 
           *GetOwner()->GetName(), DamageAmount, CombatStats.Health);
}

void UCombat_DinosaurCombatBehavior::UpdateCombatBehavior(float DeltaTime)
{
    CombatUpdateTimer += DeltaTime;
    StateChangeTimer += DeltaTime;
    
    // Update target distance
    UpdateTargetDistance();
    
    // Find new threats if we don't have a target
    if (!CurrentTarget)
    {
        AActor* NearestThreat = FindNearestThreat();
        if (NearestThreat)
        {
            SetTarget(NearestThreat);
        }
    }
    
    // Handle current state
    switch (CurrentCombatState)
    {
        case ECombat_DinosaurCombatState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ECombat_DinosaurCombatState::Hunting:
            HandleHuntingState(DeltaTime);
            break;
        case ECombat_DinosaurCombatState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
        case ECombat_DinosaurCombatState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        case ECombat_DinosaurCombatState::Defending:
            HandleDefendingState(DeltaTime);
            break;
        case ECombat_DinosaurCombatState::Stalking:
            HandleStalkingState(DeltaTime);
            break;
        case ECombat_DinosaurCombatState::Circling:
            HandleCirclingState(DeltaTime);
            break;
    }
}

AActor* UCombat_DinosaurCombatBehavior::FindNearestThreat()
{
    if (!GetOwner() || !GetWorld())
    {
        return nullptr;
    }
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = CombatStats.DetectionRange;
    
    // Find all pawns in detection range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            // Check if this is a valid threat (player character, different species, etc.)
            if (Actor->IsA<ACharacter>() || Actor->FindComponentByClass<UCombat_DinosaurCombatBehavior>())
            {
                NearestThreat = Actor;
                NearestDistance = Distance;
            }
        }
    }
    
    return NearestThreat;
}

void UCombat_DinosaurCombatBehavior::InitializeSpeciesStats()
{
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            CombatStats.MaxHealth = 500.0f;
            CombatStats.Health = 500.0f;
            CombatStats.AttackDamage = 100.0f;
            CombatStats.AttackRange = 400.0f;
            CombatStats.DetectionRange = 2000.0f;
            CombatStats.MovementSpeed = 600.0f;
            CombatStats.Aggression = 0.9f;
            CombatStats.Fear = 0.1f;
            CombatStats.bIsPackHunter = false;
            CombatStats.bIsHerbivore = false;
            AttackCooldown = 3.0f;
            break;
            
        case ECombat_DinosaurSpecies::Raptor:
            CombatStats.MaxHealth = 150.0f;
            CombatStats.Health = 150.0f;
            CombatStats.AttackDamage = 40.0f;
            CombatStats.AttackRange = 250.0f;
            CombatStats.DetectionRange = 1500.0f;
            CombatStats.MovementSpeed = 800.0f;
            CombatStats.Aggression = 0.8f;
            CombatStats.Fear = 0.3f;
            CombatStats.bIsPackHunter = true;
            CombatStats.bIsHerbivore = false;
            AttackCooldown = 1.5f;
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            CombatStats.MaxHealth = 400.0f;
            CombatStats.Health = 400.0f;
            CombatStats.AttackDamage = 60.0f;
            CombatStats.AttackRange = 300.0f;
            CombatStats.DetectionRange = 1200.0f;
            CombatStats.MovementSpeed = 400.0f;
            CombatStats.Aggression = 0.4f;
            CombatStats.Fear = 0.2f;
            CombatStats.bIsPackHunter = false;
            CombatStats.bIsHerbivore = true;
            AttackCooldown = 2.5f;
            break;
            
        case ECombat_DinosaurSpecies::Brachiosaurus:
            CombatStats.MaxHealth = 800.0f;
            CombatStats.Health = 800.0f;
            CombatStats.AttackDamage = 80.0f;
            CombatStats.AttackRange = 500.0f;
            CombatStats.DetectionRange = 1000.0f;
            CombatStats.MovementSpeed = 300.0f;
            CombatStats.Aggression = 0.2f;
            CombatStats.Fear = 0.1f;
            CombatStats.bIsPackHunter = false;
            CombatStats.bIsHerbivore = true;
            AttackCooldown = 4.0f;
            break;
            
        default:
            // Default stats already set in header
            break;
    }
}

bool UCombat_DinosaurCombatBehavior::ShouldFlee() const
{
    float HealthPercentage = CombatStats.Health / CombatStats.MaxHealth;
    float FleeThreshold = CombatStats.Fear;
    
    return HealthPercentage < FleeThreshold;
}

bool UCombat_DinosaurCombatBehavior::ShouldAttack() const
{
    if (!CurrentTarget)
    {
        return false;
    }
    
    float HealthPercentage = CombatStats.Health / CombatStats.MaxHealth;
    float AttackThreshold = CombatStats.Aggression * HealthPercentage;
    
    return AttackThreshold > 0.5f && DistanceToTarget <= CombatStats.DetectionRange;
}

FVector UCombat_DinosaurCombatBehavior::GetOptimalAttackPosition() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Position slightly closer than attack range for optimal positioning
    float OptimalDistance = CombatStats.AttackRange * 0.8f;
    return TargetLocation - (DirectionToTarget * OptimalDistance);
}

void UCombat_DinosaurCombatBehavior::JoinPack(UCombat_DinosaurCombatBehavior* PackLeader)
{
    if (PackLeader && PackLeader != this)
    {
        PackMembers.AddUnique(PackLeader);
        PackLeader->PackMembers.AddUnique(this);
    }
}

void UCombat_DinosaurCombatBehavior::LeavePack()
{
    for (UCombat_DinosaurCombatBehavior* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackMembers.Remove(this);
        }
    }
    PackMembers.Empty();
}

bool UCombat_DinosaurCombatBehavior::IsInPack() const
{
    return PackMembers.Num() > 0;
}

void UCombat_DinosaurCombatBehavior::UpdateTargetDistance()
{
    if (CurrentTarget && GetOwner())
    {
        DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    }
    else
    {
        DistanceToTarget = 0.0f;
    }
}

void UCombat_DinosaurCombatBehavior::HandleIdleState(float DeltaTime)
{
    // Look for threats periodically
    if (CombatUpdateTimer > 1.0f)
    {
        AActor* Threat = FindNearestThreat();
        if (Threat)
        {
            SetTarget(Threat);
            if (CombatStats.bIsHerbivore)
            {
                SetCombatState(ECombat_DinosaurCombatState::Fleeing);
            }
            else
            {
                SetCombatState(ECombat_DinosaurCombatState::Stalking);
            }
        }
        CombatUpdateTimer = 0.0f;
    }
}

void UCombat_DinosaurCombatBehavior::HandleHuntingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
        return;
    }
    
    if (DistanceToTarget <= CombatStats.AttackRange)
    {
        SetCombatState(ECombat_DinosaurCombatState::Attacking);
    }
    else if (DistanceToTarget > CombatStats.DetectionRange)
    {
        SetTarget(nullptr);
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
}

void UCombat_DinosaurCombatBehavior::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
        return;
    }
    
    if (CanAttackTarget())
    {
        PerformAttack();
    }
    
    // Return to hunting if target moves out of range
    if (DistanceToTarget > CombatStats.AttackRange)
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
    
    // Attack state timeout
    if (StateChangeTimer > 3.0f)
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
}

void UCombat_DinosaurCombatBehavior::HandleFleeingState(float DeltaTime)
{
    // Continue fleeing until safe distance or health recovers
    if (!CurrentTarget || DistanceToTarget > CombatStats.DetectionRange * 1.5f)
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
    
    // Stop fleeing if health is restored enough
    if (!ShouldFlee())
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
}

void UCombat_DinosaurCombatBehavior::HandleDefendingState(float DeltaTime)
{
    // Defensive posture - attack if threatened but don't pursue
    if (CurrentTarget && DistanceToTarget <= CombatStats.AttackRange)
    {
        if (CanAttackTarget())
        {
            PerformAttack();
        }
    }
    
    // Return to idle if no immediate threat
    if (!CurrentTarget || DistanceToTarget > CombatStats.AttackRange * 1.5f)
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
}

void UCombat_DinosaurCombatBehavior::HandleStalkingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
        return;
    }
    
    // Move to hunting when close enough
    if (DistanceToTarget <= CombatStats.AttackRange * 1.2f)
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
    
    // Lose interest if target is too far
    if (DistanceToTarget > CombatStats.DetectionRange)
    {
        SetTarget(nullptr);
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
}

void UCombat_DinosaurCombatBehavior::HandleCirclingState(float DeltaTime)
{
    // Circling behavior for pack hunters
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
        return;
    }
    
    // Switch to attack if opportunity arises
    if (CanAttackTarget() && IsInPack())
    {
        SetCombatState(ECombat_DinosaurCombatState::Attacking);
    }
    
    // Timeout circling
    if (StateChangeTimer > 10.0f)
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
}