#include "NPC_TerritorialBehaviorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UNPC_TerritorialBehaviorSystem::UNPC_TerritorialBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    bIsPatrolling = false;
    bDefendingTerritory = false;
    CurrentThreat = nullptr;
    ThreatDetectionRange = 2000.0f;
    WarningDistance = 1500.0f;
    AttackDistance = 800.0f;
    
    // Initialize default territory
    TerritoryData.CenterLocation = FVector::ZeroVector;
    TerritoryData.Radius = 1000.0f;
    TerritoryData.TerritoryType = ENPC_TerritoryType::NeutralZone;
    TerritoryData.AggressionLevel = 0.5f;
    TerritoryData.bActiveTerritory = true;
}

void UNPC_TerritorialBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Set territory center to owner's location if not set
    if (TerritoryData.CenterLocation.IsZero() && GetOwner())
    {
        TerritoryData.CenterLocation = GetOwner()->GetActorLocation();
    }
    
    // Generate default patrol points if none exist
    if (TerritoryData.PatrolPoints.Num() == 0)
    {
        const int32 NumPatrolPoints = 6;
        const float PatrolRadius = TerritoryData.Radius * 0.7f;
        
        for (int32 i = 0; i < NumPatrolPoints; i++)
        {
            float Angle = (360.0f / NumPatrolPoints) * i;
            float RadianAngle = FMath::DegreesToRadians(Angle);
            
            FVector PatrolPoint = TerritoryData.CenterLocation;
            PatrolPoint.X += PatrolRadius * FMath::Cos(RadianAngle);
            PatrolPoint.Y += PatrolRadius * FMath::Sin(RadianAngle);
            
            TerritoryData.PatrolPoints.Add(PatrolPoint);
        }
        
        PatrolBehavior.PatrolRoute = TerritoryData.PatrolPoints;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Territorial Behavior System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_TerritorialBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!TerritoryData.bActiveTerritory || !GetOwner())
    {
        return;
    }
    
    // Update patrol behavior
    if (bIsPatrolling && !bDefendingTerritory)
    {
        UpdatePatrolBehavior(DeltaTime);
    }
    
    // Check for intruders
    CheckForIntruders();
    
    // Handle territory defense
    if (bDefendingTerritory && CurrentThreat)
    {
        if (!IsValid(CurrentThreat))
        {
            bDefendingTerritory = false;
            CurrentThreat = nullptr;
            ReturnToTerritory();
        }
        else
        {
            float DistanceToThreat = FVector::Dist(GetOwner()->GetActorLocation(), CurrentThreat->GetActorLocation());
            
            // If threat is too far, stop defending
            if (DistanceToThreat > ThreatDetectionRange * 1.5f)
            {
                bDefendingTerritory = false;
                CurrentThreat = nullptr;
                ReturnToTerritory();
            }
        }
    }
}

void UNPC_TerritorialBehaviorSystem::SetTerritoryZone(const FNPC_TerritoryZone& NewTerritory)
{
    TerritoryData = NewTerritory;
    PatrolBehavior.PatrolRoute = TerritoryData.PatrolPoints;
    PatrolBehavior.CurrentPatrolIndex = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Territory zone updated for %s - Type: %d, Radius: %f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           (int32)TerritoryData.TerritoryType,
           TerritoryData.Radius);
}

bool UNPC_TerritorialBehaviorSystem::IsInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(TerritoryData.CenterLocation, Location);
    return Distance <= TerritoryData.Radius;
}

float UNPC_TerritorialBehaviorSystem::GetDistanceFromTerritoryCenter(const FVector& Location) const
{
    return FVector::Dist(TerritoryData.CenterLocation, Location);
}

ENPC_TerritorialResponse UNPC_TerritorialBehaviorSystem::GetResponseToIntruder(AActor* Intruder) const
{
    if (!Intruder || !ShouldRespondToActor(Intruder))
    {
        return ENPC_TerritorialResponse::Ignore;
    }
    
    float DistanceToIntruder = FVector::Dist(GetOwner()->GetActorLocation(), Intruder->GetActorLocation());
    float AggressionModifier = CalculateAggressionModifier(Intruder);
    float EffectiveAggression = TerritoryData.AggressionLevel * AggressionModifier;
    
    // Distance-based response with aggression modifier
    if (DistanceToIntruder <= AttackDistance && EffectiveAggression > 0.7f)
    {
        return ENPC_TerritorialResponse::Aggressive;
    }
    else if (DistanceToIntruder <= WarningDistance && EffectiveAggression > 0.4f)
    {
        return ENPC_TerritorialResponse::Warning;
    }
    else if (DistanceToIntruder <= ThreatDetectionRange && EffectiveAggression > 0.2f)
    {
        return ENPC_TerritorialResponse::Investigate;
    }
    else if (EffectiveAggression < 0.3f && TerritoryData.TerritoryType != ENPC_TerritoryType::ApexPredator)
    {
        return ENPC_TerritorialResponse::Flee;
    }
    
    return ENPC_TerritorialResponse::Ignore;
}

void UNPC_TerritorialBehaviorSystem::StartPatrolling()
{
    if (PatrolBehavior.PatrolRoute.Num() > 0)
    {
        bIsPatrolling = true;
        PatrolBehavior.CurrentPatrolIndex = 0;
        PatrolBehavior.LastPatrolTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Warning, TEXT("%s started patrolling territory"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UNPC_TerritorialBehaviorSystem::StopPatrolling()
{
    bIsPatrolling = false;
    UE_LOG(LogTemp, Warning, TEXT("%s stopped patrolling"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

FVector UNPC_TerritorialBehaviorSystem::GetNextPatrolPoint()
{
    if (PatrolBehavior.PatrolRoute.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    
    int32 ValidIndex = FMath::Clamp(PatrolBehavior.CurrentPatrolIndex, 0, PatrolBehavior.PatrolRoute.Num() - 1);
    return PatrolBehavior.PatrolRoute[ValidIndex];
}

void UNPC_TerritorialBehaviorSystem::AdvancePatrolPoint()
{
    if (PatrolBehavior.PatrolRoute.Num() <= 1)
    {
        return;
    }
    
    if (PatrolBehavior.bReverseRoute)
    {
        PatrolBehavior.CurrentPatrolIndex--;
        if (PatrolBehavior.CurrentPatrolIndex < 0)
        {
            PatrolBehavior.CurrentPatrolIndex = 1;
            PatrolBehavior.bReverseRoute = false;
        }
    }
    else
    {
        PatrolBehavior.CurrentPatrolIndex++;
        if (PatrolBehavior.CurrentPatrolIndex >= PatrolBehavior.PatrolRoute.Num())
        {
            PatrolBehavior.CurrentPatrolIndex = PatrolBehavior.PatrolRoute.Num() - 2;
            PatrolBehavior.bReverseRoute = true;
        }
    }
    
    PatrolBehavior.LastPatrolTime = GetWorld()->GetTimeSeconds();
}

void UNPC_TerritorialBehaviorSystem::DefendTerritory(AActor* Threat)
{
    if (!Threat)
    {
        return;
    }
    
    CurrentThreat = Threat;
    bDefendingTerritory = true;
    bIsPatrolling = false;
    
    UE_LOG(LogTemp, Warning, TEXT("%s defending territory against %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *Threat->GetName());
}

void UNPC_TerritorialBehaviorSystem::IssueWarning(AActor* Target)
{
    if (!Target)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s issuing warning to %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           *Target->GetName());
    
    // TODO: Add visual/audio warning effects
    // TODO: Notify pack members of potential threat
}

void UNPC_TerritorialBehaviorSystem::ReturnToTerritory()
{
    if (!GetOwner())
    {
        return;
    }
    
    float DistanceFromCenter = GetDistanceFromTerritoryCenter(GetOwner()->GetActorLocation());
    
    if (DistanceFromCenter > TerritoryData.Radius)
    {
        // Move back towards territory center
        UE_LOG(LogTemp, Warning, TEXT("%s returning to territory"), *GetOwner()->GetName());
        
        // Resume patrolling once back in territory
        if (!bIsPatrolling && PatrolBehavior.PatrolRoute.Num() > 0)
        {
            StartPatrolling();
        }
    }
}

void UNPC_TerritorialBehaviorSystem::ShareTerritoryWith(AActor* PackMember)
{
    if (PackMember && !TerritoryData.TerritoryOwners.Contains(PackMember))
    {
        TerritoryData.TerritoryOwners.Add(PackMember);
        UE_LOG(LogTemp, Warning, TEXT("%s sharing territory with %s"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               *PackMember->GetName());
    }
}

void UNPC_TerritorialBehaviorSystem::RemoveFromTerritory(AActor* FormerMember)
{
    if (FormerMember)
    {
        TerritoryData.TerritoryOwners.Remove(FormerMember);
        UE_LOG(LogTemp, Warning, TEXT("%s removed from territory"), *FormerMember->GetName());
    }
}

bool UNPC_TerritorialBehaviorSystem::IsPackMember(AActor* Actor) const
{
    return Actor && TerritoryData.TerritoryOwners.Contains(Actor);
}

void UNPC_TerritorialBehaviorSystem::UpdatePatrolBehavior(float DeltaTime)
{
    if (!GetOwner() || PatrolBehavior.PatrolRoute.Num() == 0)
    {
        return;
    }
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector TargetPatrolPoint = GetNextPatrolPoint();
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPatrolPoint);
    
    // Check if reached patrol point
    if (DistanceToTarget < 100.0f) // 1 meter tolerance
    {
        float TimeSinceLastAdvance = GetWorld()->GetTimeSeconds() - PatrolBehavior.LastPatrolTime;
        
        if (TimeSinceLastAdvance >= PatrolBehavior.WaitTimeAtPoint)
        {
            AdvancePatrolPoint();
        }
    }
}

void UNPC_TerritorialBehaviorSystem::CheckForIntruders()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Get all actors in detection range
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        ThreatDetectionRange,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APawn::StaticClass(),
        TArray<AActor*>{GetOwner()}, // Ignore self
        FoundActors
    );
    
    for (AActor* Actor : FoundActors)
    {
        if (ShouldRespondToActor(Actor))
        {
            ENPC_TerritorialResponse Response = GetResponseToIntruder(Actor);
            HandleTerritorialResponse(Actor, Response);
        }
    }
}

void UNPC_TerritorialBehaviorSystem::HandleTerritorialResponse(AActor* Intruder, ENPC_TerritorialResponse Response)
{
    if (!Intruder)
    {
        return;
    }
    
    switch (Response)
    {
        case ENPC_TerritorialResponse::Aggressive:
            DefendTerritory(Intruder);
            break;
            
        case ENPC_TerritorialResponse::Warning:
            IssueWarning(Intruder);
            break;
            
        case ENPC_TerritorialResponse::Investigate:
            // TODO: Implement investigation behavior
            UE_LOG(LogTemp, Log, TEXT("%s investigating %s"), 
                   GetOwner()->GetName(), *Intruder->GetName());
            break;
            
        case ENPC_TerritorialResponse::Flee:
            // TODO: Implement flee behavior
            UE_LOG(LogTemp, Log, TEXT("%s fleeing from %s"), 
                   GetOwner()->GetName(), *Intruder->GetName());
            break;
            
        default:
            break;
    }
}

bool UNPC_TerritorialBehaviorSystem::ShouldRespondToActor(AActor* Actor) const
{
    if (!Actor || Actor == GetOwner())
    {
        return false;
    }
    
    // Don't respond to pack members
    if (IsPackMember(Actor))
    {
        return false;
    }
    
    // Only respond to pawns (players, other NPCs)
    return Actor->IsA<APawn>();
}

float UNPC_TerritorialBehaviorSystem::CalculateAggressionModifier(AActor* Target) const
{
    if (!Target)
    {
        return 1.0f;
    }
    
    float Modifier = 1.0f;
    
    // Territory type affects aggression
    switch (TerritoryData.TerritoryType)
    {
        case ENPC_TerritoryType::ApexPredator:
            Modifier *= 1.5f; // Very aggressive
            break;
        case ENPC_TerritoryType::PackHunter:
            Modifier *= 1.2f; // Moderately aggressive
            break;
        case ENPC_TerritoryType::Herbivore:
            Modifier *= 0.6f; // Less aggressive, more defensive
            break;
        case ENPC_TerritoryType::WaterSource:
            Modifier *= 0.8f; // Protective but not overly aggressive
            break;
        default:
            break;
    }
    
    // Distance from territory center affects aggression
    float DistanceFromCenter = GetDistanceFromTerritoryCenter(Target->GetActorLocation());
    float NormalizedDistance = FMath::Clamp(DistanceFromCenter / TerritoryData.Radius, 0.0f, 1.0f);
    
    // More aggressive closer to territory center
    Modifier *= (1.0f - (NormalizedDistance * 0.5f));
    
    return FMath::Clamp(Modifier, 0.1f, 2.0f);
}