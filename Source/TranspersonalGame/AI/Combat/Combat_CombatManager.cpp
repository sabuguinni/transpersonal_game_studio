#include "Combat_CombatManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCombat_CombatManager::UCombat_CombatManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize combat data
    CombatData = FCombat_CombatData();
    CurrentCombatState = ECombat_CombatState::Idle;
    DetectionRadius = 1500.0f;
    EngagementRadius = 800.0f;
    RetreatRadius = 2000.0f;
    LastAttackTime = 0.0f;
}

void UCombat_CombatManager::BeginPlay()
{
    Super::BeginPlay();

    // Start combat update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CombatUpdateTimer,
            this,
            &UCombat_CombatManager::UpdateCombatLogic,
            0.5f, // Update every 0.5 seconds
            true  // Loop
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Combat Manager initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_CombatManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update combat state based on current conditions
    UpdateCombatState();
}

void UCombat_CombatManager::StartCombat(AActor* Target)
{
    if (!Target || !IsValidTarget(Target))
    {
        return;
    }

    CombatData.CurrentTarget = Target;
    CombatData.bIsInCombat = true;
    SetCombatState(ECombat_CombatState::Engaging);

    UE_LOG(LogTemp, Log, TEXT("%s starting combat with %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           Target ? *Target->GetName() : TEXT("Unknown"));
}

void UCombat_CombatManager::EndCombat()
{
    CombatData.bIsInCombat = false;
    CombatData.CurrentTarget = nullptr;
    SetCombatState(ECombat_CombatState::Idle);

    UE_LOG(LogTemp, Log, TEXT("%s ending combat"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_CombatManager::PerformAttack()
{
    if (!CanAttack())
    {
        return;
    }

    AActor* Target = CombatData.CurrentTarget;
    if (!Target || !IsValidTarget(Target))
    {
        EndCombat();
        return;
    }

    // Record attack time
    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage to target if it's a character
    if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
    {
        // In a real implementation, this would use a damage system
        UE_LOG(LogTemp, Log, TEXT("%s attacks %s for %.1f damage"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               Target->GetName(),
               CombatData.AttackDamage);
    }

    SetCombatState(ECombat_CombatState::Attacking);
}

bool UCombat_CombatManager::CanAttack() const
{
    if (!CombatData.CurrentTarget || !CombatData.bIsInCombat)
    {
        return false;
    }

    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < CombatData.AttackCooldown)
    {
        return false;
    }

    // Check range
    return IsTargetInRange();
}

AActor* UCombat_CombatManager::FindNearestEnemy()
{
    if (!GetOwner())
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    AActor* NearestEnemy = nullptr;
    float NearestDistance = DetectionRadius;

    // Find all actors in detection range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner() || !IsValidTarget(Actor))
        {
            continue;
        }

        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestEnemy = Actor;
        }
    }

    return NearestEnemy;
}

void UCombat_CombatManager::UpdateCombatState()
{
    switch (CurrentCombatState)
    {
        case ECombat_CombatState::Idle:
            HandleIdleState();
            break;
        case ECombat_CombatState::Searching:
            HandleSearchingState();
            break;
        case ECombat_CombatState::Engaging:
            HandleEngagingState();
            break;
        case ECombat_CombatState::Attacking:
            HandleAttackingState();
            break;
        case ECombat_CombatState::Retreating:
            HandleRetreatingState();
            break;
        default:
            break;
    }
}

float UCombat_CombatManager::GetDistanceToTarget() const
{
    if (!CombatData.CurrentTarget || !GetOwner())
    {
        return -1.0f;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), CombatData.CurrentTarget->GetActorLocation());
}

bool UCombat_CombatManager::IsTargetInRange() const
{
    float Distance = GetDistanceToTarget();
    return Distance >= 0.0f && Distance <= CombatData.AttackRange;
}

void UCombat_CombatManager::SetCombatState(ECombat_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        UE_LOG(LogTemp, Log, TEXT("%s combat state changed to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               (int32)NewState);
    }
}

void UCombat_CombatManager::UpdateCombatLogic()
{
    if (!GetOwner())
    {
        return;
    }

    // If not in combat, look for enemies
    if (!CombatData.bIsInCombat)
    {
        AActor* NearestEnemy = FindNearestEnemy();
        if (NearestEnemy)
        {
            StartCombat(NearestEnemy);
        }
    }
    else
    {
        // Validate current target
        if (!CombatData.CurrentTarget || !IsValidTarget(CombatData.CurrentTarget))
        {
            EndCombat();
            return;
        }

        // Check if target is too far away
        float Distance = GetDistanceToTarget();
        if (Distance > RetreatRadius)
        {
            EndCombat();
        }
    }
}

bool UCombat_CombatManager::IsValidTarget(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }

    // Don't target self
    if (Target == GetOwner())
    {
        return false;
    }

    // Only target pawns (characters)
    APawn* TargetPawn = Cast<APawn>(Target);
    if (!TargetPawn)
    {
        return false;
    }

    // Additional validation can be added here
    // For example, faction checking, line of sight, etc.

    return true;
}

void UCombat_CombatManager::HandleIdleState()
{
    // Look for enemies
    AActor* NearestEnemy = FindNearestEnemy();
    if (NearestEnemy)
    {
        SetCombatState(ECombat_CombatState::Searching);
    }
}

void UCombat_CombatManager::HandleSearchingState()
{
    AActor* NearestEnemy = FindNearestEnemy();
    if (NearestEnemy)
    {
        StartCombat(NearestEnemy);
    }
    else
    {
        SetCombatState(ECombat_CombatState::Idle);
    }
}

void UCombat_CombatManager::HandleEngagingState()
{
    if (!CombatData.CurrentTarget)
    {
        EndCombat();
        return;
    }

    float Distance = GetDistanceToTarget();
    if (Distance <= CombatData.AttackRange)
    {
        if (CanAttack())
        {
            PerformAttack();
        }
    }
    else if (Distance > EngagementRadius)
    {
        SetCombatState(ECombat_CombatState::Retreating);
    }
}

void UCombat_CombatManager::HandleAttackingState()
{
    // After attacking, return to engaging state
    SetCombatState(ECombat_CombatState::Engaging);
}

void UCombat_CombatManager::HandleRetreatingState()
{
    if (!CombatData.CurrentTarget)
    {
        EndCombat();
        return;
    }

    float Distance = GetDistanceToTarget();
    if (Distance > RetreatRadius)
    {
        EndCombat();
    }
    else if (Distance <= EngagementRadius)
    {
        SetCombatState(ECombat_CombatState::Engaging);
    }
}