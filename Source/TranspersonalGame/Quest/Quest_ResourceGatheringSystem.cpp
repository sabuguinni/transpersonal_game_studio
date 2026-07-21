#include "Quest_ResourceGatheringSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UQuest_ResourceGatheringSystem::UQuest_ResourceGatheringSystem()
{
    
}

void UQuest_ResourceGatheringSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializePresetMissions();
    
    // Initialize player inventory
    PlayerInventory.Add(EQuest_ResourceType::Stone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Wood, 0);
    PlayerInventory.Add(EQuest_ResourceType::Plant, 0);
    PlayerInventory.Add(EQuest_ResourceType::Bone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Hide, 0);
    PlayerInventory.Add(EQuest_ResourceType::Water, 0);
    PlayerInventory.Add(EQuest_ResourceType::Fire, 0);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Resource Gathering System Initialized"));
}

void UQuest_ResourceGatheringSystem::StartGatheringMission(const FString& MissionName)
{
    for (FQuest_GatheringMission& Mission : AvailableMissions)
    {
        if (Mission.MissionName == MissionName && !Mission.bIsActive)
        {
            Mission.bIsActive = true;
            ActiveMissions.Add(Mission);
            
            UE_LOG(LogTemp, Warning, TEXT("Started gathering mission: %s"), *MissionName);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    FString::Printf(TEXT("Mission Started: %s"), *MissionName));
            }
            break;
        }
    }
}

void UQuest_ResourceGatheringSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (PlayerInventory.Contains(ResourceType))
    {
        PlayerInventory[ResourceType] += Amount;
        
        UE_LOG(LogTemp, Warning, TEXT("Added %d resources of type %d"), Amount, (int32)ResourceType);
        
        // Check if any active missions are completed
        for (const FQuest_GatheringMission& Mission : ActiveMissions)
        {
            if (CheckMissionCompletion(Mission.MissionName))
            {
                CompleteMission(Mission.MissionName);
            }
        }
    }
}

bool UQuest_ResourceGatheringSystem::CheckMissionCompletion(const FString& MissionName)
{
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && Mission.bIsActive)
        {
            bool bAllResourcesGathered = true;
            
            for (const FQuest_ResourceRequirement& Requirement : Mission.RequiredResources)
            {
                int32 CurrentAmount = PlayerInventory.FindRef(Requirement.ResourceType);
                if (CurrentAmount < Requirement.RequiredAmount)
                {
                    bAllResourcesGathered = false;
                    break;
                }
            }
            
            return bAllResourcesGathered;
        }
    }
    
    return false;
}

TArray<FQuest_GatheringMission> UQuest_ResourceGatheringSystem::GetActiveMissions() const
{
    return ActiveMissions;
}

void UQuest_ResourceGatheringSystem::CreatePresetMissions()
{
    InitializePresetMissions();
}

void UQuest_ResourceGatheringSystem::InitializePresetMissions()
{
    AvailableMissions.Empty();
    
    // Mission 1: Basic Tool Crafting
    FQuest_GatheringMission ToolMission;
    ToolMission.MissionName = TEXT("Craft Basic Tools");
    ToolMission.Description = TEXT("Gather materials to craft essential survival tools");
    ToolMission.ExperienceReward = 150;
    ToolMission.TimeLimit = 600.0f;
    
    FQuest_ResourceRequirement StoneReq;
    StoneReq.ResourceType = EQuest_ResourceType::Stone;
    StoneReq.RequiredAmount = 3;
    StoneReq.ResourceName = TEXT("Sharp Stones");
    
    FQuest_ResourceRequirement WoodReq;
    WoodReq.ResourceType = EQuest_ResourceType::Wood;
    WoodReq.RequiredAmount = 2;
    WoodReq.ResourceName = TEXT("Sturdy Branches");
    
    ToolMission.RequiredResources.Add(StoneReq);
    ToolMission.RequiredResources.Add(WoodReq);
    AvailableMissions.Add(ToolMission);
    
    // Mission 2: Shelter Building
    FQuest_GatheringMission ShelterMission;
    ShelterMission.MissionName = TEXT("Build Emergency Shelter");
    ShelterMission.Description = TEXT("Collect materials to build a basic shelter for protection");
    ShelterMission.ExperienceReward = 200;
    ShelterMission.TimeLimit = 900.0f;
    
    FQuest_ResourceRequirement WoodReq2;
    WoodReq2.ResourceType = EQuest_ResourceType::Wood;
    WoodReq2.RequiredAmount = 5;
    WoodReq2.ResourceName = TEXT("Building Logs");
    
    FQuest_ResourceRequirement PlantReq;
    PlantReq.ResourceType = EQuest_ResourceType::Plant;
    PlantReq.RequiredAmount = 4;
    PlantReq.ResourceName = TEXT("Large Leaves");
    
    ShelterMission.RequiredResources.Add(WoodReq2);
    ShelterMission.RequiredResources.Add(PlantReq);
    AvailableMissions.Add(ShelterMission);
    
    // Mission 3: First Hunt Preparation
    FQuest_GatheringMission HuntPrepMission;
    HuntPrepMission.MissionName = TEXT("Prepare for First Hunt");
    HuntPrepMission.Description = TEXT("Gather materials to craft hunting weapons and tools");
    HuntPrepMission.ExperienceReward = 300;
    HuntPrepMission.TimeLimit = 1200.0f;
    
    FQuest_ResourceRequirement StoneReq2;
    StoneReq2.ResourceType = EQuest_ResourceType::Stone;
    StoneReq2.RequiredAmount = 4;
    StoneReq2.ResourceName = TEXT("Flint Stones");
    
    FQuest_ResourceRequirement WoodReq3;
    WoodReq3.ResourceType = EQuest_ResourceType::Wood;
    WoodReq3.RequiredAmount = 3;
    WoodReq3.ResourceName = TEXT("Spear Shafts");
    
    FQuest_ResourceRequirement PlantReq2;
    PlantReq2.ResourceType = EQuest_ResourceType::Plant;
    PlantReq2.RequiredAmount = 2;
    PlantReq2.ResourceName = TEXT("Binding Vines");
    
    HuntPrepMission.RequiredResources.Add(StoneReq2);
    HuntPrepMission.RequiredResources.Add(WoodReq3);
    HuntPrepMission.RequiredResources.Add(PlantReq2);
    AvailableMissions.Add(HuntPrepMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d preset gathering missions"), AvailableMissions.Num());
}

void UQuest_ResourceGatheringSystem::CompleteMission(const FString& MissionName)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (ActiveMissions[i].MissionName == MissionName)
        {
            FQuest_GatheringMission CompletedMission = ActiveMissions[i];
            ActiveMissions.RemoveAt(i);
            
            // Consume resources
            for (const FQuest_ResourceRequirement& Requirement : CompletedMission.RequiredResources)
            {
                PlayerInventory[Requirement.ResourceType] -= Requirement.RequiredAmount;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Mission Completed: %s - Reward: %d XP"), 
                *MissionName, CompletedMission.ExperienceReward);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Gold, 
                    FString::Printf(TEXT("MISSION COMPLETE: %s (+%d XP)"), 
                    *MissionName, CompletedMission.ExperienceReward));
            }
            break;
        }
    }
}

// Resource Pickup Component Implementation
UQuest_ResourcePickupComponent::UQuest_ResourcePickupComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    ResourceType = EQuest_ResourceType::Stone;
    ResourceAmount = 1;
    bCanBePickedUp = true;
}

void UQuest_ResourcePickupComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UQuest_ResourcePickupComponent::PickupResource(AActor* Player)
{
    if (!bCanBePickedUp || !Player)
    {
        return;
    }
    
    UQuest_ResourceGatheringSystem* GatheringSystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuest_ResourceGatheringSystem>();
    if (GatheringSystem)
    {
        GatheringSystem->AddResource(ResourceType, ResourceAmount);
        
        if (GEngine)
        {
            FString ResourceName;
            switch (ResourceType)
            {
                case EQuest_ResourceType::Stone: ResourceName = TEXT("Stone"); break;
                case EQuest_ResourceType::Wood: ResourceName = TEXT("Wood"); break;
                case EQuest_ResourceType::Plant: ResourceName = TEXT("Plant"); break;
                case EQuest_ResourceType::Bone: ResourceName = TEXT("Bone"); break;
                case EQuest_ResourceType::Hide: ResourceName = TEXT("Hide"); break;
                case EQuest_ResourceType::Water: ResourceName = TEXT("Water"); break;
                case EQuest_ResourceType::Fire: ResourceName = TEXT("Fire"); break;
                default: ResourceName = TEXT("Unknown"); break;
            }
            
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                FString::Printf(TEXT("Picked up %d %s"), ResourceAmount, *ResourceName));
        }
        
        // Destroy the pickup actor
        GetOwner()->Destroy();
    }
}