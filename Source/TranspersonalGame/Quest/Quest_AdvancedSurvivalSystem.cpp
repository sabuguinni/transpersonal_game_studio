#include "Quest_AdvancedSurvivalSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

AQuest_AdvancedSurvivalSystem::AQuest_AdvancedSurvivalSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    SurvivalDifficulty = 1.0f;
    DaysSurvived = 0;
    bSeasonalChallengesEnabled = true;
    LastQuestGenerationTime = 0.0f;
    QuestGenerationInterval = 120.0f; // Generate new quest every 2 minutes
    
    // Initialize resource inventory
    PlayerInventory.Add(EQuest_ResourceType::Stone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Wood, 0);
    PlayerInventory.Add(EQuest_ResourceType::Fiber, 0);
    PlayerInventory.Add(EQuest_ResourceType::Food, 0);
    PlayerInventory.Add(EQuest_ResourceType::Water, 0);
    PlayerInventory.Add(EQuest_ResourceType::Hide, 0);
    PlayerInventory.Add(EQuest_ResourceType::Bone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Metal, 0);
}

void AQuest_AdvancedSurvivalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultObjectives();
    
    // Initialize tribal relationships
    FQuest_TribalRelationship RiverTribe;
    RiverTribe.TribeName = TEXT("River Dwellers");
    RiverTribe.RelationshipValue = 10.0f;
    TribalRelationships.Add(RiverTribe);
    
    FQuest_TribalRelationship MountainTribe;
    MountainTribe.TribeName = TEXT("Mountain Hunters");
    MountainTribe.RelationshipValue = -20.0f;
    MountainTribe.bIsHostile = true;
    TribalRelationships.Add(MountainTribe);
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced Survival System initialized with %d objectives"), ActiveObjectives.Num());
}

void AQuest_AdvancedSurvivalSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateObjectiveProgress(DeltaTime);
    
    // Auto-generate new quests periodically
    if (GetWorld()->GetTimeSeconds() - LastQuestGenerationTime > QuestGenerationInterval)
    {
        // Randomly select a challenge type
        int32 RandomType = FMath::RandRange(1, 5);
        EQuest_SurvivalChallengeType ChallengeType = static_cast<EQuest_SurvivalChallengeType>(RandomType);
        GenerateRandomSurvivalQuest(ChallengeType);
        LastQuestGenerationTime = GetWorld()->GetTimeSeconds();
    }
}

void AQuest_AdvancedSurvivalSystem::GenerateRandomSurvivalQuest(EQuest_SurvivalChallengeType ChallengeType)
{
    FQuest_SurvivalObjective NewObjective;
    
    switch (ChallengeType)
    {
        case EQuest_SurvivalChallengeType::HuntingChallenge:
            NewObjective = CreateHuntingQuest();
            break;
        case EQuest_SurvivalChallengeType::CraftingMastery:
            NewObjective = CreateCraftingQuest();
            break;
        case EQuest_SurvivalChallengeType::WaterGathering:
        case EQuest_SurvivalChallengeType::ResourceManagement:
            NewObjective = CreateGatheringQuest();
            break;
        default:
            NewObjective.ObjectiveName = TEXT("Survive the Day");
            NewObjective.ChallengeType = EQuest_SurvivalChallengeType::None;
            NewObjective.TimeLimit = 600.0f; // 10 minutes
            break;
    }
    
    ActiveObjectives.Add(NewObjective);
    NotifyPlayerOfQuestUpdate(FString::Printf(TEXT("New survival challenge: %s"), *NewObjective.ObjectiveName));
    
    UE_LOG(LogTemp, Warning, TEXT("Generated new survival quest: %s"), *NewObjective.ObjectiveName);
}

void AQuest_AdvancedSurvivalSystem::CompleteObjective(int32 ObjectiveIndex, int32 AmountCompleted)
{
    if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
    {
        FQuest_SurvivalObjective& Objective = ActiveObjectives[ObjectiveIndex];
        Objective.CurrentAmount += AmountCompleted;
        
        if (Objective.CurrentAmount >= Objective.TargetAmount)
        {
            Objective.bIsCompleted = true;
            NotifyPlayerOfQuestUpdate(FString::Printf(TEXT("Objective completed: %s"), *Objective.ObjectiveName));
            
            // Reward player based on objective type
            switch (Objective.ChallengeType)
            {
                case EQuest_SurvivalChallengeType::HuntingChallenge:
                    AddResourceToInventory(EQuest_ResourceType::Food, 5);
                    AddResourceToInventory(EQuest_ResourceType::Hide, 2);
                    break;
                case EQuest_SurvivalChallengeType::CraftingMastery:
                    AddResourceToInventory(EQuest_ResourceType::Stone, 3);
                    AddResourceToInventory(EQuest_ResourceType::Wood, 3);
                    break;
                case EQuest_SurvivalChallengeType::WaterGathering:
                    AddResourceToInventory(EQuest_ResourceType::Water, 10);
                    break;
                default:
                    break;
            }
        }
    }
}

bool AQuest_AdvancedSurvivalSystem::CheckObjectiveCompletion(int32 ObjectiveIndex)
{
    if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
    {
        return ActiveObjectives[ObjectiveIndex].bIsCompleted;
    }
    return false;
}

void AQuest_AdvancedSurvivalSystem::UpdateTribalRelationship(const FString& TribeName, float RelationshipChange)
{
    for (FQuest_TribalRelationship& Relationship : TribalRelationships)
    {
        if (Relationship.TribeName == TribeName)
        {
            Relationship.RelationshipValue = FMath::Clamp(
                Relationship.RelationshipValue + RelationshipChange, 
                -100.0f, 
                100.0f
            );
            
            // Update hostility/alliance status
            if (Relationship.RelationshipValue < -50.0f)
            {
                Relationship.bIsHostile = true;
                Relationship.bIsAllied = false;
            }
            else if (Relationship.RelationshipValue > 50.0f)
            {
                Relationship.bIsHostile = false;
                Relationship.bIsAllied = true;
            }
            else
            {
                Relationship.bIsHostile = false;
                Relationship.bIsAllied = false;
            }
            
            NotifyPlayerOfQuestUpdate(FString::Printf(
                TEXT("Relationship with %s changed by %.1f (Now: %.1f)"), 
                *TribeName, 
                RelationshipChange, 
                Relationship.RelationshipValue
            ));
            break;
        }
    }
}

void AQuest_AdvancedSurvivalSystem::AddResourceToInventory(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (PlayerInventory.Contains(ResourceType))
    {
        PlayerInventory[ResourceType] += Amount;
    }
    else
    {
        PlayerInventory.Add(ResourceType, Amount);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Added %d of resource type %d to inventory"), Amount, static_cast<int32>(ResourceType));
}

int32 AQuest_AdvancedSurvivalSystem::GetResourceAmount(EQuest_ResourceType ResourceType)
{
    if (PlayerInventory.Contains(ResourceType))
    {
        return PlayerInventory[ResourceType];
    }
    return 0;
}

bool AQuest_AdvancedSurvivalSystem::HasSufficientResources(EQuest_ResourceType ResourceType, int32 RequiredAmount)
{
    return GetResourceAmount(ResourceType) >= RequiredAmount;
}

void AQuest_AdvancedSurvivalSystem::TriggerSeasonalEvent()
{
    if (!bSeasonalChallengesEnabled) return;
    
    // Generate seasonal survival challenges
    TArray<FString> SeasonalEvents = {
        TEXT("Harsh Winter - Find Shelter"),
        TEXT("Drought Season - Secure Water"),
        TEXT("Migration Time - Follow Herds"),
        TEXT("Storm Season - Gather Resources")
    };
    
    FString SelectedEvent = SeasonalEvents[FMath::RandRange(0, SeasonalEvents.Num() - 1)];
    
    FQuest_SurvivalObjective SeasonalObjective;
    SeasonalObjective.ObjectiveName = SelectedEvent;
    SeasonalObjective.ChallengeType = EQuest_SurvivalChallengeType::SeasonalMigration;
    SeasonalObjective.TimeLimit = 900.0f; // 15 minutes for seasonal events
    SeasonalObjective.TargetAmount = 1;
    
    ActiveObjectives.Add(SeasonalObjective);
    NotifyPlayerOfQuestUpdate(FString::Printf(TEXT("Seasonal Challenge: %s"), *SelectedEvent));
}

void AQuest_AdvancedSurvivalSystem::HandleWeatherChange(const FString& WeatherType)
{
    if (WeatherType == TEXT("Storm"))
    {
        // Increase difficulty during storms
        SurvivalDifficulty *= 1.5f;
        NotifyPlayerOfQuestUpdate(TEXT("Storm incoming! Survival difficulty increased."));
    }
    else if (WeatherType == TEXT("Clear"))
    {
        // Reset difficulty
        SurvivalDifficulty = FMath::Max(1.0f, SurvivalDifficulty * 0.8f);
        NotifyPlayerOfQuestUpdate(TEXT("Weather cleared. Survival conditions improved."));
    }
}

void AQuest_AdvancedSurvivalSystem::SpawnPredatorThreat(const FVector& ThreatLocation)
{
    // Create emergency defense objective
    FQuest_SurvivalObjective ThreatObjective;
    ThreatObjective.ObjectiveName = TEXT("Defend Against Predator");
    ThreatObjective.ChallengeType = EQuest_SurvivalChallengeType::TerritoryDefense;
    ThreatObjective.TargetLocation = ThreatLocation;
    ThreatObjective.TimeLimit = 180.0f; // 3 minutes to deal with threat
    ThreatObjective.TargetAmount = 1;
    
    ActiveObjectives.Add(ThreatObjective);
    NotifyPlayerOfQuestUpdate(TEXT("DANGER: Predator detected nearby! Defend yourself!"));
    
    UE_LOG(LogTemp, Warning, TEXT("Predator threat spawned at location: %s"), *ThreatLocation.ToString());
}

void AQuest_AdvancedSurvivalSystem::InitializeDefaultObjectives()
{
    // Create initial survival objectives
    FQuest_SurvivalObjective WaterObjective;
    WaterObjective.ObjectiveName = TEXT("Find Fresh Water");
    WaterObjective.ChallengeType = EQuest_SurvivalChallengeType::WaterGathering;
    WaterObjective.RequiredResource = EQuest_ResourceType::Water;
    WaterObjective.TargetAmount = 5;
    WaterObjective.TimeLimit = 600.0f;
    ActiveObjectives.Add(WaterObjective);
    
    FQuest_SurvivalObjective ShelterObjective;
    ShelterObjective.ObjectiveName = TEXT("Build Basic Shelter");
    ShelterObjective.ChallengeType = EQuest_SurvivalChallengeType::ShelterBuilding;
    ShelterObjective.RequiredResource = EQuest_ResourceType::Wood;
    ShelterObjective.TargetAmount = 10;
    ShelterObjective.TimeLimit = 900.0f;
    ActiveObjectives.Add(ShelterObjective);
}

void AQuest_AdvancedSurvivalSystem::UpdateObjectiveProgress(float DeltaTime)
{
    for (FQuest_SurvivalObjective& Objective : ActiveObjectives)
    {
        if (!Objective.bIsCompleted && Objective.TimeLimit > 0.0f)
        {
            Objective.TimeLimit -= DeltaTime;
            
            if (Objective.TimeLimit <= 0.0f)
            {
                NotifyPlayerOfQuestUpdate(FString::Printf(TEXT("Objective failed: %s (Time expired)"), *Objective.ObjectiveName));
                // Apply survival penalty for failed objectives
                SurvivalDifficulty *= 1.1f;
            }
        }
    }
    
    // Remove completed or expired objectives
    ActiveObjectives.RemoveAll([](const FQuest_SurvivalObjective& Obj) {
        return Obj.bIsCompleted || Obj.TimeLimit <= 0.0f;
    });
}

FQuest_SurvivalObjective AQuest_AdvancedSurvivalSystem::CreateHuntingQuest()
{
    FQuest_SurvivalObjective HuntingQuest;
    HuntingQuest.ObjectiveName = TEXT("Hunt for Food");
    HuntingQuest.ChallengeType = EQuest_SurvivalChallengeType::HuntingChallenge;
    HuntingQuest.RequiredResource = EQuest_ResourceType::Food;
    HuntingQuest.TargetAmount = FMath::RandRange(2, 5);
    HuntingQuest.TimeLimit = 480.0f; // 8 minutes
    return HuntingQuest;
}

FQuest_SurvivalObjective AQuest_AdvancedSurvivalSystem::CreateCraftingQuest()
{
    FQuest_SurvivalObjective CraftingQuest;
    CraftingQuest.ObjectiveName = TEXT("Craft Essential Tools");
    CraftingQuest.ChallengeType = EQuest_SurvivalChallengeType::CraftingMastery;
    CraftingQuest.RequiredResource = EQuest_ResourceType::Stone;
    CraftingQuest.TargetAmount = FMath::RandRange(3, 8);
    CraftingQuest.TimeLimit = 360.0f; // 6 minutes
    return CraftingQuest;
}

FQuest_SurvivalObjective AQuest_AdvancedSurvivalSystem::CreateGatheringQuest()
{
    FQuest_SurvivalObjective GatheringQuest;
    GatheringQuest.ObjectiveName = TEXT("Gather Natural Resources");
    GatheringQuest.ChallengeType = EQuest_SurvivalChallengeType::ResourceManagement;
    
    // Randomly select resource type
    TArray<EQuest_ResourceType> ResourceTypes = {
        EQuest_ResourceType::Wood,
        EQuest_ResourceType::Fiber,
        EQuest_ResourceType::Stone
    };
    
    GatheringQuest.RequiredResource = ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
    GatheringQuest.TargetAmount = FMath::RandRange(5, 12);
    GatheringQuest.TimeLimit = 300.0f; // 5 minutes
    return GatheringQuest;
}

void AQuest_AdvancedSurvivalSystem::NotifyPlayerOfQuestUpdate(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest Update: %s"), *Message);
    
    // In a full implementation, this would trigger UI notifications
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }
}