#include "Combat_DinosaurCombatAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UCombat_DinosaurCombatAI::UCombat_DinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI updates
    
    CurrentCombatState = ECombat_DinosaurCombatState::Passive;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    bIsPackLeader = false;
    StateTimer = 0.0f;
    ThreatScanInterval = 1.0f;
    LastThreatScan = 0.0f;
    
    PreferredAttackPattern = ECombat_AttackPattern::DirectCharge;
    
    // Default combat stats for medium predator
    CombatStats.AttackDamage = 50.0f;
    CombatStats.AttackRange = 300.0f;
    CombatStats.DetectionRadius = 1000.0f;
    CombatStats.MovementSpeed = 400.0f;
    CombatStats.Aggressiveness = 0.7f;
    CombatStats.TerritorialRadius = 2000.0f;
    CombatStats.bIsPackHunter = false;
    CombatStats.PackSize = 1;
}

void UCombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    
    TerritoryCenter = GetOwner()->GetActorLocation();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI initialized for %s at location %s"), 
           *GetOwner()->GetName(), 
           *TerritoryCenter.ToString());
}

void UCombat_DinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
    
    // Process threat detection at intervals
    if (GetWorld()->GetTimeSeconds() - LastThreatScan > ThreatScanInterval)
    {
        ProcessThreatDetection();
        LastThreatScan = GetWorld()->GetTimeSeconds();
    }
    
    // Update combat behavior based on current state
    UpdateCombatBehavior(DeltaTime);
    
    // Update pack coordination if part of a pack
    if (CombatStats.bIsPackHunter && PackMembers.Num() > 0)
    {
        UpdatePackCoordination();
    }
}

void UCombat_DinosaurCombatAI::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombat_DinosaurCombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("%s combat state changed from %d to %d"), 
               *GetOwner()->GetName(), 
               (int32)PreviousState, 
               (int32)NewState);
        
        // State entry logic
        switch (NewState)
        {
        case ECombat_DinosaurCombatState::Alert:
            // Increase scan frequency when alert
            ThreatScanInterval = 0.5f;
            break;
        case ECombat_DinosaurCombatState::Hunting:
            ThreatScanInterval = 0.3f;
            break;
        case ECombat_DinosaurCombatState::Attacking:
            ThreatScanInterval = 0.2f;
            break;
        case ECombat_DinosaurCombatState::Retreating:
            CurrentTarget = nullptr;
            ThreatScanInterval = 1.0f;
            break;
        default:
            ThreatScanInterval = 1.0f;
            break;
        }
    }
}

void UCombat_DinosaurCombatAI::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            UE_LOG(LogTemp, Log, TEXT("%s acquired new target: %s"), 
                   *GetOwner()->GetName(), 
                   *NewTarget->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("%s lost target"), *GetOwner()->GetName());
        }
    }
}

void UCombat_DinosaurCombatAI::ExecuteAttack()
{
    if (!CurrentTarget || !IsInAttackRange(CurrentTarget))
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float AttackCooldown = 2.0f; // 2 second cooldown between attacks
    
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return;
    }
    
    LastAttackTime = CurrentTime;
    
    // Apply damage to target if it's a character
    if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
    {
        // In a real implementation, this would use a proper damage system
        UE_LOG(LogTemp, Warning, TEXT("%s attacks %s for %f damage!"), 
               *GetOwner()->GetName(), 
               *CurrentTarget->GetName(), 
               CombatStats.AttackDamage);
        
        // Broadcast pack alert if successful attack
        if (CombatStats.bIsPackHunter)
        {
            BroadcastPackAlert(CurrentTarget);
        }
    }
}

void UCombat_DinosaurCombatAI::StartHunting(AActor* Target)
{
    if (!Target || !IsValidTarget(Target))
    {
        return;
    }
    
    SetTarget(Target);
    SetCombatState(ECombat_DinosaurCombatState::Hunting);
    
    // If pack hunter, coordinate with pack
    if (CombatStats.bIsPackHunter && bIsPackLeader)
    {
        CoordinatePackAttack(Target);
    }
}

void UCombat_DinosaurCombatAI::InitiateRetreat()
{
    SetCombatState(ECombat_DinosaurCombatState::Retreating);
    SetTarget(nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("%s initiating retreat!"), *GetOwner()->GetName());
}

void UCombat_DinosaurCombatAI::DefendTerritory()
{
    SetCombatState(ECombat_DinosaurCombatState::Territorial);
    
    // Move back towards territory center
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceFromTerritory = FVector::Dist(OwnerLocation, TerritoryCenter);
    
    if (DistanceFromTerritory > CombatStats.TerritorialRadius)
    {
        // Move back to territory
        FVector DirectionToTerritory = (TerritoryCenter - OwnerLocation).GetSafeNormal();
        FVector TargetLocation = OwnerLocation + DirectionToTerritory * CombatStats.MovementSpeed * 0.1f;
        
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            // In a real implementation, this would use proper movement components
            UE_LOG(LogTemp, Log, TEXT("%s returning to territory"), *GetOwner()->GetName());
        }
    }
}

void UCombat_DinosaurCombatAI::JoinPack(TArray<UCombat_DinosaurCombatAI*> NewPackMembers)
{
    PackMembers = NewPackMembers;
    CombatStats.bIsPackHunter = true;
    
    // First member becomes pack leader
    bIsPackLeader = (PackMembers.Num() > 0 && PackMembers[0] == this);
    
    UE_LOG(LogTemp, Log, TEXT("%s joined pack of %d members (Leader: %s)"), 
           *GetOwner()->GetName(), 
           PackMembers.Num(), 
           bIsPackLeader ? TEXT("Yes") : TEXT("No"));
}

void UCombat_DinosaurCombatAI::CoordinatePackAttack(AActor* Target)
{
    if (!bIsPackLeader || !Target)
    {
        return;
    }
    
    // Assign different attack patterns to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i] && PackMembers[i] != this)
        {
            // Assign flanking positions
            PackMembers[i]->SetTarget(Target);
            PackMembers[i]->SetCombatState(ECombat_DinosaurCombatState::Hunting);
            
            // Vary attack patterns for coordination
            if (i % 2 == 0)
            {
                PackMembers[i]->PreferredAttackPattern = ECombat_AttackPattern::PackFlank;
            }
            else
            {
                PackMembers[i]->PreferredAttackPattern = ECombat_AttackPattern::CircleStalk;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pack leader %s coordinating attack on %s"), 
           *GetOwner()->GetName(), 
           *Target->GetName());
}

float UCombat_DinosaurCombatAI::CalculateThreatLevel(AActor* PotentialThreat)
{
    if (!PotentialThreat)
    {
        return 0.0f;
    }
    
    float ThreatLevel = 0.0f;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());
    
    // Base threat based on distance (closer = more threatening)
    if (Distance < CombatStats.AttackRange)
    {
        ThreatLevel += 0.8f;
    }
    else if (Distance < CombatStats.DetectionRadius * 0.5f)
    {
        ThreatLevel += 0.5f;
    }
    else if (Distance < CombatStats.DetectionRadius)
    {
        ThreatLevel += 0.2f;
    }
    
    // Check if it's a feared class
    for (TSubclassOf<APawn> FearClass : FearClasses)
    {
        if (PotentialThreat->IsA(FearClass))
        {
            ThreatLevel += 0.6f;
            break;
        }
    }
    
    // Check if it's preferred prey
    if (PreferredPreyClass && PotentialThreat->IsA(PreferredPreyClass))
    {
        ThreatLevel -= 0.3f; // Prey is less threatening, more like opportunity
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

TArray<AActor*> UCombat_DinosaurCombatAI::ScanForThreats()
{
    TArray<AActor*> Threats;
    TArray<AActor*> AllActors;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : AllActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= CombatStats.DetectionRadius)
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            if (ThreatLevel > 0.3f) // Minimum threat threshold
            {
                Threats.Add(Actor);
            }
        }
    }
    
    return Threats;
}

void UCombat_DinosaurCombatAI::UpdateCombatBehavior(float DeltaTime)
{
    switch (CurrentCombatState)
    {
    case ECombat_DinosaurCombatState::Passive:
        // Randomly patrol or rest
        if (StateTimer > 10.0f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
        {
            SetCombatState(ECombat_DinosaurCombatState::Alert);
        }
        break;
        
    case ECombat_DinosaurCombatState::Alert:
        // Look for threats or return to passive
        if (StateTimer > 5.0f)
        {
            TArray<AActor*> Threats = ScanForThreats();
            if (Threats.Num() > 0)
            {
                AActor* HighestThreat = Threats[0];
                float HighestThreatLevel = CalculateThreatLevel(HighestThreat);
                
                for (AActor* Threat : Threats)
                {
                    float ThreatLevel = CalculateThreatLevel(Threat);
                    if (ThreatLevel > HighestThreatLevel)
                    {
                        HighestThreat = Threat;
                        HighestThreatLevel = ThreatLevel;
                    }
                }
                
                if (HighestThreatLevel > 0.7f)
                {
                    StartHunting(HighestThreat);
                }
            }
            else
            {
                SetCombatState(ECombat_DinosaurCombatState::Passive);
            }
        }
        break;
        
    case ECombat_DinosaurCombatState::Hunting:
        if (CurrentTarget)
        {
            if (IsInAttackRange(CurrentTarget))
            {
                SetCombatState(ECombat_DinosaurCombatState::Attacking);
            }
            else
            {
                MoveTowardsTarget(DeltaTime);
            }
        }
        else
        {
            SetCombatState(ECombat_DinosaurCombatState::Alert);
        }
        break;
        
    case ECombat_DinosaurCombatState::Attacking:
        if (CurrentTarget && IsInAttackRange(CurrentTarget))
        {
            ExecuteAttackPattern();
        }
        else
        {
            SetCombatState(ECombat_DinosaurCombatState::Hunting);
        }
        break;
        
    case ECombat_DinosaurCombatState::Retreating:
        // Move away from threats
        if (StateTimer > 8.0f)
        {
            SetCombatState(ECombat_DinosaurCombatState::Alert);
        }
        break;
        
    case ECombat_DinosaurCombatState::Territorial:
        DefendTerritory();
        if (StateTimer > 15.0f)
        {
            SetCombatState(ECombat_DinosaurCombatState::Passive);
        }
        break;
    }
}

void UCombat_DinosaurCombatAI::ProcessThreatDetection()
{
    TArray<AActor*> Threats = ScanForThreats();
    
    if (Threats.Num() > 0 && CurrentCombatState == ECombat_DinosaurCombatState::Passive)
    {
        SetCombatState(ECombat_DinosaurCombatState::Alert);
    }
}

void UCombat_DinosaurCombatAI::ExecuteAttackPattern()
{
    switch (PreferredAttackPattern)
    {
    case ECombat_AttackPattern::DirectCharge:
        ExecuteAttack();
        break;
    case ECombat_AttackPattern::CircleStalk:
        // Circle around target before attacking
        if (StateTimer > 2.0f)
        {
            ExecuteAttack();
        }
        break;
    case ECombat_AttackPattern::PackFlank:
        ExecuteFlankingManeuver();
        break;
    default:
        ExecuteAttack();
        break;
    }
}

void UCombat_DinosaurCombatAI::UpdatePackCoordination()
{
    // Remove invalid pack members
    PackMembers.RemoveAll([](UCombat_DinosaurCombatAI* Member) {
        return !Member || !IsValid(Member->GetOwner());
    });
    
    // Update pack leader status
    if (PackMembers.Num() == 0)
    {
        CombatStats.bIsPackHunter = false;
        bIsPackLeader = false;
    }
}

bool UCombat_DinosaurCombatAI::IsInAttackRange(AActor* Target)
{
    if (!Target)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= CombatStats.AttackRange;
}

bool UCombat_DinosaurCombatAI::IsValidTarget(AActor* PotentialTarget)
{
    if (!PotentialTarget || PotentialTarget == GetOwner())
    {
        return false;
    }
    
    // Check if target is within detection range
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialTarget->GetActorLocation());
    if (Distance > CombatStats.DetectionRadius)
    {
        return false;
    }
    
    // Check if target is a pawn
    if (!Cast<APawn>(PotentialTarget))
    {
        return false;
    }
    
    return true;
}

void UCombat_DinosaurCombatAI::MoveTowardsTarget(float DeltaTime)
{
    if (!CurrentTarget)
    {
        return;
    }
    
    LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    
    // In a real implementation, this would use proper movement components
    // For now, just log the movement intention
    UE_LOG(LogTemp, VeryVerbose, TEXT("%s moving towards target %s"), 
           *GetOwner()->GetName(), 
           *CurrentTarget->GetName());
}

void UCombat_DinosaurCombatAI::ExecuteFlankingManeuver()
{
    if (!CurrentTarget)
    {
        return;
    }
    
    // Calculate flanking position
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Move to side of target
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector FlankPosition = TargetLocation + FlankDirection * CombatStats.AttackRange * 0.8f;
    
    UE_LOG(LogTemp, Log, TEXT("%s executing flanking maneuver"), *GetOwner()->GetName());
    
    // Attack when in position
    if (FVector::Dist(OwnerLocation, FlankPosition) < 100.0f)
    {
        ExecuteAttack();
    }
}

void UCombat_DinosaurCombatAI::BroadcastPackAlert(AActor* Threat)
{
    for (UCombat_DinosaurCombatAI* PackMember : PackMembers)
    {
        if (PackMember && PackMember != this)
        {
            PackMember->SetTarget(Threat);
            PackMember->SetCombatState(ECombat_DinosaurCombatState::Alert);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s broadcasting pack alert about %s"), 
           *GetOwner()->GetName(), 
           Threat ? *Threat->GetName() : TEXT("Unknown"));
}