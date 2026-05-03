#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI updates
    
    CurrentState = ECombat_TacticalState::Idle;
    DinosaurType = ECombat_DinosaurType::Predator_Solo;
    CurrentTarget = nullptr;
    LastThreatCheckTime = 0.0f;
    bIsPackLeader = false;
    
    // Initialize tactical data with defaults
    TacticalData = FCombat_TacticalData();
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure based on dinosaur type
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::Predator_Pack:
            TacticalData.bUsesPackTactics = true;
            TacticalData.bCanFlank = true;
            TacticalData.PackSize = 3;
            break;
            
        case ECombat_DinosaurType::Apex_Predator:
            TacticalData.ThreatDetectionRange = 1500.0f;
            TacticalData.AttackRange = 300.0f;
            TacticalData.bUsesPackTactics = false;
            break;
            
        case ECombat_DinosaurType::Herbivore_Large:
            TacticalData.bCanFlank = false;
            TacticalData.RetreatThreshold = 0.7f;
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI initialized for %s"), 
           *GetOwner()->GetName());
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
        
    // Update threat detection periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastThreatCheckTime > 1.0f)
    {
        DetectThreats();
        LastThreatCheckTime = CurrentTime;
    }
    
    // Execute current tactical behavior
    ExecuteTacticalBehavior();
    
    // Update pack coordination if applicable
    if (TacticalData.bUsesPackTactics)
    {
        UpdatePackCoordination();
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_TacticalState OldState = CurrentState;
        CurrentState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s: Tactical state changed from %d to %d"), 
               *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
    }
}

void UCombat_TacticalAI::DetectThreats()
{
    if (!GetOwner() || !GetWorld())
        return;
        
    AActor* NearestThreat = FindNearestThreat();
    
    if (NearestThreat)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                     NearestThreat->GetActorLocation());
        
        if (Distance <= TacticalData.ThreatDetectionRange)
        {
            CurrentTarget = NearestThreat;
            
            if (CurrentState == ECombat_TacticalState::Idle)
            {
                SetTacticalState(ECombat_TacticalState::Alert);
            }
            
            // Escalate to hunt/territorial based on distance and type
            if (Distance <= TacticalData.AttackRange * 2.0f)
            {
                if (DinosaurType == ECombat_DinosaurType::Predator_Pack || 
                    DinosaurType == ECombat_DinosaurType::Predator_Solo)
                {
                    SetTacticalState(ECombat_TacticalState::Hunt);
                }
                else if (DinosaurType == ECombat_DinosaurType::Apex_Predator)
                {
                    SetTacticalState(ECombat_TacticalState::Territorial);
                }
            }
        }
    }
    else
    {
        // No threats detected, return to idle
        if (CurrentState != ECombat_TacticalState::Idle)
        {
            CurrentTarget = nullptr;
            SetTacticalState(ECombat_TacticalState::Idle);
        }
    }
}

void UCombat_TacticalAI::ExecuteTacticalBehavior()
{
    switch (CurrentState)
    {
        case ECombat_TacticalState::Hunt:
            ExecuteHuntBehavior();
            break;
            
        case ECombat_TacticalState::Territorial:
            ExecuteTerritorialBehavior();
            break;
            
        case ECombat_TacticalState::Retreat:
            ExecuteRetreatBehavior();
            break;
            
        case ECombat_TacticalState::PackCoord:
            CoordinateWithPack();
            break;
            
        default:
            break;
    }
    
    // Check if should retreat
    if (ShouldRetreat() && CurrentState != ECombat_TacticalState::Retreat)
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
    }
}

void UCombat_TacticalAI::ExecuteHuntBehavior()
{
    if (!CurrentTarget || !GetOwner())
        return;
        
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    
    if (IsInAttackRange(CurrentTarget))
    {
        SetTacticalState(ECombat_TacticalState::Attack);
        UE_LOG(LogTemp, Log, TEXT("%s: Entering attack range of %s"), 
               *GetOwner()->GetName(), *CurrentTarget->GetName());
    }
    else if (TacticalData.bCanFlank && TacticalData.bUsesPackTactics)
    {
        // Calculate flanking position
        FVector FlankPosition = CalculateFlankingPosition(CurrentTarget);
        
        // Move towards flanking position (would integrate with movement component)
        UE_LOG(LogTemp, Log, TEXT("%s: Moving to flank position"), 
               *GetOwner()->GetName());
    }
    else
    {
        // Direct approach
        FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
        UE_LOG(LogTemp, Log, TEXT("%s: Direct approach to target"), 
               *GetOwner()->GetName());
    }
}

void UCombat_TacticalAI::ExecuteTerritorialBehavior()
{
    if (!CurrentTarget || !GetOwner())
        return;
        
    // Territorial behavior - intimidation display, area control
    UE_LOG(LogTemp, Log, TEXT("%s: Displaying territorial behavior"), 
           *GetOwner()->GetName());
    
    // Check if target is backing down
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                 CurrentTarget->GetActorLocation());
    
    if (Distance > TacticalData.ThreatDetectionRange * 1.5f)
    {
        SetTacticalState(ECombat_TacticalState::Alert);
    }
}

void UCombat_TacticalAI::ExecuteRetreatBehavior()
{
    if (!GetOwner())
        return;
        
    FVector RetreatDirection = GetRetreatDirection();
    
    UE_LOG(LogTemp, Log, TEXT("%s: Retreating"), *GetOwner()->GetName());
    
    // Check if safe distance reached
    if (!CurrentTarget || 
        FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation()) 
        > TacticalData.ThreatDetectionRange * 2.0f)
    {
        SetTacticalState(ECombat_TacticalState::Idle);
    }
}

void UCombat_TacticalAI::CoordinateWithPack()
{
    if (!TacticalData.bUsesPackTactics)
        return;
        
    // Update pack member positions and states
    for (AActor* PackMember : PackMembers)
    {
        if (PackMember && PackMember != GetOwner())
        {
            UCombat_TacticalAI* MemberAI = PackMember->FindComponentByClass<UCombat_TacticalAI>();
            if (MemberAI)
            {
                // Synchronize pack behavior
                if (bIsPackLeader && CurrentState == ECombat_TacticalState::Hunt)
                {
                    MemberAI->SetTacticalState(ECombat_TacticalState::PackCoord);
                }
            }
        }
    }
}

FVector UCombat_TacticalAI::CalculateFlankingPosition(AActor* Target)
{
    if (!Target || !GetOwner())
        return GetOwner()->GetActorLocation();
        
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate perpendicular flanking position
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    
    return TargetLocation + (FlankDirection * TacticalData.FlankingDistance);
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    // Simple health-based retreat logic (would integrate with health component)
    // For now, use random chance based on retreat threshold
    return FMath::RandRange(0.0f, 1.0f) < TacticalData.RetreatThreshold * 0.1f;
}

void UCombat_TacticalAI::SetDinosaurType(ECombat_DinosaurType Type)
{
    DinosaurType = Type;
    
    // Reconfigure tactical data based on type
    switch (Type)
    {
        case ECombat_DinosaurType::Predator_Pack:
            TacticalData.bUsesPackTactics = true;
            TacticalData.bCanFlank = true;
            break;
            
        case ECombat_DinosaurType::Apex_Predator:
            TacticalData.ThreatDetectionRange = 1500.0f;
            TacticalData.AttackRange = 300.0f;
            break;
            
        default:
            break;
    }
}

void UCombat_TacticalAI::UpdateThreatAssessment()
{
    // Advanced threat assessment logic
    DetectThreats();
}

AActor* UCombat_TacticalAI::FindNearestThreat()
{
    if (!GetWorld())
        return nullptr;
        
    // Find player character as primary threat
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                     PlayerPawn->GetActorLocation());
        if (Distance <= TacticalData.ThreatDetectionRange)
        {
            return PlayerPawn;
        }
    }
    
    return nullptr;
}

bool UCombat_TacticalAI::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetOwner())
        return false;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                 Target->GetActorLocation());
    return Distance <= TacticalData.AttackRange;
}

FVector UCombat_TacticalAI::GetRetreatDirection() const
{
    if (!GetOwner())
        return FVector::ZeroVector;
        
    if (CurrentTarget)
    {
        // Retreat away from target
        FVector AwayFromTarget = (GetOwner()->GetActorLocation() - 
                                CurrentTarget->GetActorLocation()).GetSafeNormal();
        return AwayFromTarget;
    }
    
    // Default retreat direction
    return -GetOwner()->GetActorForwardVector();
}

void UCombat_TacticalAI::UpdatePackCoordination()
{
    if (!TacticalData.bUsesPackTactics)
        return;
        
    // Update pack member list and coordination
    CoordinateWithPack();
}