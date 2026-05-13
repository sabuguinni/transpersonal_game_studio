#include "Quest_SurvivalResourceSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UQuest_SurvivalResourceSystem::UQuest_SurvivalResourceSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MaxActiveMissions = 3;
    MissionCheckInterval = 2.0f;
    LastMissionCheck = 0.0f;
    bTutorialCompleted = false;
    CurrentTutorialStep = 0;
}

void UQuest_SurvivalResourceSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize resource inventory
    ResourceInventory.Empty();
    ResourceInventory.Add(EQuest_ResourceType::Stone, 0);
    ResourceInventory.Add(EQuest_ResourceType::Wood, 0);
    ResourceInventory.Add(EQuest_ResourceType::Water, 0);
    ResourceInventory.Add(EQuest_ResourceType::Food, 0);
    ResourceInventory.Add(EQuest_ResourceType::Fiber, 0);
    ResourceInventory.Add(EQuest_ResourceType::Hide, 0);
    ResourceInventory.Add(EQuest_ResourceType::Bone, 0);
    
    // Initialize survival missions
    InitializeSurvivalMissions();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalResourceSystem initialized"));
}

void UQuest_SurvivalResourceSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastMissionCheck += DeltaTime;
    if (LastMissionCheck >= MissionCheckInterval)
    {
        UpdateMissionProgress();
        LastMissionCheck = 0.0f;
    }
}

void UQuest_SurvivalResourceSystem::InitializeSurvivalMissions()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    // Generate basic survival missions
    GenerateBasicSurvivalMissions();
    
    UE_LOG(LogTemp, Warning, TEXT("Survival missions initialized with %d active missions"), ActiveMissions.Num());
}

void UQuest_SurvivalResourceSystem::StartMission(EQuest_SurvivalMissionType MissionType)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission: Maximum active missions reached"));
        return;
    }
    
    FQuest_SurvivalMission NewMission;
    
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::GatherStone:
            NewMission = CreateGatheringMission(EQuest_ResourceType::Stone, 5);
            break;
        case EQuest_SurvivalMissionType::CollectWood:
            NewMission = CreateGatheringMission(EQuest_ResourceType::Wood, 3);
            break;
        case EQuest_SurvivalMissionType::FindWater:
            NewMission = CreateGatheringMission(EQuest_ResourceType::Water, 1);
            break;
        case EQuest_SurvivalMissionType::HuntFood:
            NewMission = CreateGatheringMission(EQuest_ResourceType::Food, 2);
            break;
        case EQuest_SurvivalMissionType::BuildShelter:
            {
                TArray<FQuest_ResourceRequirement> ShelterRequirements;
                FQuest_ResourceRequirement WoodReq;
                WoodReq.ResourceType = EQuest_ResourceType::Wood;
                WoodReq.RequiredAmount = 10;
                ShelterRequirements.Add(WoodReq);
                
                FQuest_ResourceRequirement StoneReq;
                StoneReq.ResourceType = EQuest_ResourceType::Stone;
                StoneReq.RequiredAmount = 5;
                ShelterRequirements.Add(StoneReq);
                
                NewMission = CreateCraftingMission(TEXT("Basic Shelter"), ShelterRequirements);
            }
            break;
        case EQuest_SurvivalMissionType::CraftTool:
            {
                TArray<FQuest_ResourceRequirement> ToolRequirements;
                FQuest_ResourceRequirement StoneReq;
                StoneReq.ResourceType = EQuest_ResourceType::Stone;
                StoneReq.RequiredAmount = 2;
                ToolRequirements.Add(StoneReq);
                
                FQuest_ResourceRequirement WoodReq;
                WoodReq.ResourceType = EQuest_ResourceType::Wood;
                WoodReq.RequiredAmount = 1;
                ToolRequirements.Add(WoodReq);
                
                NewMission = CreateCraftingMission(TEXT("Stone Axe"), ToolRequirements);
            }
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown mission type"));
            return;
    }
    
    NewMission.bIsActive = true;
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Started mission: %s"), *NewMission.MissionName);
}

void UQuest_SurvivalResourceSystem::CompleteMission(int32 MissionIndex)
{
    if (ActiveMissions.IsValidIndex(MissionIndex))
    {
        FQuest_SurvivalMission CompletedMission = ActiveMissions[MissionIndex];
        CompletedMission.bIsCompleted = true;
        CompletedMission.bIsActive = false;
        
        CompletedMissions.Add(CompletedMission);
        ActiveMissions.RemoveAt(MissionIndex);
        
        UE_LOG(LogTemp, Warning, TEXT("Completed mission: %s (Reward: %d XP)"), 
               *CompletedMission.MissionName, CompletedMission.ExperienceReward);
    }
}

bool UQuest_SurvivalResourceSystem::CheckMissionCompletion(int32 MissionIndex)
{
    if (!ActiveMissions.IsValidIndex(MissionIndex))
    {
        return false;
    }
    
    const FQuest_SurvivalMission& Mission = ActiveMissions[MissionIndex];
    
    // Check if all resource requirements are met
    if (Mission.ResourceRequirements.Num() > 0)
    {
        return HasRequiredResources(Mission.ResourceRequirements);
    }
    
    // Check location-based completion
    if (Mission.TargetLocation != FVector::ZeroVector)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            float Distance = FVector::Dist(Owner->GetActorLocation(), Mission.TargetLocation);
            return Distance <= Mission.CompletionRadius;
        }
    }
    
    return false;
}

void UQuest_SurvivalResourceSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceInventory.Contains(ResourceType))
    {
        ResourceInventory[ResourceType] += Amount;
    }
    else
    {
        ResourceInventory.Add(ResourceType, Amount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Added %d %s (Total: %d)"), 
           Amount, 
           *UEnum::GetValueAsString(ResourceType),
           ResourceInventory[ResourceType]);
}

int32 UQuest_SurvivalResourceSystem::GetResourceCount(EQuest_ResourceType ResourceType)
{
    if (ResourceInventory.Contains(ResourceType))
    {
        return ResourceInventory[ResourceType];
    }
    return 0;
}

bool UQuest_SurvivalResourceSystem::HasRequiredResources(const TArray<FQuest_ResourceRequirement>& Requirements)
{
    for (const FQuest_ResourceRequirement& Requirement : Requirements)
    {
        int32 CurrentAmount = GetResourceCount(Requirement.ResourceType);
        if (CurrentAmount < Requirement.RequiredAmount)
        {
            return false;
        }
    }
    return true;
}

void UQuest_SurvivalResourceSystem::UpdateMissionProgress()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (CheckMissionCompletion(i))
        {
            CompleteMission(i);
        }
    }
    
    // Generate new missions if needed
    if (ActiveMissions.Num() < MaxActiveMissions && !bTutorialCompleted)
    {
        GenerateBasicSurvivalMissions();
    }
}

TArray<FQuest_SurvivalMission> UQuest_SurvivalResourceSystem::GetActiveMissions()
{
    return ActiveMissions;
}

FQuest_SurvivalMission UQuest_SurvivalResourceSystem::GetMissionByType(EQuest_SurvivalMissionType MissionType)
{
    for (const FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionType == MissionType)
        {
            return Mission;
        }
    }
    
    return FQuest_SurvivalMission();
}

void UQuest_SurvivalResourceSystem::CheckLocationBasedMissions(FVector PlayerLocation)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        if (Mission.TargetLocation != FVector::ZeroVector)
        {
            float Distance = FVector::Dist(PlayerLocation, Mission.TargetLocation);
            if (Distance <= Mission.CompletionRadius)
            {
                UE_LOG(LogTemp, Warning, TEXT("Player reached mission location: %s"), *Mission.MissionName);
                CompleteMission(i);
                break;
            }
        }
    }
}

void UQuest_SurvivalResourceSystem::CreateResourceGatheringMission(EQuest_ResourceType ResourceType, FVector Location)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission = CreateGatheringMission(ResourceType, 3);
    NewMission.TargetLocation = Location;
    NewMission.CompletionRadius = 300.0f;
    NewMission.bIsActive = true;
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Created resource gathering mission at location: %s"), *Location.ToString());
}

void UQuest_SurvivalResourceSystem::CreateCraftingMission(const TArray<FQuest_ResourceRequirement>& Requirements)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        return;
    }
    
    FQuest_SurvivalMission NewMission = CreateCraftingMission(TEXT("Custom Crafting"), Requirements);
    NewMission.bIsActive = true;
    
    ActiveMissions.Add(NewMission);
}

void UQuest_SurvivalResourceSystem::GenerateBasicSurvivalMissions()
{
    if (CurrentTutorialStep == 0)
    {
        StartMission(EQuest_SurvivalMissionType::GatherStone);
        CurrentTutorialStep++;
    }
    else if (CurrentTutorialStep == 1 && CompletedMissions.Num() >= 1)
    {
        StartMission(EQuest_SurvivalMissionType::CollectWood);
        CurrentTutorialStep++;
    }
    else if (CurrentTutorialStep == 2 && CompletedMissions.Num() >= 2)
    {
        StartMission(EQuest_SurvivalMissionType::CraftTool);
        CurrentTutorialStep++;
    }
    else if (CurrentTutorialStep == 3 && CompletedMissions.Num() >= 3)
    {
        StartMission(EQuest_SurvivalMissionType::FindWater);
        CurrentTutorialStep++;
    }
    else if (CurrentTutorialStep == 4 && CompletedMissions.Num() >= 4)
    {
        StartMission(EQuest_SurvivalMissionType::BuildShelter);
        bTutorialCompleted = true;
    }
}

void UQuest_SurvivalResourceSystem::GenerateAdvancedSurvivalMissions()
{
    // Advanced missions for post-tutorial gameplay
    if (bTutorialCompleted && ActiveMissions.Num() < MaxActiveMissions)
    {
        TArray<EQuest_SurvivalMissionType> AdvancedMissions = {
            EQuest_SurvivalMissionType::HuntFood,
            EQuest_SurvivalMissionType::ExploreArea,
            EQuest_SurvivalMissionType::DefendCamp
        };
        
        int32 RandomIndex = FMath::RandRange(0, AdvancedMissions.Num() - 1);
        StartMission(AdvancedMissions[RandomIndex]);
    }
}

FQuest_SurvivalMission UQuest_SurvivalResourceSystem::CreateGatheringMission(EQuest_ResourceType ResourceType, int32 Amount)
{
    FQuest_SurvivalMission Mission;
    
    FQuest_ResourceRequirement Requirement;
    Requirement.ResourceType = ResourceType;
    Requirement.RequiredAmount = Amount;
    Mission.ResourceRequirements.Add(Requirement);
    
    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone:
            Mission.MissionName = TEXT("Gather Stones");
            Mission.Description = FString::Printf(TEXT("Collect %d stones for crafting"), Amount);
            Mission.MissionType = EQuest_SurvivalMissionType::GatherStone;
            break;
        case EQuest_ResourceType::Wood:
            Mission.MissionName = TEXT("Collect Wood");
            Mission.Description = FString::Printf(TEXT("Gather %d pieces of wood"), Amount);
            Mission.MissionType = EQuest_SurvivalMissionType::CollectWood;
            break;
        case EQuest_ResourceType::Water:
            Mission.MissionName = TEXT("Find Water");
            Mission.Description = FString::Printf(TEXT("Locate and collect %d water sources"), Amount);
            Mission.MissionType = EQuest_SurvivalMissionType::FindWater;
            break;
        case EQuest_ResourceType::Food:
            Mission.MissionName = TEXT("Hunt for Food");
            Mission.Description = FString::Printf(TEXT("Hunt and gather %d food items"), Amount);
            Mission.MissionType = EQuest_SurvivalMissionType::HuntFood;
            break;
        default:
            Mission.MissionName = TEXT("Gather Resources");
            Mission.Description = TEXT("Collect required resources");
            break;
    }
    
    Mission.ExperienceReward = Amount * 5;
    return Mission;
}

FQuest_SurvivalMission UQuest_SurvivalResourceSystem::CreateCraftingMission(const FString& ItemName, const TArray<FQuest_ResourceRequirement>& Requirements)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = FString::Printf(TEXT("Craft %s"), *ItemName);
    Mission.Description = FString::Printf(TEXT("Gather materials and craft a %s"), *ItemName);
    Mission.MissionType = EQuest_SurvivalMissionType::CraftTool;
    Mission.ResourceRequirements = Requirements;
    Mission.ExperienceReward = Requirements.Num() * 10;
    
    return Mission;
}

FQuest_SurvivalMission UQuest_SurvivalResourceSystem::CreateExplorationMission(FVector TargetLocation, const FString& AreaName)
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = FString::Printf(TEXT("Explore %s"), *AreaName);
    Mission.Description = FString::Printf(TEXT("Travel to and explore the %s area"), *AreaName);
    Mission.MissionType = EQuest_SurvivalMissionType::ExploreArea;
    Mission.TargetLocation = TargetLocation;
    Mission.CompletionRadius = 500.0f;
    Mission.ExperienceReward = 25;
    
    return Mission;
}

bool UQuest_SurvivalResourceSystem::ValidateMissionRequirements(const FQuest_SurvivalMission& Mission)
{
    // Validate that mission requirements are reasonable
    for (const FQuest_ResourceRequirement& Requirement : Mission.ResourceRequirements)
    {
        if (Requirement.RequiredAmount <= 0 || Requirement.RequiredAmount > 100)
        {
            return false;
        }
    }
    return true;
}

void UQuest_SurvivalResourceSystem::CleanupCompletedMissions()
{
    // Keep only the last 10 completed missions to prevent memory bloat
    if (CompletedMissions.Num() > 10)
    {
        CompletedMissions.RemoveAt(0, CompletedMissions.Num() - 10);
    }
}

void UQuest_SurvivalResourceSystem::UpdateMissionObjectives()
{
    // Update mission objectives based on current game state
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionType == EQuest_SurvivalMissionType::ExploreArea)
        {
            // Update exploration missions with dynamic objectives
            AActor* Owner = GetOwner();
            if (Owner && Mission.TargetLocation != FVector::ZeroVector)
            {
                float Distance = FVector::Dist(Owner->GetActorLocation(), Mission.TargetLocation);
                if (Distance < 1000.0f)
                {
                    Mission.Description = TEXT("You're getting close to the target area!");
                }
            }
        }
    }
}

// AQuest_SurvivalResourceManager Implementation
AQuest_SurvivalResourceManager::AQuest_SurvivalResourceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f;
    
    SurvivalSystem = CreateDefaultSubobject<UQuest_SurvivalResourceSystem>(TEXT("SurvivalSystem"));
    bGlobalSystemActive = true;
}

void AQuest_SurvivalResourceManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeGlobalSurvivalSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalResourceManager started"));
}

void AQuest_SurvivalResourceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bGlobalSystemActive)
    {
        UpdateGlobalMissionState();
        CheckPlayerSurvivalStatus();
    }
}

void AQuest_SurvivalResourceManager::InitializeGlobalSurvivalSystem()
{
    if (SurvivalSystem)
    {
        SurvivalSystem->InitializeSurvivalMissions();
    }
    
    // Generate environmental missions
    GenerateEnvironmentalMissions();
    
    UE_LOG(LogTemp, Warning, TEXT("Global survival system initialized"));
}

void AQuest_SurvivalResourceManager::RegisterPlayer(APawn* Player)
{
    if (Player && !RegisteredPlayers.Contains(Player))
    {
        RegisteredPlayers.Add(Player);
        UE_LOG(LogTemp, Warning, TEXT("Player registered with survival system"));
    }
}

void AQuest_SurvivalResourceManager::CreateEmergencyMission(EQuest_SurvivalMissionType MissionType, FVector Location)
{
    if (SurvivalSystem)
    {
        SurvivalSystem->StartMission(MissionType);
        UE_LOG(LogTemp, Warning, TEXT("Emergency mission created: %s"), *UEnum::GetValueAsString(MissionType));
    }
}

void AQuest_SurvivalResourceManager::BroadcastMissionUpdate(const FQuest_SurvivalMission& Mission)
{
    // Broadcast mission updates to all registered players
    for (AActor* Player : RegisteredPlayers)
    {
        if (Player)
        {
            UE_LOG(LogTemp, Warning, TEXT("Broadcasting mission update: %s"), *Mission.MissionName);
        }
    }
}

void AQuest_SurvivalResourceManager::UpdateGlobalMissionState()
{
    // Update global mission state and check for new mission opportunities
    if (SurvivalSystem)
    {
        TArray<FQuest_SurvivalMission> ActiveMissions = SurvivalSystem->GetActiveMissions();
        
        // Check if we need to generate new missions
        if (ActiveMissions.Num() < 2)
        {
            GenerateEnvironmentalMissions();
        }
    }
}

void AQuest_SurvivalResourceManager::CheckPlayerSurvivalStatus()
{
    // Check player survival status and generate emergency missions if needed
    for (AActor* Player : RegisteredPlayers)
    {
        if (Player)
        {
            // Example: Check if player is in danger and needs emergency missions
            // This could be expanded with actual player health/status checks
        }
    }
}

void AQuest_SurvivalResourceManager::GenerateEnvironmentalMissions()
{
    // Generate missions based on environmental factors
    TArray<FVector> MissionLocations = {
        FVector(2000, 1000, 150),   // Stone quarry
        FVector(-1500, 2000, 200),  // Wood grove
        FVector(0, -2000, 100),     // Water source
        FVector(1000, -1000, 180),  // Shelter site
        FVector(-2000, 0, 220)      // Hunting ground
    };
    
    TArray<EQuest_ResourceType> ResourceTypes = {
        EQuest_ResourceType::Stone,
        EQuest_ResourceType::Wood,
        EQuest_ResourceType::Water,
        EQuest_ResourceType::Food
    };
    
    // Create random environmental missions
    if (SurvivalSystem && FMath::RandRange(0, 100) < 30) // 30% chance
    {
        int32 LocationIndex = FMath::RandRange(0, MissionLocations.Num() - 1);
        int32 ResourceIndex = FMath::RandRange(0, ResourceTypes.Num() - 1);
        
        SurvivalSystem->CreateResourceGatheringMission(
            ResourceTypes[ResourceIndex], 
            MissionLocations[LocationIndex]
        );
    }
}