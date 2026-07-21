#include "Quest_SettlementManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AQuest_SettlementManager::AQuest_SettlementManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    SettlementName = TEXT("New Settlement");
    SettlementStatus = EQuest_SettlementStatus::Establishing;
    Population = 0;
    SettlementMorale = 50.0f;
    DefenseRating = 25.0f;
    SettlementCenter = FVector::ZeroVector;
    SettlementRadius = 1000.0f;
    
    LastUpdateTime = 0.0f;
    ResourceUpdateInterval = 5.0f; // Update resources every 5 seconds
    
    // Initialize default resources
    FQuest_SettlementResource StoneResource;
    StoneResource.ResourceType = EQuest_SettlementResource::Stone;
    StoneResource.CurrentAmount = 50;
    StoneResource.MaxCapacity = 200;
    StoneResource.ConsumptionRate = 0.5f;
    SettlementResources.Add(StoneResource);
    
    FQuest_SettlementResource WoodResource;
    WoodResource.ResourceType = EQuest_SettlementResource::Wood;
    WoodResource.CurrentAmount = 30;
    WoodResource.MaxCapacity = 150;
    WoodResource.ConsumptionRate = 1.0f;
    SettlementResources.Add(WoodResource);
    
    FQuest_SettlementResource FoodResource;
    FoodResource.ResourceType = EQuest_SettlementResource::Food;
    FoodResource.CurrentAmount = 40;
    FoodResource.MaxCapacity = 100;
    FoodResource.ConsumptionRate = 2.0f;
    SettlementResources.Add(FoodResource);
}

void AQuest_SettlementManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize settlement if not already done
    if (SettlementCenter == FVector::ZeroVector)
    {
        SettlementCenter = GetActorLocation();
    }
    
    // Set up default tribal leader
    if (TribalLeader.NPCName.IsEmpty())
    {
        TribalLeader.NPCName = TEXT("Chief Karak");
        TribalLeader.Role = EQuest_SettlementRole::Leader;
        TribalLeader.Position = SettlementCenter + FVector(100, 0, 0);
        TribalLeader.TrustLevel = 75.0f;
        TribalLeader.bIsAvailable = true;
        TribalLeader.AvailableQuests.Add(TEXT("WelcomeToSettlement"));
        TribalLeader.AvailableQuests.Add(TEXT("DefendTheSettlement"));
    }
    
    // Add default NPCs
    if (SettlementNPCs.Num() == 0)
    {
        FQuest_SettlementNPC CraftMaster;
        CraftMaster.NPCName = TEXT("Master Thane");
        CraftMaster.Role = EQuest_SettlementRole::CraftMaster;
        CraftMaster.Position = SettlementCenter + FVector(200, -200, 0);
        CraftMaster.TrustLevel = 60.0f;
        CraftMaster.AvailableQuests.Add(TEXT("CraftStoneAxe"));
        CraftMaster.AvailableQuests.Add(TEXT("GatherCraftingMaterials"));
        SettlementNPCs.Add(CraftMaster);
        
        FQuest_SettlementNPC Scout;
        Scout.NPCName = TEXT("Scout Vera");
        Scout.Role = EQuest_SettlementRole::Scout;
        Scout.Position = SettlementCenter + FVector(150, 100, 0);
        Scout.TrustLevel = 55.0f;
        Scout.AvailableQuests.Add(TEXT("ExploreNearbyArea"));
        Scout.AvailableQuests.Add(TEXT("FindNewResources"));
        SettlementNPCs.Add(Scout);
        
        FQuest_SettlementNPC Guard;
        Guard.NPCName = TEXT("Guard Borin");
        Guard.Role = EQuest_SettlementRole::Guard;
        Guard.Position = SettlementCenter + FVector(-150, -150, 0);
        Guard.TrustLevel = 70.0f;
        Guard.AvailableQuests.Add(TEXT("PatrolPerimeter"));
        Guard.AvailableQuests.Add(TEXT("TrainWithWeapons"));
        SettlementNPCs.Add(Guard);
        
        FQuest_SettlementNPC Elder;
        Elder.NPCName = TEXT("Elder Mira");
        Elder.Role = EQuest_SettlementRole::Elder;
        Elder.Position = SettlementCenter + FVector(-50, 150, 0);
        Elder.TrustLevel = 80.0f;
        Elder.AvailableQuests.Add(TEXT("LearnTribalWisdom"));
        Elder.AvailableQuests.Add(TEXT("HealTheWounded"));
        SettlementNPCs.Add(Elder);
    }
    
    Population = SettlementNPCs.Num() + 1; // +1 for tribal leader
    UpdateSettlementStatus();
}

void AQuest_SettlementManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= ResourceUpdateInterval)
    {
        ProcessResourceConsumption(LastUpdateTime);
        UpdateSettlementStatus();
        ProcessNPCInteractions();
        LastUpdateTime = 0.0f;
    }
}

void AQuest_SettlementManager::InitializeSettlement(const FString& Name, const FVector& Center)
{
    SettlementName = Name;
    SettlementCenter = Center;
    SetActorLocation(Center);
    
    UE_LOG(LogTemp, Warning, TEXT("Settlement '%s' initialized at location: %s"), 
           *SettlementName, *Center.ToString());
}

void AQuest_SettlementManager::AddNPCToSettlement(const FQuest_SettlementNPC& NewNPC)
{
    SettlementNPCs.Add(NewNPC);
    Population = SettlementNPCs.Num() + 1; // +1 for tribal leader
    
    UpdateMorale();
    OnNewNPCArrived(NewNPC);
    
    UE_LOG(LogTemp, Warning, TEXT("NPC '%s' added to settlement. New population: %d"), 
           *NewNPC.NPCName, Population);
}

void AQuest_SettlementManager::RemoveNPCFromSettlement(const FString& NPCName)
{
    for (int32 i = SettlementNPCs.Num() - 1; i >= 0; i--)
    {
        if (SettlementNPCs[i].NPCName == NPCName)
        {
            SettlementNPCs.RemoveAt(i);
            Population = SettlementNPCs.Num() + 1;
            UpdateMorale();
            break;
        }
    }
}

void AQuest_SettlementManager::UpdateSettlementStatus()
{
    EQuest_SettlementStatus OldStatus = SettlementStatus;
    
    // Determine status based on population, morale, and resources
    if (Population < 3)
    {
        SettlementStatus = EQuest_SettlementStatus::Establishing;
    }
    else if (Population >= 3 && Population < 8 && SettlementMorale > 40.0f)
    {
        SettlementStatus = EQuest_SettlementStatus::Growing;
    }
    else if (Population >= 8 && SettlementMorale > 60.0f)
    {
        SettlementStatus = EQuest_SettlementStatus::Thriving;
    }
    else if (SettlementMorale < 30.0f)
    {
        SettlementStatus = EQuest_SettlementStatus::Declining;
    }
    else if (Population == 0)
    {
        SettlementStatus = EQuest_SettlementStatus::Abandoned;
    }
    
    if (OldStatus != SettlementStatus)
    {
        OnSettlementStatusChanged(SettlementStatus);
        UE_LOG(LogTemp, Warning, TEXT("Settlement status changed to: %d"), (int32)SettlementStatus);
    }
    
    UpdateMorale();
    UpdateDefenseRating();
}

void AQuest_SettlementManager::ProcessResourceConsumption(float DeltaTime)
{
    for (FQuest_SettlementResource& Resource : SettlementResources)
    {
        int32 Consumption = FMath::RoundToInt(Resource.ConsumptionRate * Population * (DeltaTime / ResourceUpdateInterval));
        Resource.CurrentAmount = FMath::Max(0, Resource.CurrentAmount - Consumption);
        
        if (Resource.CurrentAmount == 0)
        {
            OnResourceDepleted(Resource.ResourceType);
            SettlementMorale = FMath::Max(0.0f, SettlementMorale - 10.0f);
        }
    }
}

bool AQuest_SettlementManager::AddResource(EQuest_SettlementResource ResourceType, int32 Amount)
{
    for (FQuest_SettlementResource& Resource : SettlementResources)
    {
        if (Resource.ResourceType == ResourceType)
        {
            int32 SpaceAvailable = Resource.MaxCapacity - Resource.CurrentAmount;
            int32 AmountToAdd = FMath::Min(Amount, SpaceAvailable);
            Resource.CurrentAmount += AmountToAdd;
            
            if (AmountToAdd > 0)
            {
                SettlementMorale = FMath::Min(100.0f, SettlementMorale + 2.0f);
            }
            
            return AmountToAdd == Amount;
        }
    }
    return false;
}

bool AQuest_SettlementManager::ConsumeResource(EQuest_SettlementResource ResourceType, int32 Amount)
{
    for (FQuest_SettlementResource& Resource : SettlementResources)
    {
        if (Resource.ResourceType == ResourceType)
        {
            if (Resource.CurrentAmount >= Amount)
            {
                Resource.CurrentAmount -= Amount;
                return true;
            }
            return false;
        }
    }
    return false;
}

int32 AQuest_SettlementManager::GetResourceAmount(EQuest_SettlementResource ResourceType)
{
    for (const FQuest_SettlementResource& Resource : SettlementResources)
    {
        if (Resource.ResourceType == ResourceType)
        {
            return Resource.CurrentAmount;
        }
    }
    return 0;
}

void AQuest_SettlementManager::AddBuilding(const FQuest_SettlementBuilding& NewBuilding)
{
    SettlementBuildings.Add(NewBuilding);
    UE_LOG(LogTemp, Warning, TEXT("Building '%s' added to settlement"), *NewBuilding.BuildingName);
}

void AQuest_SettlementManager::UpdateBuildingConstruction(const FString& BuildingName, float Progress)
{
    for (FQuest_SettlementBuilding& Building : SettlementBuildings)
    {
        if (Building.BuildingName == BuildingName)
        {
            Building.ConstructionProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            if (Building.ConstructionProgress >= 100.0f && !Building.bIsConstructed)
            {
                Building.bIsConstructed = true;
                OnBuildingCompleted(BuildingName);
                SettlementMorale = FMath::Min(100.0f, SettlementMorale + 5.0f);
            }
            break;
        }
    }
}

bool AQuest_SettlementManager::IsBuildingComplete(const FString& BuildingName)
{
    for (const FQuest_SettlementBuilding& Building : SettlementBuildings)
    {
        if (Building.BuildingName == BuildingName)
        {
            return Building.bIsConstructed;
        }
    }
    return false;
}

void AQuest_SettlementManager::AssignTribalLeader(const FQuest_SettlementNPC& Leader)
{
    TribalLeader = Leader;
    TribalLeader.Role = EQuest_SettlementRole::Leader;
    UpdateMorale();
}

void AQuest_SettlementManager::ProcessLeadershipDecisions()
{
    // Tribal leader makes decisions based on settlement needs
    if (TribalLeader.TrustLevel > 60.0f)
    {
        // Good leadership improves morale
        SettlementMorale = FMath::Min(100.0f, SettlementMorale + 1.0f);
    }
    else if (TribalLeader.TrustLevel < 40.0f)
    {
        // Poor leadership decreases morale
        SettlementMorale = FMath::Max(0.0f, SettlementMorale - 1.0f);
    }
}

void AQuest_SettlementManager::HandleSettlementCrisis(const FString& CrisisType)
{
    UE_LOG(LogTemp, Warning, TEXT("Settlement crisis: %s"), *CrisisType);
    
    if (CrisisType == TEXT("DinosaurAttack"))
    {
        SettlementMorale = FMath::Max(0.0f, SettlementMorale - 20.0f);
        // Consume resources for defense
        ConsumeResource(EQuest_SettlementResource::Wood, 10);
        ConsumeResource(EQuest_SettlementResource::Stone, 5);
    }
    else if (CrisisType == TEXT("ResourceShortage"))
    {
        SettlementMorale = FMath::Max(0.0f, SettlementMorale - 15.0f);
    }
    else if (CrisisType == TEXT("Disease"))
    {
        SettlementMorale = FMath::Max(0.0f, SettlementMorale - 25.0f);
        ConsumeResource(EQuest_SettlementResource::Medicine, 15);
    }
}

void AQuest_SettlementManager::StartSettlementQuest(const FString& QuestID)
{
    if (!ActiveSettlementQuests.Contains(QuestID))
    {
        ActiveSettlementQuests.Add(QuestID);
        UE_LOG(LogTemp, Warning, TEXT("Started settlement quest: %s"), *QuestID);
    }
}

void AQuest_SettlementManager::CompleteSettlementQuest(const FString& QuestID)
{
    if (ActiveSettlementQuests.Contains(QuestID))
    {
        ActiveSettlementQuests.Remove(QuestID);
        CompletedSettlementQuests.Add(QuestID);
        SettlementMorale = FMath::Min(100.0f, SettlementMorale + 5.0f);
        
        // Increase trust with relevant NPCs
        for (FQuest_SettlementNPC& NPC : SettlementNPCs)
        {
            if (NPC.AvailableQuests.Contains(QuestID))
            {
                NPC.TrustLevel = FMath::Min(100.0f, NPC.TrustLevel + 10.0f);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Completed settlement quest: %s"), *QuestID);
    }
}

TArray<FString> AQuest_SettlementManager::GetAvailableSettlementQuests()
{
    TArray<FString> AvailableQuests;
    
    // Add leader quests
    for (const FString& Quest : TribalLeader.AvailableQuests)
    {
        if (!ActiveSettlementQuests.Contains(Quest) && !CompletedSettlementQuests.Contains(Quest))
        {
            AvailableQuests.Add(Quest);
        }
    }
    
    // Add NPC quests
    for (const FQuest_SettlementNPC& NPC : SettlementNPCs)
    {
        if (NPC.bIsAvailable)
        {
            for (const FString& Quest : NPC.AvailableQuests)
            {
                if (!ActiveSettlementQuests.Contains(Quest) && !CompletedSettlementQuests.Contains(Quest))
                {
                    AvailableQuests.AddUnique(Quest);
                }
            }
        }
    }
    
    return AvailableQuests;
}

bool AQuest_SettlementManager::IsPlayerWelcomeInSettlement()
{
    // Player is welcome if tribal leader trusts them and settlement morale is decent
    return TribalLeader.TrustLevel > 30.0f && SettlementMorale > 25.0f;
}

void AQuest_SettlementManager::UpdateMorale()
{
    float BaseMorale = 50.0f;
    
    // Population affects morale
    if (Population > 5)
    {
        BaseMorale += 10.0f;
    }
    else if (Population < 3)
    {
        BaseMorale -= 15.0f;
    }
    
    // Resource availability affects morale
    int32 TotalResources = 0;
    int32 MaxResources = 0;
    for (const FQuest_SettlementResource& Resource : SettlementResources)
    {
        TotalResources += Resource.CurrentAmount;
        MaxResources += Resource.MaxCapacity;
    }
    
    float ResourceRatio = MaxResources > 0 ? (float)TotalResources / MaxResources : 0.0f;
    BaseMorale += (ResourceRatio - 0.5f) * 20.0f;
    
    // Leadership affects morale
    BaseMorale += (TribalLeader.TrustLevel - 50.0f) * 0.3f;
    
    SettlementMorale = FMath::Clamp(BaseMorale, 0.0f, 100.0f);
}

void AQuest_SettlementManager::UpdateDefenseRating()
{
    float BaseDefense = 25.0f;
    
    // Count guards
    int32 GuardCount = 0;
    for (const FQuest_SettlementNPC& NPC : SettlementNPCs)
    {
        if (NPC.Role == EQuest_SettlementRole::Guard)
        {
            GuardCount++;
        }
    }
    
    BaseDefense += GuardCount * 15.0f;
    
    // Count defensive buildings
    int32 DefensiveBuildings = 0;
    for (const FQuest_SettlementBuilding& Building : SettlementBuildings)
    {
        if (Building.bIsConstructed && 
            (Building.BuildingName.Contains(TEXT("Tower")) || Building.BuildingName.Contains(TEXT("Wall"))))
        {
            DefensiveBuildings++;
        }
    }
    
    BaseDefense += DefensiveBuildings * 10.0f;
    
    DefenseRating = FMath::Clamp(BaseDefense, 0.0f, 100.0f);
}

void AQuest_SettlementManager::CheckSettlementNeeds()
{
    // Check for critical resource shortages
    for (const FQuest_SettlementResource& Resource : SettlementResources)
    {
        if (Resource.CurrentAmount < Resource.MaxCapacity * 0.2f)
        {
            // Generate urgent quest for this resource
            FString UrgentQuest = FString::Printf(TEXT("Urgent_%s_Gathering"), 
                                                 *UEnum::GetValueAsString(Resource.ResourceType));
            if (!ActiveSettlementQuests.Contains(UrgentQuest))
            {
                StartSettlementQuest(UrgentQuest);
            }
        }
    }
}

void AQuest_SettlementManager::ProcessNPCInteractions()
{
    // Update NPC availability and trust levels based on settlement conditions
    for (FQuest_SettlementNPC& NPC : SettlementNPCs)
    {
        // NPCs become unavailable if morale is too low
        if (SettlementMorale < 20.0f)
        {
            NPC.bIsAvailable = false;
        }
        else
        {
            NPC.bIsAvailable = true;
        }
        
        // Trust slowly increases over time if settlement is doing well
        if (SettlementMorale > 60.0f)
        {
            NPC.TrustLevel = FMath::Min(100.0f, NPC.TrustLevel + 0.5f);
        }
        else if (SettlementMorale < 30.0f)
        {
            NPC.TrustLevel = FMath::Max(0.0f, NPC.TrustLevel - 0.5f);
        }
    }
}