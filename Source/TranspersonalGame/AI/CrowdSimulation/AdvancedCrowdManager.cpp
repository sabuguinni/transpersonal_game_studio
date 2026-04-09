#include "AdvancedCrowdManager.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

void UAdvancedCrowdManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Initializing..."));
    
    // Configuração padrão de densidade dinâmica
    DensityConfig.HighDetailRadius = 2000.0f;
    DensityConfig.MediumDetailRadius = 5000.0f;
    DensityConfig.LowDetailRadius = 15000.0f;
    DensityConfig.CullingRadius = 25000.0f;
    DensityConfig.HighDetailDensityMultiplier = 1.0f;
    DensityConfig.MediumDetailDensityMultiplier = 0.5f;
    DensityConfig.LowDetailDensityMultiplier = 0.2f;
    DensityConfig.MaxAgentsHighDetail = 5000;
    DensityConfig.MaxAgentsMediumDetail = 15000;
    DensityConfig.MaxAgentsLowDetail = 30000;
    
    // Inicializar eventos emergentes padrão
    FEmergentEventConfig PredatorHuntEvent;
    PredatorHuntEvent.EventName = TEXT("PredatorHunt");
    PredatorHuntEvent.EventProbabilityPerHour = 0.3f;
    PredatorHuntEvent.EventDuration = 180.0f;
    PredatorHuntEvent.EventRadius = 3000.0f;
    PredatorHuntEvent.AffectedSpecies = {EJurassicAgentType::Herbivore, EJurassicAgentType::SmallCarnivore};
    PredatorHuntEvent.ForcedBehaviorState = EJurassicBehaviorState::Fleeing;
    PredatorHuntEvent.bRequiresPlayerProximity = true;
    PredatorHuntEvent.MinPlayerDistance = 500.0f;
    PredatorHuntEvent.MaxPlayerDistance = 5000.0f;
    EmergentEventConfigs.Add(PredatorHuntEvent);
    
    FEmergentEventConfig HerdMigrationEvent;
    HerdMigrationEvent.EventName = TEXT("HerdMigration");
    HerdMigrationEvent.EventProbabilityPerHour = 0.1f;
    HerdMigrationEvent.EventDuration = 600.0f;
    HerdMigrationEvent.EventRadius = 8000.0f;
    HerdMigrationEvent.AffectedSpecies = {EJurassicAgentType::Herbivore};
    HerdMigrationEvent.ForcedBehaviorState = EJurassicBehaviorState::Moving;
    HerdMigrationEvent.bRequiresPlayerProximity = false;
    EmergentEventConfigs.Add(HerdMigrationEvent);
    
    // Inicializar regras de comunicação
    FSpeciesCommunicationRule HerbivorePanicRule;
    HerbivorePanicRule.SourceSpecies = EJurassicAgentType::Herbivore;
    HerbivorePanicRule.TargetSpecies = EJurassicAgentType::Herbivore;
    HerbivorePanicRule.TriggerState = EJurassicBehaviorState::Fleeing;
    HerbivorePanicRule.ResponseState = EJurassicBehaviorState::Alert;
    HerbivorePanicRule.CommunicationRange = 2000.0f;
    HerbivorePanicRule.ResponseProbability = 0.9f;
    HerbivorePanicRule.ResponseDelay = 1.0f;
    CommunicationRules.Add(HerbivorePanicRule);
    
    FSpeciesCommunicationRule PredatorHuntRule;
    PredatorHuntRule.SourceSpecies = EJurassicAgentType::LargeCarnivore;
    PredatorHuntRule.TargetSpecies = EJurassicAgentType::Herbivore;
    PredatorHuntRule.TriggerState = EJurassicBehaviorState::Hunting;
    PredatorHuntRule.ResponseState = EJurassicBehaviorState::Fleeing;
    PredatorHuntRule.CommunicationRange = 5000.0f;
    PredatorHuntRule.ResponseProbability = 1.0f;
    PredatorHuntRule.ResponseDelay = 0.5f;
    CommunicationRules.Add(PredatorHuntRule);
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Initialized with %d event configs and %d communication rules"), 
           EmergentEventConfigs.Num(), CommunicationRules.Num());
}

void UAdvancedCrowdManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Deinitializing..."));
    
    // Limpar eventos ativos
    ActiveEvents.Empty();
    CurrentMigrationWaypoint.Empty();
    
    Super::Deinitialize();
}

void UAdvancedCrowdManager::Tick(float DeltaTime)
{
    // Atualizar tempo de estação
    CurrentSeasonTime += DeltaTime;
    
    // Processar eventos emergentes ativos
    for (int32 i = ActiveEvents.Num() - 1; i >= 0; --i)
    {
        FActiveEmergentEvent& Event = ActiveEvents[i];
        Event.RemainingDuration -= DeltaTime;
        
        if (Event.RemainingDuration <= 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Event '%s' has ended"), *Event.EventName);
            ActiveEvents.RemoveAt(i);
        }
    }
    
    // Atualizar migração sazonal
    UpdateMigrationProgress(DeltaTime);
    
    // Verificar se devemos disparar novos eventos emergentes
    static float EventCheckTimer = 0.0f;
    EventCheckTimer += DeltaTime;
    
    if (EventCheckTimer >= 60.0f) // Verificar a cada minuto
    {
        EventCheckTimer = 0.0f;
        CheckForEmergentEvents();
    }
}

bool UAdvancedCrowdManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UAdvancedCrowdManager::UpdateDynamicDensity(const FVector& PlayerLocation)
{
    UMassEntitySubsystem* MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedCrowdManager: MassEntitySubsystem not found"));
        return;
    }
    
    // Implementar lógica de densidade dinâmica baseada na distância do jogador
    // Esta é uma implementação simplificada - em produção seria mais complexa
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AdvancedCrowdManager: Updating dynamic density around player at %s"), 
           *PlayerLocation.ToString());
}

void UAdvancedCrowdManager::SetDynamicDensityConfig(const FDynamicDensityConfig& NewConfig)
{
    DensityConfig = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Updated density config - High detail radius: %.1f"), 
           DensityConfig.HighDetailRadius);
}

int32 UAdvancedCrowdManager::GetOptimalAgentCountForLocation(const FVector& Location) const
{
    // Obter localização do jogador
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return DensityConfig.MaxAgentsLowDetail;
    }
    
    float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
    
    if (DistanceToPlayer <= DensityConfig.HighDetailRadius)
    {
        return DensityConfig.MaxAgentsHighDetail;
    }
    else if (DistanceToPlayer <= DensityConfig.MediumDetailRadius)
    {
        return DensityConfig.MaxAgentsMediumDetail;
    }
    else if (DistanceToPlayer <= DensityConfig.LowDetailRadius)
    {
        return DensityConfig.MaxAgentsLowDetail;
    }
    
    return 0; // Além do raio de culling
}

void UAdvancedCrowdManager::RegisterEmergentEvent(const FEmergentEventConfig& EventConfig)
{
    EmergentEventConfigs.Add(EventConfig);
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Registered emergent event '%s'"), *EventConfig.EventName);
}

bool UAdvancedCrowdManager::TriggerEmergentEvent(const FString& EventName, const FVector& Location)
{
    // Encontrar configuração do evento
    const FEmergentEventConfig* EventConfig = EmergentEventConfigs.FindByPredicate(
        [&EventName](const FEmergentEventConfig& Config)
        {
            return Config.EventName == EventName;
        });
    
    if (!EventConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedCrowdManager: Event config '%s' not found"), *EventName);
        return false;
    }
    
    // Verificar se o evento já está ativo
    bool bEventAlreadyActive = ActiveEvents.ContainsByPredicate(
        [&EventName](const FActiveEmergentEvent& Event)
        {
            return Event.EventName == EventName;
        });
    
    if (bEventAlreadyActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedCrowdManager: Event '%s' is already active"), *EventName);
        return false;
    }
    
    // Verificar proximidade do jogador se necessário
    if (EventConfig->bRequiresPlayerProximity)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            if (DistanceToPlayer < EventConfig->MinPlayerDistance || 
                DistanceToPlayer > EventConfig->MaxPlayerDistance)
            {
                UE_LOG(LogTemp, VeryVerbose, TEXT("AdvancedCrowdManager: Event '%s' failed player proximity check"), *EventName);
                return false;
            }
        }
    }
    
    // Criar evento ativo
    FActiveEmergentEvent NewEvent;
    NewEvent.EventName = EventName;
    NewEvent.EventLocation = Location;
    NewEvent.EventRadius = EventConfig->EventRadius;
    NewEvent.RemainingDuration = EventConfig->EventDuration;
    NewEvent.ForcedState = EventConfig->ForcedBehaviorState;
    NewEvent.AffectedSpecies = EventConfig->AffectedSpecies;
    
    ActiveEvents.Add(NewEvent);
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Triggered event '%s' at %s"), 
           *EventName, *Location.ToString());
    
    return true;
}

void UAdvancedCrowdManager::StopEmergentEvent(const FString& EventName)
{
    int32 RemovedCount = ActiveEvents.RemoveAll(
        [&EventName](const FActiveEmergentEvent& Event)
        {
            return Event.EventName == EventName;
        });
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Stopped event '%s'"), *EventName);
    }
}

TArray<FString> UAdvancedCrowdManager::GetActiveEmergentEvents() const
{
    TArray<FString> ActiveEventNames;
    for (const FActiveEmergentEvent& Event : ActiveEvents)
    {
        ActiveEventNames.Add(Event.EventName);
    }
    return ActiveEventNames;
}

void UAdvancedCrowdManager::StartSeasonalMigration(const FSeasonalMigrationConfig& MigrationConfig)
{
    MigrationConfigs.Add(MigrationConfig);
    
    // Inicializar waypoints para as espécies migratórias
    for (EJurassicAgentType Species : MigrationConfig.MigratingSpecies)
    {
        CurrentMigrationWaypoint.Add(Species, 0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Started seasonal migration for %d species"), 
           MigrationConfig.MigratingSpecies.Num());
}

void UAdvancedCrowdManager::UpdateMigrationProgress(float DeltaTime)
{
    for (const FSeasonalMigrationConfig& MigrationConfig : MigrationConfigs)
    {
        if (MigrationConfig.bCyclicalMigration)
        {
            // Calcular progresso da estação
            float SeasonProgress = FMath::Fmod(CurrentSeasonTime, MigrationConfig.SeasonDuration) / MigrationConfig.SeasonDuration;
            
            // Determinar waypoint atual baseado no progresso
            int32 TargetWaypoint = FMath::FloorToInt(SeasonProgress * MigrationConfig.MigrationWaypoints.Num());
            TargetWaypoint = FMath::Clamp(TargetWaypoint, 0, MigrationConfig.MigrationWaypoints.Num() - 1);
            
            // Atualizar waypoint para cada espécie
            for (EJurassicAgentType Species : MigrationConfig.MigratingSpecies)
            {
                if (int32* CurrentWaypoint = CurrentMigrationWaypoint.Find(Species))
                {
                    if (*CurrentWaypoint != TargetWaypoint)
                    {
                        *CurrentWaypoint = TargetWaypoint;
                        UE_LOG(LogTemp, VeryVerbose, TEXT("AdvancedCrowdManager: Species %d migrating to waypoint %d"), 
                               (int32)Species, TargetWaypoint);
                    }
                }
            }
        }
    }
}

bool UAdvancedCrowdManager::IsSpeciesMigrating(EJurassicAgentType Species) const
{
    for (const FSeasonalMigrationConfig& MigrationConfig : MigrationConfigs)
    {
        if (MigrationConfig.MigratingSpecies.Contains(Species))
        {
            return true;
        }
    }
    return false;
}

void UAdvancedCrowdManager::RegisterCommunicationRule(const FSpeciesCommunicationRule& Rule)
{
    CommunicationRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Registered communication rule from %d to %d"), 
           (int32)Rule.SourceSpecies, (int32)Rule.TargetSpecies);
}

void UAdvancedCrowdManager::ProcessSpeciesCommunication(FMassEntityHandle SourceEntity, EJurassicBehaviorState NewState)
{
    // Esta função seria chamada quando um agente muda de estado
    // Implementação simplificada - em produção seria mais complexa
    UE_LOG(LogTemp, VeryVerbose, TEXT("AdvancedCrowdManager: Processing species communication for state change to %d"), 
           (int32)NewState);
}

void UAdvancedCrowdManager::PropagateAlertSignal(const FVector& AlertLocation, EJurassicAgentType SourceSpecies, float AlertRadius, float AlertIntensity)
{
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Propagating alert signal from %d at %s (radius: %.1f, intensity: %.2f)"), 
           (int32)SourceSpecies, *AlertLocation.ToString(), AlertRadius, AlertIntensity);
    
    // Implementação simplificada - em produção afetaria agentes na área
}

void UAdvancedCrowdManager::EnableDebugVisualization(bool bEnable)
{
    bDebugVisualization = bEnable;
    UE_LOG(LogTemp, Log, TEXT("AdvancedCrowdManager: Debug visualization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FString UAdvancedCrowdManager::GetCrowdSimulationStats() const
{
    FString Stats = FString::Printf(TEXT("Active Events: %d, Migration Configs: %d, Communication Rules: %d, Season Time: %.1f"),
                                   ActiveEvents.Num(),
                                   MigrationConfigs.Num(),
                                   CommunicationRules.Num(),
                                   CurrentSeasonTime);
    return Stats;
}

TArray<FVector> UAdvancedCrowdManager::GetHerdCentersInRadius(const FVector& Center, float Radius) const
{
    TArray<FVector> HerdCenters;
    
    // Implementação simplificada - em produção consultaria o sistema Mass Entity
    // para encontrar centros de rebanhos reais
    
    return HerdCenters;
}

void UAdvancedCrowdManager::OptimizeAgentLOD(const FVector& PlayerLocation)
{
    // Implementar otimização de LOD baseada na distância do jogador
    UE_LOG(LogTemp, VeryVerbose, TEXT("AdvancedCrowdManager: Optimizing agent LOD around %s"), 
           *PlayerLocation.ToString());
}

void UAdvancedCrowdManager::CullDistantAgents(const FVector& PlayerLocation)
{
    // Implementar culling de agentes distantes
    UE_LOG(LogTemp, VeryVerbose, TEXT("AdvancedCrowdManager: Culling distant agents from %s"), 
           *PlayerLocation.ToString());
}

void UAdvancedCrowdManager::CheckForEmergentEvents()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (const FEmergentEventConfig& EventConfig : EmergentEventConfigs)
    {
        // Verificar probabilidade
        float RandomValue = FMath::RandRange(0.0f, 1.0f);
        float HourlyProbability = EventConfig.EventProbabilityPerHour / 60.0f; // Converter para por minuto
        
        if (RandomValue <= HourlyProbability)
        {
            // Gerar localização aleatória próxima ao jogador
            FVector EventLocation = PlayerLocation + FMath::VRand() * FMath::RandRange(EventConfig.MinPlayerDistance, EventConfig.MaxPlayerDistance);
            
            TriggerEmergentEvent(EventConfig.EventName, EventLocation);
        }
    }
}