#include "Crowd_CombatEvacuationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowd_CombatEvacuationManager::UCrowd_CombatEvacuationManager()
{
    MaxEvacuationDistance = 2000.0f;
    PanicDecayRate = 0.5f;
    EvacuationSpeedMultiplier = 2.0f;
    CrowdDensityThreshold = 10.0f;
}

void UCrowd_CombatEvacuationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_CombatEvacuationManager initialized"));
    
    // Create default evacuation zones
    CreateEvacuationZone(FVector(0, 0, 0), 1500.0f);
    CreateEvacuationZone(FVector(3000, 0, 0), 1200.0f);
    CreateEvacuationZone(FVector(-3000, 0, 0), 1200.0f);
    CreateEvacuationZone(FVector(0, 3000, 0), 1000.0f);
    CreateEvacuationZone(FVector(0, -3000, 0), 1000.0f);
}

void UCrowd_CombatEvacuationManager::Deinitialize()
{
    ResetAllEvacuations();
    EvacuationZones.Empty();
    RegisteredNPCs.Empty();
    
    Super::Deinitialize();
}

void UCrowd_CombatEvacuationManager::TriggerEvacuation(FVector CombatLocation, float ThreatRadius, float ThreatLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering evacuation at location: %s, Radius: %f, Threat: %f"), 
           *CombatLocation.ToString(), ThreatRadius, ThreatLevel);

    // Find all NPCs within threat radius
    TArray<AActor*> NPCsInDanger = GetNPCsInRadius(CombatLocation, ThreatRadius);
    
    for (AActor* NPC : NPCsInDanger)
    {
        if (NPC)
        {
            // Set panic level based on distance to threat
            float Distance = FVector::Dist(NPC->GetActorLocation(), CombatLocation);
            float PanicLevel = FMath::Clamp(1.0f - (Distance / ThreatRadius), 0.2f, 1.0f);
            PanicLevel *= ThreatLevel;
            
            SetNPCPanicLevel(NPC, PanicLevel);
            
            // Calculate evacuation target
            FVector EvacuationTarget = CalculateEvacuationTarget(NPC, CombatLocation);
            
            // Update or add NPC evacuation data
            bool bFound = false;
            for (FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
            {
                if (NPCData.NPCActor == NPC)
                {
                    NPCData.EvacuationTarget = EvacuationTarget;
                    NPCData.PanicLevel = PanicLevel;
                    NPCData.bIsEvacuating = true;
                    NPCData.EvacuationSpeed = 600.0f * (1.0f + PanicLevel);
                    bFound = true;
                    break;
                }
            }
            
            if (!bFound)
            {
                FCrowd_NPCEvacuationData NewNPCData;
                NewNPCData.NPCActor = NPC;
                NewNPCData.OriginalPosition = NPC->GetActorLocation();
                NewNPCData.EvacuationTarget = EvacuationTarget;
                NewNPCData.PanicLevel = PanicLevel;
                NewNPCData.bIsEvacuating = true;
                NewNPCData.EvacuationSpeed = 600.0f * (1.0f + PanicLevel);
                RegisteredNPCs.Add(NewNPCData);
            }
        }
    }
    
    // Update evacuation zones
    for (FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        float DistanceToThreat = FVector::Dist(Zone.Center, CombatLocation);
        if (DistanceToThreat <= (Zone.Radius + ThreatRadius))
        {
            Zone.ThreatLevel = FMath::Max(Zone.ThreatLevel, ThreatLevel * (1.0f - DistanceToThreat / (Zone.Radius + ThreatRadius)));
            
            if (Zone.ThreatLevel > 0.8f)
            {
                Zone.CurrentState = ECrowd_EvacuationState::Panic;
            }
            else if (Zone.ThreatLevel > 0.5f)
            {
                Zone.CurrentState = ECrowd_EvacuationState::Evacuation;
            }
            else if (Zone.ThreatLevel > 0.2f)
            {
                Zone.CurrentState = ECrowd_EvacuationState::Alert;
            }
        }
    }
}

void UCrowd_CombatEvacuationManager::UpdateEvacuationZones(float DeltaTime)
{
    for (FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        // Decay threat level over time
        Zone.ThreatLevel = FMath::Max(0.0f, Zone.ThreatLevel - (PanicDecayRate * DeltaTime));
        
        // Update zone state based on threat level
        if (Zone.ThreatLevel <= 0.1f)
        {
            Zone.CurrentState = ECrowd_EvacuationState::Normal;
        }
        else if (Zone.ThreatLevel <= 0.3f)
        {
            Zone.CurrentState = ECrowd_EvacuationState::Alert;
        }
        else if (Zone.ThreatLevel <= 0.7f)
        {
            Zone.CurrentState = ECrowd_EvacuationState::Evacuation;
        }
        else
        {
            Zone.CurrentState = ECrowd_EvacuationState::Panic;
        }
        
        // Update NPCs in zone
        Zone.NPCsInZone.Empty();
        for (const FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
        {
            if (NPCData.NPCActor && IsNPCInEvacuationZone(NPCData.NPCActor, Zone))
            {
                Zone.NPCsInZone.Add(NPCData.NPCActor);
            }
        }
        
        CalculateZoneThreatLevel(Zone);
    }
    
    // Update NPC evacuations
    for (FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        if (NPCData.bIsEvacuating)
        {
            UpdateNPCEvacuation(NPCData, DeltaTime);
        }
    }
}

void UCrowd_CombatEvacuationManager::RegisterNPCForEvacuation(AActor* NPCActor)
{
    if (!NPCActor) return;
    
    // Check if already registered
    for (const FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        if (NPCData.NPCActor == NPCActor)
        {
            return; // Already registered
        }
    }
    
    FCrowd_NPCEvacuationData NewNPCData;
    NewNPCData.NPCActor = NPCActor;
    NewNPCData.OriginalPosition = NPCActor->GetActorLocation();
    RegisteredNPCs.Add(NewNPCData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered NPC for evacuation: %s"), *NPCActor->GetName());
}

void UCrowd_CombatEvacuationManager::UnregisterNPCFromEvacuation(AActor* NPCActor)
{
    if (!NPCActor) return;
    
    for (int32 i = RegisteredNPCs.Num() - 1; i >= 0; --i)
    {
        if (RegisteredNPCs[i].NPCActor == NPCActor)
        {
            RegisteredNPCs.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered NPC from evacuation: %s"), *NPCActor->GetName());
            break;
        }
    }
}

void UCrowd_CombatEvacuationManager::CreateEvacuationZone(FVector Center, float Radius)
{
    FCrowd_EvacuationZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.CurrentState = ECrowd_EvacuationState::Normal;
    NewZone.ThreatLevel = 0.0f;
    
    EvacuationZones.Add(NewZone);
    
    UE_LOG(LogTemp, Log, TEXT("Created evacuation zone at %s with radius %f"), *Center.ToString(), Radius);
}

void UCrowd_CombatEvacuationManager::RemoveEvacuationZone(int32 ZoneIndex)
{
    if (EvacuationZones.IsValidIndex(ZoneIndex))
    {
        EvacuationZones.RemoveAt(ZoneIndex);
    }
}

FCrowd_EvacuationZone UCrowd_CombatEvacuationManager::GetEvacuationZone(int32 ZoneIndex)
{
    if (EvacuationZones.IsValidIndex(ZoneIndex))
    {
        return EvacuationZones[ZoneIndex];
    }
    return FCrowd_EvacuationZone();
}

void UCrowd_CombatEvacuationManager::SetNPCPanicLevel(AActor* NPCActor, float PanicLevel)
{
    if (!NPCActor) return;
    
    for (FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        if (NPCData.NPCActor == NPCActor)
        {
            NPCData.PanicLevel = FMath::Clamp(PanicLevel, 0.0f, 1.0f);
            NPCData.EvacuationSpeed = 600.0f * (1.0f + NPCData.PanicLevel);
            break;
        }
    }
}

FVector UCrowd_CombatEvacuationManager::CalculateEvacuationTarget(AActor* NPCActor, FVector ThreatLocation)
{
    if (!NPCActor) return FVector::ZeroVector;
    
    FVector NPCLocation = NPCActor->GetActorLocation();
    FVector DirectionAwayFromThreat = (NPCLocation - ThreatLocation).GetSafeNormal();
    
    // Find the safest evacuation point
    FVector BestTarget = NPCLocation + (DirectionAwayFromThreat * MaxEvacuationDistance);
    
    // Try to find a safe zone
    for (const FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        if (Zone.ThreatLevel < 0.2f && Zone.CurrentState == ECrowd_EvacuationState::Normal)
        {
            float DistanceToZone = FVector::Dist(NPCLocation, Zone.Center);
            if (DistanceToZone < MaxEvacuationDistance)
            {
                BestTarget = Zone.Center + (FMath::VRand() * Zone.Radius * 0.5f);
                break;
            }
        }
    }
    
    return BestTarget;
}

void UCrowd_CombatEvacuationManager::ExecuteEvacuationMovement(AActor* NPCActor, float DeltaTime)
{
    if (!NPCActor) return;
    
    for (FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        if (NPCData.NPCActor == NPCActor && NPCData.bIsEvacuating)
        {
            FVector CurrentLocation = NPCActor->GetActorLocation();
            FVector Direction = (NPCData.EvacuationTarget - CurrentLocation).GetSafeNormal();
            FVector NewLocation = CurrentLocation + (Direction * NPCData.EvacuationSpeed * DeltaTime);
            
            NPCActor->SetActorLocation(NewLocation);
            
            // Check if reached evacuation target
            float DistanceToTarget = FVector::Dist(CurrentLocation, NPCData.EvacuationTarget);
            if (DistanceToTarget < 100.0f)
            {
                NPCData.bIsEvacuating = false;
                NPCData.PanicLevel = FMath::Max(0.0f, NPCData.PanicLevel - 0.5f);
            }
            break;
        }
    }
}

void UCrowd_CombatEvacuationManager::OnCombatStarted(FVector CombatLocation, float ThreatLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("Combat started at %s with threat level %f"), *CombatLocation.ToString(), ThreatLevel);
    TriggerEvacuation(CombatLocation, 1500.0f, ThreatLevel);
}

void UCrowd_CombatEvacuationManager::OnCombatEnded(FVector CombatLocation)
{
    UE_LOG(LogTemp, Log, TEXT("Combat ended at %s"), *CombatLocation.ToString());
    
    // Gradually reduce panic levels
    for (FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        NPCData.PanicLevel = FMath::Max(0.0f, NPCData.PanicLevel - 0.3f);
        if (NPCData.PanicLevel <= 0.1f)
        {
            NPCData.bIsEvacuating = false;
        }
    }
}

void UCrowd_CombatEvacuationManager::OnDinosaurSpotted(AActor* DinosaurActor, FVector Location)
{
    if (!DinosaurActor) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur spotted: %s at %s"), *DinosaurActor->GetName(), *Location.ToString());
    
    float ThreatLevel = 0.8f; // High threat for dinosaur sighting
    if (DinosaurActor->GetName().Contains(TEXT("TRex")))
    {
        ThreatLevel = 1.0f; // Maximum threat for T-Rex
    }
    else if (DinosaurActor->GetName().Contains(TEXT("Raptor")))
    {
        ThreatLevel = 0.9f; // Very high threat for raptors
    }
    
    TriggerEvacuation(Location, 2000.0f, ThreatLevel);
}

float UCrowd_CombatEvacuationManager::GetCrowdDensityAtLocation(FVector Location, float Radius)
{
    TArray<AActor*> NPCsInArea = GetNPCsInRadius(Location, Radius);
    float Area = PI * Radius * Radius;
    return NPCsInArea.Num() / (Area / 10000.0f); // NPCs per 100x100 unit area
}

void UCrowd_CombatEvacuationManager::AdjustCrowdDensityForCombat(FVector CombatLocation, float Radius)
{
    float CurrentDensity = GetCrowdDensityAtLocation(CombatLocation, Radius);
    
    if (CurrentDensity > CrowdDensityThreshold)
    {
        // Force evacuation to reduce density
        TriggerEvacuation(CombatLocation, Radius, 0.7f);
    }
}

bool UCrowd_CombatEvacuationManager::IsLocationSafe(FVector Location)
{
    for (const FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        float Distance = FVector::Dist(Location, Zone.Center);
        if (Distance <= Zone.Radius && Zone.ThreatLevel > 0.3f)
        {
            return false;
        }
    }
    return true;
}

TArray<AActor*> UCrowd_CombatEvacuationManager::GetNPCsInRadius(FVector Center, float Radius)
{
    TArray<AActor*> NPCsInRadius;
    
    for (const FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        if (NPCData.NPCActor)
        {
            float Distance = FVector::Dist(NPCData.NPCActor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                NPCsInRadius.Add(NPCData.NPCActor);
            }
        }
    }
    
    return NPCsInRadius;
}

void UCrowd_CombatEvacuationManager::ResetAllEvacuations()
{
    for (FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
    {
        NPCData.bIsEvacuating = false;
        NPCData.PanicLevel = 0.0f;
    }
    
    for (FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        Zone.ThreatLevel = 0.0f;
        Zone.CurrentState = ECrowd_EvacuationState::Normal;
        Zone.NPCsInZone.Empty();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Reset all evacuations"));
}

void UCrowd_CombatEvacuationManager::UpdateNPCEvacuation(FCrowd_NPCEvacuationData& NPCData, float DeltaTime)
{
    if (!NPCData.NPCActor) return;
    
    // Decay panic level over time
    NPCData.PanicLevel = FMath::Max(0.0f, NPCData.PanicLevel - (PanicDecayRate * DeltaTime));
    
    // Update evacuation speed based on panic level
    NPCData.EvacuationSpeed = 600.0f * (1.0f + NPCData.PanicLevel * EvacuationSpeedMultiplier);
    
    // Move towards evacuation target
    FVector CurrentLocation = NPCData.NPCActor->GetActorLocation();
    FVector Direction = (NPCData.EvacuationTarget - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + (Direction * NPCData.EvacuationSpeed * DeltaTime);
    
    NPCData.NPCActor->SetActorLocation(NewLocation);
    
    // Check if reached evacuation target or panic level is too low
    float DistanceToTarget = FVector::Dist(CurrentLocation, NPCData.EvacuationTarget);
    if (DistanceToTarget < 100.0f || NPCData.PanicLevel <= 0.1f)
    {
        NPCData.bIsEvacuating = false;
    }
}

void UCrowd_CombatEvacuationManager::CalculateZoneThreatLevel(FCrowd_EvacuationZone& Zone)
{
    // Threat level is influenced by number of NPCs and their panic levels
    float TotalPanic = 0.0f;
    int32 PanickedNPCs = 0;
    
    for (AActor* NPC : Zone.NPCsInZone)
    {
        for (const FCrowd_NPCEvacuationData& NPCData : RegisteredNPCs)
        {
            if (NPCData.NPCActor == NPC && NPCData.PanicLevel > 0.2f)
            {
                TotalPanic += NPCData.PanicLevel;
                PanickedNPCs++;
            }
        }
    }
    
    if (PanickedNPCs > 0)
    {
        float AveragePanic = TotalPanic / PanickedNPCs;
        Zone.ThreatLevel = FMath::Max(Zone.ThreatLevel, AveragePanic * 0.5f);
    }
}

FVector UCrowd_CombatEvacuationManager::FindSafeEvacuationPoint(FVector StartLocation, FVector ThreatLocation)
{
    FVector DirectionAway = (StartLocation - ThreatLocation).GetSafeNormal();
    FVector SafePoint = StartLocation + (DirectionAway * MaxEvacuationDistance);
    
    // Try to find the safest zone
    float BestSafety = 0.0f;
    FVector BestPoint = SafePoint;
    
    for (const FCrowd_EvacuationZone& Zone : EvacuationZones)
    {
        if (Zone.ThreatLevel < 0.3f)
        {
            float Safety = 1.0f - Zone.ThreatLevel;
            float Distance = FVector::Dist(StartLocation, Zone.Center);
            Safety *= (1.0f - FMath::Clamp(Distance / MaxEvacuationDistance, 0.0f, 1.0f));
            
            if (Safety > BestSafety)
            {
                BestSafety = Safety;
                BestPoint = Zone.Center + (FMath::VRand() * Zone.Radius * 0.3f);
            }
        }
    }
    
    return BestPoint;
}

bool UCrowd_CombatEvacuationManager::IsNPCInEvacuationZone(AActor* NPCActor, const FCrowd_EvacuationZone& Zone)
{
    if (!NPCActor) return false;
    
    float Distance = FVector::Dist(NPCActor->GetActorLocation(), Zone.Center);
    return Distance <= Zone.Radius;
}