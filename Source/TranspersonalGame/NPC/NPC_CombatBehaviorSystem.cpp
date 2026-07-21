#include "NPC_CombatBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UNPC_CombatBehaviorSystem::UNPC_CombatBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for combat responsiveness
    
    // Initialize combat state
    CurrentCombatState = ENPC_CombatState::Passive;
    LastAttackTime = 0.0f;
    CombatStateTimer = 0.0f;
    LastThreatScanTime = 0.0f;
    
    // Initialize combat stats with default values
    CombatStats = FNPC_CombatStats();
    
    // Initialize target
    PrimaryTarget = FNPC_CombatTarget();
}

void UNPC_CombatBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find nearby allies at start
    FindNearbyAllies();
    
    // Set initial scan time
    LastThreatScanTime = GetWorld()->GetTimeSeconds();
}

void UNPC_CombatBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsAlive())
    {
        SetCombatState(ENPC_CombatState::Dead);
        return;
    }
    
    // Update combat state timer
    CombatStateTimer += DeltaTime;
    
    // Periodic threat scanning
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastThreatScanTime >= ThreatScanInterval)
    {
        ScanForThreats();
        LastThreatScanTime = CurrentTime;
    }
    
    // Update target information
    UpdateTargetThreatLevels();
    
    // Process combat behavior based on current state
    ProcessCombatBehavior(DeltaTime);
    
    // Clean up invalid targets
    CleanupInvalidTargets();
}

void UNPC_CombatBehaviorSystem::SetCombatState(ENPC_CombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ENPC_CombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        CombatStateTimer = 0.0f;
        
        // Handle state transitions
        switch (NewState)
        {
            case ENPC_CombatState::Alert:
                if (PreviousState == ENPC_CombatState::Passive)
                {
                    // Just became aware of potential threat
                    ScanForThreats();
                }
                break;
                
            case ENPC_CombatState::Aggressive:
                EnterCombatMode();
                break;
                
            case ENPC_CombatState::Fleeing:
                ExitCombatMode();
                break;
                
            case ENPC_CombatState::Dead:
                ExitCombatMode();
                PrimaryTarget = FNPC_CombatTarget(); // Clear target
                break;
                
            default:
                break;
        }
    }
}

bool UNPC_CombatBehaviorSystem::CanAttack() const
{
    if (!IsAlive() || !GetPrimaryTarget())
        return false;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < CombatStats.AttackCooldown)
        return false;
        
    // Check if target is in range
    if (PrimaryTarget.DistanceToTarget > CombatStats.AttackRange)
        return false;
        
    return CurrentCombatState == ENPC_CombatState::Attacking || CurrentCombatState == ENPC_CombatState::Aggressive;
}

void UNPC_CombatBehaviorSystem::ExecuteAttack(ENPC_AttackType AttackType)
{
    if (!CanAttack())
        return;
        
    APawn* Target = GetPrimaryTarget();
    if (!Target)
        return;
        
    // Calculate damage based on attack type
    float DamageAmount = CombatStats.AttackDamage;
    
    switch (AttackType)
    {
        case ENPC_AttackType::Bite:
            DamageAmount *= 1.2f; // Bite does extra damage
            break;
        case ENPC_AttackType::Claw:
            DamageAmount *= 1.0f; // Standard damage
            break;
        case ENPC_AttackType::Tail:
            DamageAmount *= 0.8f; // Tail whip does less damage but has knockback
            break;
        case ENPC_AttackType::Charge:
            DamageAmount *= 1.5f; // Charge does high damage
            break;
        case ENPC_AttackType::Stomp:
            DamageAmount *= 2.0f; // Stomp does massive damage but slow
            break;
        case ENPC_AttackType::Roar:
            DamageAmount = 0.0f; // Roar doesn't do damage but intimidates
            break;
    }
    
    // Deal damage to target
    DealDamageToTarget(Target, DamageAmount);
    
    // Update last attack time
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Set attacking state
    SetCombatState(ENPC_CombatState::Attacking);
}

void UNPC_CombatBehaviorSystem::SetPrimaryTarget(APawn* NewTarget)
{
    if (NewTarget && IsValidTarget(NewTarget))
    {
        PrimaryTarget.TargetPawn = NewTarget;
        PrimaryTarget.ThreatLevel = CalculateThreatLevel(NewTarget);
        PrimaryTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
        PrimaryTarget.LastKnownLocation = NewTarget->GetActorLocation();
        
        // Calculate distance
        if (GetOwner())
        {
            PrimaryTarget.DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), NewTarget->GetActorLocation());
        }
        
        // Enter alert or aggressive state based on threat level
        if (PrimaryTarget.ThreatLevel > CombatStats.AggressionLevel)
        {
            SetCombatState(ENPC_CombatState::Aggressive);
        }
        else
        {
            SetCombatState(ENPC_CombatState::Alert);
        }
    }
    else
    {
        PrimaryTarget = FNPC_CombatTarget();
        if (CurrentCombatState != ENPC_CombatState::Wounded && CurrentCombatState != ENPC_CombatState::Dead)
        {
            SetCombatState(ENPC_CombatState::Passive);
        }
    }
}

APawn* UNPC_CombatBehaviorSystem::GetPrimaryTarget() const
{
    return PrimaryTarget.TargetPawn.IsValid() ? PrimaryTarget.TargetPawn.Get() : nullptr;
}

void UNPC_CombatBehaviorSystem::ScanForThreats()
{
    if (!GetOwner())
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Clear potential targets
    PotentialTargets.Empty();
    
    // Get all pawns in detection range
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        World,
        GetOwner()->GetActorLocation(),
        DetectionRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>(),
        FoundActors
    );
    
    // Evaluate each potential target
    for (AActor* Actor : FoundActors)
    {
        APawn* Pawn = Cast<APawn>(Actor);
        if (Pawn && Pawn != GetOwner() && IsValidTarget(Pawn))
        {
            FNPC_CombatTarget NewTarget;
            NewTarget.TargetPawn = Pawn;
            NewTarget.ThreatLevel = CalculateThreatLevel(Pawn);
            NewTarget.LastSeenTime = World->GetTimeSeconds();
            NewTarget.LastKnownLocation = Pawn->GetActorLocation();
            NewTarget.DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), Pawn->GetActorLocation());
            
            PotentialTargets.Add(NewTarget);
        }
    }
    
    // Sort by threat level (highest first)
    PotentialTargets.Sort([](const FNPC_CombatTarget& A, const FNPC_CombatTarget& B) {
        return A.ThreatLevel > B.ThreatLevel;
    });
    
    // Set primary target to highest threat
    if (PotentialTargets.Num() > 0 && (!GetPrimaryTarget() || PotentialTargets[0].ThreatLevel > PrimaryTarget.ThreatLevel))
    {
        SetPrimaryTarget(PotentialTargets[0].TargetPawn.Get());
    }
}

void UNPC_CombatBehaviorSystem::UpdateTargetThreatLevels()
{
    // Update primary target
    if (APawn* Target = GetPrimaryTarget())
    {
        PrimaryTarget.ThreatLevel = CalculateThreatLevel(Target);
        PrimaryTarget.LastKnownLocation = Target->GetActorLocation();
        PrimaryTarget.DistanceToTarget = GetOwner() ? FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation()) : 0.0f;
    }
    
    // Update potential targets
    for (FNPC_CombatTarget& Target : PotentialTargets)
    {
        if (APawn* TargetPawn = Target.TargetPawn.Get())
        {
            Target.ThreatLevel = CalculateThreatLevel(TargetPawn);
            Target.LastKnownLocation = TargetPawn->GetActorLocation();
            Target.DistanceToTarget = GetOwner() ? FVector::Dist(GetOwner()->GetActorLocation(), TargetPawn->GetActorLocation()) : 0.0f;
        }
    }
}

void UNPC_CombatBehaviorSystem::TakeDamage(float DamageAmount, APawn* DamageSource)
{
    if (!IsAlive())
        return;
        
    // Apply defense rating
    float ActualDamage = DamageAmount * (1.0f - CombatStats.DefenseRating);
    CombatStats.CurrentHealth = FMath::Max(0.0f, CombatStats.CurrentHealth - ActualDamage);
    
    // Set damage source as primary target if not already targeting something more threatening
    if (DamageSource && (!GetPrimaryTarget() || CalculateThreatLevel(DamageSource) > PrimaryTarget.ThreatLevel))
    {
        SetPrimaryTarget(DamageSource);
    }
    
    // Determine reaction based on health and aggression
    if (CombatStats.CurrentHealth <= 0.0f)
    {
        SetCombatState(ENPC_CombatState::Dead);
    }
    else if (GetHealthPercentage() < CombatStats.FearThreshold)
    {
        if (ShouldFlee())
        {
            SetCombatState(ENPC_CombatState::Fleeing);
        }
        else
        {
            SetCombatState(ENPC_CombatState::Wounded);
        }
    }
    else
    {
        SetCombatState(ENPC_CombatState::Aggressive);
        
        // Call for help if severely damaged
        if (GetHealthPercentage() < 0.5f && bCanCallForHelp)
        {
            CallForHelp();
        }
    }
}

void UNPC_CombatBehaviorSystem::DealDamageToTarget(APawn* Target, float DamageAmount)
{
    if (!Target || DamageAmount <= 0.0f)
        return;
        
    // Try to find target's combat system
    if (UNPC_CombatBehaviorSystem* TargetCombat = Target->FindComponentByClass<UNPC_CombatBehaviorSystem>())
    {
        TargetCombat->TakeDamage(DamageAmount, Cast<APawn>(GetOwner()));
    }
    else
    {
        // If target doesn't have combat system, use UE5's damage system
        UGameplayStatics::ApplyDamage(Target, DamageAmount, nullptr, GetOwner(), UDamageType::StaticClass());
    }
}

float UNPC_CombatBehaviorSystem::GetHealthPercentage() const
{
    return CombatStats.MaxHealth > 0.0f ? (CombatStats.CurrentHealth / CombatStats.MaxHealth) : 0.0f;
}

void UNPC_CombatBehaviorSystem::EnterCombatMode()
{
    // Increase aggression temporarily
    CombatStats.AggressionLevel = FMath::Min(1.0f, CombatStats.AggressionLevel + 0.2f);
    
    // Notify allies
    if (bCanCallForHelp)
    {
        CallForHelp();
    }
}

void UNPC_CombatBehaviorSystem::ExitCombatMode()
{
    // Reset aggression to normal levels over time
    CombatStats.AggressionLevel = FMath::Max(0.1f, CombatStats.AggressionLevel - 0.1f);
}

void UNPC_CombatBehaviorSystem::FleeFromThreat()
{
    SetCombatState(ENPC_CombatState::Fleeing);
    
    // Clear primary target while fleeing
    PrimaryTarget = FNPC_CombatTarget();
}

bool UNPC_CombatBehaviorSystem::ShouldFlee() const
{
    // Flee if health is critically low
    if (GetHealthPercentage() < CombatStats.FearThreshold)
        return true;
        
    // Flee if outnumbered significantly
    int32 NearbyEnemies = 0;
    int32 NearbyAllies = 0;
    
    for (const FNPC_CombatTarget& Target : PotentialTargets)
    {
        if (Target.DistanceToTarget < DetectionRange * 0.5f)
        {
            NearbyEnemies++;
        }
    }
    
    for (const TWeakObjectPtr<UNPC_CombatBehaviorSystem>& Ally : AlliedCombatants)
    {
        if (Ally.IsValid() && GetOwner())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Ally->GetOwner()->GetActorLocation());
            if (Distance < DetectionRange * 0.5f)
            {
                NearbyAllies++;
            }
        }
    }
    
    return NearbyEnemies > (NearbyAllies + 1) * 2; // Flee if outnumbered 2:1
}

void UNPC_CombatBehaviorSystem::CallForHelp()
{
    if (!bCanCallForHelp)
        return;
        
    BroadcastHelpCall();
}

void UNPC_CombatBehaviorSystem::RespondToHelpCall(APawn* Caller)
{
    if (!bCanRespondToHelp || !Caller)
        return;
        
    // If not currently in combat or caller is closer/more important
    if (CurrentCombatState == ENPC_CombatState::Passive || CurrentCombatState == ENPC_CombatState::Alert)
    {
        float DistanceToCaller = GetOwner() ? FVector::Dist(GetOwner()->GetActorLocation(), Caller->GetActorLocation()) : FLT_MAX;
        
        if (DistanceToCaller <= HelpCallRange)
        {
            // Find what the caller is fighting
            if (UNPC_CombatBehaviorSystem* CallerCombat = Caller->FindComponentByClass<UNPC_CombatBehaviorSystem>())
            {
                if (APawn* CallerTarget = CallerCombat->GetPrimaryTarget())
                {
                    SetPrimaryTarget(CallerTarget);
                }
            }
        }
    }
}

void UNPC_CombatBehaviorSystem::CoordinateGroupAttack()
{
    // Simple group coordination - focus fire on primary target
    APawn* Target = GetPrimaryTarget();
    if (!Target)
        return;
        
    // Tell allies to target the same enemy
    for (const TWeakObjectPtr<UNPC_CombatBehaviorSystem>& Ally : AlliedCombatants)
    {
        if (Ally.IsValid() && Ally->GetOwner())
        {
            float Distance = GetOwner() ? FVector::Dist(GetOwner()->GetActorLocation(), Ally->GetOwner()->GetActorLocation()) : FLT_MAX;
            if (Distance <= HelpCallRange)
            {
                Ally->SetPrimaryTarget(Target);
            }
        }
    }
}

void UNPC_CombatBehaviorSystem::ProcessCombatBehavior(float DeltaTime)
{
    switch (CurrentCombatState)
    {
        case ENPC_CombatState::Passive:
            // Do nothing, just exist peacefully
            break;
            
        case ENPC_CombatState::Alert:
            // Look around, assess threats
            if (CombatStateTimer > 5.0f) // Stay alert for 5 seconds
            {
                if (PotentialTargets.Num() == 0)
                {
                    SetCombatState(ENPC_CombatState::Passive);
                }
            }
            break;
            
        case ENPC_CombatState::Aggressive:
            // Move toward target and attack when in range
            if (APawn* Target = GetPrimaryTarget())
            {
                if (PrimaryTarget.DistanceToTarget <= CombatStats.AttackRange)
                {
                    if (CanAttack())
                    {
                        ExecuteAttack(ENPC_AttackType::Bite); // Default attack
                    }
                }
                
                // Coordinate with allies
                if (CombatStateTimer > 2.0f) // Coordinate every 2 seconds
                {
                    CoordinateGroupAttack();
                    CombatStateTimer = 0.0f;
                }
            }
            else
            {
                SetCombatState(ENPC_CombatState::Alert);
            }
            break;
            
        case ENPC_CombatState::Attacking:
            // Brief state after executing an attack
            if (CombatStateTimer > 1.0f)
            {
                SetCombatState(ENPC_CombatState::Aggressive);
            }
            break;
            
        case ENPC_CombatState::Fleeing:
            // Move away from threats
            if (CombatStateTimer > 10.0f) // Flee for 10 seconds
            {
                if (GetHealthPercentage() > CombatStats.FearThreshold)
                {
                    SetCombatState(ENPC_CombatState::Alert);
                }
            }
            break;
            
        case ENPC_CombatState::Wounded:
            // Reduced activity, defensive behavior
            if (GetHealthPercentage() > 0.5f)
            {
                SetCombatState(ENPC_CombatState::Alert);
            }
            break;
            
        case ENPC_CombatState::Dead:
            // Stop all activity
            SetComponentTickEnabled(false);
            break;
    }
}

float UNPC_CombatBehaviorSystem::CalculateThreatLevel(APawn* PotentialThreat) const
{
    if (!PotentialThreat || !GetOwner())
        return 0.0f;
        
    float ThreatLevel = 0.0f;
    
    // Base threat from being a player
    if (PotentialThreat->IsPlayerControlled())
    {
        ThreatLevel += 0.7f;
    }
    else
    {
        ThreatLevel += 0.3f; // Other NPCs are less threatening by default
    }
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / DetectionRange), 0.0f, 1.0f);
    ThreatLevel *= (0.5f + DistanceFactor * 0.5f);
    
    // Size/scale factor (bigger = more threatening)
    FVector ThreatScale = PotentialThreat->GetActorScale3D();
    float ScaleFactor = (ThreatScale.X + ThreatScale.Y + ThreatScale.Z) / 3.0f;
    ThreatLevel *= FMath::Clamp(ScaleFactor, 0.5f, 2.0f);
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool UNPC_CombatBehaviorSystem::IsValidTarget(APawn* PotentialTarget) const
{
    if (!PotentialTarget || PotentialTarget == GetOwner())
        return false;
        
    // Check if target is alive
    if (UNPC_CombatBehaviorSystem* TargetCombat = PotentialTarget->FindComponentByClass<UNPC_CombatBehaviorSystem>())
    {
        if (!TargetCombat->IsAlive())
            return false;
    }
    
    // Add more validation logic here (faction checks, etc.)
    
    return true;
}

void UNPC_CombatBehaviorSystem::CleanupInvalidTargets()
{
    // Clean up primary target
    if (PrimaryTarget.TargetPawn.IsValid() && !IsValidTarget(PrimaryTarget.TargetPawn.Get()))
    {
        PrimaryTarget = FNPC_CombatTarget();
    }
    
    // Clean up potential targets
    for (int32 i = PotentialTargets.Num() - 1; i >= 0; i--)
    {
        if (!PotentialTargets[i].TargetPawn.IsValid() || !IsValidTarget(PotentialTargets[i].TargetPawn.Get()))
        {
            PotentialTargets.RemoveAt(i);
        }
    }
    
    // Clean up allied combatants
    for (int32 i = AlliedCombatants.Num() - 1; i >= 0; i--)
    {
        if (!AlliedCombatants[i].IsValid())
        {
            AlliedCombatants.RemoveAt(i);
        }
    }
}

FVector UNPC_CombatBehaviorSystem::GetFleeDirection() const
{
    if (!GetOwner())
        return FVector::ZeroVector;
        
    FVector FleeDirection = FVector::ZeroVector;
    int32 ThreatCount = 0;
    
    // Calculate average direction away from all threats
    for (const FNPC_CombatTarget& Target : PotentialTargets)
    {
        if (Target.TargetPawn.IsValid())
        {
            FVector DirectionFromThreat = GetOwner()->GetActorLocation() - Target.TargetPawn->GetActorLocation();
            DirectionFromThreat.Normalize();
            FleeDirection += DirectionFromThreat * Target.ThreatLevel;
            ThreatCount++;
        }
    }
    
    if (ThreatCount > 0)
    {
        FleeDirection /= ThreatCount;
        FleeDirection.Normalize();
    }
    else
    {
        // Random direction if no specific threats
        FleeDirection = FMath::VRand();
        FleeDirection.Z = 0.0f; // Keep on ground level
        FleeDirection.Normalize();
    }
    
    return FleeDirection;
}

void UNPC_CombatBehaviorSystem::BroadcastHelpCall()
{
    if (!GetOwner())
        return;
        
    // Find all allied combat systems in range
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        World,
        GetOwner()->GetActorLocation(),
        HelpCallRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>(),
        FoundActors
    );
    
    for (AActor* Actor : FoundActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (UNPC_CombatBehaviorSystem* AllySystem = Pawn->FindComponentByClass<UNPC_CombatBehaviorSystem>())
            {
                if (AllySystem != this && AllySystem->bCanRespondToHelp)
                {
                    AllySystem->RespondToHelpCall(Cast<APawn>(GetOwner()));
                }
            }
        }
    }
}

void UNPC_CombatBehaviorSystem::FindNearbyAllies()
{
    AlliedCombatants.Empty();
    
    if (!GetOwner())
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        World,
        GetOwner()->GetActorLocation(),
        HelpCallRange * 2.0f, // Larger range for finding allies
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>(),
        FoundActors
    );
    
    for (AActor* Actor : FoundActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (UNPC_CombatBehaviorSystem* AllySystem = Pawn->FindComponentByClass<UNPC_CombatBehaviorSystem>())
            {
                if (AllySystem != this)
                {
                    // Add logic here to determine if this is actually an ally
                    // For now, assume same species are allies
                    AlliedCombatants.Add(AllySystem);
                }
            }
        }
    }
}