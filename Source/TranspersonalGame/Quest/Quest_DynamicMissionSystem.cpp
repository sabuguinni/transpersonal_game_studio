#include "Quest_DynamicMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"

UQuest_DynamicMissionSystem::UQuest_DynamicMissionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize mission types
    InitializeMissionTypes();
    
    // Set default values
    MaxActiveMissions = 5;
    MissionCheckRadius = 5000.0f;
    PlayerLevel = 1;
    bSystemActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Component initialized"));
}

void UQuest_DynamicMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get world and player references
    World = GetWorld();
    if (World)
    {
        PlayerController = World->GetFirstPlayerController();
        if (PlayerController && PlayerController->GetPawn())
        {
            PlayerPawn = PlayerController->GetPawn();
            UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Player references acquired"));
        }
    }
    
    // Initialize mission system
    InitializeBiomeMissions();
    StartMissionGeneration();
}

void UQuest_DynamicMissionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bSystemActive || !PlayerPawn)
        return;
    
    // Update active missions
    UpdateActiveMissions(DeltaTime);
    
    // Check for new mission opportunities
    CheckForNewMissions();
    
    // Clean up completed missions
    CleanupCompletedMissions();
}

void UQuest_DynamicMissionSystem::InitializeMissionTypes()
{
    // Clear existing mission types
    MissionTypes.Empty();
    
    // SURVIVAL MISSION: Water Collection
    FQuest_MissionType WaterMission;
    WaterMission.MissionID = "water_collection";
    WaterMission.MissionName = "Find Clean Water";
    WaterMission.Description = "Locate and collect clean water to survive in the harsh prehistoric environment";
    WaterMission.RequiredBiome = EBiomeType::Swamp;
    WaterMission.DifficultyLevel = 1;
    WaterMission.RewardExperience = 100;
    WaterMission.TimeLimit = 300.0f; // 5 minutes
    WaterMission.ObjectiveType = EQuestObjectiveType::Collect;
    WaterMission.TargetQuantity = 3;
    MissionTypes.Add(WaterMission);
    
    // SURVIVAL MISSION: Wood Gathering
    FQuest_MissionType WoodMission;
    WoodMission.MissionID = "wood_gathering";
    WoodMission.MissionName = "Gather Building Materials";
    WoodMission.Description = "Collect fallen branches and wood for shelter construction";
    WoodMission.RequiredBiome = EBiomeType::Forest;
    WoodMission.DifficultyLevel = 1;
    WoodMission.RewardExperience = 80;
    WoodMission.TimeLimit = 240.0f; // 4 minutes
    WoodMission.ObjectiveType = EQuestObjectiveType::Collect;
    WoodMission.TargetQuantity = 5;
    MissionTypes.Add(WoodMission);
    
    // SURVIVAL MISSION: Hunting
    FQuest_MissionType HuntMission;
    HuntMission.MissionID = "basic_hunting";
    HuntMission.MissionName = "Hunt for Food";
    HuntMission.Description = "Hunt small creatures for meat to maintain energy and health";
    HuntMission.RequiredBiome = EBiomeType::Savanna;
    HuntMission.DifficultyLevel = 2;
    HuntMission.RewardExperience = 150;
    HuntMission.TimeLimit = 600.0f; // 10 minutes
    HuntMission.ObjectiveType = EQuestObjectiveType::Kill;
    HuntMission.TargetQuantity = 2;
    MissionTypes.Add(HuntMission);
    
    // SURVIVAL MISSION: Shelter Building
    FQuest_MissionType ShelterMission;
    ShelterMission.MissionID = "shelter_construction";
    ShelterMission.MissionName = "Build Emergency Shelter";
    ShelterMission.Description = "Construct a basic shelter to protect from desert storms";
    ShelterMission.RequiredBiome = EBiomeType::Desert;
    ShelterMission.DifficultyLevel = 3;
    ShelterMission.RewardExperience = 200;
    ShelterMission.TimeLimit = 480.0f; // 8 minutes
    ShelterMission.ObjectiveType = EQuestObjectiveType::Build;
    ShelterMission.TargetQuantity = 1;
    MissionTypes.Add(ShelterMission);
    
    // SURVIVAL MISSION: Tool Crafting
    FQuest_MissionType CraftMission;
    CraftMission.MissionID = "tool_crafting";
    CraftMission.MissionName = "Craft Survival Tools";
    CraftMission.Description = "Create essential tools from stone and wood for better survival chances";
    CraftMission.RequiredBiome = EBiomeType::Mountain;
    CraftMission.DifficultyLevel = 2;
    CraftMission.RewardExperience = 120;
    CraftMission.TimeLimit = 360.0f; // 6 minutes
    CraftMission.ObjectiveType = EQuestObjectiveType::Craft;
    CraftMission.TargetQuantity = 2;
    MissionTypes.Add(CraftMission);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Initialized %d mission types"), MissionTypes.Num());
}

void UQuest_DynamicMissionSystem::InitializeBiomeMissions()
{
    // Clear existing biome missions
    BiomeMissions.Empty();
    
    // Map mission types to biomes
    for (const FQuest_MissionType& MissionType : MissionTypes)
    {
        if (!BiomeMissions.Contains(MissionType.RequiredBiome))
        {
            BiomeMissions.Add(MissionType.RequiredBiome, TArray<FQuest_MissionType>());
        }
        BiomeMissions[MissionType.RequiredBiome].Add(MissionType);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Mapped missions to %d biomes"), BiomeMissions.Num());
}

void UQuest_DynamicMissionSystem::StartMissionGeneration()
{
    if (!World)
        return;
    
    // Start mission generation timer
    World->GetTimerManager().SetTimer(
        MissionGenerationTimer,
        this,
        &UQuest_DynamicMissionSystem::GenerateNewMission,
        30.0f, // Generate new mission every 30 seconds
        true
    );
    
    // Generate initial mission
    GenerateNewMission();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Mission generation started"));
}

void UQuest_DynamicMissionSystem::GenerateNewMission()
{
    if (!PlayerPawn || ActiveMissions.Num() >= MaxActiveMissions)
        return;
    
    // Determine player's current biome
    EBiomeType CurrentBiome = DeterminePlayerBiome();
    
    // Get available missions for current biome
    if (!BiomeMissions.Contains(CurrentBiome))
        return;
    
    const TArray<FQuest_MissionType>& AvailableMissions = BiomeMissions[CurrentBiome];
    if (AvailableMissions.Num() == 0)
        return;
    
    // Select random mission appropriate for player level
    TArray<FQuest_MissionType> SuitableMissions;
    for (const FQuest_MissionType& Mission : AvailableMissions)
    {
        if (Mission.DifficultyLevel <= PlayerLevel + 1) // Allow slightly harder missions
        {
            SuitableMissions.Add(Mission);
        }
    }
    
    if (SuitableMissions.Num() == 0)
        return;
    
    // Create new mission instance
    int32 RandomIndex = FMath::RandRange(0, SuitableMissions.Num() - 1);
    const FQuest_MissionType& SelectedMission = SuitableMissions[RandomIndex];
    
    FQuest_ActiveMission NewMission;
    NewMission.MissionType = SelectedMission;
    NewMission.StartTime = World->GetTimeSeconds();
    NewMission.CurrentProgress = 0;
    NewMission.bCompleted = false;
    NewMission.bFailed = false;
    NewMission.MissionInstanceID = FGuid::NewGuid().ToString();
    
    // Set mission location based on biome
    NewMission.TargetLocation = GetBiomeCenter(CurrentBiome);
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Generated new mission '%s' in biome %d"), 
           *SelectedMission.MissionName, (int32)CurrentBiome);
    
    // Notify player of new mission
    NotifyPlayerOfNewMission(NewMission);
}

EBiomeType UQuest_DynamicMissionSystem::DeterminePlayerBiome()
{
    if (!PlayerPawn)
        return EBiomeType::Savanna; // Default to savanna
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // PANTANO (Swamp) - Southwest
    if (PlayerLocation.X >= -77500 && PlayerLocation.X <= -25000 &&
        PlayerLocation.Y >= -76500 && PlayerLocation.Y <= -15000)
    {
        return EBiomeType::Swamp;
    }
    
    // FLORESTA (Forest) - Northwest
    if (PlayerLocation.X >= -77500 && PlayerLocation.X <= -15000 &&
        PlayerLocation.Y >= 15000 && PlayerLocation.Y <= 76500)
    {
        return EBiomeType::Forest;
    }
    
    // DESERTO (Desert) - East
    if (PlayerLocation.X >= 25000 && PlayerLocation.X <= 79500 &&
        PlayerLocation.Y >= -30000 && PlayerLocation.Y <= 30000)
    {
        return EBiomeType::Desert;
    }
    
    // MONTANHA (Mountain) - Northeast
    if (PlayerLocation.X >= 15000 && PlayerLocation.X <= 79500 &&
        PlayerLocation.Y >= 20000 && PlayerLocation.Y <= 76500)
    {
        return EBiomeType::Mountain;
    }
    
    // SAVANA (Center) - Default
    return EBiomeType::Savanna;
}

FVector UQuest_DynamicMissionSystem::GetBiomeCenter(EBiomeType Biome)
{
    switch (Biome)
    {
        case EBiomeType::Swamp:
            return FVector(-50000, -45000, 0);
        case EBiomeType::Forest:
            return FVector(-45000, 40000, 0);
        case EBiomeType::Desert:
            return FVector(55000, 0, 0);
        case EBiomeType::Mountain:
            return FVector(40000, 50000, 500);
        case EBiomeType::Savanna:
        default:
            return FVector(0, 0, 0);
    }
}

void UQuest_DynamicMissionSystem::UpdateActiveMissions(float DeltaTime)
{
    if (!World)
        return;
    
    float CurrentTime = World->GetTimeSeconds();
    
    for (FQuest_ActiveMission& Mission : ActiveMissions)
    {
        if (Mission.bCompleted || Mission.bFailed)
            continue;
        
        // Check for mission timeout
        float ElapsedTime = CurrentTime - Mission.StartTime;
        if (ElapsedTime >= Mission.MissionType.TimeLimit)
        {
            Mission.bFailed = true;
            UE_LOG(LogTemp, Warning, TEXT("Quest_DynamicMissionSystem: Mission '%s' timed out"), 
                   *Mission.MissionType.MissionName);
            NotifyPlayerOfMissionFailure(Mission);
            continue;
        }
        
        // Update mission progress based on type
        UpdateMissionProgress(Mission);
    }
}

void UQuest_DynamicMissionSystem::UpdateMissionProgress(FQuest_ActiveMission& Mission)
{
    // This would be connected to actual game systems
    // For now, simulate some progress for testing
    
    switch (Mission.MissionType.ObjectiveType)
    {
        case EQuestObjectiveType::Collect:
            // Check for collected items in player inventory
            break;
            
        case EQuestObjectiveType::Kill:
            // Check for killed creatures
            break;
            
        case EQuestObjectiveType::Build:
            // Check for constructed buildings
            break;
            
        case EQuestObjectiveType::Craft:
            // Check for crafted items
            break;
            
        case EQuestObjectiveType::Reach:
            // Check if player reached target location
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Mission.TargetLocation);
                if (Distance <= MissionCheckRadius)
                {
                    Mission.CurrentProgress = Mission.MissionType.TargetQuantity;
                }
            }
            break;
    }
    
    // Check if mission is completed
    if (Mission.CurrentProgress >= Mission.MissionType.TargetQuantity && !Mission.bCompleted)
    {
        Mission.bCompleted = true;
        CompleteMission(Mission);
    }
}

void UQuest_DynamicMissionSystem::CompleteMission(const FQuest_ActiveMission& Mission)
{
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Mission '%s' completed!"), 
           *Mission.MissionType.MissionName);
    
    // Award experience
    PlayerLevel += Mission.MissionType.RewardExperience / 100; // Simple leveling
    
    // Notify player
    NotifyPlayerOfMissionCompletion(Mission);
    
    // Trigger mission completion event
    OnMissionCompleted.ExecuteIfBound(Mission);
}

void UQuest_DynamicMissionSystem::CheckForNewMissions()
{
    // Check if we should generate new missions based on player activity
    if (ActiveMissions.Num() < MaxActiveMissions / 2)
    {
        // Player has few active missions, consider generating more
        if (FMath::RandRange(0, 100) < 30) // 30% chance per check
        {
            GenerateNewMission();
        }
    }
}

void UQuest_DynamicMissionSystem::CleanupCompletedMissions()
{
    // Remove completed or failed missions after some time
    if (!World)
        return;
    
    float CurrentTime = World->GetTimeSeconds();
    
    ActiveMissions.RemoveAll([CurrentTime](const FQuest_ActiveMission& Mission)
    {
        if (Mission.bCompleted || Mission.bFailed)
        {
            // Remove after 60 seconds
            return (CurrentTime - Mission.StartTime) > 60.0f;
        }
        return false;
    });
}

void UQuest_DynamicMissionSystem::NotifyPlayerOfNewMission(const FQuest_ActiveMission& Mission)
{
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("NEW MISSION: %s - %s"), 
                                        *Mission.MissionType.MissionName, 
                                        *Mission.MissionType.Description);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Notified player of new mission"));
}

void UQuest_DynamicMissionSystem::NotifyPlayerOfMissionCompletion(const FQuest_ActiveMission& Mission)
{
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("MISSION COMPLETED: %s (+%d XP)"), 
                                        *Mission.MissionType.MissionName, 
                                        Mission.MissionType.RewardExperience);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Notified player of mission completion"));
}

void UQuest_DynamicMissionSystem::NotifyPlayerOfMissionFailure(const FQuest_ActiveMission& Mission)
{
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("MISSION FAILED: %s (Time Limit Exceeded)"), 
                                        *Mission.MissionType.MissionName);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, Message);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: Notified player of mission failure"));
}

TArray<FQuest_ActiveMission> UQuest_DynamicMissionSystem::GetActiveMissions() const
{
    return ActiveMissions;
}

bool UQuest_DynamicMissionSystem::ForceCompleteMission(const FString& MissionInstanceID)
{
    for (FQuest_ActiveMission& Mission : ActiveMissions)
    {
        if (Mission.MissionInstanceID == MissionInstanceID && !Mission.bCompleted && !Mission.bFailed)
        {
            Mission.bCompleted = true;
            Mission.CurrentProgress = Mission.MissionType.TargetQuantity;
            CompleteMission(Mission);
            return true;
        }
    }
    return false;
}

void UQuest_DynamicMissionSystem::SetSystemActive(bool bActive)
{
    bSystemActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("Quest_DynamicMissionSystem: System %s"), 
           bActive ? TEXT("activated") : TEXT("deactivated"));
}