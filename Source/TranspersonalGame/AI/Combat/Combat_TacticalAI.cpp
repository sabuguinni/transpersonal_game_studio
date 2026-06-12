#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize tactical parameters
    ThreatLevel = ECombat_ThreatLevel::Low;
    CurrentTactic = ECombat_TacticType::Patrol;
    EngagementRange = 1500.0f;
    FlankingDistance = 800.0f;
    RetreatThreshold = 0.3f;
    TacticalUpdateInterval = 2.0f;
    bIsInCombat = false;
    bCanFlanking = true;
    bCanAmbush = true;
    bCanRetreat = true;
    
    // Initialize arrays
    NearbyAllies.Empty();
    KnownEnemies.Empty();
    CoverPoints.Empty();
    FlankingRoutes.Empty();
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Start tactical evaluation timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TacticalUpdateTimer,
            this,
            &UCombat_TacticalAI::UpdateTacticalState,
            TacticalUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInCombat)
    {
        ExecuteCurrentTactic(DeltaTime);
        UpdateThreatAssessment();
    }
}

void UCombat_TacticalAI::UpdateTacticalState()
{
    if (!GetOwner()) return;
    
    // Scan for enemies and allies
    ScanForTargets();
    
    // Assess current threat level
    AssessThreatLevel();
    
    // Choose appropriate tactic
    SelectOptimalTactic();
    
    // Update formation if part of group
    UpdateFormationPosition();
}

void UCombat_TacticalAI::ScanForTargets()
{
    if (!GetWorld()) return;
    
    NearbyAllies.Empty();
    KnownEnemies.Empty();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all pawns in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner()) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= EngagementRange)
        {
            // Simple faction check based on actor name
            FString ActorName = Actor->GetName().ToLower();
            
            if (ActorName.Contains("player") || ActorName.Contains("character"))
            {
                KnownEnemies.Add(Actor);
            }
            else if (ActorName.Contains("dino") || ActorName.Contains("ai"))
            {
                NearbyAllies.Add(Actor);
            }
        }
    }
}

void UCombat_TacticalAI::AssessThreatLevel()
{
    int32 EnemyCount = KnownEnemies.Num();
    int32 AllyCount = NearbyAllies.Num();
    
    if (EnemyCount == 0)
    {
        ThreatLevel = ECombat_ThreatLevel::Low;
        bIsInCombat = false;
    }
    else if (EnemyCount <= AllyCount)
    {
        ThreatLevel = ECombat_ThreatLevel::Medium;
        bIsInCombat = true;
    }
    else
    {
        ThreatLevel = ECombat_ThreatLevel::High;
        bIsInCombat = true;
    }
}

void UCombat_TacticalAI::SelectOptimalTactic()
{
    switch (ThreatLevel)
    {
        case ECombat_ThreatLevel::Low:
            CurrentTactic = ECombat_TacticType::Patrol;
            break;
            
        case ECombat_ThreatLevel::Medium:
            if (NearbyAllies.Num() >= 2 && bCanFlanking)
            {
                CurrentTactic = ECombat_TacticType::Flanking;
            }
            else
            {
                CurrentTactic = ECombat_TacticType::DirectAssault;
            }
            break;
            
        case ECombat_ThreatLevel::High:
            if (GetOwner() && GetOwner()->GetClass()->GetName().Contains("Health"))
            {
                // Check health component if available
                CurrentTactic = ECombat_TacticType::Retreat;
            }
            else if (bCanAmbush && NearbyAllies.Num() >= 3)
            {
                CurrentTactic = ECombat_TacticType::Ambush;
            }
            else
            {
                CurrentTactic = ECombat_TacticType::DefensivePosition;
            }
            break;
    }
}

void UCombat_TacticalAI::ExecuteCurrentTactic(float DeltaTime)
{
    switch (CurrentTactic)
    {
        case ECombat_TacticType::Patrol:
            ExecutePatrol();
            break;
            
        case ECombat_TacticType::DirectAssault:
            ExecuteDirectAssault();
            break;
            
        case ECombat_TacticType::Flanking:
            ExecuteFlanking();
            break;
            
        case ECombat_TacticType::Ambush:
            ExecuteAmbush();
            break;
            
        case ECombat_TacticType::DefensivePosition:
            ExecuteDefensivePosition();
            break;
            
        case ECombat_TacticType::Retreat:
            ExecuteRetreat();
            break;
    }
}

void UCombat_TacticalAI::ExecutePatrol()
{
    // Basic patrol behavior - move to random nearby location
    if (GetOwner() && GetWorld())
    {
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector PatrolTarget = CurrentLocation + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        
        // Simple movement towards patrol point
        FVector Direction = (PatrolTarget - CurrentLocation).GetSafeNormal();
        GetOwner()->SetActorLocation(CurrentLocation + Direction * 50.0f * GetWorld()->GetDeltaSeconds());
    }
}

void UCombat_TacticalAI::ExecuteDirectAssault()
{
    if (KnownEnemies.Num() > 0 && GetOwner())
    {
        AActor* Target = KnownEnemies[0];
        FVector TargetLocation = Target->GetActorLocation();
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        
        // Move towards target
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        GetOwner()->SetActorLocation(CurrentLocation + Direction * 100.0f * GetWorld()->GetDeltaSeconds());
        
        // Face target
        FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        GetOwner()->SetActorRotation(LookRotation);
    }
}

void UCombat_TacticalAI::ExecuteFlanking()
{
    if (KnownEnemies.Num() > 0 && GetOwner())
    {
        AActor* Target = KnownEnemies[0];
        FVector TargetLocation = Target->GetActorLocation();
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        
        // Calculate flanking position (90 degrees to the side)
        FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
        FVector FlankPosition = TargetLocation + FlankDirection * FlankingDistance;
        
        // Move towards flanking position
        FVector Direction = (FlankPosition - CurrentLocation).GetSafeNormal();
        GetOwner()->SetActorLocation(CurrentLocation + Direction * 80.0f * GetWorld()->GetDeltaSeconds());
    }
}

void UCombat_TacticalAI::ExecuteAmbush()
{
    // Stay hidden and wait for optimal moment
    if (GetOwner())
    {
        // Reduce movement, prepare for surprise attack
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        
        // Check if enemy is in ambush range
        for (AActor* Enemy : KnownEnemies)
        {
            float Distance = FVector::Dist(CurrentLocation, Enemy->GetActorLocation());
            if (Distance <= 300.0f)
            {
                // Spring ambush - switch to direct assault
                CurrentTactic = ECombat_TacticType::DirectAssault;
                break;
            }
        }
    }
}

void UCombat_TacticalAI::ExecuteDefensivePosition()
{
    // Hold position and prepare for incoming threats
    if (GetOwner() && KnownEnemies.Num() > 0)
    {
        AActor* NearestThreat = KnownEnemies[0];
        FVector ThreatLocation = NearestThreat->GetActorLocation();
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        
        // Face the threat but don't advance
        FVector Direction = (ThreatLocation - CurrentLocation).GetSafeNormal();
        FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        GetOwner()->SetActorRotation(LookRotation);
        
        // Slight backing away if too close
        float Distance = FVector::Dist(CurrentLocation, ThreatLocation);
        if (Distance < 200.0f)
        {
            FVector RetreatDirection = -Direction;
            GetOwner()->SetActorLocation(CurrentLocation + RetreatDirection * 30.0f * GetWorld()->GetDeltaSeconds());
        }
    }
}

void UCombat_TacticalAI::ExecuteRetreat()
{
    if (GetOwner() && KnownEnemies.Num() > 0)
    {
        // Find direction away from all enemies
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector RetreatDirection = FVector::ZeroVector;
        
        for (AActor* Enemy : KnownEnemies)
        {
            FVector EnemyLocation = Enemy->GetActorLocation();
            FVector AwayFromEnemy = (CurrentLocation - EnemyLocation).GetSafeNormal();
            RetreatDirection += AwayFromEnemy;
        }
        
        RetreatDirection = RetreatDirection.GetSafeNormal();
        GetOwner()->SetActorLocation(CurrentLocation + RetreatDirection * 120.0f * GetWorld()->GetDeltaSeconds());
        
        // Check if far enough to stop retreating
        bool bSafeDistance = true;
        for (AActor* Enemy : KnownEnemies)
        {
            float Distance = FVector::Dist(CurrentLocation, Enemy->GetActorLocation());
            if (Distance < EngagementRange * 1.5f)
            {
                bSafeDistance = false;
                break;
            }
        }
        
        if (bSafeDistance)
        {
            CurrentTactic = ECombat_TacticType::Patrol;
            bIsInCombat = false;
        }
    }
}

void UCombat_TacticalAI::UpdateFormationPosition()
{
    // Basic formation logic - maintain distance from allies
    if (NearbyAllies.Num() > 0 && GetOwner())
    {
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector FormationCenter = FVector::ZeroVector;
        
        // Calculate center of formation
        for (AActor* Ally : NearbyAllies)
        {
            FormationCenter += Ally->GetActorLocation();
        }
        FormationCenter /= NearbyAllies.Num();
        
        // Maintain proper spacing
        float DesiredSpacing = 200.0f;
        for (AActor* Ally : NearbyAllies)
        {
            float Distance = FVector::Dist(CurrentLocation, Ally->GetActorLocation());
            if (Distance < DesiredSpacing)
            {
                FVector AwayDirection = (CurrentLocation - Ally->GetActorLocation()).GetSafeNormal();
                GetOwner()->SetActorLocation(CurrentLocation + AwayDirection * 20.0f * GetWorld()->GetDeltaSeconds());
            }
        }
    }
}

void UCombat_TacticalAI::UpdateThreatAssessment()
{
    // Re-evaluate threats based on current situation
    if (KnownEnemies.Num() > 0)
    {
        // Sort enemies by distance/threat level
        KnownEnemies.Sort([this](const AActor& A, const AActor& B)
        {
            float DistA = FVector::Dist(GetOwner()->GetActorLocation(), A.GetActorLocation());
            float DistB = FVector::Dist(GetOwner()->GetActorLocation(), B.GetActorLocation());
            return DistA < DistB;
        });
    }
}

void UCombat_TacticalAI::SetTacticalParameters(float NewEngagementRange, float NewFlankingDistance, float NewRetreatThreshold)
{
    EngagementRange = NewEngagementRange;
    FlankingDistance = NewFlankingDistance;
    RetreatThreshold = NewRetreatThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("Tactical parameters updated: Range=%.1f, Flanking=%.1f, Retreat=%.2f"), 
           EngagementRange, FlankingDistance, RetreatThreshold);
}

void UCombat_TacticalAI::ForceTacticChange(ECombat_TacticType NewTactic)
{
    CurrentTactic = NewTactic;
    UE_LOG(LogTemp, Log, TEXT("Tactic forced to: %d"), (int32)NewTactic);
}

ECombat_TacticType UCombat_TacticalAI::GetCurrentTactic() const
{
    return CurrentTactic;
}

ECombat_ThreatLevel UCombat_TacticalAI::GetThreatLevel() const
{
    return ThreatLevel;
}

bool UCombat_TacticalAI::IsInCombat() const
{
    return bIsInCombat;
}

int32 UCombat_TacticalAI::GetEnemyCount() const
{
    return KnownEnemies.Num();
}

int32 UCombat_TacticalAI::GetAllyCount() const
{
    return NearbyAllies.Num();
}