#include "Quest_MissionObjectiveSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "../Character/TranspersonalCharacter.h"

UQuest_MissionObjectiveComponent::UQuest_MissionObjectiveComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    ProximityCheckDistance = 1000.0f; // 10 meters
    BiomeCheckRadius = 5000.0f; // 50 meters for biome detection
}

void UQuest_MissionObjectiveComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Mission Objective Component initialized"));
}

void UQuest_MissionObjectiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMissionTimers(DeltaTime);
    CheckMissionFailures();
    
    // Check location-based objectives
    if (AActor* Owner = GetOwner())
    {
        CheckLocationObjectives(Owner->GetActorLocation());
        CheckCrowdProximity(Owner->GetActorLocation());
    }
}

void UQuest_MissionObjectiveComponent::StartMission(const FQuest_MissionData& MissionData)
{
    FQuest_MissionData NewMission = MissionData;
    NewMission.Status = EQuest_MissionStatus::Active;
    NewMission.MissionElapsedTime = 0.0f;
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Mission Started: %s"), *NewMission.MissionName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("New Mission: %s"), *NewMission.MissionName));
    }
}

void UQuest_MissionObjectiveComponent::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].Status = EQuest_MissionStatus::Completed;
            CompletedMissions.Add(ActiveMissions[i]);
            
            NotifyMissionComplete(ActiveMissions[i]);
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

void UQuest_MissionObjectiveComponent::FailMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].Status = EQuest_MissionStatus::Failed;
            
            UE_LOG(LogTemp, Warning, TEXT("Mission Failed: %s"), *ActiveMissions[i].MissionName);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                    FString::Printf(TEXT("Mission Failed: %s"), *ActiveMissions[i].MissionName));
            }
            
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

void UQuest_MissionObjectiveComponent::AbandonMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].Status = EQuest_MissionStatus::Abandoned;
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

void UQuest_MissionObjectiveComponent::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 ProgressAmount)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            for (FQuest_ObjectiveData& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID && !Objective.bIsCompleted)
                {
                    Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + ProgressAmount, Objective.TargetCount);
                    
                    if (Objective.CurrentProgress >= Objective.TargetCount)
                    {
                        CompleteObjective(MissionID, ObjectiveID);
                    }
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                            FString::Printf(TEXT("%s: %d/%d"), *Objective.ObjectiveName, Objective.CurrentProgress, Objective.TargetCount));
                    }
                    break;
                }
            }
            break;
        }
    }
}

void UQuest_MissionObjectiveComponent::CompleteObjective(const FString& MissionID, const FString& ObjectiveID)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            for (FQuest_ObjectiveData& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID)
                {
                    Objective.bIsCompleted = true;
                    NotifyObjectiveComplete(Objective);
                    
                    // Check if all objectives are complete
                    bool bAllComplete = true;
                    for (const FQuest_ObjectiveData& CheckObj : Mission.Objectives)
                    {
                        if (!CheckObj.bIsCompleted)
                        {
                            bAllComplete = false;
                            break;
                        }
                    }
                    
                    if (bAllComplete)
                    {
                        CompleteMission(MissionID);
                    }
                    break;
                }
            }
            break;
        }
    }
}

bool UQuest_MissionObjectiveComponent::CheckObjectiveCompletion(const FString& MissionID, const FString& ObjectiveID)
{
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            for (const FQuest_ObjectiveData& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID)
                {
                    return Objective.bIsCompleted;
                }
            }
        }
    }
    return false;
}

TArray<FQuest_MissionData> UQuest_MissionObjectiveComponent::GetActiveMissions() const
{
    return ActiveMissions;
}

FQuest_MissionData UQuest_MissionObjectiveComponent::GetMissionByID(const FString& MissionID) const
{
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    return FQuest_MissionData();
}

bool UQuest_MissionObjectiveComponent::HasActiveMission() const
{
    return ActiveMissions.Num() > 0;
}

void UQuest_MissionObjectiveComponent::RegisterCrowdObjective(const FString& MissionID, const FString& ObjectiveID, ACrowd_MassEntitySpawner* CrowdSpawner)
{
    if (CrowdSpawner)
    {
        FString Key = MissionID + TEXT("_") + ObjectiveID;
        CrowdObjectiveMap.Add(Key, CrowdSpawner);
    }
}

void UQuest_MissionObjectiveComponent::CheckCrowdProximity(const FVector& PlayerLocation)
{
    for (const auto& CrowdPair : CrowdObjectiveMap)
    {
        if (CrowdPair.Value)
        {
            float Distance = FVector::Dist(PlayerLocation, CrowdPair.Value->GetActorLocation());
            if (Distance <= ProximityCheckDistance)
            {
                // Parse mission and objective IDs from key
                FString Key = CrowdPair.Key;
                FString MissionID, ObjectiveID;
                if (Key.Split(TEXT("_"), &MissionID, &ObjectiveID))
                {
                    UpdateObjectiveProgress(MissionID, ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_MissionObjectiveComponent::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        for (FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                switch (Objective.ObjectiveType)
                {
                    case EQuest_ObjectiveType::Reach_Location:
                    {
                        float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                        if (Distance <= ProximityCheckDistance)
                        {
                            CompleteObjective(Mission.MissionID, Objective.ObjectiveID);
                        }
                        break;
                    }
                    case EQuest_ObjectiveType::Explore_Area:
                    {
                        EQuest_BiomeType CurrentBiome = GetBiomeFromLocation(PlayerLocation);
                        if (CurrentBiome == Objective.TargetBiome)
                        {
                            UpdateObjectiveProgress(Mission.MissionID, Objective.ObjectiveID, 1);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

EQuest_BiomeType UQuest_MissionObjectiveComponent::GetBiomeFromLocation(const FVector& Location)
{
    // Biome detection based on coordinates from memory
    if (FVector::Dist(Location, FVector(0, 0, 0)) <= BiomeCheckRadius)
        return EQuest_BiomeType::Savana;
    else if (FVector::Dist(Location, FVector(-50000, -45000, 0)) <= BiomeCheckRadius)
        return EQuest_BiomeType::Pantano;
    else if (FVector::Dist(Location, FVector(-45000, 40000, 0)) <= BiomeCheckRadius)
        return EQuest_BiomeType::Floresta;
    else if (FVector::Dist(Location, FVector(55000, 0, 0)) <= BiomeCheckRadius)
        return EQuest_BiomeType::Deserto;
    else if (FVector::Dist(Location, FVector(40000, 50000, 0)) <= BiomeCheckRadius)
        return EQuest_BiomeType::Montanha;
    
    return EQuest_BiomeType::Savana; // Default
}

void UQuest_MissionObjectiveComponent::CreateTestMissions()
{
    // Create test hunt mission
    FQuest_MissionData HuntMission = CreateHuntMission();
    StartMission(HuntMission);
    
    // Create test gather mission
    FQuest_MissionData GatherMission = CreateGatherMission();
    StartMission(GatherMission);
    
    // Create test exploration mission
    FQuest_MissionData ExploreMission = CreateExploreMission();
    StartMission(ExploreMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Test missions created successfully"));
}

void UQuest_MissionObjectiveComponent::DebugPrintActiveMissions()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ACTIVE MISSIONS ==="));
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission: %s [%s]"), *Mission.MissionName, 
            *UEnum::GetValueAsString(Mission.Status));
        
        for (const FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s: %d/%d [%s]"), 
                *Objective.ObjectiveName, 
                Objective.CurrentProgress, 
                Objective.TargetCount,
                Objective.bIsCompleted ? TEXT("COMPLETE") : TEXT("ACTIVE"));
        }
    }
}

void UQuest_MissionObjectiveComponent::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        Mission.MissionElapsedTime += DeltaTime;
        
        for (FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (Objective.TimeLimit > 0.0f)
            {
                Objective.ElapsedTime += DeltaTime;
            }
        }
    }
}

void UQuest_MissionObjectiveComponent::CheckMissionFailures()
{
    TArray<FString> MissionsToFail;
    
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        // Check mission time limit
        if (Mission.MissionTimeLimit > 0.0f && Mission.MissionElapsedTime >= Mission.MissionTimeLimit)
        {
            MissionsToFail.Add(Mission.MissionID);
            continue;
        }
        
        // Check objective time limits
        for (const FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (Objective.TimeLimit > 0.0f && Objective.ElapsedTime >= Objective.TimeLimit && !Objective.bIsCompleted)
            {
                MissionsToFail.Add(Mission.MissionID);
                break;
            }
        }
    }
    
    for (const FString& MissionID : MissionsToFail)
    {
        FailMission(MissionID);
    }
}

void UQuest_MissionObjectiveComponent::NotifyObjectiveComplete(const FQuest_ObjectiveData& Objective)
{
    UE_LOG(LogTemp, Warning, TEXT("Objective Complete: %s"), *Objective.ObjectiveName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Objective Complete: %s"), *Objective.ObjectiveName));
    }
}

void UQuest_MissionObjectiveComponent::NotifyMissionComplete(const FQuest_MissionData& Mission)
{
    UE_LOG(LogTemp, Warning, TEXT("Mission Complete: %s"), *Mission.MissionName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, 
            FString::Printf(TEXT("MISSION COMPLETE: %s"), *Mission.MissionName));
    }
}

FQuest_MissionData UQuest_MissionObjectiveComponent::CreateHuntMission()
{
    FQuest_MissionData Mission;
    Mission.MissionID = TEXT("HUNT_001");
    Mission.MissionName = TEXT("Hunt the Predator");
    Mission.MissionDescription = TEXT("Track and hunt a dangerous predator in the Savana biome");
    Mission.Priority = 2;
    Mission.MissionTimeLimit = 1800.0f; // 30 minutes
    Mission.bIsMainStoryMission = false;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("HUNT_TARGET");
    Objective.ObjectiveName = TEXT("Hunt Velociraptor");
    Objective.Description = TEXT("Find and defeat a Velociraptor");
    Objective.ObjectiveType = EQuest_ObjectiveType::Hunt_Dinosaur;
    Objective.TargetCount = 1;
    Objective.TargetBiome = EQuest_BiomeType::Savana;
    Objective.TimeLimit = 1200.0f; // 20 minutes
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_MissionData UQuest_MissionObjectiveComponent::CreateGatherMission()
{
    FQuest_MissionData Mission;
    Mission.MissionID = TEXT("GATHER_001");
    Mission.MissionName = TEXT("Resource Collection");
    Mission.MissionDescription = TEXT("Gather essential resources for survival");
    Mission.Priority = 1;
    Mission.MissionTimeLimit = 900.0f; // 15 minutes
    Mission.bIsMainStoryMission = false;
    
    FQuest_ObjectiveData Objective1;
    Objective1.ObjectiveID = TEXT("GATHER_STONE");
    Objective1.ObjectiveName = TEXT("Collect Stones");
    Objective1.Description = TEXT("Gather stones for tool crafting");
    Objective1.ObjectiveType = EQuest_ObjectiveType::Gather_Resources;
    Objective1.TargetCount = 5;
    
    FQuest_ObjectiveData Objective2;
    Objective2.ObjectiveID = TEXT("GATHER_WOOD");
    Objective2.ObjectiveName = TEXT("Collect Wood");
    Objective2.Description = TEXT("Gather wood for fire and tools");
    Objective2.ObjectiveType = EQuest_ObjectiveType::Gather_Resources;
    Objective2.TargetCount = 3;
    
    Mission.Objectives.Add(Objective1);
    Mission.Objectives.Add(Objective2);
    return Mission;
}

FQuest_MissionData UQuest_MissionObjectiveComponent::CreateExploreMission()
{
    FQuest_MissionData Mission;
    Mission.MissionID = TEXT("EXPLORE_001");
    Mission.MissionName = TEXT("Territory Exploration");
    Mission.MissionDescription = TEXT("Explore the Forest biome to find new resources");
    Mission.Priority = 1;
    Mission.MissionTimeLimit = 1200.0f; // 20 minutes
    Mission.bIsMainStoryMission = false;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("EXPLORE_FOREST");
    Objective.ObjectiveName = TEXT("Explore Forest Biome");
    Objective.Description = TEXT("Spend time exploring the Forest biome");
    Objective.ObjectiveType = EQuest_ObjectiveType::Explore_Area;
    Objective.TargetCount = 60; // 60 seconds in biome
    Objective.TargetBiome = EQuest_BiomeType::Floresta;
    Objective.TargetLocation = FVector(-45000, 40000, 100);
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_MissionData UQuest_MissionObjectiveComponent::CreateCrowdMission()
{
    FQuest_MissionData Mission;
    Mission.MissionID = TEXT("CROWD_001");
    Mission.MissionName = TEXT("Follow the Tribe");
    Mission.MissionDescription = TEXT("Follow the tribal group to their gathering area");
    Mission.Priority = 2;
    Mission.MissionTimeLimit = 600.0f; // 10 minutes
    Mission.bIsMainStoryMission = true;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("FOLLOW_CROWD");
    Objective.ObjectiveName = TEXT("Stay Near Tribe");
    Objective.Description = TEXT("Remain close to the tribal group");
    Objective.ObjectiveType = EQuest_ObjectiveType::Follow_Crowd;
    Objective.TargetCount = 30; // 30 seconds near crowd
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

// Mission Manager Implementation
AQuest_MissionManager::AQuest_MissionManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Check every 5 seconds
    
    MaxActiveMissionsPerPlayer = 3;
    MissionGenerationInterval = 300.0f; // 5 minutes
    LastMissionGenerationTime = 0.0f;
    MissionCounter = 1;
    
    SetupBiomeLocations();
}

void AQuest_MissionManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMissionSystem();
    UE_LOG(LogTemp, Warning, TEXT("Quest Mission Manager initialized"));
}

void AQuest_MissionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastMissionGenerationTime += DeltaTime;
    
    if (LastMissionGenerationTime >= MissionGenerationInterval)
    {
        GenerateRandomMission();
        LastMissionGenerationTime = 0.0f;
    }
}

void AQuest_MissionManager::AssignMissionToPlayer(ATranspersonalCharacter* Player, const FQuest_MissionData& Mission)
{
    if (Player)
    {
        UQuest_MissionObjectiveComponent* MissionComp = Player->FindComponentByClass<UQuest_MissionObjectiveComponent>();
        if (MissionComp)
        {
            TArray<FQuest_MissionData> ActiveMissions = MissionComp->GetActiveMissions();
            if (ActiveMissions.Num() < MaxActiveMissionsPerPlayer)
            {
                MissionComp->StartMission(Mission);
            }
        }
    }
}

void AQuest_MissionManager::GenerateRandomMission()
{
    TArray<EQuest_BiomeType> Biomes = {EQuest_BiomeType::Savana, EQuest_BiomeType::Floresta, EQuest_BiomeType::Deserto};
    EQuest_BiomeType RandomBiome = Biomes[FMath::RandRange(0, Biomes.Num() - 1)];
    
    int32 MissionType = FMath::RandRange(0, 2);
    FQuest_MissionData NewMission;
    
    switch (MissionType)
    {
        case 0:
            NewMission = CreateSurvivalMission(RandomBiome, 900.0f);
            break;
        case 1:
            NewMission = CreateHuntingMission(RandomBiome, TEXT("Raptor"));
            break;
        case 2:
            NewMission = CreateExplorationMission(RandomBiome, BiomeLocations[RandomBiome]);
            break;
    }
    
    // Find player and assign mission
    if (UWorld* World = GetWorld())
    {
        ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
        if (Player)
        {
            AssignMissionToPlayer(Player, NewMission);
        }
    }
}

void AQuest_MissionManager::GenerateCrowdBasedMission()
{
    // Find crowd spawners in the world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> CrowdSpawners;
        UGameplayStatics::GetAllActorsOfClass(World, ACrowd_MassEntitySpawner::StaticClass(), CrowdSpawners);
        
        if (CrowdSpawners.Num() > 0)
        {
            ACrowd_MassEntitySpawner* RandomSpawner = Cast<ACrowd_MassEntitySpawner>(CrowdSpawners[FMath::RandRange(0, CrowdSpawners.Num() - 1)]);
            if (RandomSpawner)
            {
                FQuest_MissionData CrowdMission = CreateCrowdInteractionMission(RandomSpawner);
                
                ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
                if (Player)
                {
                    AssignMissionToPlayer(Player, CrowdMission);
                }
            }
        }
    }
}

FQuest_MissionData AQuest_MissionManager::CreateSurvivalMission(EQuest_BiomeType Biome, float Duration)
{
    FQuest_MissionData Mission;
    Mission.MissionID = FString::Printf(TEXT("SURVIVAL_%d"), MissionCounter++);
    Mission.MissionName = FString::Printf(TEXT("Survive in %s"), *UEnum::GetValueAsString(Biome));
    Mission.MissionDescription = TEXT("Test your survival skills in a dangerous biome");
    Mission.Priority = 1;
    Mission.MissionTimeLimit = Duration;
    Mission.bIsMainStoryMission = false;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("SURVIVE_TIME");
    Objective.ObjectiveName = TEXT("Survive the Duration");
    Objective.Description = FString::Printf(TEXT("Survive for %.0f seconds"), Duration * 0.8f);
    Objective.ObjectiveType = EQuest_ObjectiveType::Survive_Time;
    Objective.TargetCount = FMath::FloorToInt(Duration * 0.8f);
    Objective.TargetBiome = Biome;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_MissionData AQuest_MissionManager::CreateHuntingMission(EQuest_BiomeType Biome, const FString& TargetSpecies)
{
    FQuest_MissionData Mission;
    Mission.MissionID = FString::Printf(TEXT("HUNT_%d"), MissionCounter++);
    Mission.MissionName = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    Mission.MissionDescription = FString::Printf(TEXT("Track and hunt a %s in the %s biome"), *TargetSpecies, *UEnum::GetValueAsString(Biome));
    Mission.Priority = 2;
    Mission.MissionTimeLimit = 1800.0f;
    Mission.bIsMainStoryMission = false;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("HUNT_TARGET");
    Objective.ObjectiveName = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    Objective.Description = FString::Printf(TEXT("Successfully hunt a %s"), *TargetSpecies);
    Objective.ObjectiveType = EQuest_ObjectiveType::Hunt_Dinosaur;
    Objective.TargetCount = 1;
    Objective.TargetBiome = Biome;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_MissionData AQuest_MissionManager::CreateExplorationMission(EQuest_BiomeType Biome, const FVector& TargetLocation)
{
    FQuest_MissionData Mission;
    Mission.MissionID = FString::Printf(TEXT("EXPLORE_%d"), MissionCounter++);
    Mission.MissionName = FString::Printf(TEXT("Explore %s"), *UEnum::GetValueAsString(Biome));
    Mission.MissionDescription = FString::Printf(TEXT("Explore and map the %s biome"), *UEnum::GetValueAsString(Biome));
    Mission.Priority = 1;
    Mission.MissionTimeLimit = 1200.0f;
    Mission.bIsMainStoryMission = false;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("REACH_LOCATION");
    Objective.ObjectiveName = TEXT("Reach Target Area");
    Objective.Description = TEXT("Reach the designated exploration point");
    Objective.ObjectiveType = EQuest_ObjectiveType::Reach_Location;
    Objective.TargetCount = 1;
    Objective.TargetLocation = TargetLocation;
    Objective.TargetBiome = Biome;
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

FQuest_MissionData AQuest_MissionManager::CreateCrowdInteractionMission(ACrowd_MassEntitySpawner* CrowdSpawner)
{
    FQuest_MissionData Mission;
    Mission.MissionID = FString::Printf(TEXT("CROWD_%d"), MissionCounter++);
    Mission.MissionName = TEXT("Tribal Interaction");
    Mission.MissionDescription = TEXT("Interact with the tribal group and learn from them");
    Mission.Priority = 3;
    Mission.MissionTimeLimit = 600.0f;
    Mission.bIsMainStoryMission = true;
    
    FQuest_ObjectiveData Objective;
    Objective.ObjectiveID = TEXT("APPROACH_CROWD");
    Objective.ObjectiveName = TEXT("Approach Tribal Group");
    Objective.Description = TEXT("Get close to the tribal group and observe their behavior");
    Objective.ObjectiveType = EQuest_ObjectiveType::Follow_Crowd;
    Objective.TargetCount = 30; // 30 seconds near crowd
    
    Mission.Objectives.Add(Objective);
    return Mission;
}

void AQuest_MissionManager::InitializeMissionSystem()
{
    SpawnMissionObjectives();
    UE_LOG(LogTemp, Warning, TEXT("Mission system initialized with biome locations"));
}

void AQuest_MissionManager::SpawnMissionObjectives()
{
    // This could spawn mission-related actors in the world
    // For now, just log that the system is ready
    UE_LOG(LogTemp, Warning, TEXT("Mission objectives spawned across all biomes"));
}

void AQuest_MissionManager::SetupBiomeLocations()
{
    BiomeLocations.Add(EQuest_BiomeType::Savana, FVector(0, 0, 100));
    BiomeLocations.Add(EQuest_BiomeType::Pantano, FVector(-50000, -45000, 100));
    BiomeLocations.Add(EQuest_BiomeType::Floresta, FVector(-45000, 40000, 100));
    BiomeLocations.Add(EQuest_BiomeType::Deserto, FVector(55000, 0, 100));
    BiomeLocations.Add(EQuest_BiomeType::Montanha, FVector(40000, 50000, 100));
}

FString AQuest_MissionManager::GenerateUniqueID()
{
    return FString::Printf(TEXT("MISSION_%d_%d"), MissionCounter++, FMath::RandRange(1000, 9999));
}