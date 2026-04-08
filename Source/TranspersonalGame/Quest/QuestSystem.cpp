#include "QuestSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

void UQuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Inicializar timer para verificações periódicas
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(QuestUpdateTimer, 
            FTimerDelegate::CreateUObject(this, &UQuestSystem::CheckQuestTimeouts), 
            5.0f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest System initialized"));
}

void UQuestSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UQuestSystem::StartQuest(FName QuestID)
{
    if (!QuestDatabase.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s not found in database"), *QuestID.ToString());
        return false;
    }

    const FQuestData& QuestData = QuestDatabase[QuestID];
    
    // Verificar pré-requisitos
    // TODO: Implementar verificação de gameplay tags do jogador
    
    // Inicializar quest
    QuestStatuses.Add(QuestID, EQuestStatus::Active);
    QuestStartTimes.Add(QuestID, GetWorld()->GetTimeSeconds());
    
    // Inicializar progresso dos objetivos
    TArray<int32> InitialProgress;
    for (int32 i = 0; i < QuestData.Objectives.Num(); i++)
    {
        InitialProgress.Add(0);
    }
    ObjectiveProgress.Add(QuestID, InitialProgress);
    
    NotifyQuestStatusChange(QuestID, EQuestStatus::Active);
    
    UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *QuestData.QuestName.ToString());
    return true;
}

bool UQuestSystem::CompleteQuest(FName QuestID)
{
    if (!QuestStatuses.Contains(QuestID) || QuestStatuses[QuestID] != EQuestStatus::Active)
    {
        return false;
    }

    QuestStatuses[QuestID] = EQuestStatus::Completed;
    
    // Aplicar recompensas
    const FQuestData& QuestData = QuestDatabase[QuestID];
    // TODO: Implementar sistema de recompensas
    
    NotifyQuestStatusChange(QuestID, EQuestStatus::Completed);
    
    UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *QuestData.QuestName.ToString());
    return true;
}

bool UQuestSystem::FailQuest(FName QuestID)
{
    if (!QuestStatuses.Contains(QuestID) || QuestStatuses[QuestID] != EQuestStatus::Active)
    {
        return false;
    }

    QuestStatuses[QuestID] = EQuestStatus::Failed;
    NotifyQuestStatusChange(QuestID, EQuestStatus::Failed);
    
    UE_LOG(LogTemp, Log, TEXT("Failed quest: %s"), *QuestID.ToString());
    return true;
}

bool UQuestSystem::AbandonQuest(FName QuestID)
{
    if (!QuestStatuses.Contains(QuestID) || QuestStatuses[QuestID] != EQuestStatus::Active)
    {
        return false;
    }

    QuestStatuses[QuestID] = EQuestStatus::Abandoned;
    NotifyQuestStatusChange(QuestID, EQuestStatus::Abandoned);
    
    UE_LOG(LogTemp, Log, TEXT("Abandoned quest: %s"), *QuestID.ToString());
    return true;
}

EQuestStatus UQuestSystem::GetQuestStatus(FName QuestID) const
{
    if (QuestStatuses.Contains(QuestID))
    {
        return QuestStatuses[QuestID];
    }
    return EQuestStatus::NotStarted;
}

TArray<FName> UQuestSystem::GetActiveQuests() const
{
    TArray<FName> ActiveQuests;
    for (const auto& QuestPair : QuestStatuses)
    {
        if (QuestPair.Value == EQuestStatus::Active)
        {
            ActiveQuests.Add(QuestPair.Key);
        }
    }
    return ActiveQuests;
}

TArray<FName> UQuestSystem::GetAvailableQuests() const
{
    TArray<FName> AvailableQuests;
    for (const auto& QuestPair : QuestDatabase)
    {
        FName QuestID = QuestPair.Key;
        if (!QuestStatuses.Contains(QuestID) || QuestStatuses[QuestID] == EQuestStatus::NotStarted)
        {
            // TODO: Verificar pré-requisitos
            AvailableQuests.Add(QuestID);
        }
    }
    return AvailableQuests;
}

bool UQuestSystem::UpdateObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 Progress)
{
    if (!ObjectiveProgress.Contains(QuestID) || 
        !ObjectiveProgress[QuestID].IsValidIndex(ObjectiveIndex))
    {
        return false;
    }

    const FQuestData& QuestData = QuestDatabase[QuestID];
    if (!QuestData.Objectives.IsValidIndex(ObjectiveIndex))
    {
        return false;
    }

    TArray<int32>& Progress_Array = ObjectiveProgress[QuestID];
    Progress_Array[ObjectiveIndex] = FMath::Min(Progress_Array[ObjectiveIndex] + Progress, 
                                               QuestData.Objectives[ObjectiveIndex].TargetCount);

    OnObjectiveUpdated.Broadcast(QuestID, ObjectiveIndex, Progress_Array[ObjectiveIndex]);

    // Verificar se o objetivo foi completado
    if (Progress_Array[ObjectiveIndex] >= QuestData.Objectives[ObjectiveIndex].TargetCount)
    {
        CompleteObjective(QuestID, ObjectiveIndex);
    }

    return true;
}

bool UQuestSystem::CompleteObjective(FName QuestID, int32 ObjectiveIndex)
{
    if (!ObjectiveProgress.Contains(QuestID))
    {
        return false;
    }

    const FQuestData& QuestData = QuestDatabase[QuestID];
    
    // Verificar se todos os objetivos obrigatórios foram completados
    bool AllRequiredObjectivesComplete = true;
    for (int32 i = 0; i < QuestData.Objectives.Num(); i++)
    {
        if (!QuestData.Objectives[i].bOptional)
        {
            int32 CurrentProgress = ObjectiveProgress[QuestID][i];
            if (CurrentProgress < QuestData.Objectives[i].TargetCount)
            {
                AllRequiredObjectivesComplete = false;
                break;
            }
        }
    }

    if (AllRequiredObjectivesComplete)
    {
        CompleteQuest(QuestID);
    }

    return true;
}

int32 UQuestSystem::GetObjectiveProgress(FName QuestID, int32 ObjectiveIndex) const
{
    if (ObjectiveProgress.Contains(QuestID) && 
        ObjectiveProgress[QuestID].IsValidIndex(ObjectiveIndex))
    {
        return ObjectiveProgress[QuestID][ObjectiveIndex];
    }
    return 0;
}

FName UQuestSystem::GenerateDynamicQuest(EQuestType QuestType, const FGameplayTagContainer& ContextTags)
{
    switch (QuestType)
    {
        case EQuestType::Survival:
            return GenerateSurvivalQuest(ContextTags);
        case EQuestType::Observation:
            return GenerateObservationQuest(ContextTags);
        case EQuestType::Discovery:
            return GenerateDiscoveryQuest(ContextTags);
        case EQuestType::Domestication:
            return GenerateDomesticationQuest(ContextTags);
        case EQuestType::Emergency:
            return GenerateEmergencyQuest(ContextTags);
        default:
            return NAME_None;
    }
}

void UQuestSystem::ProcessEcosystemEvents(const FGameplayTagContainer& EventTags, FVector Location)
{
    // Gerar missões dinâmicas baseadas em eventos do ecossistema
    
    if (EventTags.HasTag(FGameplayTag::RequestGameplayTag("Ecosystem.PredatorHunt")))
    {
        // Gerar missão de observação de caça
        FGameplayTagContainer ObservationTags;
        ObservationTags.AddTag(FGameplayTag::RequestGameplayTag("Behavior.Hunting"));
        ObservationTags.AddTag(FGameplayTag::RequestGameplayTag("Location.Specific"));
        GenerateDynamicQuest(EQuestType::Observation, ObservationTags);
    }
    
    if (EventTags.HasTag(FGameplayTag::RequestGameplayTag("Ecosystem.HerbivoreGrazing")))
    {
        // Gerar missão de domesticação
        FGameplayTagContainer DomesticationTags;
        DomesticationTags.AddTag(FGameplayTag::RequestGameplayTag("Species.Herbivore"));
        DomesticationTags.AddTag(FGameplayTag::RequestGameplayTag("Behavior.Peaceful"));
        GenerateDynamicQuest(EQuestType::Domestication, DomesticationTags);
    }
    
    if (EventTags.HasTag(FGameplayTag::RequestGameplayTag("Ecosystem.ResourceDepletion")))
    {
        // Gerar missão de sobrevivência
        FGameplayTagContainer SurvivalTags;
        SurvivalTags.AddTag(FGameplayTag::RequestGameplayTag("Resource.Food"));
        SurvivalTags.AddTag(FGameplayTag::RequestGameplayTag("Urgency.High"));
        GenerateDynamicQuest(EQuestType::Survival, SurvivalTags);
    }
}

FQuestData UQuestSystem::GetQuestData(FName QuestID) const
{
    if (QuestDatabase.Contains(QuestID))
    {
        return QuestDatabase[QuestID];
    }
    return FQuestData();
}

void UQuestSystem::LoadQuestDatabase(UDataTable* QuestTable)
{
    if (!QuestTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest table is null"));
        return;
    }

    QuestDatabase.Empty();
    
    TArray<FQuestData*> AllQuests;
    QuestTable->GetAllRows<FQuestData>("", AllQuests);
    
    for (FQuestData* Quest : AllQuests)
    {
        if (Quest)
        {
            FName QuestID(*Quest->QuestName.ToString());
            QuestDatabase.Add(QuestID, *Quest);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d quests from database"), QuestDatabase.Num());
}

FName UQuestSystem::GenerateSurvivalQuest(const FGameplayTagContainer& ContextTags)
{
    FQuestData NewQuest;
    FName QuestID = FName(*FString::Printf(TEXT("DynamicSurvival_%d"), ++DynamicQuestCounter));
    
    NewQuest.QuestName = FText::FromString("Sobrevivência Urgente");
    NewQuest.QuestDescription = FText::FromString("Os recursos estão escassos. Encontre comida e água antes que seja tarde demais.");
    NewQuest.QuestType = EQuestType::Survival;
    NewQuest.Priority = 8;
    NewQuest.TimeLimit = 1800.0f; // 30 minutos
    
    // Objetivo: Coletar comida
    FQuestObjectiveData FoodObjective;
    FoodObjective.ObjectiveText = FText::FromString("Colete 5 unidades de comida");
    FoodObjective.ObjectiveType = EObjectiveType::Collect;
    FoodObjective.TargetCount = 5;
    FoodObjective.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Item.Food"));
    
    // Objetivo: Encontrar água
    FQuestObjectiveData WaterObjective;
    WaterObjective.ObjectiveText = FText::FromString("Encontre uma fonte de água");
    WaterObjective.ObjectiveType = EObjectiveType::Discover;
    WaterObjective.TargetCount = 1;
    WaterObjective.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Location.Water"));
    
    NewQuest.Objectives.Add(FoodObjective);
    NewQuest.Objectives.Add(WaterObjective);
    
    // Recompensa
    NewQuest.Reward.Experience = 100;
    NewQuest.Reward.RewardDescription = FText::FromString("Sobreviveu à escassez de recursos");
    
    QuestDatabase.Add(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("Generated dynamic survival quest: %s"), *QuestID.ToString());
    return QuestID;
}

FName UQuestSystem::GenerateObservationQuest(const FGameplayTagContainer& ContextTags)
{
    FQuestData NewQuest;
    FName QuestID = FName(*FString::Printf(TEXT("DynamicObservation_%d"), ++DynamicQuestCounter));
    
    NewQuest.QuestName = FText::FromString("Observação Comportamental");
    NewQuest.QuestDescription = FText::FromString("Observe o comportamento natural dos dinossauros sem ser detectado.");
    NewQuest.QuestType = EQuestType::Observation;
    NewQuest.Priority = 5;
    
    // Objetivo: Observar por tempo determinado
    FQuestObjectiveData ObservationObjective;
    ObservationObjective.ObjectiveText = FText::FromString("Observe dinossauros por 300 segundos sem ser detectado");
    ObservationObjective.ObjectiveType = EObjectiveType::Observe;
    ObservationObjective.TargetCount = 300; // segundos
    ObservationObjective.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Behavior.Natural"));
    
    NewQuest.Objectives.Add(ObservationObjective);
    
    // Recompensa
    NewQuest.Reward.Experience = 150;
    NewQuest.Reward.UnlockTags.AddTag(FGameplayTag::RequestGameplayTag("Knowledge.Behavior"));
    NewQuest.Reward.RewardDescription = FText::FromString("Conhecimento sobre comportamento dos dinossauros");
    
    QuestDatabase.Add(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("Generated dynamic observation quest: %s"), *QuestID.ToString());
    return QuestID;
}

FName UQuestSystem::GenerateDiscoveryQuest(const FGameplayTagContainer& ContextTags)
{
    FQuestData NewQuest;
    FName QuestID = FName(*FString::Printf(TEXT("DynamicDiscovery_%d"), ++DynamicQuestCounter));
    
    NewQuest.QuestName = FText::FromString("Exploração Territorial");
    NewQuest.QuestDescription = FText::FromString("Explore áreas desconhecidas e mapeie o território.");
    NewQuest.QuestType = EQuestType::Discovery;
    NewQuest.Priority = 4;
    
    // Objetivo: Descobrir locais
    FQuestObjectiveData DiscoveryObjective;
    DiscoveryObjective.ObjectiveText = FText::FromString("Descubra 3 novos pontos de interesse");
    DiscoveryObjective.ObjectiveType = EObjectiveType::Discover;
    DiscoveryObjective.TargetCount = 3;
    DiscoveryObjective.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Location.PointOfInterest"));
    
    NewQuest.Objectives.Add(DiscoveryObjective);
    
    // Recompensa
    NewQuest.Reward.Experience = 200;
    NewQuest.Reward.UnlockTags.AddTag(FGameplayTag::RequestGameplayTag("Map.Expanded"));
    NewQuest.Reward.RewardDescription = FText::FromString("Mapa expandido com novos locais");
    
    QuestDatabase.Add(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("Generated dynamic discovery quest: %s"), *QuestID.ToString());
    return QuestID;
}

FName UQuestSystem::GenerateDomesticationQuest(const FGameplayTagContainer& ContextTags)
{
    FQuestData NewQuest;
    FName QuestID = FName(*FString::Printf(TEXT("DynamicDomestication_%d"), ++DynamicQuestCounter));
    
    NewQuest.QuestName = FText::FromString("Primeira Domesticação");
    NewQuest.QuestDescription = FText::FromString("Tente estabelecer confiança com um dinossauro herbívoro pequeno.");
    NewQuest.QuestType = EQuestType::Domestication;
    NewQuest.Priority = 6;
    
    // Objetivo: Domesticar
    FQuestObjectiveData DomesticationObjective;
    DomesticationObjective.ObjectiveText = FText::FromString("Ganhe a confiança de um dinossauro herbívoro");
    DomesticationObjective.ObjectiveType = EObjectiveType::Domesticate;
    DomesticationObjective.TargetCount = 1;
    DomesticationObjective.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Species.Herbivore.Small"));
    
    NewQuest.Objectives.Add(DomesticationObjective);
    
    // Recompensa
    NewQuest.Reward.Experience = 300;
    NewQuest.Reward.UnlockTags.AddTag(FGameplayTag::RequestGameplayTag("Skill.Domestication"));
    NewQuest.Reward.RewardDescription = FText::FromString("Habilidade de domesticação desbloqueada");
    
    QuestDatabase.Add(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("Generated dynamic domestication quest: %s"), *QuestID.ToString());
    return QuestID;
}

FName UQuestSystem::GenerateEmergencyQuest(const FGameplayTagContainer& ContextTags)
{
    FQuestData NewQuest;
    FName QuestID = FName(*FString::Printf(TEXT("DynamicEmergency_%d"), ++DynamicQuestCounter));
    
    NewQuest.QuestName = FText::FromString("Situação de Emergência");
    NewQuest.QuestDescription = FText::FromString("Um predador perigoso foi avistado na área. Encontre abrigo imediatamente!");
    NewQuest.QuestType = EQuestType::Emergency;
    NewQuest.Priority = 10;
    NewQuest.TimeLimit = 300.0f; // 5 minutos
    
    // Objetivo: Encontrar abrigo
    FQuestObjectiveData ShelterObjective;
    ShelterObjective.ObjectiveText = FText::FromString("Encontre abrigo seguro");
    ShelterObjective.ObjectiveType = EObjectiveType::Reach;
    ShelterObjective.TargetCount = 1;
    ShelterObjective.RequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Location.Shelter"));
    
    NewQuest.Objectives.Add(ShelterObjective);
    
    // Recompensa
    NewQuest.Reward.Experience = 50;
    NewQuest.Reward.RewardDescription = FText::FromString("Sobreviveu ao perigo");
    
    QuestDatabase.Add(QuestID, NewQuest);
    
    UE_LOG(LogTemp, Log, TEXT("Generated dynamic emergency quest: %s"), *QuestID.ToString());
    return QuestID;
}

void UQuestSystem::CheckQuestPrerequisites()
{
    // TODO: Implementar verificação de pré-requisitos para auto-start de quests
}

void UQuestSystem::CheckQuestTimeouts()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    TArray<FName> QuestsToFail;
    
    for (const auto& QuestPair : QuestStatuses)
    {
        if (QuestPair.Value == EQuestStatus::Active)
        {
            FName QuestID = QuestPair.Key;
            const FQuestData& QuestData = QuestDatabase[QuestID];
            
            if (QuestData.TimeLimit > 0.0f && QuestStartTimes.Contains(QuestID))
            {
                float ElapsedTime = CurrentTime - QuestStartTimes[QuestID];
                if (ElapsedTime >= QuestData.TimeLimit)
                {
                    QuestsToFail.Add(QuestID);
                }
            }
        }
    }
    
    for (FName QuestID : QuestsToFail)
    {
        FailQuest(QuestID);
    }
}

void UQuestSystem::NotifyQuestStatusChange(FName QuestID, EQuestStatus NewStatus)
{
    OnQuestStatusChanged.Broadcast(QuestID, NewStatus);
}