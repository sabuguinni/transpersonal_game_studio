#include "Crowd_MassEntityCrowdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdManager, Log, All);

ACrowd_MassEntityCrowdManager::ACrowd_MassEntityCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS para performance
    
    // Configuração inicial
    MaxCrowdEntities = 50000;
    CurrentCrowdCount = 0;
    CrowdUpdateRadius = 5000.0f;
    LODDistanceNear = 1000.0f;
    LODDistanceMedium = 3000.0f;
    LODDistanceFar = 8000.0f;
    
    // Configuração dos biomas
    InitializeBiomeSettings();
}

void ACrowd_MassEntityCrowdManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogCrowdManager, Log, TEXT("Crowd Manager iniciado - Max Entities: %d"), MaxCrowdEntities);
    
    // Inicializar sistema de multidões
    InitializeCrowdSystem();
    
    // Spawnar multidões iniciais
    SpawnInitialCrowds();
}

void ACrowd_MassEntityCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Actualizar sistema de multidões
    UpdateCrowdSystem(DeltaTime);
    
    // Actualizar LOD baseado na distância do jogador
    UpdateCrowdLOD();
    
    // Actualizar comportamentos de grupo
    UpdateGroupBehaviors(DeltaTime);
}

void ACrowd_MassEntityCrowdManager::InitializeBiomeSettings()
{
    // Configurar biomas conforme coordenadas do brain memory
    BiomeSettings.Empty();
    
    // Pântano (sudoeste)
    FCrowd_BiomeSettings PantanoSettings;
    PantanoSettings.BiomeCenter = FVector(-50000, -45000, 0);
    PantanoSettings.BiomeRadius = 15000.0f;
    PantanoSettings.MaxEntitiesPerBiome = 8000;
    PantanoSettings.PreferredSpecies.Add(ECrowd_SpeciesType::Aquatic);
    PantanoSettings.PreferredSpecies.Add(ECrowd_SpeciesType::Amphibian);
    BiomeSettings.Add(ECrowd_BiomeType::Swamp, PantanoSettings);
    
    // Floresta (noroeste)
    FCrowd_BiomeSettings FlorestaSettings;
    FlorestaSettings.BiomeCenter = FVector(-45000, 40000, 0);
    FlorestaSettings.BiomeRadius = 20000.0f;
    FlorestaSettings.MaxEntitiesPerBiome = 12000;
    FlorestaSettings.PreferredSpecies.Add(ECrowd_SpeciesType::Carnivore);
    FlorestaSettings.PreferredSpecies.Add(ECrowd_SpeciesType::SmallHerbivore);
    BiomeSettings.Add(ECrowd_BiomeType::Forest, FlorestaSettings);
    
    // Savana (centro)
    FCrowd_BiomeSettings SavanaSettings;
    SavanaSettings.BiomeCenter = FVector(0, 0, 0);
    SavanaSettings.BiomeRadius = 15000.0f;
    SavanaSettings.MaxEntitiesPerBiome = 15000;
    SavanaSettings.PreferredSpecies.Add(ECrowd_SpeciesType::LargeHerbivore);
    SavanaSettings.PreferredSpecies.Add(ECrowd_SpeciesType::SmallHerbivore);
    BiomeSettings.Add(ECrowd_BiomeType::Savanna, SavanaSettings);
    
    // Deserto (leste)
    FCrowd_BiomeSettings DesertoSettings;
    DesertoSettings.BiomeCenter = FVector(55000, 0, 0);
    DesertoSettings.BiomeRadius = 18000.0f;
    DesertoSettings.MaxEntitiesPerBiome = 6000;
    DesertoSettings.PreferredSpecies.Add(ECrowd_SpeciesType::Scavenger);
    BiomeSettings.Add(ECrowd_BiomeType::Desert, DesertoSettings);
    
    // Montanha (nordeste)
    FCrowd_BiomeSettings MontanhaSettings;
    MontanhaSettings.BiomeCenter = FVector(40000, 50000, 500);
    MontanhaSettings.BiomeRadius = 12000.0f;
    MontanhaSettings.MaxEntitiesPerBiome = 4000;
    MontanhaSettings.PreferredSpecies.Add(ECrowd_SpeciesType::Flying);
    BiomeSettings.Add(ECrowd_BiomeType::Mountain, MontanhaSettings);
}

void ACrowd_MassEntityCrowdManager::InitializeCrowdSystem()
{
    // Limpar arrays existentes
    CrowdGroups.Empty();
    ActiveEntities.Empty();
    
    UE_LOG(LogCrowdManager, Log, TEXT("Sistema de multidões inicializado"));
}

void ACrowd_MassEntityCrowdManager::SpawnInitialCrowds()
{
    // Spawnar multidões em cada bioma
    for (const auto& BiomePair : BiomeSettings)
    {
        ECrowd_BiomeType BiomeType = BiomePair.Key;
        const FCrowd_BiomeSettings& Settings = BiomePair.Value;
        
        SpawnCrowdInBiome(BiomeType, Settings);
    }
    
    UE_LOG(LogCrowdManager, Log, TEXT("Multidões iniciais spawnadas - Total: %d"), CurrentCrowdCount);
}

void ACrowd_MassEntityCrowdManager::SpawnCrowdInBiome(ECrowd_BiomeType BiomeType, const FCrowd_BiomeSettings& Settings)
{
    int32 NumGroupsToSpawn = FMath::RandRange(5, 15); // 5-15 grupos por bioma
    
    for (int32 i = 0; i < NumGroupsToSpawn; i++)
    {
        // Escolher espécie aleatória das preferidas do bioma
        if (Settings.PreferredSpecies.Num() > 0)
        {
            ECrowd_SpeciesType Species = Settings.PreferredSpecies[FMath::RandRange(0, Settings.PreferredSpecies.Num() - 1)];
            
            // Gerar localização aleatória no bioma
            FVector SpawnLocation = GenerateRandomLocationInBiome(Settings);
            
            // Criar grupo de multidão
            FCrowd_GroupData NewGroup;
            NewGroup.GroupID = FMath::Rand();
            NewGroup.BiomeType = BiomeType;
            NewGroup.SpeciesType = Species;
            NewGroup.GroupCenter = SpawnLocation;
            NewGroup.GroupSize = FMath::RandRange(10, 50); // 10-50 entidades por grupo
            NewGroup.CurrentBehavior = ECrowd_GroupBehavior::Grazing;
            
            CrowdGroups.Add(NewGroup);
            CurrentCrowdCount += NewGroup.GroupSize;
            
            UE_LOG(LogCrowdManager, Log, TEXT("Grupo %d criado no bioma %d - Espécie: %d, Tamanho: %d"), 
                NewGroup.GroupID, (int32)BiomeType, (int32)Species, NewGroup.GroupSize);
        }
    }
}

FVector ACrowd_MassEntityCrowdManager::GenerateRandomLocationInBiome(const FCrowd_BiomeSettings& Settings)
{
    // Gerar localização aleatória dentro do raio do bioma
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Settings.BiomeRadius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomDistance;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    RandomOffset.Z = FMath::RandRange(-100.0f, 100.0f); // Pequena variação em altura
    
    return Settings.BiomeCenter + RandomOffset;
}

void ACrowd_MassEntityCrowdManager::UpdateCrowdSystem(float DeltaTime)
{
    // Actualizar posições e comportamentos dos grupos
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        UpdateGroupMovement(Group, DeltaTime);
        UpdateGroupBehavior(Group, DeltaTime);
    }
    
    // Verificar se precisamos de spawnar mais multidões
    if (CurrentCrowdCount < MaxCrowdEntities * 0.8f) // 80% da capacidade
    {
        ConsiderSpawningNewGroups();
    }
}

void ACrowd_MassEntityCrowdManager::UpdateGroupMovement(FCrowd_GroupData& Group, float DeltaTime)
{
    // Movimento baseado no comportamento actual
    FVector MovementDirection = FVector::ZeroVector;
    float MovementSpeed = 100.0f; // Velocidade base
    
    switch (Group.CurrentBehavior)
    {
        case ECrowd_GroupBehavior::Grazing:
            // Movimento lento e aleatório
            MovementSpeed = 50.0f;
            MovementDirection = FMath::VRand() * 0.1f;
            break;
            
        case ECrowd_GroupBehavior::Migrating:
            // Movimento direccional
            MovementSpeed = 200.0f;
            MovementDirection = (Group.TargetLocation - Group.GroupCenter).GetSafeNormal();
            break;
            
        case ECrowd_GroupBehavior::Fleeing:
            // Movimento rápido para longe da ameaça
            MovementSpeed = 400.0f;
            if (Group.ThreatLocation != FVector::ZeroVector)
            {
                MovementDirection = (Group.GroupCenter - Group.ThreatLocation).GetSafeNormal();
            }
            break;
            
        case ECrowd_GroupBehavior::Hunting:
            // Movimento em direcção à presa
            MovementSpeed = 300.0f;
            if (Group.TargetLocation != FVector::ZeroVector)
            {
                MovementDirection = (Group.TargetLocation - Group.GroupCenter).GetSafeNormal();
            }
            break;
    }
    
    // Aplicar movimento
    Group.GroupCenter += MovementDirection * MovementSpeed * DeltaTime;
    
    // Manter dentro dos limites do bioma
    ClampGroupToBiome(Group);
}

void ACrowd_MassEntityCrowdManager::UpdateGroupBehavior(FCrowd_GroupData& Group, float DeltaTime)
{
    // Actualizar timer de comportamento
    Group.BehaviorTimer += DeltaTime;
    
    // Mudar comportamento baseado em condições
    if (Group.BehaviorTimer > 10.0f) // Mudar comportamento a cada 10 segundos
    {
        Group.BehaviorTimer = 0.0f;
        
        // Escolher novo comportamento baseado na espécie
        switch (Group.SpeciesType)
        {
            case ECrowd_SpeciesType::LargeHerbivore:
            case ECrowd_SpeciesType::SmallHerbivore:
                Group.CurrentBehavior = FMath::RandBool() ? ECrowd_GroupBehavior::Grazing : ECrowd_GroupBehavior::Migrating;
                break;
                
            case ECrowd_SpeciesType::Carnivore:
                Group.CurrentBehavior = FMath::RandBool() ? ECrowd_GroupBehavior::Hunting : ECrowd_GroupBehavior::Grazing;
                break;
                
            case ECrowd_SpeciesType::Scavenger:
                Group.CurrentBehavior = ECrowd_GroupBehavior::Grazing; // Procurar comida
                break;
                
            default:
                Group.CurrentBehavior = ECrowd_GroupBehavior::Grazing;
                break;
        }
    }
}

void ACrowd_MassEntityCrowdManager::ClampGroupToBiome(FCrowd_GroupData& Group)
{
    // Obter configurações do bioma
    const FCrowd_BiomeSettings* Settings = BiomeSettings.Find(Group.BiomeType);
    if (!Settings)
        return;
    
    // Verificar distância do centro do bioma
    float DistanceFromCenter = FVector::Dist(Group.GroupCenter, Settings->BiomeCenter);
    
    if (DistanceFromCenter > Settings->BiomeRadius)
    {
        // Mover de volta para dentro do bioma
        FVector DirectionToCenter = (Settings->BiomeCenter - Group.GroupCenter).GetSafeNormal();
        Group.GroupCenter = Settings->BiomeCenter + DirectionToCenter * (Settings->BiomeRadius * 0.9f);
    }
}

void ACrowd_MassEntityCrowdManager::UpdateCrowdLOD()
{
    // Obter localização do jogador
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Actualizar LOD de cada grupo baseado na distância
    for (FCrowd_GroupData& Group : CrowdGroups)
    {
        float DistanceToPlayer = FVector::Dist(Group.GroupCenter, PlayerLocation);
        
        if (DistanceToPlayer < LODDistanceNear)
        {
            Group.CurrentLOD = ECrowd_LODLevel::High;
        }
        else if (DistanceToPlayer < LODDistanceMedium)
        {
            Group.CurrentLOD = ECrowd_LODLevel::Medium;
        }
        else if (DistanceToPlayer < LODDistanceFar)
        {
            Group.CurrentLOD = ECrowd_LODLevel::Low;
        }
        else
        {
            Group.CurrentLOD = ECrowd_LODLevel::Culled;
        }
    }
}

void ACrowd_MassEntityCrowdManager::UpdateGroupBehaviors(float DeltaTime)
{
    // Detectar interacções entre grupos
    for (int32 i = 0; i < CrowdGroups.Num(); i++)
    {
        for (int32 j = i + 1; j < CrowdGroups.Num(); j++)
        {
            FCrowd_GroupData& GroupA = CrowdGroups[i];
            FCrowd_GroupData& GroupB = CrowdGroups[j];
            
            float Distance = FVector::Dist(GroupA.GroupCenter, GroupB.GroupCenter);
            
            // Interacção entre grupos próximos
            if (Distance < 2000.0f) // 2km de distância de interacção
            {
                ProcessGroupInteraction(GroupA, GroupB);
            }
        }
    }
}

void ACrowd_MassEntityCrowdManager::ProcessGroupInteraction(FCrowd_GroupData& GroupA, FCrowd_GroupData& GroupB)
{
    // Lógica de interacção baseada nas espécies
    bool IsAPredator = (GroupA.SpeciesType == ECrowd_SpeciesType::Carnivore);
    bool IsBPredator = (GroupB.SpeciesType == ECrowd_SpeciesType::Carnivore);
    bool IsAHerbivore = (GroupA.SpeciesType == ECrowd_SpeciesType::LargeHerbivore || GroupA.SpeciesType == ECrowd_SpeciesType::SmallHerbivore);
    bool IsBHerbivore = (GroupB.SpeciesType == ECrowd_SpeciesType::LargeHerbivore || GroupB.SpeciesType == ECrowd_SpeciesType::SmallHerbivore);
    
    // Predador vs Herbívoro
    if (IsAPredator && IsBHerbivore)
    {
        GroupA.CurrentBehavior = ECrowd_GroupBehavior::Hunting;
        GroupA.TargetLocation = GroupB.GroupCenter;
        
        GroupB.CurrentBehavior = ECrowd_GroupBehavior::Fleeing;
        GroupB.ThreatLocation = GroupA.GroupCenter;
    }
    else if (IsBPredator && IsAHerbivore)
    {
        GroupB.CurrentBehavior = ECrowd_GroupBehavior::Hunting;
        GroupB.TargetLocation = GroupA.GroupCenter;
        
        GroupA.CurrentBehavior = ECrowd_GroupBehavior::Fleeing;
        GroupA.ThreatLocation = GroupB.GroupCenter;
    }
    // Herbívoros juntos - comportamento de rebanho
    else if (IsAHerbivore && IsBHerbivore)
    {
        // Mover em direcção um ao outro (comportamento de rebanho)
        FVector MidPoint = (GroupA.GroupCenter + GroupB.GroupCenter) * 0.5f;
        GroupA.TargetLocation = MidPoint;
        GroupB.TargetLocation = MidPoint;
        GroupA.CurrentBehavior = ECrowd_GroupBehavior::Migrating;
        GroupB.CurrentBehavior = ECrowd_GroupBehavior::Migrating;
    }
}

void ACrowd_MassEntityCrowdManager::ConsiderSpawningNewGroups()
{
    // Spawnar novos grupos se a população estiver baixa
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% de chance por frame
    {
        // Escolher bioma aleatório
        TArray<ECrowd_BiomeType> BiomeKeys;
        BiomeSettings.GetKeys(BiomeKeys);
        
        if (BiomeKeys.Num() > 0)
        {
            ECrowd_BiomeType RandomBiome = BiomeKeys[FMath::RandRange(0, BiomeKeys.Num() - 1)];
            const FCrowd_BiomeSettings& Settings = BiomeSettings[RandomBiome];
            
            SpawnCrowdInBiome(RandomBiome, Settings);
        }
    }
}

int32 ACrowd_MassEntityCrowdManager::GetTotalCrowdCount() const
{
    return CurrentCrowdCount;
}

int32 ACrowd_MassEntityCrowdManager::GetActiveGroupCount() const
{
    return CrowdGroups.Num();
}

TArray<FCrowd_GroupData> ACrowd_MassEntityCrowdManager::GetCrowdGroups() const
{
    return CrowdGroups;
}