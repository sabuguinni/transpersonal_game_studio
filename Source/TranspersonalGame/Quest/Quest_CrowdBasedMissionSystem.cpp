#include "Quest_CrowdBasedMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UQuest_CrowdBasedMissionSystem::UQuest_CrowdBasedMissionSystem()
{
    CurrentMission = FQuest_CrowdMissionData();
    MissionStartTime = 0.0f;
    LastProgressUpdate = 0.0f;
    ConsecutiveSuccessfulChecks = 0;
}

void UQuest_CrowdBasedMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdBasedMissionSystem: Initializing crowd-based mission system"));
    
    // Get crowd subsystem reference
    if (UWorld* World = GetWorld())
    {
        CrowdSubsystem = World->GetGameInstance()->GetSubsystem<UCrowd_MassEntitySubsystem>();
        if (!CrowdSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdBasedMissionSystem: Could not find Crowd_MassEntitySubsystem"));
        }
    }
    
    InitializeAvailableMissions();
}

void UQuest_CrowdBasedMissionSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UQuest_CrowdBasedMissionSystem::StartCrowdMission(EQuest_CrowdMissionType MissionType, FVector Location, float Radius)
{
    if (CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdBasedMissionSystem: Cannot start new mission - one already active"));
        return;
    }
    
    // Find mission template
    FQuest_CrowdMissionData* MissionTemplate = nullptr;
    for (FQuest_CrowdMissionData& Mission : AvailableMissions)
    {
        if (Mission.MissionType == MissionType)
        {
            MissionTemplate = &Mission;
            break;
        }
    }
    
    if (!MissionTemplate)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_CrowdBasedMissionSystem: Mission type not found"));
        return;
    }
    
    // Initialize mission
    CurrentMission = *MissionTemplate;
    CurrentMission.TargetLocation = Location;
    CurrentMission.MissionRadius = Radius;
    CurrentMission.bIsActive = true;
    CurrentMission.bIsCompleted = false;
    CurrentMission.Progress = 0.0f;
    
    MissionStartTime = GetWorld()->GetTimeSeconds();
    LastProgressUpdate = 0.0f;
    ConsecutiveSuccessfulChecks = 0;
    
    // Start mission update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MissionUpdateTimer, this, 
            &UQuest_CrowdBasedMissionSystem::UpdateCrowdMissionProgress, 0.5f, true);
    }
    
    // Broadcast mission start
    OnCrowdMissionStarted.Broadcast(MissionType, Location);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdBasedMissionSystem: Started mission %s at location %s"), 
        *CurrentMission.MissionName, *Location.ToString());
}

void UQuest_CrowdBasedMissionSystem::CompleteMission(bool bSuccess)
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    CurrentMission.bIsActive = false;
    CurrentMission.bIsCompleted = bSuccess;
    CurrentMission.Progress = bSuccess ? 1.0f : CurrentMission.Progress;
    
    float CompletionTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionUpdateTimer);
    }
    
    // Broadcast completion
    OnCrowdMissionCompleted.Broadcast(bSuccess, CompletionTime);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdBasedMissionSystem: Mission completed - Success: %s, Time: %.2f"), 
        bSuccess ? TEXT("Yes") : TEXT("No"), CompletionTime);
}

void UQuest_CrowdBasedMissionSystem::UpdateCrowdMissionProgress(float DeltaTime)
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    // Check time limit
    float ElapsedTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
    if (ElapsedTime >= CurrentMission.TimeLimit)
    {
        CompleteMission(false);
        return;
    }
    
    // Update progress based on mission type
    switch (CurrentMission.MissionType)
    {
        case EQuest_CrowdMissionType::AvoidStampede:
            ProcessAvoidStampedeMission(DeltaTime);
            break;
        case EQuest_CrowdMissionType::FollowMigration:
            ProcessFollowMigrationMission(DeltaTime);
            break;
        case EQuest_CrowdMissionType::EscortThroughCrowd:
            ProcessEscortMission(DeltaTime);
            break;
        case EQuest_CrowdMissionType::ObserveBehavior:
            ProcessObservationMission(DeltaTime);
            break;
        default:
            break;
    }
    
    // Check for completion
    if (ValidateMissionCompletion())
    {
        CompleteMission(true);
    }
    
    // Broadcast progress update
    if (FMath::Abs(CurrentMission.Progress - LastProgressUpdate) > 0.1f)
    {
        OnCrowdMissionProgressUpdated.Broadcast(CurrentMission.Progress);
        LastProgressUpdate = CurrentMission.Progress;
    }
}

bool UQuest_CrowdBasedMissionSystem::CheckPlayerInCrowdArea(FVector PlayerLocation)
{
    int32 CrowdCount = GetCrowdCountInArea(PlayerLocation, 500.0f);
    return CrowdCount >= 5;
}

int32 UQuest_CrowdBasedMissionSystem::GetCrowdCountInArea(FVector Center, float Radius)
{
    if (!GetWorld())
    {
        return 0;
    }
    
    TArray<AActor*> CrowdActors = GetCrowdActorsInArea(Center, Radius);
    return CrowdActors.Num();
}

TArray<AActor*> UQuest_CrowdBasedMissionSystem::GetCrowdActorsInArea(FVector Center, float Radius)
{
    TArray<AActor*> Result;
    
    if (!GetWorld())
    {
        return Result;
    }
    
    // Get all actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    // Filter by distance and crowd-related names
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        bool bIsCrowdActor = ActorName.Contains(TEXT("crowd")) || 
                            ActorName.Contains(TEXT("herd")) || 
                            ActorName.Contains(TEXT("pack")) ||
                            ActorName.Contains(TEXT("flock"));
        
        if (bIsCrowdActor)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                Result.Add(Actor);
            }
        }
    }
    
    return Result;
}

void UQuest_CrowdBasedMissionSystem::ProcessAvoidStampedeMission(float DeltaTime)
{
    // Get player location
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Check if player is in danger zone
            bool bInDanger = IsPlayerInDanger(PlayerLocation);
            
            if (!bInDanger)
            {
                ConsecutiveSuccessfulChecks++;
                
                // Progress based on time spent safely
                float SafetyBonus = FMath::Min(ConsecutiveSuccessfulChecks * 0.02f, 0.1f);
                CurrentMission.Progress = FMath::Clamp(CurrentMission.Progress + SafetyBonus, 0.0f, 1.0f);
            }
            else
            {
                ConsecutiveSuccessfulChecks = 0;
                
                // Lose progress if in danger
                CurrentMission.Progress = FMath::Max(CurrentMission.Progress - 0.05f, 0.0f);
            }
        }
    }
}

void UQuest_CrowdBasedMissionSystem::ProcessFollowMigrationMission(float DeltaTime)
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Check if player is following the migration
            bool bFollowing = IsPlayerFollowingMigration(PlayerLocation);
            
            if (bFollowing)
            {
                CurrentMission.Progress = FMath::Clamp(CurrentMission.Progress + 0.03f, 0.0f, 1.0f);
            }
            else
            {
                // Slowly lose progress if not following
                CurrentMission.Progress = FMath::Max(CurrentMission.Progress - 0.01f, 0.0f);
            }
        }
    }
}

void UQuest_CrowdBasedMissionSystem::ProcessEscortMission(float DeltaTime)
{
    // Check if escort target is safe and moving towards destination
    FVector TargetLocation = CurrentMission.TargetLocation;
    int32 CrowdInArea = GetCrowdCountInArea(TargetLocation, CurrentMission.MissionRadius);
    
    if (CrowdInArea >= CurrentMission.RequiredCrowdSize)
    {
        CurrentMission.Progress = FMath::Clamp(CurrentMission.Progress + 0.02f, 0.0f, 1.0f);
    }
}

void UQuest_CrowdBasedMissionSystem::ProcessObservationMission(float DeltaTime)
{
    // Progress based on time spent observing crowd behavior
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Check if player is in observation range
            float DistanceToTarget = FVector::Dist(PlayerLocation, CurrentMission.TargetLocation);
            
            if (DistanceToTarget <= CurrentMission.MissionRadius)
            {
                int32 CrowdCount = GetCrowdCountInArea(CurrentMission.TargetLocation, CurrentMission.MissionRadius);
                
                if (CrowdCount >= CurrentMission.RequiredCrowdSize)
                {
                    CurrentMission.Progress = FMath::Clamp(CurrentMission.Progress + 0.025f, 0.0f, 1.0f);
                }
            }
        }
    }
}

void UQuest_CrowdBasedMissionSystem::InitializeAvailableMissions()
{
    AvailableMissions.Empty();
    
    // Avoid Stampede Mission
    AvailableMissions.Add(CreateMissionData(
        EQuest_CrowdMissionType::AvoidStampede,
        TEXT("Survive the Stampede"),
        TEXT("Avoid being trampled by the migrating herd for 3 minutes")
    ));
    
    // Follow Migration Mission
    AvailableMissions.Add(CreateMissionData(
        EQuest_CrowdMissionType::FollowMigration,
        TEXT("Follow the Great Migration"),
        TEXT("Stay close to the migrating herd and follow their path")
    ));
    
    // Escort Mission
    AvailableMissions.Add(CreateMissionData(
        EQuest_CrowdMissionType::EscortThroughCrowd,
        TEXT("Safe Passage"),
        TEXT("Guide injured tribe member through dangerous crowd area")
    ));
    
    // Observation Mission
    AvailableMissions.Add(CreateMissionData(
        EQuest_CrowdMissionType::ObserveBehavior,
        TEXT("Study the Herd"),
        TEXT("Observe dinosaur herd behavior patterns for 5 minutes")
    ));
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdBasedMissionSystem: Initialized %d available missions"), AvailableMissions.Num());
}

FQuest_CrowdMissionData UQuest_CrowdBasedMissionSystem::CreateMissionData(EQuest_CrowdMissionType Type, const FString& Name, const FString& Desc)
{
    FQuest_CrowdMissionData Mission;
    Mission.MissionType = Type;
    Mission.MissionName = Name;
    Mission.Description = Desc;
    
    switch (Type)
    {
        case EQuest_CrowdMissionType::AvoidStampede:
            Mission.TimeLimit = 180.0f;
            Mission.RequiredCrowdSize = 15;
            Mission.MissionRadius = 1500.0f;
            break;
        case EQuest_CrowdMissionType::FollowMigration:
            Mission.TimeLimit = 240.0f;
            Mission.RequiredCrowdSize = 20;
            Mission.MissionRadius = 2000.0f;
            break;
        case EQuest_CrowdMissionType::EscortThroughCrowd:
            Mission.TimeLimit = 300.0f;
            Mission.RequiredCrowdSize = 10;
            Mission.MissionRadius = 1000.0f;
            break;
        case EQuest_CrowdMissionType::ObserveBehavior:
            Mission.TimeLimit = 300.0f;
            Mission.RequiredCrowdSize = 12;
            Mission.MissionRadius = 800.0f;
            break;
    }
    
    return Mission;
}

bool UQuest_CrowdBasedMissionSystem::ValidateMissionCompletion()
{
    return CurrentMission.Progress >= 1.0f;
}

bool UQuest_CrowdBasedMissionSystem::IsPlayerInDanger(FVector PlayerLocation)
{
    // Check for nearby crowd actors moving fast (stampede risk)
    TArray<AActor*> NearbyCrowd = GetCrowdActorsInArea(PlayerLocation, 300.0f);
    
    if (NearbyCrowd.Num() >= 8)
    {
        return true; // High density = danger
    }
    
    return false;
}

float UQuest_CrowdBasedMissionSystem::CalculateStampedeRisk(FVector PlayerLocation)
{
    int32 CrowdCount = GetCrowdCountInArea(PlayerLocation, 500.0f);
    return FMath::Clamp(CrowdCount / 20.0f, 0.0f, 1.0f);
}

bool UQuest_CrowdBasedMissionSystem::IsPlayerFollowingMigration(FVector PlayerLocation)
{
    // Check if player is within migration corridor
    float DistanceToMigrationPath = FVector::Dist(PlayerLocation, CurrentMission.TargetLocation);
    return DistanceToMigrationPath <= CurrentMission.MissionRadius * 0.5f;
}