#include "DynamicQuestGenerator.h"
#include "QuestSystem.h"
#include "../CrowdSimulation/MassDinosaurSystem.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

void UDynamicQuestGenerator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    QuestSystem = GetGameInstance()->GetSubsystem<UQuestSystem>();

    // Configurar timers para monitoramento contínuo
    GetWorld()->GetTimerManager().SetTimer(
        EcosystemMonitorTimer,
        this,
        &UDynamicQuestGenerator::MonitorEcosystem,
        5.0f, // A cada 5 segundos
        true
    );

    GetWorld()->GetTimerManager().SetTimer(
        PatternAnalysisTimer,
        this,
        &UDynamicQuestGenerator::AnalyzeEntityClusters,
        PatternAnalysisInterval,
        true
    );

    UE_LOG(LogTemp, Warning, TEXT("DynamicQuestGenerator initialized - monitoring ecosystem"));
}

void UDynamicQuestGenerator::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(EcosystemMonitorTimer);
        GetWorld()->GetTimerManager().ClearTimer(PatternAnalysisTimer);
    }

    Super::Deinitialize();
}

void UDynamicQuestGenerator::MonitorEcosystem()
{
    if (!MassEntitySubsystem || !QuestSystem)
    {
        return;
    }

    // Atualizar contexto do jogador
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        UpdatePlayerContext(PlayerPawn->GetActorLocation(), FGameplayTagContainer());
    }

    // Detectar eventos emergentes
    DetectEmergentBehaviors();

    // Limpar eventos antigos
    CleanupOldEvents();

    // Atualizar cooldowns de templates
    UpdateTemplateCooldowns();
}

void UDynamicQuestGenerator::RegisterEcosystemEvent(const FEcosystemEvent& Event)
{
    // Verificar se o evento é relevante para o jogador
    float Relevance = CalculateQuestRelevance(Event, LastPlayerLocation);
    
    if (Relevance < EventRelevanceThreshold)
    {
        return;
    }

    // Adicionar evento à lista
    RecentEvents.Add(Event);

    // Manter apenas os eventos mais recentes
    if (RecentEvents.Num() > MaxActiveEvents)
    {
        RecentEvents.RemoveAt(0);
    }

    // Tentar gerar quest baseada no evento
    FName GeneratedQuest = NAME_None;

    if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag(\"Ecosystem.Hunting\")))
    {
        GeneratedQuest = GenerateHuntingQuest(Event);
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag(\"Ecosystem.Migration\")))
    {
        // Analisar se é parte de uma migração maior
        TArray<FMassEntityHandle> MigratingHerd;
        FVector Destination;
        if (DetectHerdMigration(MigratingHerd, Destination))
        {
            GeneratedQuest = GenerateMigrationQuest(MigratingHerd);
        }
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag(\"Ecosystem.Nesting\")))
    {
        FGameplayTag SpeciesTag = FGameplayTag::RequestGameplayTag(\"Species.Unknown\");
        GeneratedQuest = GenerateNestingQuest(Event.Location, SpeciesTag);
    }
    else if (Event.EventType.MatchesTag(FGameplayTag::RequestGameplayTag(\"Ecosystem.Emergency\")))
    {
        GeneratedQuest = GenerateEmergencyQuest(Event);
    }

    if (GeneratedQuest != NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT(\"Generated dynamic quest: %s from ecosystem event\"), *GeneratedQuest.ToString());
    }
}

TArray<FDinosaurBehaviorPattern> UDynamicQuestGenerator::AnalyzeBehaviorPatterns()
{
    TArray<FDinosaurBehaviorPattern> Patterns;

    if (!MassEntitySubsystem)
    {
        return Patterns;
    }

    // Analisar clusters de entidades e seus comportamentos
    AnalyzeEntityClusters();

    return ActivePatterns;
}

FName UDynamicQuestGenerator::GenerateObservationQuest(const FDinosaurBehaviorPattern& Pattern)
{
    if (!QuestSystem)
    {
        return NAME_None;
    }

    // Verificar se já existe uma quest similar ativa
    TArray<FName> ActiveQuests = QuestSystem->GetActiveQuests();
    for (const FName& QuestID : ActiveQuests)
    {
        FQuestData QuestData = QuestSystem->GetQuestData(QuestID);
        if (QuestData.QuestType == EQuestType::Observation)
        {
            // Já existe uma quest de observação ativa
            return NAME_None;
        }
    }

    // Gerar ID único para a quest
    FString QuestIDString = FString::Printf(TEXT(\"DynObs_%d_%s\"), 
        FMath::RandRange(1000, 9999),
        *Pattern.SpeciesTag.ToString()
    );
    FName QuestID = FName(*QuestIDString);

    // Criar dados da quest
    FQuestData NewQuest;
    NewQuest.QuestName = FText::FromString(FString::Printf(TEXT(\"Observe %s Behavior\"), 
        *Pattern.SpeciesTag.ToString()));
    NewQuest.QuestDescription = FText::FromString(FString::Printf(
        TEXT(\"A group of %s has been spotted displaying interesting %s behavior. Observe them from a safe distance to learn more about their habits.\"),
        *Pattern.SpeciesTag.ToString(),
        *UEnum::GetValueAsString(Pattern.DominantBehavior)
    ));
    NewQuest.QuestType = EQuestType::Observation;
    NewQuest.Priority = FMath::RoundToInt(Pattern.PatternStrength * 5.0f);
    NewQuest.TimeLimit = 600.0f; // 10 minutos
    NewQuest.bAutoStart = true;

    // Criar objetivo de observação
    FQuestObjectiveData ObservationObjective;
    ObservationObjective.ObjectiveText = FText::FromString(TEXT(\"Observe the dinosaur behavior for 60 seconds\"));
    ObservationObjective.ObjectiveType = EObjectiveType::Observe;
    ObservationObjective.TargetCount = 60; // 60 segundos
    ObservationObjective.TargetLocation = Pattern.CenterLocation;
    ObservationObjective.TargetRadius = Pattern.PatternRadius * 1.5f; // Margem de segurança
    ObservationObjective.RequiredTags.AddTag(Pattern.SpeciesTag);

    NewQuest.Objectives.Add(ObservationObjective);

    // Configurar recompensas
    NewQuest.Reward.Experience = 50;
    NewQuest.Reward.RewardDescription = FText::FromString(TEXT(\"Knowledge about dinosaur behavior\"));
    NewQuest.Reward.UnlockTags.AddTag(FGameplayTag::RequestGameplayTag(\"Knowledge.Behavior.\" + Pattern.SpeciesTag.ToString()));

    // Adicionar quest ao sistema
    QuestSystem->QuestDatabase.Add(QuestID, NewQuest);
    QuestSystem->StartQuest(QuestID);

    return QuestID;
}

FName UDynamicQuestGenerator::GenerateMigrationQuest(const TArray<FMassEntityHandle>& MigratingHerd)
{
    if (!QuestSystem || MigratingHerd.Num() == 0)
    {
        return NAME_None;
    }

    FString QuestIDString = FString::Printf(TEXT(\"DynMig_%d\"), FMath::RandRange(1000, 9999));
    FName QuestID = FName(*QuestIDString);

    FQuestData NewQuest;
    NewQuest.QuestName = FText::FromString(TEXT(\"Follow the Great Migration\"));
    NewQuest.QuestDescription = FText::FromString(TEXT(\"A large herd of dinosaurs has begun migrating. Follow them at a safe distance to discover their destination and learn about their migration patterns.\"));
    NewQuest.QuestType = EQuestType::Discovery;
    NewQuest.Priority = 7;
    NewQuest.TimeLimit = 1200.0f; // 20 minutos
    NewQuest.bAutoStart = true;

    // Objetivo: Seguir a migração
    FQuestObjectiveData FollowObjective;
    FollowObjective.ObjectiveText = FText::FromString(TEXT(\"Follow the migrating herd\"));
    FollowObjective.ObjectiveType = EObjectiveType::Reach;
    FollowObjective.TargetCount = 3; // 3 pontos de controle
    FollowObjective.TargetRadius = 200.0f;

    NewQuest.Objectives.Add(FollowObjective);

    // Recompensas
    NewQuest.Reward.Experience = 100;
    NewQuest.Reward.RewardDescription = FText::FromString(TEXT(\"Understanding of migration patterns\"));

    QuestSystem->QuestDatabase.Add(QuestID, NewQuest);
    QuestSystem->StartQuest(QuestID);

    return QuestID;
}

FName UDynamicQuestGenerator::GenerateHuntingQuest(const FEcosystemEvent& HuntingEvent)
{
    if (!QuestSystem)
    {
        return NAME_None;
    }

    FString QuestIDString = FString::Printf(TEXT(\"DynHunt_%d\"), FMath::RandRange(1000, 9999));
    FName QuestID = FName(*QuestIDString);

    FQuestData NewQuest;
    NewQuest.QuestName = FText::FromString(TEXT(\"Witness the Hunt\"));
    NewQuest.QuestDescription = FText::FromString(TEXT(\"A predator has begun stalking its prey nearby. Observe this natural hunting behavior, but stay hidden and safe!\"));
    NewQuest.QuestType = EQuestType::Observation;
    NewQuest.Priority = 8;
    NewQuest.TimeLimit = 300.0f; // 5 minutos
    NewQuest.bAutoStart = true;

    // Objetivo: Observar sem ser detectado
    FQuestObjectiveData WitnessObjective;
    WitnessObjective.ObjectiveText = FText::FromString(TEXT(\"Observe the hunt without being detected\"));
    WitnessObjective.ObjectiveType = EObjectiveType::Observe;
    WitnessObjective.TargetCount = 30; // 30 segundos
    WitnessObjective.TargetLocation = HuntingEvent.Location;
    WitnessObjective.TargetRadius = 300.0f;

    NewQuest.Objectives.Add(WitnessObjective);

    // Recompensas
    NewQuest.Reward.Experience = 75;
    NewQuest.Reward.RewardDescription = FText::FromString(TEXT(\"Knowledge of predator behavior\"));

    QuestSystem->QuestDatabase.Add(QuestID, NewQuest);
    QuestSystem->StartQuest(QuestID);

    return QuestID;
}

FName UDynamicQuestGenerator::GenerateNestingQuest(FVector NestLocation, FGameplayTag SpeciesTag)
{
    if (!QuestSystem)
    {
        return NAME_None;
    }

    FString QuestIDString = FString::Printf(TEXT(\"DynNest_%d\"), FMath::RandRange(1000, 9999));
    FName QuestID = FName(*QuestIDString);

    FQuestData NewQuest;
    NewQuest.QuestName = FText::FromString(TEXT(\"Discover the Nesting Ground\"));
    NewQuest.QuestDescription = FText::FromString(TEXT(\"Dinosaurs have been spotted preparing a nesting area. Investigate this rare opportunity to observe nesting behavior, but approach with extreme caution.\"));
    NewQuest.QuestType = EQuestType::Discovery;
    NewQuest.Priority = 9;
    NewQuest.TimeLimit = 900.0f; // 15 minutos
    NewQuest.bAutoStart = true;

    // Objetivo: Investigar o ninho
    FQuestObjectiveData InvestigateObjective;
    InvestigateObjective.ObjectiveText = FText::FromString(TEXT(\"Investigate the nesting ground\"));
    InvestigateObjective.ObjectiveType = EObjectiveType::Reach;
    InvestigateObjective.TargetLocation = NestLocation;
    InvestigateObjective.TargetRadius = 100.0f;

    NewQuest.Objectives.Add(InvestigateObjective);

    // Recompensas
    NewQuest.Reward.Experience = 150;
    NewQuest.Reward.RewardDescription = FText::FromString(TEXT(\"Rare knowledge of nesting behavior\"));

    QuestSystem->QuestDatabase.Add(QuestID, NewQuest);
    QuestSystem->StartQuest(QuestID);

    return QuestID;
}

FName UDynamicQuestGenerator::GenerateEmergencyQuest(const FEcosystemEvent& EmergencyEvent)
{
    if (!QuestSystem)
    {
        return NAME_None;
    }

    FString QuestIDString = FString::Printf(TEXT(\"DynEmerg_%d\"), FMath::RandRange(1000, 9999));
    FName QuestID = FName(*QuestIDString);

    FQuestData NewQuest;
    NewQuest.QuestName = FText::FromString(TEXT(\"Emergency: Ecosystem Disturbance\"));
    NewQuest.QuestDescription = FText::FromString(TEXT(\"Something has disturbed the local ecosystem. Investigate the cause while staying safe.\"));
    NewQuest.QuestType = EQuestType::Emergency;
    NewQuest.Priority = 10;
    NewQuest.TimeLimit = 600.0f;
    NewQuest.bAutoStart = true;

    // Objetivo: Investigar a perturbação
    FQuestObjectiveData InvestigateObjective;
    InvestigateObjective.ObjectiveText = FText::FromString(TEXT(\"Investigate the ecosystem disturbance\"));
    InvestigateObjective.ObjectiveType = EObjectiveType::Reach;
    InvestigateObjective.TargetLocation = EmergencyEvent.Location;
    InvestigateObjective.TargetRadius = 150.0f;

    NewQuest.Objectives.Add(InvestigateObjective);

    // Recompensas
    NewQuest.Reward.Experience = 200;
    NewQuest.Reward.RewardDescription = FText::FromString(TEXT(\"Crisis management experience\"));

    QuestSystem->QuestDatabase.Add(QuestID, NewQuest);
    QuestSystem->StartQuest(QuestID);

    return QuestID;
}

bool UDynamicQuestGenerator::DetectHerdMigration(TArray<FMassEntityHandle>& OutHerd, FVector& OutDestination)
{
    // Implementação simplificada - seria expandida com análise real do Mass
    return false;
}

bool UDynamicQuestGenerator::DetectPredatorHunt(FMassEntityHandle& OutPredator, TArray<FMassEntityHandle>& OutPrey)
{
    // Implementação simplificada
    return false;
}

bool UDynamicQuestGenerator::DetectNestingBehavior(FVector& OutNestLocation, FGameplayTag& OutSpecies)
{
    // Implementação simplificada
    return false;
}

bool UDynamicQuestGenerator::DetectTerritorialDispute(TArray<FMassEntityHandle>& OutCompetitors, FVector& OutTerritory)
{
    // Implementação simplificada
    return false;
}

void UDynamicQuestGenerator::UpdatePlayerContext(FVector PlayerLocation, const FGameplayTagContainer& PlayerTags)
{
    LastPlayerLocation = PlayerLocation;
    PlayerContextTags = PlayerTags;
    LastPlayerUpdate = GetWorld()->GetTimeSeconds();
}

float UDynamicQuestGenerator::CalculateQuestRelevance(const FEcosystemEvent& Event, FVector PlayerLocation)
{
    float Distance = FVector::Dist(Event.Location, PlayerLocation);
    float MaxRelevantDistance = EcosystemMonitoringRadius;
    
    // Relevância baseada na distância (mais próximo = mais relevante)
    float DistanceRelevance = FMath::Clamp(1.0f - (Distance / MaxRelevantDistance), 0.0f, 1.0f);
    
    // Relevância baseada na intensidade do evento
    float IntensityRelevance = Event.Intensity;
    
    // Combinação final
    return (DistanceRelevance * 0.6f) + (IntensityRelevance * 0.4f);
}

void UDynamicQuestGenerator::AnalyzeEntityClusters()
{
    // Esta função seria expandida para analisar clusters reais do Mass Entity System
    // Por agora, implementação placeholder
    
    UE_LOG(LogTemp, Log, TEXT(\"Analyzing entity clusters for behavior patterns...\"));
}

void UDynamicQuestGenerator::DetectEmergentBehaviors()
{
    // Detectar comportamentos emergentes baseados nos dados do Mass
    // Implementação placeholder que seria expandida
    
    // Simular detecção de eventos ocasionais para teste
    if (FMath::RandRange(0, 100) < 5) // 5% chance por ciclo
    {
        FEcosystemEvent TestEvent;
        TestEvent.EventType = FGameplayTag::RequestGameplayTag(\"Ecosystem.Hunting\");
        TestEvent.Location = LastPlayerLocation + FVector(FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000), 0);
        TestEvent.Radius = 500.0f;
        TestEvent.Timestamp = GetWorld()->GetTimeSeconds();
        TestEvent.Intensity = FMath::RandRange(0.3f, 1.0f);
        
        RegisterEcosystemEvent(TestEvent);
    }
}

void UDynamicQuestGenerator::CleanupOldEvents()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MaxEventAge = 600.0f; // 10 minutos
    
    RecentEvents.RemoveAll([CurrentTime, MaxEventAge](const FEcosystemEvent& Event)
    {
        return (CurrentTime - Event.Timestamp) > MaxEventAge;
    });
}

void UDynamicQuestGenerator::UpdateTemplateCooldowns()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = 5.0f; // Intervalo do timer
    
    for (auto& Cooldown : TemplateCooldowns)
    {
        Cooldown.Value = FMath::Max(0.0f, Cooldown.Value - DeltaTime);
    }
}