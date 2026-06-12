#include "Combat_CombatManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "DrawDebugHelpers.h"

ACombat_CombatManager::ACombat_CombatManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;
    
    // Set default values
    GlobalThreatLevel = 0.0f;
    CombatUpdateInterval = 0.5f;
    MaxCombatRange = 2000.0f;
    MaxSimultaneousEncounters = 3;
    LastUpdateTime = 0.0f;
    PlayerCharacter = nullptr;
}

void ACombat_CombatManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCombatSystem();
    
    // Find player character
    if (UWorld* World = GetWorld())
    {
        if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0))
        {
            PlayerCharacter = Player;
            UE_LOG(LogTemp, Warning, TEXT("Combat Manager: Player character found"));
        }
    }
}

void ACombat_CombatManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= CombatUpdateInterval)
    {
        UpdateTacticalSituation();
        ProcessCombatAI(DeltaTime);
        LastUpdateTime = 0.0f;
    }
}

void ACombat_CombatManager::InitializeCombatSystem()
{
    ActiveEncounters.Empty();
    TrackedEnemies.Empty();
    GlobalThreatLevel = 0.0f;
    
    // Initialize tactical data
    CurrentTacticalSituation.PlayerThreatLevel = 0.0f;
    CurrentTacticalSituation.bPlayerInCombat = false;
    CurrentTacticalSituation.TimeSinceLastSighting = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Combat System initialized"));
}

void ACombat_CombatManager::RegisterEnemy(AActor* Enemy, float ThreatLevel)
{
    if (Enemy && !TrackedEnemies.Contains(Enemy))
    {
        TrackedEnemies.Add(Enemy);
        GlobalThreatLevel += ThreatLevel * 0.1f; // Each enemy adds to global threat
        
        UE_LOG(LogTemp, Warning, TEXT("Enemy registered: %s, Threat: %f"), 
               *Enemy->GetName(), ThreatLevel);
    }
}

void ACombat_CombatManager::UnregisterEnemy(AActor* Enemy)
{
    if (Enemy && TrackedEnemies.Contains(Enemy))
    {
        TrackedEnemies.Remove(Enemy);
        GlobalThreatLevel = FMath::Max(0.0f, GlobalThreatLevel - 0.1f);
        
        UE_LOG(LogTemp, Warning, TEXT("Enemy unregistered: %s"), *Enemy->GetName());
    }
}

void ACombat_CombatManager::StartCombatEncounter(const FVector& Location, const TArray<AActor*>& Enemies)
{
    if (ActiveEncounters.Num() >= MaxSimultaneousEncounters)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum encounters reached, cannot start new encounter"));
        return;
    }
    
    FCombat_CombatEncounter NewEncounter;
    NewEncounter.Location = Location;
    NewEncounter.Enemies = Enemies;
    NewEncounter.ThreatLevel = Enemies.Num() * 0.3f; // Base threat per enemy
    NewEncounter.EncounterRadius = 1500.0f;
    NewEncounter.bIsActive = true;
    
    ActiveEncounters.Add(NewEncounter);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat encounter started at %s with %d enemies"), 
           *Location.ToString(), Enemies.Num());
}

void ACombat_CombatManager::EndCombatEncounter(int32 EncounterIndex)
{
    if (ActiveEncounters.IsValidIndex(EncounterIndex))
    {
        ActiveEncounters[EncounterIndex].bIsActive = false;
        ActiveEncounters.RemoveAt(EncounterIndex);
        
        UE_LOG(LogTemp, Warning, TEXT("Combat encounter ended"));
    }
}

void ACombat_CombatManager::UpdateTacticalSituation()
{
    if (!PlayerCharacter)
        return;
    
    FVector PlayerPos = PlayerCharacter->GetActorLocation();
    CurrentTacticalSituation.LastKnownPlayerPosition = PlayerPos;
    CurrentTacticalSituation.PlayerThreatLevel = CalculatePlayerThreatLevel();
    
    // Check if player is in combat range of any enemies
    bool bInCombat = false;
    for (AActor* Enemy : TrackedEnemies)
    {
        if (Enemy)
        {
            float Distance = FVector::Dist(PlayerPos, Enemy->GetActorLocation());
            if (Distance <= MaxCombatRange)
            {
                bInCombat = true;
                break;
            }
        }
    }
    
    CurrentTacticalSituation.bPlayerInCombat = bInCombat;
    
    if (bInCombat)
    {
        CurrentTacticalSituation.TimeSinceLastSighting = 0.0f;
        CurrentTacticalSituation.FlankingPositions = GenerateFlankingPositions(PlayerPos, 4);
    }
    else
    {
        CurrentTacticalSituation.TimeSinceLastSighting += CombatUpdateInterval;
    }
}

float ACombat_CombatManager::CalculatePlayerThreatLevel() const
{
    if (!PlayerCharacter)
        return 0.0f;
    
    float ThreatLevel = 0.0f;
    FVector PlayerPos = PlayerCharacter->GetActorLocation();
    
    // Base threat from nearby enemies
    for (AActor* Enemy : TrackedEnemies)
    {
        if (Enemy)
        {
            float Distance = FVector::Dist(PlayerPos, Enemy->GetActorLocation());
            if (Distance <= MaxCombatRange)
            {
                float ProximityThreat = 1.0f - (Distance / MaxCombatRange);
                ThreatLevel += ProximityThreat * 0.5f;
            }
        }
    }
    
    // Add global threat modifier
    ThreatLevel += GlobalThreatLevel * 0.3f;
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

TArray<FVector> ACombat_CombatManager::GenerateFlankingPositions(const FVector& PlayerPos, int32 NumPositions) const
{
    TArray<FVector> Positions;
    float AngleStep = 360.0f / NumPositions;
    float FlankingRadius = 800.0f;
    
    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector FlankPos = PlayerPos + FVector(
            FMath::Cos(RadianAngle) * FlankingRadius,
            FMath::Sin(RadianAngle) * FlankingRadius,
            0.0f
        );
        
        Positions.Add(FlankPos);
    }
    
    return Positions;
}

bool ACombat_CombatManager::IsPlayerInCombat() const
{
    return CurrentTacticalSituation.bPlayerInCombat;
}

AActor* ACombat_CombatManager::GetNearestEnemy(const FVector& Position) const
{
    AActor* NearestEnemy = nullptr;
    float MinDistance = MAX_FLT;
    
    for (AActor* Enemy : TrackedEnemies)
    {
        if (Enemy)
        {
            float Distance = FVector::Dist(Position, Enemy->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestEnemy = Enemy;
            }
        }
    }
    
    return NearestEnemy;
}

void ACombat_CombatManager::SetGlobalThreatLevel(float NewThreatLevel)
{
    GlobalThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 1.0f);
}

void ACombat_CombatManager::ProcessCombatAI(float DeltaTime)
{
    if (!PlayerCharacter)
        return;
    
    // Process each active encounter
    for (int32 i = ActiveEncounters.Num() - 1; i >= 0; i--)
    {
        FCombat_CombatEncounter& Encounter = ActiveEncounters[i];
        
        if (!Encounter.bIsActive)
        {
            EndCombatEncounter(i);
            continue;
        }
        
        // Update encounter based on player proximity
        FVector PlayerPos = PlayerCharacter->GetActorLocation();
        float DistanceToEncounter = FVector::Dist(PlayerPos, Encounter.Location);
        
        if (DistanceToEncounter > Encounter.EncounterRadius * 2.0f)
        {
            // Player moved too far, deactivate encounter
            Encounter.bIsActive = false;
        }
        else if (DistanceToEncounter <= Encounter.EncounterRadius)
        {
            // Player in encounter range, increase threat
            Encounter.ThreatLevel = FMath::Min(1.0f, Encounter.ThreatLevel + DeltaTime * 0.1f);
        }
    }
    
    // Debug visualization
    if (GetWorld())
    {
        FVector PlayerPos = PlayerCharacter->GetActorLocation();
        
        // Draw combat range
        DrawDebugSphere(GetWorld(), PlayerPos, MaxCombatRange, 32, 
                       CurrentTacticalSituation.bPlayerInCombat ? FColor::Red : FColor::Yellow, 
                       false, CombatUpdateInterval + 0.1f);
        
        // Draw flanking positions
        for (const FVector& FlankPos : CurrentTacticalSituation.FlankingPositions)
        {
            DrawDebugSphere(GetWorld(), FlankPos, 50.0f, 12, FColor::Orange, 
                           false, CombatUpdateInterval + 0.1f);
        }
    }
}