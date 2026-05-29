#include "Crowd_CombatEvacuationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Combat/Combat_CombatStateManager.h"

UCrowd_CombatEvacuationProcessor::UCrowd_CombatEvacuationProcessor()
{
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    PanicSpreadRadius = 1000.0f;
    PanicSpreadRate = 0.5f;
    MaxEvacuationDistance = 5000.0f;
}

void UCrowd_CombatEvacuationProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FMassTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_CombatEvacuationFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_CombatEvacuationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    if (!GetWorld())
    {
        return;
    }

    UCrowd_CombatEvacuationManager* EvacuationManager = GetWorld()->GetSubsystem<UCrowd_CombatEvacuationManager>();
    if (!EvacuationManager)
    {
        return;
    }

    const float DeltaTime = GetWorld()->GetDeltaSeconds();

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const int32 NumEntities = EntityCollection.GetNumEntities();
        const TArrayView<FMassTransformFragment> TransformList = EntityCollection.GetMutableFragmentView<FMassTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = EntityCollection.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_CombatEvacuationFragment> EvacuationList = EntityCollection.GetMutableFragmentView<FCrowd_CombatEvacuationFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FMassTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_CombatEvacuationFragment& Evacuation = EvacuationList[EntityIndex];

            // Update panic behavior
            if (Evacuation.PanicBehavior.bInPanic)
            {
                // Calculate flee direction if not set
                if (Evacuation.PanicBehavior.FleeDirection.IsNearlyZero())
                {
                    FVector FleeDir;
                    EvacuationManager->CalculateFleeDirection(Transform.GetTransform().GetLocation(), 
                        Evacuation.LastKnownThreatLocation, FleeDir);
                    Evacuation.PanicBehavior.FleeDirection = FleeDir;
                }

                // Apply panic movement
                const FVector DesiredVelocity = Evacuation.PanicBehavior.FleeDirection * Evacuation.PanicBehavior.FleeSpeed;
                Velocity.Value = FMath::VInterpTo(Velocity.Value, DesiredVelocity, DeltaTime, 5.0f);

                // Decay panic over time
                Evacuation.PanicBehavior.PanicLevel = FMath::Max(0.0f, 
                    Evacuation.PanicBehavior.PanicLevel - Evacuation.PanicBehavior.PanicDecayRate * DeltaTime);

                if (Evacuation.PanicBehavior.PanicLevel <= 0.1f)
                {
                    Evacuation.PanicBehavior.bInPanic = false;
                    Evacuation.PanicBehavior.FleeDirection = FVector::ZeroVector;
                }
            }

            // Check if entity reached safe zone
            if (Evacuation.AssignedEvacuationZone >= 0)
            {
                float DistanceToSafeZone;
                FVector SafeZoneLocation = EvacuationManager->GetNearestSafeZone(Transform.GetTransform().GetLocation(), DistanceToSafeZone);
                
                if (DistanceToSafeZone < 500.0f) // Reached safe zone
                {
                    Evacuation.PanicBehavior.PanicLevel = FMath::Max(0.0f, Evacuation.PanicBehavior.PanicLevel - 0.5f * DeltaTime);
                    Velocity.Value *= 0.5f; // Slow down in safe zone
                }
            }

            // Update threat awareness
            Evacuation.ThreatAwareness = FMath::Max(0.0f, Evacuation.ThreatAwareness - 0.1f * DeltaTime);
        }
    });
}

UCrowd_CombatEvacuationManager::UCrowd_CombatEvacuationManager()
{
    GlobalPanicThreshold = 0.6f;
    PanicDecayRate = 0.05f;
    MaxEvacuationSpeed = 800.0f;
    MaxSimultaneousEvacuees = 1000;
    LastUpdateTime = 0.0f;
    bSystemInitialized = false;
    CombatStateManager = nullptr;
}

void UCrowd_CombatEvacuationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Combat Evacuation Manager initialized"));
    
    // Initialize default evacuation zones
    InitializeDefaultEvacuationZones();
    
    // Get reference to combat state manager
    if (GetWorld())
    {
        CombatStateManager = GetWorld()->GetSubsystem<UCombat_CombatStateManager>();
    }
    
    bSystemInitialized = true;
}

void UCrowd_CombatEvacuationManager::Deinitialize()
{
    EvacuationZones.Empty();
    RegisteredEntities.Empty();
    CombatStateManager = nullptr;
    bSystemInitialized = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Combat Evacuation Manager deinitialized"));
    
    Super::Deinitialize();
}

int32 UCrowd_CombatEvacuationManager::CreateEvacuationZone(const FVector& Center, float Radius, int32 MaxCapacity)
{
    FCrowd_EvacuationZone NewZone;
    NewZone.SafeZoneCenter = Center;
    NewZone.SafeZoneRadius = Radius;
    NewZone.MaxCapacity = MaxCapacity;
    NewZone.CurrentOccupancy = 0;
    NewZone.bIsActive = true;
    
    int32 ZoneIndex = EvacuationZones.Add(NewZone);
    
    UE_LOG(LogTemp, Log, TEXT("Created evacuation zone %d at location %s with radius %.2f"), 
        ZoneIndex, *Center.ToString(), Radius);
    
    return ZoneIndex;
}

void UCrowd_CombatEvacuationManager::RemoveEvacuationZone(int32 ZoneIndex)
{
    if (EvacuationZones.IsValidIndex(ZoneIndex))
    {
        EvacuationZones.RemoveAt(ZoneIndex);
        UE_LOG(LogTemp, Log, TEXT("Removed evacuation zone %d"), ZoneIndex);
    }
}

void UCrowd_CombatEvacuationManager::ActivateEvacuationZone(int32 ZoneIndex, bool bActivate)
{
    if (EvacuationZones.IsValidIndex(ZoneIndex))
    {
        EvacuationZones[ZoneIndex].bIsActive = bActivate;
        UE_LOG(LogTemp, Log, TEXT("Evacuation zone %d %s"), ZoneIndex, bActivate ? TEXT("activated") : TEXT("deactivated"));
    }
}

FVector UCrowd_CombatEvacuationManager::GetNearestSafeZone(const FVector& Location, float& OutDistance)
{
    OutDistance = FLT_MAX;
    FVector NearestZoneCenter = Location;
    
    for (const FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        if (!Zone.bIsActive || Zone.CurrentOccupancy >= Zone.MaxCapacity)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Location, Zone.SafeZoneCenter);
        if (Distance < OutDistance)
        {
            OutDistance = Distance;
            NearestZoneCenter = Zone.SafeZoneCenter;
        }
    }
    
    return NearestZoneCenter;
}

void UCrowd_CombatEvacuationManager::TriggerMassEvacuation(const FVector& ThreatLocation, float ThreatRadius, float PanicLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("MASS EVACUATION TRIGGERED at %s, radius %.2f, panic level %.2f"), 
        *ThreatLocation.ToString(), ThreatRadius, PanicLevel);
    
    // Spread panic to all registered entities within threat radius
    SpreadPanic(ThreatLocation, ThreatRadius, PanicLevel);
    
    // Assign entities to evacuation zones
    for (auto& EntityPair : RegisteredEntities)
    {
        FCrowd_CombatEvacuationFragment& EvacuationData = EntityPair.Value;
        
        // Calculate distance from threat
        float ThreatDistance = FVector::Dist(ThreatLocation, EvacuationData.LastKnownThreatLocation);
        
        if (ThreatDistance <= ThreatRadius)
        {
            // Entity is in danger zone
            EvacuationData.PanicBehavior.bInPanic = true;
            EvacuationData.PanicBehavior.PanicLevel = FMath::Max(EvacuationData.PanicBehavior.PanicLevel, PanicLevel);
            EvacuationData.ThreatAwareness = 1.0f;
            EvacuationData.LastKnownThreatLocation = ThreatLocation;
            
            // Assign to nearest evacuation zone
            int32 BestZone = FindBestEvacuationZone(ThreatLocation);
            if (BestZone >= 0)
            {
                AssignEntityToEvacuationZone(EntityPair.Key, BestZone);
            }
        }
    }
}

void UCrowd_CombatEvacuationManager::ProcessCombatThreat(const FVector& ThreatLocation, float ThreatLevel, float AffectedRadius)
{
    // Integrate with combat state manager if available
    if (CombatStateManager)
    {
        // Query combat state for additional threat information
        // This would normally call CombatStateManager methods
    }
    
    // Process threat based on level
    if (ThreatLevel >= 0.8f)
    {
        TriggerMassEvacuation(ThreatLocation, AffectedRadius, ThreatLevel);
    }
    else if (ThreatLevel >= 0.5f)
    {
        SpreadPanic(ThreatLocation, AffectedRadius * 0.7f, ThreatLevel * 0.8f);
    }
    else
    {
        // Low level threat - just increase awareness
        for (auto& EntityPair : RegisteredEntities)
        {
            FCrowd_CombatEvacuationFragment& EvacuationData = EntityPair.Value;
            float Distance = FVector::Dist(ThreatLocation, EvacuationData.LastKnownThreatLocation);
            
            if (Distance <= AffectedRadius)
            {
                EvacuationData.ThreatAwareness = FMath::Min(1.0f, EvacuationData.ThreatAwareness + ThreatLevel * 0.3f);
            }
        }
    }
}

void UCrowd_CombatEvacuationManager::UpdateEvacuationBehavior(float DeltaTime)
{
    if (!bSystemInitialized)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    UpdatePanicSpread(DeltaTime);
    ProcessEvacuationMovement(DeltaTime);
    ValidateEvacuationZones();
}

void UCrowd_CombatEvacuationManager::RegisterMassEntity(FMassEntityHandle EntityHandle, const FVector& Location)
{
    FCrowd_CombatEvacuationFragment NewEvacuationData;
    NewEvacuationData.LastKnownThreatLocation = Location;
    
    RegisteredEntities.Add(EntityHandle, NewEvacuationData);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Registered mass entity for evacuation tracking"));
}

void UCrowd_CombatEvacuationManager::UnregisterMassEntity(FMassEntityHandle EntityHandle)
{
    RegisteredEntities.Remove(EntityHandle);
}

void UCrowd_CombatEvacuationManager::AssignEntityToEvacuationZone(FMassEntityHandle EntityHandle, int32 ZoneIndex)
{
    if (RegisteredEntities.Contains(EntityHandle) && EvacuationZones.IsValidIndex(ZoneIndex))
    {
        FCrowd_CombatEvacuationFragment& EvacuationData = RegisteredEntities[EntityHandle];
        
        // Remove from previous zone
        if (EvacuationData.AssignedEvacuationZone >= 0 && EvacuationZones.IsValidIndex(EvacuationData.AssignedEvacuationZone))
        {
            EvacuationZones[EvacuationData.AssignedEvacuationZone].CurrentOccupancy--;
        }
        
        // Assign to new zone
        EvacuationData.AssignedEvacuationZone = ZoneIndex;
        EvacuationZones[ZoneIndex].CurrentOccupancy++;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Assigned entity to evacuation zone %d"), ZoneIndex);
    }
}

void UCrowd_CombatEvacuationManager::SpreadPanic(const FVector& PanicSource, float PanicRadius, float PanicIntensity)
{
    for (auto& EntityPair : RegisteredEntities)
    {
        FCrowd_CombatEvacuationFragment& EvacuationData = EntityPair.Value;
        
        float Distance = FVector::Dist(PanicSource, EvacuationData.LastKnownThreatLocation);
        
        if (Distance <= PanicRadius)
        {
            float DistanceFactor = 1.0f - (Distance / PanicRadius);
            float PanicIncrease = PanicIntensity * DistanceFactor;
            
            EvacuationData.PanicBehavior.PanicLevel = FMath::Min(1.0f, 
                EvacuationData.PanicBehavior.PanicLevel + PanicIncrease);
            
            if (EvacuationData.PanicBehavior.PanicLevel >= GlobalPanicThreshold)
            {
                EvacuationData.PanicBehavior.bInPanic = true;
                EvacuationData.LastKnownThreatLocation = PanicSource;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Panic spread from %s affecting radius %.2f with intensity %.2f"), 
        *PanicSource.ToString(), PanicRadius, PanicIntensity);
}

void UCrowd_CombatEvacuationManager::CalculateFleeDirection(const FVector& EntityLocation, const FVector& ThreatLocation, FVector& OutFleeDirection)
{
    // Calculate basic flee direction (away from threat)
    FVector FleeDirection = (EntityLocation - ThreatLocation).GetSafeNormal();
    
    // Find nearest safe zone and adjust direction
    float DistanceToSafeZone;
    FVector SafeZoneLocation = GetNearestSafeZone(EntityLocation, DistanceToSafeZone);
    
    if (DistanceToSafeZone < 10000.0f) // If there's a reasonable safe zone
    {
        FVector ToSafeZone = (SafeZoneLocation - EntityLocation).GetSafeNormal();
        
        // Blend flee direction with safe zone direction
        OutFleeDirection = (FleeDirection * 0.6f + ToSafeZone * 0.4f).GetSafeNormal();
    }
    else
    {
        OutFleeDirection = FleeDirection;
    }
}

bool UCrowd_CombatEvacuationManager::IsLocationSafe(const FVector& Location, float SafetyRadius)
{
    for (const FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        if (!Zone.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Location, Zone.SafeZoneCenter);
        if (Distance <= Zone.SafeZoneRadius + SafetyRadius)
        {
            return true;
        }
    }
    
    return false;
}

int32 UCrowd_CombatEvacuationManager::GetTotalEvacuatingEntities() const
{
    int32 Count = 0;
    for (const auto& EntityPair : RegisteredEntities)
    {
        if (EntityPair.Value.PanicBehavior.bInPanic)
        {
            Count++;
        }
    }
    return Count;
}

float UCrowd_CombatEvacuationManager::GetAveragePanicLevel() const
{
    if (RegisteredEntities.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalPanic = 0.0f;
    for (const auto& EntityPair : RegisteredEntities)
    {
        TotalPanic += EntityPair.Value.PanicBehavior.PanicLevel;
    }
    
    return TotalPanic / RegisteredEntities.Num();
}

void UCrowd_CombatEvacuationManager::GetEvacuationStatistics(int32& OutActiveZones, int32& OutEvacuatingEntities, float& OutAveragePanic)
{
    OutActiveZones = 0;
    for (const FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        if (Zone.bIsActive)
        {
            OutActiveZones++;
        }
    }
    
    OutEvacuatingEntities = GetTotalEvacuatingEntities();
    OutAveragePanic = GetAveragePanicLevel();
}

void UCrowd_CombatEvacuationManager::InitializeDefaultEvacuationZones()
{
    // Create default evacuation zones around the map
    CreateEvacuationZone(FVector(5000.0f, 5000.0f, 100.0f), 2000.0f, 500);   // Northeast safe zone
    CreateEvacuationZone(FVector(-5000.0f, 5000.0f, 100.0f), 2000.0f, 500);  // Northwest safe zone
    CreateEvacuationZone(FVector(5000.0f, -5000.0f, 100.0f), 2000.0f, 500);  // Southeast safe zone
    CreateEvacuationZone(FVector(-5000.0f, -5000.0f, 100.0f), 2000.0f, 500); // Southwest safe zone
    CreateEvacuationZone(FVector(0.0f, 0.0f, 1000.0f), 1500.0f, 300);        // Central elevated safe zone
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default evacuation zones"), EvacuationZones.Num());
}

void UCrowd_CombatEvacuationManager::UpdatePanicSpread(float DeltaTime)
{
    // Process panic contagion between nearby entities
    TArray<FMassEntityHandle> PanickedEntities;
    
    for (const auto& EntityPair : RegisteredEntities)
    {
        if (EntityPair.Value.PanicBehavior.bInPanic)
        {
            PanickedEntities.Add(EntityPair.Key);
        }
    }
    
    // Spread panic from panicked entities to nearby calm entities
    for (FMassEntityHandle PanickedEntity : PanickedEntities)
    {
        if (!RegisteredEntities.Contains(PanickedEntity))
        {
            continue;
        }
        
        const FCrowd_CombatEvacuationFragment& PanickedData = RegisteredEntities[PanickedEntity];
        
        for (auto& EntityPair : RegisteredEntities)
        {
            if (EntityPair.Key == PanickedEntity || EntityPair.Value.PanicBehavior.bInPanic)
            {
                continue;
            }
            
            // Calculate distance between entities (simplified - would need actual positions)
            float Distance = 500.0f; // Placeholder distance
            
            if (Distance <= 1000.0f) // Panic spread radius
            {
                float PanicSpread = 0.1f * DeltaTime * (1.0f - Distance / 1000.0f);
                EntityPair.Value.PanicBehavior.PanicLevel += PanicSpread;
                
                if (EntityPair.Value.PanicBehavior.PanicLevel >= GlobalPanicThreshold)
                {
                    EntityPair.Value.PanicBehavior.bInPanic = true;
                }
            }
        }
    }
}

void UCrowd_CombatEvacuationManager::ProcessEvacuationMovement(float DeltaTime)
{
    // Update evacuation zone occupancy
    for (FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        Zone.CurrentOccupancy = 0;
    }
    
    for (const auto& EntityPair : RegisteredEntities)
    {
        const FCrowd_CombatEvacuationFragment& EvacuationData = EntityPair.Value;
        
        if (EvacuationData.AssignedEvacuationZone >= 0 && EvacuationZones.IsValidIndex(EvacuationData.AssignedEvacuationZone))
        {
            EvacuationZones[EvacuationData.AssignedEvacuationZone].CurrentOccupancy++;
        }
    }
}

int32 UCrowd_CombatEvacuationManager::FindBestEvacuationZone(const FVector& Location)
{
    int32 BestZone = -1;
    float BestScore = -1.0f;
    
    for (int32 i = 0; i < EvacuationZones.Num(); i++)
    {
        const FCrowd_EvacuationZone& Zone = EvacuationZones[i];
        
        if (!Zone.bIsActive || Zone.CurrentOccupancy >= Zone.MaxCapacity)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Location, Zone.SafeZoneCenter);
        float CapacityFactor = 1.0f - (float)Zone.CurrentOccupancy / Zone.MaxCapacity;
        float DistanceFactor = 1.0f / (1.0f + Distance / 1000.0f);
        
        float Score = CapacityFactor * 0.6f + DistanceFactor * 0.4f;
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestZone = i;
        }
    }
    
    return BestZone;
}

void UCrowd_CombatEvacuationManager::ValidateEvacuationZones()
{
    // Remove invalid or overcrowded zones
    for (int32 i = EvacuationZones.Num() - 1; i >= 0; i--)
    {
        FCrowd_EvacuationZone& Zone = EvacuationZones[i];
        
        // Reset occupancy if it becomes negative
        if (Zone.CurrentOccupancy < 0)
        {
            Zone.CurrentOccupancy = 0;
        }
        
        // Deactivate zones that are consistently over capacity
        if (Zone.CurrentOccupancy > Zone.MaxCapacity * 1.2f)
        {
            Zone.bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Deactivated evacuation zone %d due to overcrowding"), i);
        }
    }
}