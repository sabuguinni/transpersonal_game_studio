#include "Combat_DinosaurCombatSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "../TranspersonalCharacter.h"

UCombat_DinosaurCombatSystem::UCombat_DinosaurCombatSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default stats
    DinosaurStats = FCombat_DinosaurStats();
    CurrentState = ECombat_DinosaurState::Idle;
    bIsAggressive = true;
    StateUpdateInterval = 0.5f;
    LastStateUpdateTime = 0.0f;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;
}

void UCombat_DinosaurCombatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize component
    CurrentState = ECombat_DinosaurState::Patrolling;
    LastStateUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatSystem initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_DinosaurCombatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetWorld() || !GetOwner())
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update combat behavior at intervals
    if (CurrentTime - LastStateUpdateTime >= StateUpdateInterval)
    {
        UpdateCombatBehavior();
        LastStateUpdateTime = CurrentTime;
    }
}

void UCombat_DinosaurCombatSystem::SetCombatState(ECombat_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_DinosaurState OldState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s: Combat state changed from %d to %d"), 
               *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
    }
}

bool UCombat_DinosaurCombatSystem::DetectNearbyTargets()
{
    if (!GetWorld() || !GetOwner())
        return false;
    
    // Find player character within detection radius
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    if (!Player)
        return false;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = Player->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
    
    // Debug visualization
    if (GetWorld()->IsGameWorld())
    {
        DrawDebugSphere(GetWorld(), OwnerLocation, DinosaurStats.DetectionRadius, 
                       12, FColor::Yellow, false, 0.1f, 0, 2.0f);
    }
    
    if (Distance <= DinosaurStats.DetectionRadius)
    {
        CurrentTarget = Player;
        return true;
    }
    
    return false;
}

void UCombat_DinosaurCombatSystem::AttackTarget()
{
    if (!CurrentTarget || !GetWorld())
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
        return;
    
    if (!IsTargetInRange())
        return;
    
    // Deal damage to target
    if (ATranspersonalCharacter* PlayerTarget = Cast<ATranspersonalCharacter>(CurrentTarget))
    {
        // Apply damage through player's health system
        UE_LOG(LogTemp, Warning, TEXT("%s attacks %s for %.1f damage!"), 
               *GetOwner()->GetName(), *CurrentTarget->GetName(), DinosaurStats.AttackDamage);
        
        // Visual feedback
        if (GetWorld()->IsGameWorld())
        {
            FVector AttackLocation = CurrentTarget->GetActorLocation();
            DrawDebugSphere(GetWorld(), AttackLocation, 100.0f, 
                           8, FColor::Red, false, 1.0f, 0, 5.0f);
        }
    }
    
    LastAttackTime = CurrentTime;
    SetCombatState(ECombat_DinosaurState::Attacking);
}

void UCombat_DinosaurCombatSystem::TakeDamage(float DamageAmount, AActor* DamageSource)
{
    DinosaurStats.Health = FMath::Max(0.0f, DinosaurStats.Health - DamageAmount);
    
    UE_LOG(LogTemp, Warning, TEXT("%s takes %.1f damage! Health: %.1f/%.1f"), 
           *GetOwner()->GetName(), DamageAmount, DinosaurStats.Health, DinosaurStats.MaxHealth);
    
    if (DinosaurStats.Health <= 0.0f)
    {
        SetCombatState(ECombat_DinosaurState::Dead);
        UE_LOG(LogTemp, Error, TEXT("%s has died!"), *GetOwner()->GetName());
    }
    else if (DamageSource)
    {
        // Become aggressive toward damage source
        if (ATranspersonalCharacter* PlayerSource = Cast<ATranspersonalCharacter>(DamageSource))
        {
            CurrentTarget = PlayerSource;
            SetCombatState(ECombat_DinosaurState::Hunting);
        }
    }
}

bool UCombat_DinosaurCombatSystem::IsTargetInRange() const
{
    if (!CurrentTarget || !GetOwner())
        return false;
    
    float Distance = GetDistanceToTarget();
    return Distance <= DinosaurStats.AttackRange;
}

float UCombat_DinosaurCombatSystem::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return -1.0f;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void UCombat_DinosaurCombatSystem::StartHunting(ATranspersonalCharacter* Target)
{
    CurrentTarget = Target;
    SetCombatState(ECombat_DinosaurState::Hunting);
    bIsAggressive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("%s starts hunting %s"), 
           *GetOwner()->GetName(), Target ? *Target->GetName() : TEXT("Unknown"));
}

void UCombat_DinosaurCombatSystem::StopHunting()
{
    CurrentTarget = nullptr;
    SetCombatState(ECombat_DinosaurState::Patrolling);
    
    UE_LOG(LogTemp, Log, TEXT("%s stops hunting"), *GetOwner()->GetName());
}

void UCombat_DinosaurCombatSystem::UpdateCombatBehavior()
{
    if (CurrentState == ECombat_DinosaurState::Dead)
        return;
    
    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
        case ECombat_DinosaurState::Patrolling:
            HandleIdleState();
            break;
            
        case ECombat_DinosaurState::Hunting:
            HandleHuntingState();
            break;
            
        case ECombat_DinosaurState::Attacking:
            HandleAttackingState();
            break;
            
        default:
            break;
    }
}

void UCombat_DinosaurCombatSystem::HandleIdleState()
{
    if (bIsAggressive && DetectNearbyTargets())
    {
        SetCombatState(ECombat_DinosaurState::Hunting);
    }
}

void UCombat_DinosaurCombatSystem::HandleHuntingState()
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinosaurState::Patrolling);
        return;
    }
    
    float Distance = GetDistanceToTarget();
    
    // Lost target
    if (Distance > DinosaurStats.DetectionRadius * 1.5f)
    {
        StopHunting();
        return;
    }
    
    // Close enough to attack
    if (Distance <= DinosaurStats.AttackRange)
    {
        AttackTarget();
    }
    else
    {
        // Move toward target (this would be handled by AI controller)
        UE_LOG(LogTemp, VeryVerbose, TEXT("%s hunting target at distance %.1f"), 
               *GetOwner()->GetName(), Distance);
    }
}

void UCombat_DinosaurCombatSystem::HandleAttackingState()
{
    if (!CurrentTarget || !IsTargetInRange())
    {
        SetCombatState(ECombat_DinosaurState::Hunting);
    }
}