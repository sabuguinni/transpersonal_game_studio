#include "Quest_SurvivalMissionManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UQuest_SurvivalMissionManager::UQuest_SurvivalMissionManager()
{
    MaxActiveMissions = 5;
    MissionGenerationInterval = 120.0f; // 2 minutes
}

void UQuest_SurvivalMissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultMissions();
    
    // Start mission generation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MissionGenerationTimer,
            this,
            &UQuest_SurvivalMissionManager::OnMissionGenerationTimer,
            MissionGenerationInterval,
            true
        );
    }
}

void UQuest_SurvivalMissionManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionGenerationTimer);
    }
    
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    Super::Deinitialize();
}

void UQuest_SurvivalMissionManager::StartMission(const FString& MissionName)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && !Mission.bIsActive)
        {
            Mission.bIsActive = true;
            UE_LOG(LogTemp, Warning, TEXT("Started mission: %s"), *MissionName);
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::CompleteMission(const FString& MissionName)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (ActiveMissions[i].MissionName == MissionName)
        {
            ActiveMissions[i].bIsCompleted = true;
            CompletedMissions.Add(ActiveMissions[i]);
            ActiveMissions.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Completed mission: %s"), *MissionName);
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::UpdateObjective(const FString& MissionName, int32 ObjectiveIndex, bool bCompleted)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && Mission.Objectives.IsValidIndex(ObjectiveIndex))
        {
            Mission.Objectives[ObjectiveIndex].bIsCompleted = bCompleted;
            
            // Check if all objectives are completed
            bool bAllCompleted = true;
            for (const FQuest_MissionObjective& Objective : Mission.Objectives)
            {
                if (!Objective.bIsCompleted)
                {
                    bAllCompleted = false;
                    break;
                }
            }
            
            if (bAllCompleted)
            {
                CompleteMission(MissionName);
            }
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::CreateGatheringMission(EResourceType ResourceType, int32 Amount, FVector Location)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission;
    NewMission.MissionName = FString::Printf(TEXT("Gather %d %s"), Amount, *UEnum::GetValueAsString(ResourceType));
    NewMission.MissionDescription = FString::Printf(TEXT("Collect %d units of %s for survival"), Amount, *UEnum::GetValueAsString(ResourceType));
    NewMission.TargetBiome = EBiomeType::Temperate;
    NewMission.MissionPriority = 2;
    NewMission.TimeLimit = 300.0f;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = FString::Printf(TEXT("Collect %d %s"), Amount, *UEnum::GetValueAsString(ResourceType));
    Objective.RequiredResource = ResourceType;
    Objective.RequiredAmount = Amount;
    Objective.TargetLocation = Location;
    Objective.CompletionRadius = 1000.0f;
    
    NewMission.Objectives.Add(Objective);
    ActiveMissions.Add(NewMission);
}

void UQuest_SurvivalMissionManager::CreateHuntingMission(EDinosaurSpecies TargetSpecies, FVector HuntingGrounds)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission;
    NewMission.MissionName = FString::Printf(TEXT("Hunt %s"), *UEnum::GetValueAsString(TargetSpecies));
    NewMission.MissionDescription = FString::Printf(TEXT("Track and hunt a %s for meat and materials"), *UEnum::GetValueAsString(TargetSpecies));
    NewMission.TargetBiome = EBiomeType::Savanna;
    NewMission.MissionPriority = 3;
    NewMission.TimeLimit = 600.0f;
    
    FQuest_MissionObjective TrackObjective;
    TrackObjective.ObjectiveText = FString::Printf(TEXT("Track %s"), *UEnum::GetValueAsString(TargetSpecies));
    TrackObjective.TargetLocation = HuntingGrounds;
    TrackObjective.CompletionRadius = 500.0f;
    
    FQuest_MissionObjective HuntObjective;
    HuntObjective.ObjectiveText = FString::Printf(TEXT("Hunt %s"), *UEnum::GetValueAsString(TargetSpecies));
    HuntObjective.RequiredResource = EResourceType::Meat;
    HuntObjective.RequiredAmount = 1;
    
    NewMission.Objectives.Add(TrackObjective);
    NewMission.Objectives.Add(HuntObjective);
    ActiveMissions.Add(NewMission);
}

void UQuest_SurvivalMissionManager::CreateExplorationMission(EBiomeType BiomeToExplore, FVector ExploreCenter)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission;
    NewMission.MissionName = FString::Printf(TEXT("Explore %s"), *UEnum::GetValueAsString(BiomeToExplore));
    NewMission.MissionDescription = FString::Printf(TEXT("Venture into the %s biome and map the area"), *UEnum::GetValueAsString(BiomeToExplore));
    NewMission.TargetBiome = BiomeToExplore;
    NewMission.MissionPriority = 1;
    NewMission.TimeLimit = 900.0f;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = FString::Printf(TEXT("Explore %s biome"), *UEnum::GetValueAsString(BiomeToExplore));
    Objective.TargetLocation = ExploreCenter;
    Objective.CompletionRadius = 2000.0f;
    
    NewMission.Objectives.Add(Objective);
    ActiveMissions.Add(NewMission);
}

void UQuest_SurvivalMissionManager::CreateCraftingMission(EResourceType CraftedItem, int32 Quantity)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission;
    NewMission.MissionName = FString::Printf(TEXT("Craft %d %s"), Quantity, *UEnum::GetValueAsString(CraftedItem));
    NewMission.MissionDescription = FString::Printf(TEXT("Create %d units of %s using available materials"), Quantity, *UEnum::GetValueAsString(CraftedItem));
    NewMission.TargetBiome = EBiomeType::Temperate;
    NewMission.MissionPriority = 2;
    NewMission.TimeLimit = 400.0f;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = FString::Printf(TEXT("Craft %d %s"), Quantity, *UEnum::GetValueAsString(CraftedItem));
    Objective.RequiredResource = CraftedItem;
    Objective.RequiredAmount = Quantity;
    
    NewMission.Objectives.Add(Objective);
    ActiveMissions.Add(NewMission);
}

void UQuest_SurvivalMissionManager::CreateShelterMission(FVector BuildLocation, float ShelterSize)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission;
    NewMission.MissionName = TEXT("Build Shelter");
    NewMission.MissionDescription = TEXT("Construct a shelter to protect against weather and predators");
    NewMission.TargetBiome = EBiomeType::Temperate;
    NewMission.MissionPriority = 4;
    NewMission.TimeLimit = 800.0f;
    
    FQuest_MissionObjective GatherMaterials;
    GatherMaterials.ObjectiveText = TEXT("Gather building materials");
    GatherMaterials.RequiredResource = EResourceType::Wood;
    GatherMaterials.RequiredAmount = 10;
    
    FQuest_MissionObjective BuildShelter;
    BuildShelter.ObjectiveText = TEXT("Construct shelter");
    BuildShelter.TargetLocation = BuildLocation;
    BuildShelter.CompletionRadius = ShelterSize;
    
    NewMission.Objectives.Add(GatherMaterials);
    NewMission.Objectives.Add(BuildShelter);
    ActiveMissions.Add(NewMission);
}

TArray<FQuest_SurvivalMission> UQuest_SurvivalMissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

FQuest_SurvivalMission UQuest_SurvivalMissionManager::GetMissionByName(const FString& MissionName) const
{
    for (const FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName)
        {
            return Mission;
        }
    }
    return FQuest_SurvivalMission();
}

bool UQuest_SurvivalMissionManager::IsMissionCompleted(const FString& MissionName) const
{
    for (const FQuest_SurvivalMission& Mission : CompletedMissions)
    {
        if (Mission.MissionName == MissionName)
        {
            return true;
        }
    }
    return false;
}

void UQuest_SurvivalMissionManager::CheckLocationObjectives(FVector PlayerLocation)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (!Mission.bIsActive) continue;
        
        for (FQuest_MissionObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted && Objective.TargetLocation != FVector::ZeroVector)
            {
                float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                if (Distance <= Objective.CompletionRadius)
                {
                    Objective.bIsCompleted = true;
                    UE_LOG(LogTemp, Warning, TEXT("Location objective completed: %s"), *Objective.ObjectiveText);
                }
            }
        }
    }
}

void UQuest_SurvivalMissionManager::CheckResourceObjectives(EResourceType ResourceType, int32 Amount)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (!Mission.bIsActive) continue;
        
        for (FQuest_MissionObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted && 
                Objective.RequiredResource == ResourceType && 
                Amount >= Objective.RequiredAmount)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Resource objective completed: %s"), *Objective.ObjectiveText);
            }
        }
    }
}

void UQuest_SurvivalMissionManager::GenerateRandomSurvivalMission()
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    int32 MissionType = FMath::RandRange(0, 2);
    
    switch (MissionType)
    {
        case 0:
            ActiveMissions.Add(CreateRandomGatheringMission());
            break;
        case 1:
            ActiveMissions.Add(CreateRandomHuntingMission());
            break;
        case 2:
            ActiveMissions.Add(CreateRandomExplorationMission());
            break;
    }
}

void UQuest_SurvivalMissionManager::CleanupCompletedMissions()
{
    // Keep only last 10 completed missions
    if (CompletedMissions.Num() > 10)
    {
        int32 ExcessCount = CompletedMissions.Num() - 10;
        CompletedMissions.RemoveAt(0, ExcessCount);
    }
}

void UQuest_SurvivalMissionManager::InitializeDefaultMissions()
{
    // Create initial survival missions
    CreateGatheringMission(EResourceType::Stone, 5, FVector(1000, 0, 0));
    CreateGatheringMission(EResourceType::Wood, 3, FVector(-1000, 0, 0));
    CreateExplorationMission(EBiomeType::Forest, FVector(0, 2000, 0));
}

void UQuest_SurvivalMissionManager::OnMissionGenerationTimer()
{
    GenerateRandomSurvivalMission();
    CleanupCompletedMissions();
}

FQuest_SurvivalMission UQuest_SurvivalMissionManager::CreateRandomGatheringMission()
{
    FQuest_SurvivalMission Mission;
    
    TArray<EResourceType> Resources = {EResourceType::Stone, EResourceType::Wood, EResourceType::Fiber};
    EResourceType RandomResource = Resources[FMath::RandRange(0, Resources.Num() - 1)];
    int32 RandomAmount = FMath::RandRange(3, 8);
    
    Mission.MissionName = FString::Printf(TEXT("Gather %d %s"), RandomAmount, *UEnum::GetValueAsString(RandomResource));
    Mission.MissionDescription = FString::Printf(TEXT("Collect %d units of %s for survival needs"), RandomAmount, *UEnum::GetValueAsString(RandomResource));
    Mission.TargetBiome = EBiomeType::Temperate;
    Mission.MissionPriority = FMath::RandRange(1, 3);
    Mission.TimeLimit = FMath::RandRange(200, 400);
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = Mission.MissionName;
    Objective.RequiredResource = RandomResource;
    Objective.RequiredAmount = RandomAmount;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_SurvivalMission UQuest_SurvivalMissionManager::CreateRandomHuntingMission()
{
    FQuest_SurvivalMission Mission;
    
    TArray<EDinosaurSpecies> Species = {EDinosaurSpecies::Compsognathus, EDinosaurSpecies::Parasaurolophus};
    EDinosaurSpecies RandomSpecies = Species[FMath::RandRange(0, Species.Num() - 1)];
    
    Mission.MissionName = FString::Printf(TEXT("Hunt %s"), *UEnum::GetValueAsString(RandomSpecies));
    Mission.MissionDescription = FString::Printf(TEXT("Track and hunt a %s for meat and resources"), *UEnum::GetValueAsString(RandomSpecies));
    Mission.TargetBiome = EBiomeType::Savanna;
    Mission.MissionPriority = FMath::RandRange(2, 4);
    Mission.TimeLimit = FMath::RandRange(400, 700);
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = Mission.MissionName;
    Objective.RequiredResource = EResourceType::Meat;
    Objective.RequiredAmount = 1;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_SurvivalMission UQuest_SurvivalMissionManager::CreateRandomExplorationMission()
{
    FQuest_SurvivalMission Mission;
    
    TArray<EBiomeType> Biomes = {EBiomeType::Forest, EBiomeType::Swamp, EBiomeType::Desert};
    EBiomeType RandomBiome = Biomes[FMath::RandRange(0, Biomes.Num() - 1)];
    
    Mission.MissionName = FString::Printf(TEXT("Explore %s"), *UEnum::GetValueAsString(RandomBiome));
    Mission.MissionDescription = FString::Printf(TEXT("Venture into the dangerous %s biome"), *UEnum::GetValueAsString(RandomBiome));
    Mission.TargetBiome = RandomBiome;
    Mission.MissionPriority = FMath::RandRange(1, 2);
    Mission.TimeLimit = FMath::RandRange(600, 900);
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = Mission.MissionName;
    Objective.TargetLocation = FVector(FMath::RandRange(-5000, 5000), FMath::RandRange(-5000, 5000), 0);
    Objective.CompletionRadius = 1500.0f;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}