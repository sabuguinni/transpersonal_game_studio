#include "NPC_DinosaurTerritorialSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurTerritorialSystem::UNPC_DinosaurTerritorialSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds for performance
    
    // Initialize default territory bounds
    TerritoryBounds.CenterLocation = FVector::ZeroVector;
    TerritoryBounds.TerritoryRadius = 5000.0f;
    TerritoryBounds.PatrolRadius = 2500.0f;
    TerritoryBounds.DefenseRadius = 1000.0f;
    TerritoryBounds.bIsActiveTerritory = true;
    
    CurrentPatrolIndex = 0;
    bIsDefendingTerritory = false;
    ThreatDetectionRadius = 6000.0f;
    ThreatUpdateInterval = 2.0f;
    LastThreatUpdate = 0.0f;
}

void UNPC_DinosaurTerritorialSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Set territory center to owner's initial location if not set
    if (TerritoryBounds.CenterLocation.IsZero() && GetOwner())
    {
        TerritoryBounds.CenterLocation = GetOwner()->GetActorLocation();
    }
    
    // Generate initial patrol points
    GeneratePatrolPoints();
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_DinosaurTerritorialSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update threat detection periodically
    if (CurrentTime - LastThreatUpdate >= ThreatUpdateInterval)
    {
        DetectTerritorialThreats();
        CleanupOldThreats();
        LastThreatUpdate = CurrentTime;
    }
    
    // Debug visualization in development builds
    #if WITH_EDITOR
    if (GetOwner())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        
        // Draw territory bounds
        DrawDebugCircle(GetWorld(), TerritoryBounds.CenterLocation, TerritoryBounds.TerritoryRadius, 
                       32, FColor::Blue, false, -1.0f, 0, 50.0f, FVector(0,1,0), FVector(1,0,0));
        
        // Draw patrol area
        DrawDebugCircle(GetWorld(), TerritoryBounds.CenterLocation, TerritoryBounds.PatrolRadius, 
                       24, FColor::Green, false, -1.0f, 0, 30.0f, FVector(0,1,0), FVector(1,0,0));
        
        // Draw defense zone
        DrawDebugCircle(GetWorld(), TerritoryBounds.CenterLocation, TerritoryBounds.DefenseRadius, 
                       16, FColor::Red, false, -1.0f, 0, 20.0f, FVector(0,1,0), FVector(1,0,0));
    }
    #endif
}

void UNPC_DinosaurTerritorialSystem::SetTerritoryCenter(const FVector& NewCenter)
{
    TerritoryBounds.CenterLocation = NewCenter;
    GeneratePatrolPoints();
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Territory center set to %s"), 
           *NewCenter.ToString());
}

void UNPC_DinosaurTerritorialSystem::SetTerritoryRadius(float NewRadius)
{
    TerritoryBounds.TerritoryRadius = FMath::Max(NewRadius, 500.0f); // Minimum 500 units
    TerritoryBounds.PatrolRadius = FMath::Min(TerritoryBounds.PatrolRadius, TerritoryBounds.TerritoryRadius * 0.7f);
    TerritoryBounds.DefenseRadius = FMath::Min(TerritoryBounds.DefenseRadius, TerritoryBounds.PatrolRadius * 0.5f);
    
    GeneratePatrolPoints();
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Territory radius set to %f"), NewRadius);
}

bool UNPC_DinosaurTerritorialSystem::IsLocationInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, TerritoryBounds.CenterLocation);
    return Distance <= TerritoryBounds.TerritoryRadius;
}

bool UNPC_DinosaurTerritorialSystem::IsLocationInPatrolArea(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, TerritoryBounds.CenterLocation);
    return Distance <= TerritoryBounds.PatrolRadius;
}

bool UNPC_DinosaurTerritorialSystem::IsLocationInDefenseZone(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, TerritoryBounds.CenterLocation);
    return Distance <= TerritoryBounds.DefenseRadius;
}

void UNPC_DinosaurTerritorialSystem::DetectTerritorialThreats()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<AActor*> FoundActors;
    
    // Find all actors within threat detection radius
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), OwnerLocation);
        
        // Check if actor is within threat detection range and in our territory
        if (Distance <= ThreatDetectionRadius && IsLocationInTerritory(Actor->GetActorLocation()))
        {
            // Calculate threat level based on distance and actor type
            float ThreatLevel = CalculateThreatLevel(Actor);
            
            if (ThreatLevel > 0.1f) // Only consider significant threats
            {
                // Check if this threat already exists
                bool bThreatExists = false;
                for (FNPC_TerritorialThreat& ExistingThreat : ActiveThreats)
                {
                    if (ExistingThreat.ThreatActor == Actor)
                    {
                        // Update existing threat
                        ExistingThreat.ThreatLocation = Actor->GetActorLocation();
                        ExistingThreat.ThreatLevel = ThreatLevel;
                        ExistingThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
                        ExistingThreat.bIsActiveThreat = true;
                        bThreatExists = true;
                        break;
                    }
                }
                
                if (!bThreatExists)
                {
                    AddThreat(Actor, ThreatLevel);
                }
            }
        }
    }
}

void UNPC_DinosaurTerritorialSystem::AddThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor || !GetWorld())
    {
        return;
    }
    
    FNPC_TerritorialThreat NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLocation = ThreatActor->GetActorLocation();
    NewThreat.ThreatLevel = ThreatLevel;
    NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewThreat.bIsActiveThreat = true;
    
    ActiveThreats.Add(NewThreat);
    
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurTerritorialSystem: New threat detected - %s (Level: %f)"), 
           *ThreatActor->GetName(), ThreatLevel);
    
    // Start defending if threat level is high enough
    if (ThreatLevel > 0.5f && !bIsDefendingTerritory)
    {
        StartTerritorialDefense();
    }
}

void UNPC_DinosaurTerritorialSystem::RemoveThreat(AActor* ThreatActor)
{
    for (int32 i = ActiveThreats.Num() - 1; i >= 0; i--)
    {
        if (ActiveThreats[i].ThreatActor == ThreatActor)
        {
            ActiveThreats.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Threat removed - %s"), 
                   ThreatActor ? *ThreatActor->GetName() : TEXT("Unknown"));
            break;
        }
    }
    
    // Stop defending if no more significant threats
    if (ActiveThreats.Num() == 0 && bIsDefendingTerritory)
    {
        StopTerritorialDefense();
    }
}

FNPC_TerritorialThreat UNPC_DinosaurTerritorialSystem::GetHighestThreat() const
{
    FNPC_TerritorialThreat HighestThreat;
    float MaxThreatLevel = 0.0f;
    
    for (const FNPC_TerritorialThreat& Threat : ActiveThreats)
    {
        if (Threat.bIsActiveThreat && Threat.ThreatLevel > MaxThreatLevel)
        {
            HighestThreat = Threat;
            MaxThreatLevel = Threat.ThreatLevel;
        }
    }
    
    return HighestThreat;
}

FVector UNPC_DinosaurTerritorialSystem::GetDefensePosition(const FVector& ThreatLocation) const
{
    // Calculate a defensive position between territory center and threat
    FVector DirectionToThreat = (ThreatLocation - TerritoryBounds.CenterLocation).GetSafeNormal();
    FVector DefensePosition = TerritoryBounds.CenterLocation + (DirectionToThreat * TerritoryBounds.DefenseRadius * 0.8f);
    
    return DefensePosition;
}

bool UNPC_DinosaurTerritorialSystem::ShouldDefendTerritory() const
{
    // Defend if we have active threats with significant threat levels
    for (const FNPC_TerritorialThreat& Threat : ActiveThreats)
    {
        if (Threat.bIsActiveThreat && Threat.ThreatLevel > 0.3f)
        {
            return true;
        }
    }
    
    return false;
}

void UNPC_DinosaurTerritorialSystem::StartTerritorialDefense()
{
    bIsDefendingTerritory = true;
    
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurTerritorialSystem: Starting territorial defense for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_DinosaurTerritorialSystem::StopTerritorialDefense()
{
    bIsDefendingTerritory = false;
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Stopping territorial defense for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

FVector UNPC_DinosaurTerritorialSystem::GetNextPatrolPoint() const
{
    if (PatrolPoints.Num() == 0)
    {
        return TerritoryBounds.CenterLocation;
    }
    
    int32 SafeIndex = FMath::Clamp(CurrentPatrolIndex, 0, PatrolPoints.Num() - 1);
    return PatrolPoints[SafeIndex];
}

void UNPC_DinosaurTerritorialSystem::UpdatePatrolRoute()
{
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % FMath::Max(PatrolPoints.Num(), 1);
}

void UNPC_DinosaurTerritorialSystem::GeneratePatrolPoints()
{
    PatrolPoints.Empty();
    
    // Generate 8 patrol points around the territory perimeter
    const int32 NumPatrolPoints = 8;
    const float AngleStep = 360.0f / NumPatrolPoints;
    
    for (int32 i = 0; i < NumPatrolPoints; i++)
    {
        float Angle = FMath::DegreesToRadians(i * AngleStep);
        float X = TerritoryBounds.CenterLocation.X + (TerritoryBounds.PatrolRadius * FMath::Cos(Angle));
        float Y = TerritoryBounds.CenterLocation.Y + (TerritoryBounds.PatrolRadius * FMath::Sin(Angle));
        float Z = TerritoryBounds.CenterLocation.Z;
        
        PatrolPoints.Add(FVector(X, Y, Z));
    }
    
    CurrentPatrolIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Generated %d patrol points"), PatrolPoints.Num());
}

void UNPC_DinosaurTerritorialSystem::CleanupOldThreats()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    const float ThreatTimeoutDuration = 10.0f; // Remove threats not seen for 10 seconds
    
    for (int32 i = ActiveThreats.Num() - 1; i >= 0; i--)
    {
        FNPC_TerritorialThreat& Threat = ActiveThreats[i];
        
        // Remove threats that are too old or have invalid actors
        if (!Threat.ThreatActor || !IsValid(Threat.ThreatActor) || 
            (CurrentTime - Threat.LastSeenTime) > ThreatTimeoutDuration)
        {
            UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurTerritorialSystem: Cleaning up old threat"));
            ActiveThreats.RemoveAt(i);
        }
    }
}

float UNPC_DinosaurTerritorialSystem::CalculateThreatLevel(AActor* ThreatActor) const
{
    if (!ThreatActor || !GetOwner())
    {
        return 0.0f;
    }
    
    float Distance = FVector::Dist(ThreatActor->GetActorLocation(), GetOwner()->GetActorLocation());
    float BaseThreatLevel = 0.5f;
    
    // Higher threat for closer actors
    float DistanceFactor = 1.0f - FMath::Clamp(Distance / ThreatDetectionRadius, 0.0f, 1.0f);
    
    // Check if it's a player character (higher threat)
    if (ThreatActor->IsA<APawn>())
    {
        APawn* ThreatPawn = Cast<APawn>(ThreatActor);
        if (ThreatPawn && ThreatPawn->IsPlayerControlled())
        {
            BaseThreatLevel = 0.8f; // Player characters are high priority threats
        }
    }
    
    return BaseThreatLevel * DistanceFactor;
}