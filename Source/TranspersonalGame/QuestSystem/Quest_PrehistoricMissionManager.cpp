#include "Quest_PrehistoricMissionManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UQuest_PrehistoricMissionManager::UQuest_PrehistoricMissionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    MaxActiveMissions = 5;
    bAutoGenerateMissions = true;
    MissionGenerationInterval = 300.0f; // 5 minutes
    LastMissionGenerationTime = 0.0f;

    // Initialize mission templates
    InitializeHuntMissionTemplates();
    InitializeExplorationMissionTemplates();
    InitializeCraftingMissionTemplates();
}

void UQuest_PrehistoricMissionManager::BeginPlay()
{
    Super::BeginPlay();

    // Generate initial missions
    if (bAutoGenerateMissions)
    {
        AutoGenerateMissions();
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Initialized with %d mission templates"), 
        HuntMissionTemplates.Num() + ExplorationMissionTemplates.Num() + CraftingMissionTemplates.Num());
}

void UQuest_PrehistoricMissionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update mission timers
    UpdateMissionTimers(DeltaTime);

    // Check location-based objectives
    CheckLocationBasedObjectives();

    // Auto-generate missions if enabled
    if (bAutoGenerateMissions)
    {
        LastMissionGenerationTime += DeltaTime;
        if (LastMissionGenerationTime >= MissionGenerationInterval)
        {
            AutoGenerateMissions();
            LastMissionGenerationTime = 0.0f;
        }
    }
}

void UQuest_PrehistoricMissionManager::CreateMission(const FQuest_PrehistoricMission& MissionData)
{
    FQuest_PrehistoricMission NewMission = MissionData;
    
    // Generate unique ID if not provided
    if (NewMission.MissionID.IsEmpty())
    {
        NewMission.MissionID = GenerateUniqueMissionID();
    }

    // Set initial status
    NewMission.Status = EQuest_MissionStatus::NotStarted;
    NewMission.ElapsedTime = 0.0f;

    // Initialize objectives
    for (FQuest_MissionObjective& Objective : NewMission.Objectives)
    {
        Objective.bIsCompleted = false;
        Objective.CurrentCount = 0;
    }

    AllMissions.Add(NewMission);

    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Created mission '%s' - %s"), 
        *NewMission.MissionID, *NewMission.MissionTitle);
}

bool UQuest_PrehistoricMissionManager::StartMission(const FString& MissionID)
{
    // Check if we can start more missions
    if (ActiveMissionIDs.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Cannot start mission - max active missions reached"));
        return false;
    }

    // Find the mission
    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::NotStarted)
        {
            // Check prerequisites
            if (!CheckMissionPrerequisites(Mission))
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Mission prerequisites not met for '%s'"), *MissionID);
                return false;
            }

            // Start the mission
            Mission.Status = EQuest_MissionStatus::Active;
            Mission.ElapsedTime = 0.0f;
            ActiveMissionIDs.Add(MissionID);

            // Trigger event
            OnMissionStarted(Mission);

            UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Started mission '%s' - %s"), 
                *Mission.MissionID, *Mission.MissionTitle);
            return true;
        }
    }

    return false;
}

bool UQuest_PrehistoricMissionManager::CompleteMission(const FString& MissionID)
{
    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::Active)
        {
            // Check if all required objectives are completed
            bool bAllRequiredCompleted = true;
            for (const FQuest_MissionObjective& Objective : Mission.Objectives)
            {
                if (!Objective.bIsOptional && !Objective.bIsCompleted)
                {
                    bAllRequiredCompleted = false;
                    break;
                }
            }

            if (!bAllRequiredCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Cannot complete mission - objectives not finished"));
                return false;
            }

            // Complete the mission
            Mission.Status = EQuest_MissionStatus::Completed;
            ActiveMissionIDs.Remove(MissionID);
            CompletedMissionIDs.Add(MissionID);

            // Process rewards
            ProcessMissionRewards(Mission);

            // Trigger event
            OnMissionCompleted(Mission);

            UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Completed mission '%s' - %s"), 
                *Mission.MissionID, *Mission.MissionTitle);
            return true;
        }
    }

    return false;
}

bool UQuest_PrehistoricMissionManager::FailMission(const FString& MissionID)
{
    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::Active)
        {
            Mission.Status = EQuest_MissionStatus::Failed;
            ActiveMissionIDs.Remove(MissionID);

            // Trigger event
            OnMissionFailed(Mission);

            UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Failed mission '%s' - %s"), 
                *Mission.MissionID, *Mission.MissionTitle);
            return true;
        }
    }

    return false;
}

bool UQuest_PrehistoricMissionManager::AbandonMission(const FString& MissionID)
{
    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::Active)
        {
            Mission.Status = EQuest_MissionStatus::Abandoned;
            ActiveMissionIDs.Remove(MissionID);

            UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Abandoned mission '%s' - %s"), 
                *Mission.MissionID, *Mission.MissionTitle);
            return true;
        }
    }

    return false;
}

void UQuest_PrehistoricMissionManager::UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress)
{
    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::Active)
        {
            if (ObjectiveIndex >= 0 && ObjectiveIndex < Mission.Objectives.Num())
            {
                FQuest_MissionObjective& Objective = Mission.Objectives[ObjectiveIndex];
                Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.TargetCount);

                // Check if objective is completed
                if (Objective.CurrentCount >= Objective.TargetCount)
                {
                    Objective.bIsCompleted = true;
                }

                // Trigger event
                OnObjectiveUpdated(Mission, ObjectiveIndex);

                UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Updated objective %d for mission '%s' - Progress: %d/%d"), 
                    ObjectiveIndex, *MissionID, Objective.CurrentCount, Objective.TargetCount);

                // Check if all objectives are completed
                bool bAllCompleted = true;
                for (const FQuest_MissionObjective& Obj : Mission.Objectives)
                {
                    if (!Obj.bIsOptional && !Obj.bIsCompleted)
                    {
                        bAllCompleted = false;
                        break;
                    }
                }

                if (bAllCompleted)
                {
                    CompleteMission(MissionID);
                }
            }
            break;
        }
    }
}

TArray<FQuest_PrehistoricMission> UQuest_PrehistoricMissionManager::GetActiveMissions() const
{
    TArray<FQuest_PrehistoricMission> ActiveMissions;
    
    for (const FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.Status == EQuest_MissionStatus::Active)
        {
            ActiveMissions.Add(Mission);
        }
    }

    return ActiveMissions;
}

TArray<FQuest_PrehistoricMission> UQuest_PrehistoricMissionManager::GetAvailableMissions() const
{
    TArray<FQuest_PrehistoricMission> AvailableMissions;
    
    for (const FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.Status == EQuest_MissionStatus::NotStarted && CheckMissionPrerequisites(Mission))
        {
            AvailableMissions.Add(Mission);
        }
    }

    return AvailableMissions;
}

FQuest_PrehistoricMission UQuest_PrehistoricMissionManager::GetMissionByID(const FString& MissionID) const
{
    for (const FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }

    return FQuest_PrehistoricMission();
}

TArray<FQuest_PrehistoricMission> UQuest_PrehistoricMissionManager::GetMissionsByType(EQuest_MissionType MissionType) const
{
    TArray<FQuest_PrehistoricMission> TypedMissions;
    
    for (const FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.MissionType == MissionType)
        {
            TypedMissions.Add(Mission);
        }
    }

    return TypedMissions;
}

bool UQuest_PrehistoricMissionManager::IsMissionCompleted(const FString& MissionID) const
{
    return CompletedMissionIDs.Contains(MissionID);
}

void UQuest_PrehistoricMissionManager::GenerateHuntMission(const FString& TargetSpecies, const FVector& HuntArea, int32 TargetCount)
{
    FQuest_PrehistoricMission HuntMission;
    HuntMission.MissionID = GenerateUniqueMissionID();
    HuntMission.MissionTitle = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    HuntMission.MissionDescription = FString::Printf(TEXT("Hunt %d %s in the designated area for food and materials."), TargetCount, *TargetSpecies);
    HuntMission.MissionType = EQuest_MissionType::Hunt;
    HuntMission.Priority = EQuest_MissionPriority::Medium;
    HuntMission.TargetLocation = HuntArea;
    HuntMission.TargetRadius = 2000.0f;

    // Create hunt objective
    FQuest_MissionObjective HuntObjective;
    HuntObjective.ObjectiveDescription = FString::Printf(TEXT("Hunt %d %s"), TargetCount, *TargetSpecies);
    HuntObjective.TargetCount = TargetCount;
    HuntObjective.TargetType = TargetSpecies;
    HuntMission.Objectives.Add(HuntObjective);

    // Set rewards
    HuntMission.Reward.ExperiencePoints = TargetCount * 50;
    HuntMission.Reward.ItemRewards.Add(TEXT("Meat"));
    HuntMission.Reward.ItemRewards.Add(TEXT("Hide"));
    HuntMission.Reward.ResourceReward = TargetCount * 10;

    CreateMission(HuntMission);
}

void UQuest_PrehistoricMissionManager::GenerateExplorationMission(const FVector& ExploreLocation, float ExploreRadius)
{
    FQuest_PrehistoricMission ExplorationMission;
    ExplorationMission.MissionID = GenerateUniqueMissionID();
    ExplorationMission.MissionTitle = TEXT("Explore Unknown Territory");
    ExplorationMission.MissionDescription = TEXT("Venture into unexplored territory to discover new resources and map the area.");
    ExplorationMission.MissionType = EQuest_MissionType::Explore;
    ExplorationMission.Priority = EQuest_MissionPriority::Medium;
    ExplorationMission.TargetLocation = ExploreLocation;
    ExplorationMission.TargetRadius = ExploreRadius;

    // Create exploration objective
    FQuest_MissionObjective ExploreObjective;
    ExploreObjective.ObjectiveDescription = TEXT("Reach the exploration target area");
    ExploreObjective.TargetCount = 1;
    ExploreObjective.TargetType = TEXT("Location");
    ExplorationMission.Objectives.Add(ExploreObjective);

    // Set rewards
    ExplorationMission.Reward.ExperiencePoints = 100;
    ExplorationMission.Reward.bUnlocksNewArea = true;
    ExplorationMission.Reward.UnlockReward = TEXT("New Territory Mapped");

    CreateMission(ExplorationMission);
}

void UQuest_PrehistoricMissionManager::GenerateCraftingMission(const FString& ItemToCraft, int32 Quantity)
{
    FQuest_PrehistoricMission CraftingMission;
    CraftingMission.MissionID = GenerateUniqueMissionID();
    CraftingMission.MissionTitle = FString::Printf(TEXT("Craft %s"), *ItemToCraft);
    CraftingMission.MissionDescription = FString::Printf(TEXT("Craft %d %s to improve your survival capabilities."), Quantity, *ItemToCraft);
    CraftingMission.MissionType = EQuest_MissionType::Craft;
    CraftingMission.Priority = EQuest_MissionPriority::Low;

    // Create crafting objective
    FQuest_MissionObjective CraftObjective;
    CraftObjective.ObjectiveDescription = FString::Printf(TEXT("Craft %d %s"), Quantity, *ItemToCraft);
    CraftObjective.TargetCount = Quantity;
    CraftObjective.TargetType = ItemToCraft;
    CraftingMission.Objectives.Add(CraftObjective);

    // Set rewards
    CraftingMission.Reward.ExperiencePoints = Quantity * 25;
    CraftingMission.Reward.ResourceReward = Quantity * 5;

    CreateMission(CraftingMission);
}

void UQuest_PrehistoricMissionManager::GenerateDefenseMission(const FVector& DefenseLocation, float Duration)
{
    FQuest_PrehistoricMission DefenseMission;
    DefenseMission.MissionID = GenerateUniqueMissionID();
    DefenseMission.MissionTitle = TEXT("Defend the Settlement");
    DefenseMission.MissionDescription = TEXT("Protect the settlement from incoming predator attacks.");
    DefenseMission.MissionType = EQuest_MissionType::Defend;
    DefenseMission.Priority = EQuest_MissionPriority::High;
    DefenseMission.TargetLocation = DefenseLocation;
    DefenseMission.TargetRadius = 1000.0f;
    DefenseMission.TimeLimit = Duration;

    // Create defense objective
    FQuest_MissionObjective DefendObjective;
    DefendObjective.ObjectiveDescription = FString::Printf(TEXT("Survive for %.0f seconds"), Duration);
    DefendObjective.TargetCount = 1;
    DefendObjective.TargetType = TEXT("Survival");
    DefenseMission.Objectives.Add(DefendObjective);

    // Set rewards
    DefenseMission.Reward.ExperiencePoints = 200;
    DefenseMission.Reward.ItemRewards.Add(TEXT("Defensive Tools"));
    DefenseMission.Reward.ResourceReward = 50;

    CreateMission(DefenseMission);
}

void UQuest_PrehistoricMissionManager::GenerateRescueMission(const FVector& RescueLocation, const FString& RescueTarget)
{
    FQuest_PrehistoricMission RescueMission;
    RescueMission.MissionID = GenerateUniqueMissionID();
    RescueMission.MissionTitle = FString::Printf(TEXT("Rescue %s"), *RescueTarget);
    RescueMission.MissionDescription = FString::Printf(TEXT("Rescue %s who is trapped or in danger."), *RescueTarget);
    RescueMission.MissionType = EQuest_MissionType::Rescue;
    RescueMission.Priority = EQuest_MissionPriority::High;
    RescueMission.TargetLocation = RescueLocation;
    RescueMission.TargetRadius = 500.0f;
    RescueMission.TimeLimit = 600.0f; // 10 minutes

    // Create rescue objective
    FQuest_MissionObjective RescueObjective;
    RescueObjective.ObjectiveDescription = FString::Printf(TEXT("Rescue %s"), *RescueTarget);
    RescueObjective.TargetCount = 1;
    RescueObjective.TargetType = RescueTarget;
    RescueMission.Objectives.Add(RescueObjective);

    // Set rewards
    RescueMission.Reward.ExperiencePoints = 150;
    RescueMission.Reward.ItemRewards.Add(TEXT("Gratitude Token"));
    RescueMission.Reward.ResourceReward = 30;

    CreateMission(RescueMission);
}

FString UQuest_PrehistoricMissionManager::GenerateUniqueMissionID()
{
    return FString::Printf(TEXT("MISSION_%d_%d"), FMath::RandRange(1000, 9999), AllMissions.Num());
}

bool UQuest_PrehistoricMissionManager::CheckMissionPrerequisites(const FQuest_PrehistoricMission& Mission) const
{
    for (const FString& Prerequisite : Mission.Prerequisites)
    {
        if (!CompletedMissionIDs.Contains(Prerequisite))
        {
            return false;
        }
    }
    return true;
}

void UQuest_PrehistoricMissionManager::ProcessMissionRewards(const FQuest_PrehistoricMission& Mission)
{
    // Log reward processing
    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Processing rewards for mission '%s' - XP: %d, Resources: %d"), 
        *Mission.MissionID, Mission.Reward.ExperiencePoints, Mission.Reward.ResourceReward);

    // Here you would integrate with player progression system, inventory system, etc.
    // For now, we just log the rewards
    for (const FString& Item : Mission.Reward.ItemRewards)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Rewarded item: %s"), *Item);
    }

    if (Mission.Reward.bUnlocksNewArea)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Unlocked new area: %s"), *Mission.Reward.UnlockReward);
    }
}

void UQuest_PrehistoricMissionManager::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.Status == EQuest_MissionStatus::Active)
        {
            Mission.ElapsedTime += DeltaTime;

            // Check for time limit expiration
            if (Mission.TimeLimit > 0.0f && Mission.ElapsedTime >= Mission.TimeLimit)
            {
                FailMission(Mission.MissionID);
            }
        }
    }
}

void UQuest_PrehistoricMissionManager::CheckLocationBasedObjectives()
{
    // This would check if player is in target locations for active missions
    // For now, we'll implement a basic version
    
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (FQuest_PrehistoricMission& Mission : AllMissions)
    {
        if (Mission.Status == EQuest_MissionStatus::Active)
        {
            float DistanceToTarget = FVector::Dist(PlayerLocation, Mission.TargetLocation);
            if (DistanceToTarget <= Mission.TargetRadius)
            {
                // Check for location-based objectives
                for (int32 i = 0; i < Mission.Objectives.Num(); ++i)
                {
                    FQuest_MissionObjective& Objective = Mission.Objectives[i];
                    if (Objective.TargetType == TEXT("Location") && !Objective.bIsCompleted)
                    {
                        UpdateObjectiveProgress(Mission.MissionID, i, 1);
                    }
                }
            }
        }
    }
}

void UQuest_PrehistoricMissionManager::AutoGenerateMissions()
{
    if (GetAvailableMissions().Num() >= 3) // Don't generate if we already have enough
    {
        return;
    }

    // Randomly generate different types of missions
    int32 MissionType = FMath::RandRange(0, 4);
    
    switch (MissionType)
    {
        case 0: // Hunt Mission
        {
            TArray<FString> PreySpecies = {TEXT("Compsognathus"), TEXT("Parasaurolophus"), TEXT("Triceratops")};
            FString TargetSpecies = PreySpecies[FMath::RandRange(0, PreySpecies.Num() - 1)];
            FVector HuntArea = FVector(FMath::RandRange(-5000, 5000), FMath::RandRange(-5000, 5000), 0);
            int32 TargetCount = FMath::RandRange(1, 3);
            GenerateHuntMission(TargetSpecies, HuntArea, TargetCount);
            break;
        }
        case 1: // Exploration Mission
        {
            FVector ExploreLocation = FVector(FMath::RandRange(-10000, 10000), FMath::RandRange(-10000, 10000), 0);
            float ExploreRadius = FMath::RandRange(1000, 3000);
            GenerateExplorationMission(ExploreLocation, ExploreRadius);
            break;
        }
        case 2: // Crafting Mission
        {
            TArray<FString> CraftItems = {TEXT("Stone Axe"), TEXT("Spear"), TEXT("Shelter"), TEXT("Fire Pit")};
            FString ItemToCraft = CraftItems[FMath::RandRange(0, CraftItems.Num() - 1)];
            int32 Quantity = FMath::RandRange(1, 2);
            GenerateCraftingMission(ItemToCraft, Quantity);
            break;
        }
        case 3: // Defense Mission
        {
            FVector DefenseLocation = FVector(0, 0, 0); // Settlement location
            float Duration = FMath::RandRange(120, 300); // 2-5 minutes
            GenerateDefenseMission(DefenseLocation, Duration);
            break;
        }
        case 4: // Rescue Mission
        {
            FVector RescueLocation = FVector(FMath::RandRange(-8000, 8000), FMath::RandRange(-8000, 8000), 0);
            TArray<FString> RescueTargets = {TEXT("Tribal Member"), TEXT("Injured Hunter"), TEXT("Lost Child")};
            FString RescueTarget = RescueTargets[FMath::RandRange(0, RescueTargets.Num() - 1)];
            GenerateRescueMission(RescueLocation, RescueTarget);
            break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricMissionManager: Auto-generated new mission"));
}

void UQuest_PrehistoricMissionManager::InitializeHuntMissionTemplates()
{
    // Initialize hunt mission templates (this could be loaded from data files)
    FQuest_PrehistoricMission HuntTemplate;
    HuntTemplate.MissionType = EQuest_MissionType::Hunt;
    HuntTemplate.Priority = EQuest_MissionPriority::Medium;
    HuntMissionTemplates.Add(HuntTemplate);
}

void UQuest_PrehistoricMissionManager::InitializeExplorationMissionTemplates()
{
    // Initialize exploration mission templates
    FQuest_PrehistoricMission ExploreTemplate;
    ExploreTemplate.MissionType = EQuest_MissionType::Explore;
    ExploreTemplate.Priority = EQuest_MissionPriority::Low;
    ExplorationMissionTemplates.Add(ExploreTemplate);
}

void UQuest_PrehistoricMissionManager::InitializeCraftingMissionTemplates()
{
    // Initialize crafting mission templates
    FQuest_PrehistoricMission CraftTemplate;
    CraftTemplate.MissionType = EQuest_MissionType::Craft;
    CraftTemplate.Priority = EQuest_MissionPriority::Low;
    CraftingMissionTemplates.Add(CraftTemplate);
}