#include "Quest_EmergencyMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AQuest_EmergencyMissionSystem::AQuest_EmergencyMissionSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    MaxConcurrentEmergencies = 3;
    EmergencyCheckInterval = 30.0f; // Check every 30 seconds
    LastEmergencyCheckTime = 0.0f;
    MinDistanceFromPlayer = 5000.0f;
    MaxDistanceFromPlayer = 15000.0f;
    EmergencyMissionXPReward = 150;
    
    // Initialize emergency mission rewards
    EmergencyMissionRewards.Add(TEXT("Stone Tools"));
    EmergencyMissionRewards.Add(TEXT("Food Rations"));
    EmergencyMissionRewards.Add(TEXT("Medical Supplies"));
    EmergencyMissionRewards.Add(TEXT("Crafting Materials"));
}

void AQuest_EmergencyMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Emergency Mission System initialized"));
    
    // Start checking for emergencies after a delay
    LastEmergencyCheckTime = GetWorld()->GetTimeSeconds();
}

void AQuest_EmergencyMissionSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check for new emergencies periodically
    if (CurrentTime - LastEmergencyCheckTime >= EmergencyCheckInterval)
    {
        CheckForEmergencies();
        LastEmergencyCheckTime = CurrentTime;
    }
    
    // Update active emergency missions
    CleanupExpiredEmergencies();
}

void AQuest_EmergencyMissionSystem::TriggerEmergencyMission(EQuest_EmergencyType EmergencyType, FVector Location)
{
    // Check if we can add more emergency missions
    if (ActiveEmergencyMissions.Num() >= MaxConcurrentEmergencies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger emergency mission - maximum concurrent emergencies reached"));
        return;
    }
    
    // Create new emergency mission
    FQuest_EmergencyMission NewMission;
    NewMission.EmergencyType = EmergencyType;
    NewMission.EmergencyLocation = Location;
    NewMission.MissionTitle = GenerateEmergencyMissionTitle(EmergencyType);
    NewMission.MissionDescription = GenerateEmergencyMissionDescription(EmergencyType);
    NewMission.UrgencyLevel = FMath::RandRange(0.5f, 1.0f);
    NewMission.TimeLimit = FMath::RandRange(180.0f, 600.0f); // 3-10 minutes
    NewMission.bIsActive = true;
    
    // Add to active missions
    ActiveEmergencyMissions.Add(NewMission);
    
    // Spawn emergency actors
    SpawnEmergencyActors(EmergencyType, Location);
    
    // Notify player
    NotifyPlayerOfEmergency(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Emergency mission triggered: %s at location %s"), 
           *NewMission.MissionTitle, *Location.ToString());
}

void AQuest_EmergencyMissionSystem::CompleteEmergencyMission(int32 MissionIndex)
{
    if (ActiveEmergencyMissions.IsValidIndex(MissionIndex))
    {
        FQuest_EmergencyMission& Mission = ActiveEmergencyMissions[MissionIndex];
        Mission.bIsActive = false;
        
        // Award rewards
        UE_LOG(LogTemp, Warning, TEXT("Emergency mission completed: %s - XP Reward: %d"), 
               *Mission.MissionTitle, EmergencyMissionXPReward);
        
        // Remove from active missions
        ActiveEmergencyMissions.RemoveAt(MissionIndex);
    }
}

void AQuest_EmergencyMissionSystem::FailEmergencyMission(int32 MissionIndex)
{
    if (ActiveEmergencyMissions.IsValidIndex(MissionIndex))
    {
        FQuest_EmergencyMission& Mission = ActiveEmergencyMissions[MissionIndex];
        Mission.bIsActive = false;
        
        UE_LOG(LogTemp, Warning, TEXT("Emergency mission failed: %s"), *Mission.MissionTitle);
        
        // Remove from active missions
        ActiveEmergencyMissions.RemoveAt(MissionIndex);
    }
}

TArray<FQuest_EmergencyMission> AQuest_EmergencyMissionSystem::GetActiveEmergencyMissions() const
{
    return ActiveEmergencyMissions;
}

void AQuest_EmergencyMissionSystem::CheckForEmergencies()
{
    // Don't create new emergencies if at max capacity
    if (ActiveEmergencyMissions.Num() >= MaxConcurrentEmergencies)
    {
        return;
    }
    
    // Check if player is in danger or random emergency should occur
    bool bShouldTriggerEmergency = IsPlayerInDanger() || (FMath::RandRange(0.0f, 1.0f) < 0.3f);
    
    if (bShouldTriggerEmergency)
    {
        // Select random emergency type
        EQuest_EmergencyType RandomEmergencyType = static_cast<EQuest_EmergencyType>(
            FMath::RandRange(0, static_cast<int32>(EQuest_EmergencyType::WildFire))
        );
        
        // Get random location for emergency
        FVector EmergencyLocation = GetRandomEmergencyLocation();
        
        // Trigger the emergency
        TriggerEmergencyMission(RandomEmergencyType, EmergencyLocation);
    }
}

bool AQuest_EmergencyMissionSystem::IsPlayerInDanger() const
{
    // Simple danger detection - can be expanded
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        // Check if player health is low or other danger conditions
        // For now, return false - this would be connected to player health system
        return false;
    }
    
    return false;
}

void AQuest_EmergencyMissionSystem::SpawnEmergencyActors(EQuest_EmergencyType EmergencyType, FVector Location)
{
    // Spawn appropriate actors based on emergency type
    switch (EmergencyType)
    {
        case EQuest_EmergencyType::DinosaurAttack:
            UE_LOG(LogTemp, Warning, TEXT("Spawning dinosaur threat at %s"), *Location.ToString());
            // Would spawn aggressive dinosaurs
            break;
            
        case EQuest_EmergencyType::WeatherStorm:
            UE_LOG(LogTemp, Warning, TEXT("Triggering weather storm at %s"), *Location.ToString());
            // Would trigger weather effects
            break;
            
        case EQuest_EmergencyType::ResourceDepletion:
            UE_LOG(LogTemp, Warning, TEXT("Creating resource depletion zone at %s"), *Location.ToString());
            // Would remove resources from area
            break;
            
        case EQuest_EmergencyType::InjuredSurvivor:
            UE_LOG(LogTemp, Warning, TEXT("Spawning injured survivor at %s"), *Location.ToString());
            // Would spawn NPC that needs help
            break;
            
        case EQuest_EmergencyType::PredatorThreat:
            UE_LOG(LogTemp, Warning, TEXT("Spawning predator threat at %s"), *Location.ToString());
            // Would spawn predatory animals
            break;
            
        case EQuest_EmergencyType::WildFire:
            UE_LOG(LogTemp, Warning, TEXT("Starting wildfire at %s"), *Location.ToString());
            // Would spawn fire effects
            break;
    }
}

FVector AQuest_EmergencyMissionSystem::GetRandomEmergencyLocation() const
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    FVector PlayerLocation = FVector::ZeroVector;
    
    if (PlayerController && PlayerController->GetPawn())
    {
        PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    }
    
    // Generate random location within distance range from player
    float RandomDistance = FMath::RandRange(MinDistanceFromPlayer, MaxDistanceFromPlayer);
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    
    FVector RandomDirection = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
        0.0f
    );
    
    return PlayerLocation + (RandomDirection * RandomDistance);
}

FString AQuest_EmergencyMissionSystem::GenerateEmergencyMissionTitle(EQuest_EmergencyType EmergencyType) const
{
    switch (EmergencyType)
    {
        case EQuest_EmergencyType::DinosaurAttack:
            return TEXT("Dinosaur Threat Alert");
        case EQuest_EmergencyType::WeatherStorm:
            return TEXT("Severe Weather Warning");
        case EQuest_EmergencyType::ResourceDepletion:
            return TEXT("Resource Crisis");
        case EQuest_EmergencyType::InjuredSurvivor:
            return TEXT("Survivor in Need");
        case EQuest_EmergencyType::PredatorThreat:
            return TEXT("Predator Alert");
        case EQuest_EmergencyType::WildFire:
            return TEXT("Fire Emergency");
        default:
            return TEXT("Emergency Situation");
    }
}

FString AQuest_EmergencyMissionSystem::GenerateEmergencyMissionDescription(EQuest_EmergencyType EmergencyType) const
{
    switch (EmergencyType)
    {
        case EQuest_EmergencyType::DinosaurAttack:
            return TEXT("Aggressive dinosaurs have been spotted in the area. Investigate and neutralize the threat.");
        case EQuest_EmergencyType::WeatherStorm:
            return TEXT("A dangerous storm is approaching. Find shelter and help others prepare.");
        case EQuest_EmergencyType::ResourceDepletion:
            return TEXT("Critical resources are running low in the area. Find alternative sources.");
        case EQuest_EmergencyType::InjuredSurvivor:
            return TEXT("A fellow survivor is injured and needs immediate assistance.");
        case EQuest_EmergencyType::PredatorThreat:
            return TEXT("Dangerous predators are stalking the area. Clear the threat.");
        case EQuest_EmergencyType::WildFire:
            return TEXT("A wildfire has started and is spreading rapidly. Help contain the blaze.");
        default:
            return TEXT("An emergency situation requires your immediate attention.");
    }
}

void AQuest_EmergencyMissionSystem::CleanupExpiredEmergencies()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveEmergencyMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_EmergencyMission& Mission = ActiveEmergencyMissions[i];
        
        // Check if mission has expired (simplified - would need proper time tracking)
        if (!Mission.bIsActive)
        {
            ActiveEmergencyMissions.RemoveAt(i);
        }
    }
}

void AQuest_EmergencyMissionSystem::NotifyPlayerOfEmergency(const FQuest_EmergencyMission& Mission)
{
    // Display emergency notification to player
    UE_LOG(LogTemp, Warning, TEXT("EMERGENCY: %s - %s"), *Mission.MissionTitle, *Mission.MissionDescription);
    
    // Would trigger UI notification system
    if (GEngine)
    {
        FString NotificationText = FString::Printf(TEXT("EMERGENCY: %s"), *Mission.MissionTitle);
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, NotificationText);
    }
}