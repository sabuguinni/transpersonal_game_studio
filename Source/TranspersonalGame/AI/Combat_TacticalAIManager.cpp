#include "Combat_TacticalAIManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_TacticalAIManager::ACombat_TacticalAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize tactical parameters
    TacticalUpdateInterval = 2.0f;
    MaxEngagementRange = 3000.0f;
    MaxActiveHunters = 8;
    LastTacticalUpdate = 0.0f;
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ACombat_TacticalAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTacticalZones();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Tactical AI Manager initialized with %d zones"), TacticalZones.Num());
}

void ACombat_TacticalAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update tactical situation at intervals
    if (GetWorld()->GetTimeSeconds() - LastTacticalUpdate > TacticalUpdateInterval)
    {
        UpdateTacticalSituation();
        ProcessCombatDecisions();
        LastTacticalUpdate = GetWorld()->GetTimeSeconds();
    }
}

void ACombat_TacticalAIManager::InitializeTacticalZones()
{
    TacticalZones.Empty();
    
    // Create hunting grounds zone
    FCombat_TacticalZone HuntingZone;
    HuntingZone.Position = FVector(3000, 1000, 100);
    HuntingZone.Radius = 1500.0f;
    HuntingZone.PreferredState = ECombat_TacticalState::Hunt;
    HuntingZone.MaxOccupants = 6;
    HuntingZone.ThreatLevel = 0.3f;
    TacticalZones.Add(HuntingZone);
    
    // Create ambush point zone
    FCombat_TacticalZone AmbushZone;
    AmbushZone.Position = FVector(-2000, 2000, 150);
    AmbushZone.Radius = 800.0f;
    AmbushZone.PreferredState = ECombat_TacticalState::Ambush;
    AmbushZone.MaxOccupants = 4;
    AmbushZone.ThreatLevel = 0.7f;
    TacticalZones.Add(AmbushZone);
    
    // Create territory defense zone
    FCombat_TacticalZone DefenseZone;
    DefenseZone.Position = FVector(0, -3000, 120);
    DefenseZone.Radius = 2000.0f;
    DefenseZone.PreferredState = ECombat_TacticalState::Defend;
    DefenseZone.MaxOccupants = 10;
    DefenseZone.ThreatLevel = 0.9f;
    TacticalZones.Add(DefenseZone);
    
    // Create patrol zone
    FCombat_TacticalZone PatrolZone;
    PatrolZone.Position = FVector(1000, -1000, 80);
    PatrolZone.Radius = 1200.0f;
    PatrolZone.PreferredState = ECombat_TacticalState::Patrol;
    PatrolZone.MaxOccupants = 3;
    PatrolZone.ThreatLevel = 0.2f;
    TacticalZones.Add(PatrolZone);
}

ECombat_TacticalState ACombat_TacticalAIManager::GetOptimalTacticalState(const FVector& Position, float ThreatLevel)
{
    FCombat_TacticalZone* NearestZone = FindNearestTacticalZone(Position);
    
    if (!NearestZone)
    {
        return ECombat_TacticalState::Patrol;
    }
    
    // Adjust state based on threat level
    if (ThreatLevel > 0.8f)
    {
        return ECombat_TacticalState::Flee;
    }
    else if (ThreatLevel > 0.6f && NearestZone->PreferredState == ECombat_TacticalState::Hunt)
    {
        return ECombat_TacticalState::Defend;
    }
    else if (ThreatLevel < 0.3f && ActiveCombatants.Num() < MaxActiveHunters)
    {
        return ECombat_TacticalState::Hunt;
    }
    
    return NearestZone->PreferredState;
}

FVector ACombat_TacticalAIManager::GetBestAmbushPosition(const FVector& TargetPosition)
{
    FVector BestPosition = TargetPosition;
    float BestScore = 0.0f;
    
    for (const FCombat_TacticalZone& Zone : TacticalZones)
    {
        if (Zone.PreferredState == ECombat_TacticalState::Ambush)
        {
            float Distance = FVector::Dist(Zone.Position, TargetPosition);
            float Score = (Zone.ThreatLevel * 100.0f) / (Distance + 1.0f);
            
            if (Score > BestScore)
            {
                BestScore = Score;
                BestPosition = Zone.Position;
            }
        }
    }
    
    return BestPosition;
}

bool ACombat_TacticalAIManager::CanEngageTarget(const FVector& HunterPosition, const FVector& TargetPosition)
{
    float Distance = FVector::Dist(HunterPosition, TargetPosition);
    
    if (Distance > MaxEngagementRange)
    {
        return false;
    }
    
    if (ActiveCombatants.Num() >= MaxActiveHunters)
    {
        return false;
    }
    
    return true;
}

void ACombat_TacticalAIManager::RegisterCombatant(AActor* Combatant, bool bIsHostile)
{
    if (!Combatant)
    {
        return;
    }
    
    if (bIsHostile)
    {
        HostileTargets.AddUnique(Combatant);
    }
    else
    {
        ActiveCombatants.AddUnique(Combatant);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered combatant: %s (Hostile: %s)"), 
           *Combatant->GetName(), bIsHostile ? TEXT("Yes") : TEXT("No"));
}

void ACombat_TacticalAIManager::UpdateTacticalSituation()
{
    // Clean up invalid actors
    ActiveCombatants.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    HostileTargets.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    // Debug draw tactical zones in development builds
    if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
    {
        for (const FCombat_TacticalZone& Zone : TacticalZones)
        {
            FColor ZoneColor = FColor::Green;
            switch (Zone.PreferredState)
            {
                case ECombat_TacticalState::Hunt:
                    ZoneColor = FColor::Red;
                    break;
                case ECombat_TacticalState::Ambush:
                    ZoneColor = FColor::Orange;
                    break;
                case ECombat_TacticalState::Defend:
                    ZoneColor = FColor::Blue;
                    break;
                case ECombat_TacticalState::Flee:
                    ZoneColor = FColor::Yellow;
                    break;
                default:
                    ZoneColor = FColor::Green;
                    break;
            }
            
            DrawDebugSphere(GetWorld(), Zone.Position, Zone.Radius, 12, ZoneColor, false, TacticalUpdateInterval + 0.1f);
        }
    }
}

void ACombat_TacticalAIManager::ProcessCombatDecisions()
{
    // Process tactical decisions for each active combatant
    for (AActor* Combatant : ActiveCombatants)
    {
        if (!IsValid(Combatant))
        {
            continue;
        }
        
        FVector CombatantPos = Combatant->GetActorLocation();
        float LocalThreatLevel = 0.5f;
        
        // Calculate threat level based on nearby hostiles
        for (AActor* Hostile : HostileTargets)
        {
            if (IsValid(Hostile))
            {
                float Distance = FVector::Dist(CombatantPos, Hostile->GetActorLocation());
                if (Distance < MaxEngagementRange)
                {
                    LocalThreatLevel += (MaxEngagementRange - Distance) / MaxEngagementRange * 0.3f;
                }
            }
        }
        
        LocalThreatLevel = FMath::Clamp(LocalThreatLevel, 0.0f, 1.0f);
        
        // Get optimal tactical state
        ECombat_TacticalState OptimalState = GetOptimalTacticalState(CombatantPos, LocalThreatLevel);
        
        // Log tactical decisions for debugging
        UE_LOG(LogTemp, Verbose, TEXT("Combatant %s: Threat=%.2f, State=%d"), 
               *Combatant->GetName(), LocalThreatLevel, (int32)OptimalState);
    }
}

FCombat_TacticalZone* ACombat_TacticalAIManager::FindNearestTacticalZone(const FVector& Position)
{
    FCombat_TacticalZone* NearestZone = nullptr;
    float NearestDistance = MAX_FLT;
    
    for (FCombat_TacticalZone& Zone : TacticalZones)
    {
        float Distance = FVector::Dist(Zone.Position, Position);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestZone = &Zone;
        }
    }
    
    return NearestZone;
}