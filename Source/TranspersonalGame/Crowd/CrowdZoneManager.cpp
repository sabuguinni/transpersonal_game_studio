#include "CrowdZoneManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UCrowdZoneManager::UCrowdZoneManager()
{
    MaxSimulatedAgents = 500;
    TotalActiveAgents = 0;
    ZoneIDCounter = 0;
}

void UCrowdZoneManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Initialized — prehistoric crowd simulation active."));
}

void UCrowdZoneManager::Deinitialize()
{
    ZoneRegistry.Empty();
    ZoneAlertStates.Empty();
    ZoneAgentCounts.Empty();
    Super::Deinitialize();
}

FString UCrowdZoneManager::GenerateZoneID(ECrowd_ZoneType Type)
{
    FString TypeStr;
    switch (Type)
    {
        case ECrowd_ZoneType::HerbivoreHerd:   TypeStr = TEXT("HERB"); break;
        case ECrowd_ZoneType::PredatorPack:    TypeStr = TEXT("PRED"); break;
        case ECrowd_ZoneType::MigrationRoute:  TypeStr = TEXT("MIGR"); break;
        case ECrowd_ZoneType::ApexTerritory:   TypeStr = TEXT("APEX"); break;
        case ECrowd_ZoneType::Scavenger:       TypeStr = TEXT("SCAV"); break;
        default:                               TypeStr = TEXT("ZONE"); break;
    }
    return FString::Printf(TEXT("CZ_%s_%04d"), *TypeStr, ++ZoneIDCounter);
}

FString UCrowdZoneManager::RegisterZone(const FVector& Center, const FCrowd_ZoneConfig& Config)
{
    FString ZoneID = GenerateZoneID(Config.ZoneType);
    ZoneRegistry.Add(ZoneID, TPair<FCrowd_ZoneConfig, FVector>(Config, Center));
    ZoneAlertStates.Add(ZoneID, ECrowd_AlertState::Calm);
    ZoneAgentCounts.Add(ZoneID, FMath::Min(Config.MaxAgentsInZone, MaxSimulatedAgents));
    TotalActiveAgents += ZoneAgentCounts[ZoneID];

    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Zone registered: %s at (%.0f, %.0f, %.0f) — %d agents"),
        *ZoneID, Center.X, Center.Y, Center.Z, ZoneAgentCounts[ZoneID]);

    return ZoneID;
}

void UCrowdZoneManager::UnregisterZone(const FString& ZoneID)
{
    if (ZoneAgentCounts.Contains(ZoneID))
    {
        TotalActiveAgents -= ZoneAgentCounts[ZoneID];
        TotalActiveAgents = FMath::Max(0, TotalActiveAgents);
    }
    ZoneRegistry.Remove(ZoneID);
    ZoneAlertStates.Remove(ZoneID);
    ZoneAgentCounts.Remove(ZoneID);
    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Zone unregistered: %s"), *ZoneID);
}

void UCrowdZoneManager::TriggerAlertAtLocation(const FVector& AlertOrigin, float AlertRadius, ECrowd_AlertState NewState)
{
    int32 AlertedZones = 0;
    for (auto& Pair : ZoneRegistry)
    {
        const FString& ZoneID = Pair.Key;
        const FVector& ZoneCenter = Pair.Value.Value;
        float Distance = FVector::Dist(AlertOrigin, ZoneCenter);

        if (Distance <= AlertRadius)
        {
            ECrowd_AlertState& CurrentState = ZoneAlertStates.FindOrAdd(ZoneID);
            // Only escalate — never de-escalate via trigger
            if ((int32)NewState > (int32)CurrentState)
            {
                CurrentState = NewState;
                AlertedZones++;
                UE_LOG(LogTemp, Verbose, TEXT("[CrowdZoneManager] Zone %s alerted: %d (dist=%.0f)"),
                    *ZoneID, (int32)NewState, Distance);
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Alert triggered at (%.0f,%.0f,%.0f) r=%.0f — %d zones affected"),
        AlertOrigin.X, AlertOrigin.Y, AlertOrigin.Z, AlertRadius, AlertedZones);
}

void UCrowdZoneManager::PropagateAlertToNeighbours(const FString& ZoneID, ECrowd_AlertState State, int32 PropagationDepth)
{
    if (PropagationDepth <= 0) return;
    if (!ZoneRegistry.Contains(ZoneID)) return;

    const FVector& SourceCenter = ZoneRegistry[ZoneID].Value;
    const FCrowd_ZoneConfig& SourceConfig = ZoneRegistry[ZoneID].Key;

    // Propagate to zones within alert radius
    for (auto& Pair : ZoneRegistry)
    {
        if (Pair.Key == ZoneID) continue;
        float Distance = FVector::Dist(SourceCenter, Pair.Value.Value);
        if (Distance <= SourceConfig.AlertRadius)
        {
            ECrowd_AlertState& NeighbourState = ZoneAlertStates.FindOrAdd(Pair.Key);
            if ((int32)State > (int32)NeighbourState)
            {
                NeighbourState = State;
                // Recursive propagation with reduced depth
                PropagateAlertToNeighbours(Pair.Key, State, PropagationDepth - 1);
            }
        }
    }
}

void UCrowdZoneManager::StartMigration(const FString& ZoneID, const FVector& Destination, float Speed)
{
    if (!ZoneRegistry.Contains(ZoneID)) return;

    FCrowd_ZoneConfig& Config = ZoneRegistry[ZoneID].Key;
    FVector& Center = ZoneRegistry[ZoneID].Value;

    Config.bIsMigrating = true;
    Config.MigrationSpeed = Speed;
    Config.MigrationDirection = (Destination - Center).GetSafeNormal();

    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Migration started for zone %s — speed=%.0f dir=(%.2f,%.2f,%.2f)"),
        *ZoneID, Speed,
        Config.MigrationDirection.X, Config.MigrationDirection.Y, Config.MigrationDirection.Z);
}

void UCrowdZoneManager::StopMigration(const FString& ZoneID)
{
    if (!ZoneRegistry.Contains(ZoneID)) return;
    ZoneRegistry[ZoneID].Key.bIsMigrating = false;
    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Migration stopped for zone %s"), *ZoneID);
}

TArray<FString> UCrowdZoneManager::GetZonesInRadius(const FVector& Center, float Radius) const
{
    TArray<FString> Result;
    for (const auto& Pair : ZoneRegistry)
    {
        if (FVector::Dist(Center, Pair.Value.Value) <= Radius)
        {
            Result.Add(Pair.Key);
        }
    }
    return Result;
}

ECrowd_AlertState UCrowdZoneManager::GetZoneAlertState(const FString& ZoneID) const
{
    if (const ECrowd_AlertState* State = ZoneAlertStates.Find(ZoneID))
    {
        return *State;
    }
    return ECrowd_AlertState::Calm;
}

int32 UCrowdZoneManager::GetTotalActiveAgents() const
{
    return TotalActiveAgents;
}

FCrowd_ZoneConfig UCrowdZoneManager::GetZoneConfig(const FString& ZoneID) const
{
    if (const TPair<FCrowd_ZoneConfig, FVector>* Entry = ZoneRegistry.Find(ZoneID))
    {
        return Entry->Key;
    }
    return FCrowd_ZoneConfig();
}

void UCrowdZoneManager::UpdateLODForPlayerLocation(const FVector& PlayerLocation)
{
    for (auto& Pair : ZoneRegistry)
    {
        float Distance = FVector::Dist(PlayerLocation, Pair.Value.Value);
        int32& AgentCount = ZoneAgentCounts.FindOrAdd(Pair.Key);
        const int32 MaxForZone = Pair.Value.Key.MaxAgentsInZone;

        if (Distance <= LOD0_Distance)
        {
            // Full simulation — all agents active
            AgentCount = MaxForZone;
        }
        else if (Distance <= LOD1_Distance)
        {
            // LOD1 — 60% agents
            AgentCount = FMath::RoundToInt(MaxForZone * 0.6f);
        }
        else if (Distance <= LOD2_Distance)
        {
            // LOD2 — 25% agents (position only)
            AgentCount = FMath::RoundToInt(MaxForZone * 0.25f);
        }
        else
        {
            // Dormant — 0 agents simulated
            AgentCount = 0;
        }
    }

    // Recount total
    TotalActiveAgents = 0;
    for (const auto& CountPair : ZoneAgentCounts)
    {
        TotalActiveAgents += CountPair.Value;
    }
}

void UCrowdZoneManager::SetMaxSimulatedAgents(int32 MaxAgents)
{
    MaxSimulatedAgents = FMath::Clamp(MaxAgents, 1, 50000);
    UE_LOG(LogTemp, Log, TEXT("[CrowdZoneManager] Max simulated agents set to: %d"), MaxSimulatedAgents);
}

float UCrowdZoneManager::GetDistanceBetweenZones(const FString& ZoneA, const FString& ZoneB) const
{
    if (!ZoneRegistry.Contains(ZoneA) || !ZoneRegistry.Contains(ZoneB)) return 9999999.0f;
    return FVector::Dist(ZoneRegistry[ZoneA].Value, ZoneRegistry[ZoneB].Value);
}

void UCrowdZoneManager::PrintCrowdStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== CROWD ZONE STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Zones: %d"), ZoneRegistry.Num());
    UE_LOG(LogTemp, Log, TEXT("Total Active Agents: %d / %d"), TotalActiveAgents, MaxSimulatedAgents);

    for (const auto& Pair : ZoneRegistry)
    {
        const FString& ZoneID = Pair.Key;
        const FCrowd_ZoneConfig& Config = Pair.Value.Key;
        const FVector& Center = Pair.Value.Value;
        ECrowd_AlertState Alert = ECrowd_AlertState::Calm;
        if (const ECrowd_AlertState* AlertPtr = ZoneAlertStates.Find(ZoneID))
        {
            Alert = *AlertPtr;
        }
        int32 Agents = 0;
        if (const int32* AgentPtr = ZoneAgentCounts.Find(ZoneID))
        {
            Agents = *AgentPtr;
        }

        UE_LOG(LogTemp, Log, TEXT("  [%s] Type=%d Center=(%.0f,%.0f,%.0f) Agents=%d Alert=%d Migrating=%s"),
            *ZoneID, (int32)Config.ZoneType,
            Center.X, Center.Y, Center.Z,
            Agents, (int32)Alert,
            Config.bIsMigrating ? TEXT("YES") : TEXT("NO"));
    }
    UE_LOG(LogTemp, Log, TEXT("=== END CROWD STATUS ==="));
}
