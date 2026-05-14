#include "Quest_CrowdInteractionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalCharacter.h"
#include "Crowd/Crowd_MassSimulationManager.h"

UQuest_CrowdInteractionManager::UQuest_CrowdInteractionManager()
{
    MissionUpdateInterval = 1.0f;
    MaxActiveMissions = 5;
}

void UQuest_CrowdInteractionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CrowdInteractionManager initialized"));
    
    // Initialize default missions after a short delay
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UQuest_CrowdInteractionManager::InitializeDefaultMissions, 2.0f, false);
    }
}

void UQuest_CrowdInteractionManager::Deinitialize()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    CrowdManagers.Empty();
    
    Super::Deinitialize();
}

bool UQuest_CrowdInteractionManager::StartTribalMission(const FString& MissionID)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission %s: Maximum active missions reached"), *MissionID);
        return false;
    }

    // Check if mission is already active
    for (const FQuest_TribalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission %s is already active"), *MissionID);
            return false;
        }
    }

    // Find mission template and activate it
    FQuest_TribalMission NewMission;
    if (MissionID.Contains(TEXT("TribalCamp")))
    {
        NewMission = CreateTribalCampMission();
    }
    else if (MissionID.Contains(TEXT("HuntingParty")))
    {
        NewMission = CreateHuntingPartyMission();
    }
    else if (MissionID.Contains(TEXT("Gathering")))
    {
        NewMission = CreateGatheringMission();
    }
    else if (MissionID.Contains(TEXT("Watchtower")))
    {
        NewMission = CreateWatchtowerMission();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown mission type: %s"), *MissionID);
        return false;
    }

    NewMission.bIsActive = true;
    NewMission.MissionStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Started tribal mission: %s"), *NewMission.MissionName);
    return true;
}

bool UQuest_CrowdInteractionManager::CompleteTribalMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_TribalMission CompletedMission = ActiveMissions[i];
            CompletedMission.bIsCompleted = true;
            CompletedMission.bIsActive = false;
            
            CompletedMissions.Add(CompletedMission);
            ActiveMissions.RemoveAt(i);
            
            NotifyMissionCompletion(CompletedMission);
            
            UE_LOG(LogTemp, Log, TEXT("Completed tribal mission: %s"), *CompletedMission.MissionName);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mission %s not found in active missions"), *MissionID);
    return false;
}

void UQuest_CrowdInteractionManager::UpdateMissionProgress(const FString& MissionID, float DeltaTime)
{
    for (FQuest_TribalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            float ElapsedTime = GetWorld() ? GetWorld()->GetTimeSeconds() - Mission.MissionStartTime : 0.0f;
            
            // Check for mission timeout
            if (ElapsedTime > Mission.MissionTimeLimit)
            {
                UE_LOG(LogTemp, Warning, TEXT("Mission %s timed out"), *Mission.MissionName);
                Mission.bIsActive = false;
                return;
            }

            // Update interaction progress
            for (FQuest_CrowdInteraction& Interaction : Mission.RequiredInteractions)
            {
                if (!Interaction.bIsCompleted)
                {
                    Interaction.CompletionTime += DeltaTime;
                }
            }
            
            // Check if all interactions are completed
            bool bAllCompleted = true;
            for (const FQuest_CrowdInteraction& Interaction : Mission.RequiredInteractions)
            {
                if (!Interaction.bIsCompleted)
                {
                    bAllCompleted = false;
                    break;
                }
            }
            
            if (bAllCompleted)
            {
                CompleteTribalMission(MissionID);
            }
            
            break;
        }
    }
}

TArray<FQuest_TribalMission> UQuest_CrowdInteractionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

FQuest_TribalMission UQuest_CrowdInteractionManager::GetMissionByID(const FString& MissionID) const
{
    for (const FQuest_TribalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    return FQuest_TribalMission(); // Return empty mission if not found
}

bool UQuest_CrowdInteractionManager::CheckCrowdInteraction(ATranspersonalCharacter* Player, ECrowdZoneType ZoneType, float InteractionRadius)
{
    if (!Player)
    {
        return false;
    }

    ACrowd_MassSimulationManager* CrowdManager = GetCrowdManagerForZone(ZoneType);
    if (!CrowdManager)
    {
        return false;
    }

    FVector PlayerLocation = Player->GetActorLocation();
    FVector CrowdLocation = CrowdManager->GetActorLocation();
    
    float Distance = FVector::Dist(PlayerLocation, CrowdLocation);
    
    // Check if player is within interaction range
    if (Distance <= InteractionRadius)
    {
        // Update relevant mission interactions
        for (FQuest_TribalMission& Mission : ActiveMissions)
        {
            if (Mission.TargetZone == ZoneType && Mission.bIsActive)
            {
                for (FQuest_CrowdInteraction& Interaction : Mission.RequiredInteractions)
                {
                    if (Interaction.ZoneType == ZoneType && !Interaction.bIsCompleted)
                    {
                        Interaction.bIsCompleted = true;
                        UE_LOG(LogTemp, Log, TEXT("Crowd interaction completed: %s"), *Interaction.InteractionID);
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

void UQuest_CrowdInteractionManager::RegisterCrowdManager(ACrowd_MassSimulationManager* CrowdManager)
{
    if (!CrowdManager)
    {
        return;
    }

    ECrowdZoneType ZoneType = CrowdManager->GetZoneType();
    CrowdManagers.Add(ZoneType, CrowdManager);
    
    UE_LOG(LogTemp, Log, TEXT("Registered crowd manager for zone: %d"), (int32)ZoneType);
}

ACrowd_MassSimulationManager* UQuest_CrowdInteractionManager::GetCrowdManagerForZone(ECrowdZoneType ZoneType) const
{
    if (CrowdManagers.Contains(ZoneType))
    {
        return CrowdManagers[ZoneType];
    }
    
    return nullptr;
}

FQuest_TribalMission UQuest_CrowdInteractionManager::CreateTribalCampMission()
{
    FQuest_TribalMission Mission;
    Mission.MissionID = TEXT("TribalCamp_001");
    Mission.MissionName = TEXT("Visit the Tribal Camp");
    Mission.MissionDescription = TEXT("Approach the tribal camp and interact with the community members to learn about their way of life.");
    Mission.TargetZone = ECrowdZoneType::TribalCamp;
    Mission.MissionTimeLimit = 600.0f; // 10 minutes
    
    FQuest_CrowdInteraction Interaction;
    Interaction.InteractionID = TEXT("TribalCamp_Approach");
    Interaction.ZoneType = ECrowdZoneType::TribalCamp;
    Interaction.RequiredAgentCount = 3;
    Interaction.InteractionRadius = 800.0f;
    
    Mission.RequiredInteractions.Add(Interaction);
    
    return Mission;
}

FQuest_TribalMission UQuest_CrowdInteractionManager::CreateHuntingPartyMission()
{
    FQuest_TribalMission Mission;
    Mission.MissionID = TEXT("HuntingParty_001");
    Mission.MissionName = TEXT("Join the Hunting Party");
    Mission.MissionDescription = TEXT("Coordinate with the hunting party to track and hunt prehistoric creatures.");
    Mission.TargetZone = ECrowdZoneType::HuntingParty;
    Mission.MissionTimeLimit = 900.0f; // 15 minutes
    
    FQuest_CrowdInteraction Interaction;
    Interaction.InteractionID = TEXT("HuntingParty_Coordination");
    Interaction.ZoneType = ECrowdZoneType::HuntingParty;
    Interaction.RequiredAgentCount = 5;
    Interaction.InteractionRadius = 600.0f;
    
    Mission.RequiredInteractions.Add(Interaction);
    
    return Mission;
}

FQuest_TribalMission UQuest_CrowdInteractionManager::CreateGatheringMission()
{
    FQuest_TribalMission Mission;
    Mission.MissionID = TEXT("Gathering_001");
    Mission.MissionName = TEXT("Resource Gathering");
    Mission.MissionDescription = TEXT("Help the tribe gather essential resources like berries, herbs, and materials.");
    Mission.TargetZone = ECrowdZoneType::Gathering;
    Mission.MissionTimeLimit = 480.0f; // 8 minutes
    
    FQuest_CrowdInteraction Interaction;
    Interaction.InteractionID = TEXT("Gathering_Collection");
    Interaction.ZoneType = ECrowdZoneType::Gathering;
    Interaction.RequiredAgentCount = 4;
    Interaction.InteractionRadius = 700.0f;
    
    Mission.RequiredInteractions.Add(Interaction);
    
    return Mission;
}

FQuest_TribalMission UQuest_CrowdInteractionManager::CreateWatchtowerMission()
{
    FQuest_TribalMission Mission;
    Mission.MissionID = TEXT("Watchtower_001");
    Mission.MissionName = TEXT("Guard Duty");
    Mission.MissionDescription = TEXT("Take your position at the watchtower and help protect the tribe from threats.");
    Mission.TargetZone = ECrowdZoneType::Watchtower;
    Mission.MissionTimeLimit = 720.0f; // 12 minutes
    
    FQuest_CrowdInteraction Interaction;
    Interaction.InteractionID = TEXT("Watchtower_Guard");
    Interaction.ZoneType = ECrowdZoneType::Watchtower;
    Interaction.RequiredAgentCount = 2;
    Interaction.InteractionRadius = 400.0f;
    
    Mission.RequiredInteractions.Add(Interaction);
    
    return Mission;
}

void UQuest_CrowdInteractionManager::InitializeDefaultMissions()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing default tribal missions"));
    
    // Start one mission of each type for testing
    StartTribalMission(TEXT("TribalCamp_001"));
    StartTribalMission(TEXT("Gathering_001"));
    
    UE_LOG(LogTemp, Log, TEXT("Default missions initialized: %d active missions"), ActiveMissions.Num());
}

void UQuest_CrowdInteractionManager::DebugPrintActiveMissions()
{
    UE_LOG(LogTemp, Log, TEXT("=== ACTIVE TRIBAL MISSIONS ==="));
    for (const FQuest_TribalMission& Mission : ActiveMissions)
    {
        UE_LOG(LogTemp, Log, TEXT("Mission: %s | Zone: %d | Active: %s"), 
            *Mission.MissionName, 
            (int32)Mission.TargetZone,
            Mission.bIsActive ? TEXT("Yes") : TEXT("No"));
    }
    UE_LOG(LogTemp, Log, TEXT("=== END MISSIONS ==="));
}

void UQuest_CrowdInteractionManager::CleanupExpiredMissions()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        const FQuest_TribalMission& Mission = ActiveMissions[i];
        float ElapsedTime = GetWorld() ? GetWorld()->GetTimeSeconds() - Mission.MissionStartTime : 0.0f;
        
        if (ElapsedTime > Mission.MissionTimeLimit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Removing expired mission: %s"), *Mission.MissionName);
            ActiveMissions.RemoveAt(i);
        }
    }
}

bool UQuest_CrowdInteractionManager::ValidateMissionRequirements(const FQuest_TribalMission& Mission) const
{
    // Check if required crowd manager exists
    if (!CrowdManagers.Contains(Mission.TargetZone))
    {
        return false;
    }
    
    // Validate interaction requirements
    for (const FQuest_CrowdInteraction& Interaction : Mission.RequiredInteractions)
    {
        if (Interaction.RequiredAgentCount <= 0 || Interaction.InteractionRadius <= 0.0f)
        {
            return false;
        }
    }
    
    return true;
}

void UQuest_CrowdInteractionManager::NotifyMissionCompletion(const FQuest_TribalMission& Mission)
{
    UE_LOG(LogTemp, Log, TEXT("MISSION COMPLETED: %s"), *Mission.MissionName);
    
    // Here you could trigger rewards, notifications, or other systems
    if (GEngine)
    {
        FString CompletionMessage = FString::Printf(TEXT("Mission Completed: %s"), *Mission.MissionName);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, CompletionMessage);
    }
}