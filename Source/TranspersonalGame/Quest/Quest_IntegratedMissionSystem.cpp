#include "Quest_IntegratedMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

AQuest_IntegratedMissionSystem::AQuest_IntegratedMissionSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveMissions = 5;
    ObjectiveCheckInterval = 1.0f;
    MissionTimeUpdateInterval = 1.0f;
    LastObjectiveCheck = 0.0f;
    LastTimeUpdate = 0.0f;
    CurrentBiome = "Unknown";
    
    // Audio URLs from text-to-speech generation
    QuestNarrationURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482111223_QuestNarrator.mp3";
    CompletionRewardURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482117472_TribalElder.mp3";
}

void AQuest_IntegratedMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMissionSystem();
    CreateDefaultMissions();
}

void AQuest_IntegratedMissionSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMissionTimers(DeltaTime);
    
    LastObjectiveCheck += DeltaTime;
    if (LastObjectiveCheck >= ObjectiveCheckInterval)
    {
        CheckObjectiveCompletion();
        ProcessCrowdInteractions();
        LastObjectiveCheck = 0.0f;
    }
}

void AQuest_IntegratedMissionSystem::InitializeMissionSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Initializing integrated mission system"));
    
    // Find system references
    if (UWorld* World = GetWorld())
    {
        // Find crowd simulation manager
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains("CrowdSimulation"))
            {
                CrowdManager = Actor->FindComponentByClass<UCrowdSimulationManager>();
                break;
            }
        }
        
        // Find player character
        PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: System initialized with %d active missions"), ActiveMissions.Num());
}

void AQuest_IntegratedMissionSystem::StartMission(const FString& MissionID)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Cannot start mission %s - maximum active missions reached"), *MissionID);
        return;
    }
    
    // Find mission in available missions
    for (int32 i = 0; i < AvailableMissions.Num(); i++)
    {
        if (AvailableMissions[i].MissionID == MissionID)
        {
            FQuest_IntegratedMission Mission = AvailableMissions[i];
            Mission.bIsActive = true;
            Mission.RemainingTime = Mission.TimeLimit;
            
            ActiveMissions.Add(Mission);
            AvailableMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Started mission %s - %s"), *Mission.MissionID, *Mission.Title);
            NotifyMissionStateChange(MissionID, "Started");
            PlayQuestNarration(MissionID, "");
            break;
        }
    }
}

void AQuest_IntegratedMissionSystem::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_IntegratedMission Mission = ActiveMissions[i];
            Mission.bIsCompleted = true;
            Mission.bIsActive = false;
            
            CompletedMissions.Add(Mission);
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Completed mission %s - Reward: %d XP"), *Mission.Title, Mission.ExperienceReward);
            NotifyMissionStateChange(MissionID, "Completed");
            PlayCompletionReward(MissionID);
            break;
        }
    }
}

void AQuest_IntegratedMissionSystem::FailMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_IntegratedMission Mission = ActiveMissions[i];
            Mission.bIsActive = false;
            
            // Return to available missions with reset progress
            for (auto& Objective : Mission.Objectives)
            {
                Objective.CurrentProgress = 0;
                Objective.bIsCompleted = false;
            }
            
            AvailableMissions.Add(Mission);
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Failed mission %s"), *Mission.Title);
            NotifyMissionStateChange(MissionID, "Failed");
            break;
        }
    }
}

void AQuest_IntegratedMissionSystem::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 Progress)
{
    FQuest_IntegratedMission* Mission = FindMissionByID(MissionID);
    if (!Mission) return;
    
    FQuest_IntegratedObjective* Objective = FindObjectiveInMission(Mission, ObjectiveID);
    if (!Objective) return;
    
    Objective->CurrentProgress = FMath::Min(Objective->CurrentProgress + Progress, Objective->RequiredCount);
    
    if (Objective->CurrentProgress >= Objective->RequiredCount && !Objective->bIsCompleted)
    {
        Objective->bIsCompleted = true;
        UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Objective completed - %s"), *Objective->Description);
        
        // Check if all objectives are completed
        bool bAllCompleted = true;
        for (const auto& Obj : Mission->Objectives)
        {
            if (!Obj.bIsCompleted)
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
}

bool AQuest_IntegratedMissionSystem::CheckObjectiveCompletion(const FString& MissionID, const FString& ObjectiveID)
{
    FQuest_IntegratedMission* Mission = FindMissionByID(MissionID);
    if (!Mission) return false;
    
    FQuest_IntegratedObjective* Objective = FindObjectiveInMission(Mission, ObjectiveID);
    if (!Objective) return false;
    
    return Objective->bIsCompleted;
}

void AQuest_IntegratedMissionSystem::RegisterCrowdInteraction(const FString& CrowdBehaviorTag, const FVector& Location)
{
    CrowdInteractionPoints.Add(CrowdBehaviorTag, Location);
    
    // Check if any active objectives require this crowd interaction
    for (auto& Mission : ActiveMissions)
    {
        for (auto& Objective : Mission.Objectives)
        {
            if (Objective.bRequiresCrowdInteraction && Objective.CrowdBehaviorTag == CrowdBehaviorTag)
            {
                if (PlayerCharacter && FVector::Dist(PlayerCharacter->GetActorLocation(), Location) <= Objective.CompletionRadius)
                {
                    UpdateObjectiveProgress(Mission.MissionID, Objective.ObjectiveID, 1);
                }
            }
        }
    }
}

void AQuest_IntegratedMissionSystem::UpdateBiomeContext(const FString& NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Biome changed to %s"), *CurrentBiome);
        
        // Activate biome-specific missions
        for (auto& Mission : AvailableMissions)
        {
            if (Mission.RequiredBiome == CurrentBiome && !Mission.bIsActive)
            {
                StartMission(Mission.MissionID);
                break; // Start one mission per biome change
            }
        }
    }
}

void AQuest_IntegratedMissionSystem::TriggerCrowdBasedObjective(const FString& MissionID, const FString& ObjectiveID)
{
    FQuest_IntegratedMission* Mission = FindMissionByID(MissionID);
    if (!Mission) return;
    
    FQuest_IntegratedObjective* Objective = FindObjectiveInMission(Mission, ObjectiveID);
    if (!Objective || !Objective->bRequiresCrowdInteraction) return;
    
    // Trigger crowd behavior through crowd manager
    if (CrowdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Triggering crowd behavior %s for objective %s"), *Objective->CrowdBehaviorTag, *ObjectiveID);
        // CrowdManager->TriggerBehavior(Objective->CrowdBehaviorTag, Objective->TargetLocation);
    }
}

void AQuest_IntegratedMissionSystem::CreateHuntMission(const FString& TargetSpecies, const FVector& HuntingGrounds)
{
    FQuest_IntegratedMission HuntMission;
    HuntMission.MissionID = FString::Printf(TEXT("HUNT_%s_%d"), *TargetSpecies, FMath::RandRange(1000, 9999));
    HuntMission.Title = FString::Printf(TEXT("Hunt the %s"), *TargetSpecies);
    HuntMission.Description = FString::Printf(TEXT("Track and hunt a %s in the designated hunting grounds. Use stealth and strategy to succeed."), *TargetSpecies);
    HuntMission.Priority = EMissionPriority::High;
    HuntMission.TimeLimit = 1800.0f; // 30 minutes
    HuntMission.ExperienceReward = 250;
    HuntMission.RequiredBiome = "Grasslands";
    
    FQuest_IntegratedObjective HuntObjective;
    HuntObjective.ObjectiveID = "HUNT_TARGET";
    HuntObjective.Description = FString::Printf(TEXT("Hunt 1 %s"), *TargetSpecies);
    HuntObjective.QuestType = EQuestType::Hunt;
    HuntObjective.TargetLocation = HuntingGrounds;
    HuntObjective.CompletionRadius = 1000.0f;
    HuntObjective.RequiredCount = 1;
    
    HuntMission.Objectives.Add(HuntObjective);
    HuntMission.RewardItems.Add("Crafted Spear");
    HuntMission.RewardItems.Add("Animal Hide");
    
    AvailableMissions.Add(HuntMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Created hunt mission for %s"), *TargetSpecies);
}

void AQuest_IntegratedMissionSystem::CreateGatherMission(const FString& ResourceType, const TArray<FVector>& GatherLocations)
{
    FQuest_IntegratedMission GatherMission;
    GatherMission.MissionID = FString::Printf(TEXT("GATHER_%s_%d"), *ResourceType, FMath::RandRange(1000, 9999));
    GatherMission.Title = FString::Printf(TEXT("Gather %s Resources"), *ResourceType);
    GatherMission.Description = FString::Printf(TEXT("Collect valuable %s resources from multiple locations across the land."), *ResourceType);
    GatherMission.Priority = EMissionPriority::Normal;
    GatherMission.TimeLimit = 1200.0f; // 20 minutes
    GatherMission.ExperienceReward = 150;
    
    for (int32 i = 0; i < GatherLocations.Num(); i++)
    {
        FQuest_IntegratedObjective GatherObjective;
        GatherObjective.ObjectiveID = FString::Printf(TEXT("GATHER_%d"), i);
        GatherObjective.Description = FString::Printf(TEXT("Collect %s from location %d"), *ResourceType, i + 1);
        GatherObjective.QuestType = EQuestType::Gather;
        GatherObjective.TargetLocation = GatherLocations[i];
        GatherObjective.CompletionRadius = 300.0f;
        GatherObjective.RequiredCount = 3;
        
        GatherMission.Objectives.Add(GatherObjective);
    }
    
    GatherMission.RewardItems.Add("Crafting Materials");
    AvailableMissions.Add(GatherMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Created gather mission for %s"), *ResourceType);
}

void AQuest_IntegratedMissionSystem::CreateEscortMission(const FString& NPCName, const FVector& StartLocation, const FVector& EndLocation)
{
    FQuest_IntegratedMission EscortMission;
    EscortMission.MissionID = FString::Printf(TEXT("ESCORT_%s_%d"), *NPCName, FMath::RandRange(1000, 9999));
    EscortMission.Title = FString::Printf(TEXT("Escort %s"), *NPCName);
    EscortMission.Description = FString::Printf(TEXT("Safely escort %s through dangerous territory to their destination."), *NPCName);
    EscortMission.Priority = EMissionPriority::High;
    EscortMission.TimeLimit = 2400.0f; // 40 minutes
    EscortMission.ExperienceReward = 300;
    
    FQuest_IntegratedObjective EscortObjective;
    EscortObjective.ObjectiveID = "ESCORT_SAFE";
    EscortObjective.Description = FString::Printf(TEXT("Escort %s safely to destination"), *NPCName);
    EscortObjective.QuestType = EQuestType::Escort;
    EscortObjective.TargetLocation = EndLocation;
    EscortObjective.CompletionRadius = 200.0f;
    EscortObjective.RequiredCount = 1;
    EscortObjective.bRequiresCrowdInteraction = true;
    EscortObjective.CrowdBehaviorTag = "EscortNPC";
    
    EscortMission.Objectives.Add(EscortObjective);
    EscortMission.CrowdInteractionTags.Add("EscortNPC");
    EscortMission.RewardItems.Add("Tribal Token");
    
    AvailableMissions.Add(EscortMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Created escort mission for %s"), *NPCName);
}

void AQuest_IntegratedMissionSystem::CreateDefenseMission(const FVector& DefensePoint, int32 WaveCount)
{
    FQuest_IntegratedMission DefenseMission;
    DefenseMission.MissionID = FString::Printf(TEXT("DEFEND_%d"), FMath::RandRange(1000, 9999));
    DefenseMission.Title = "Defend the Settlement";
    DefenseMission.Description = "Protect the tribal settlement from waves of attacking predators.";
    DefenseMission.Priority = EMissionPriority::Critical;
    DefenseMission.TimeLimit = 3000.0f; // 50 minutes
    DefenseMission.ExperienceReward = 500;
    
    FQuest_IntegratedObjective DefenseObjective;
    DefenseObjective.ObjectiveID = "DEFEND_WAVES";
    DefenseObjective.Description = FString::Printf(TEXT("Survive %d waves of attacks"), WaveCount);
    DefenseObjective.QuestType = EQuestType::Defense;
    DefenseObjective.TargetLocation = DefensePoint;
    DefenseObjective.CompletionRadius = 800.0f;
    DefenseObjective.RequiredCount = WaveCount;
    DefenseObjective.bRequiresCrowdInteraction = true;
    DefenseObjective.CrowdBehaviorTag = "DefenseFormation";
    
    DefenseMission.Objectives.Add(DefenseObjective);
    DefenseMission.CrowdInteractionTags.Add("DefenseFormation");
    DefenseMission.RewardItems.Add("Legendary Weapon");
    DefenseMission.RewardItems.Add("Tribal Honor");
    
    AvailableMissions.Add(DefenseMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Created defense mission with %d waves"), WaveCount);
}

void AQuest_IntegratedMissionSystem::PlayQuestNarration(const FString& MissionID, const FString& ObjectiveID)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Playing quest narration for mission %s"), *MissionID);
    // Audio URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482111223_QuestNarrator.mp3
}

void AQuest_IntegratedMissionSystem::PlayCompletionReward(const FString& MissionID)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Playing completion reward for mission %s"), *MissionID);
    // Audio URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781482117472_TribalElder.mp3
}

void AQuest_IntegratedMissionSystem::UpdateMissionTimers(float DeltaTime)
{
    LastTimeUpdate += DeltaTime;
    if (LastTimeUpdate >= MissionTimeUpdateInterval)
    {
        for (auto& Mission : ActiveMissions)
        {
            if (Mission.TimeLimit > 0.0f)
            {
                Mission.RemainingTime -= LastTimeUpdate;
                if (Mission.RemainingTime <= 0.0f)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Mission %s timed out"), *Mission.Title);
                    FailMission(Mission.MissionID);
                    break; // Break to avoid iterator invalidation
                }
            }
        }
        LastTimeUpdate = 0.0f;
    }
}

void AQuest_IntegratedMissionSystem::CheckObjectiveCompletion()
{
    if (!PlayerCharacter) return;
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    for (auto& Mission : ActiveMissions)
    {
        for (auto& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted && !Objective.bRequiresCrowdInteraction)
            {
                float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                if (Distance <= Objective.CompletionRadius)
                {
                    // Location-based objective completion
                    if (Objective.QuestType == EQuestType::Exploration)
                    {
                        UpdateObjectiveProgress(Mission.MissionID, Objective.ObjectiveID, 1);
                    }
                }
            }
        }
    }
}

void AQuest_IntegratedMissionSystem::ProcessCrowdInteractions()
{
    // Process crowd interaction points
    for (const auto& InteractionPair : CrowdInteractionPoints)
    {
        const FString& BehaviorTag = InteractionPair.Key;
        const FVector& Location = InteractionPair.Value;
        
        // Check if player is near any crowd interaction points
        if (PlayerCharacter)
        {
            float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), Location);
            if (Distance <= 500.0f) // Interaction range
            {
                RegisterCrowdInteraction(BehaviorTag, Location);
            }
        }
    }
}

void AQuest_IntegratedMissionSystem::ValidateMissionRequirements()
{
    // Validate that missions have proper requirements met
    for (const auto& Mission : ActiveMissions)
    {
        if (!Mission.RequiredBiome.IsEmpty() && Mission.RequiredBiome != CurrentBiome)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Mission %s requires biome %s but current is %s"), 
                   *Mission.Title, *Mission.RequiredBiome, *CurrentBiome);
        }
    }
}

FQuest_IntegratedMission* AQuest_IntegratedMissionSystem::FindMissionByID(const FString& MissionID)
{
    for (auto& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return &Mission;
        }
    }
    return nullptr;
}

FQuest_IntegratedObjective* AQuest_IntegratedMissionSystem::FindObjectiveInMission(FQuest_IntegratedMission* Mission, const FString& ObjectiveID)
{
    if (!Mission) return nullptr;
    
    for (auto& Objective : Mission->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return &Objective;
        }
    }
    return nullptr;
}

void AQuest_IntegratedMissionSystem::CreateDefaultMissions()
{
    // Create sample hunt mission
    CreateHuntMission("Triceratops", FVector(5000, 3000, 100));
    
    // Create sample gather mission
    TArray<FVector> GatherPoints;
    GatherPoints.Add(FVector(2000, 1000, 50));
    GatherPoints.Add(FVector(-1000, 2000, 75));
    GatherPoints.Add(FVector(3000, -1500, 25));
    CreateGatherMission("Rare Herbs", GatherPoints);
    
    // Create sample escort mission
    CreateEscortMission("Tribal Scout", FVector(1000, 500, 100), FVector(-2000, -1000, 150));
    
    // Create sample defense mission
    CreateDefenseMission(FVector(0, 0, 100), 3);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Created %d default missions"), AvailableMissions.Num());
}

void AQuest_IntegratedMissionSystem::NotifyMissionStateChange(const FString& MissionID, const FString& NewState)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_IntegratedMissionSystem: Mission %s state changed to %s"), *MissionID, *NewState);
    
    // Broadcast to UI or other systems
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Mission %s: %s"), *MissionID, *NewState);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Message);
    }
}