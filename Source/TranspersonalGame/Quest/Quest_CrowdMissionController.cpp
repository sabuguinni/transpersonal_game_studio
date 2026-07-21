#include "Quest_CrowdMissionController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

AQuest_CrowdMissionController::AQuest_CrowdMissionController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    MissionUpdateInterval = 2.0f;
    bAutoStartNextMission = true;
    NPCSpawnRadius = 500.0f;
    MaxConcurrentMissions = 3;
    MissionStartTime = 0.0f;
    CurrentNPCCount = 0;
    bMissionInitialized = false;
    
    // Initialize mission type weights
    MissionTypeWeights.Add(EQuest_CrowdMissionType::TribalGathering, 1.0f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::MigrationEscort, 0.8f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::CrowdControl, 0.6f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::SocialMediation, 0.7f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::GroupHunt, 0.9f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::CommunityDefense, 0.5f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::ResourceSharing, 0.8f);
    MissionTypeWeights.Add(EQuest_CrowdMissionType::CulturalExchange, 0.4f);
    
    // Initialize gathering points
    GatheringPoints.Add(FVector(0.0f, 0.0f, 100.0f));
    GatheringPoints.Add(FVector(500.0f, 500.0f, 100.0f));
    GatheringPoints.Add(FVector(-500.0f, 500.0f, 100.0f));
    GatheringPoints.Add(FVector(500.0f, -500.0f, 100.0f));
    GatheringPoints.Add(FVector(-500.0f, -500.0f, 100.0f));
}

void AQuest_CrowdMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find crowd simulation manager
    CrowdManager = GetWorld()->GetSubsystem<UCrowdSimulationManager>();
    if (!CrowdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdMissionController: CrowdSimulationManager not found"));
    }
    
    // Start mission update timer
    GetWorldTimerManager().SetTimer(MissionUpdateTimer, this, 
        &AQuest_CrowdMissionController::ProcessMissionQueue, MissionUpdateInterval, true);
    
    UE_LOG(LogTemp, Log, TEXT("CrowdMissionController initialized"));
}

void AQuest_CrowdMissionController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (CurrentMission.bIsActive)
    {
        UpdateMissionProgress(DeltaTime);
    }
}

void AQuest_CrowdMissionController::StartCrowdMission(const FQuest_CrowdMissionData& MissionData)
{
    if (CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission - another mission is already active"));
        return;
    }
    
    CurrentMission = MissionData;
    CurrentMission.bIsActive = true;
    CurrentMission.CompletionPercentage = 0.0f;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    bMissionInitialized = false;
    
    // Initialize mission-specific behavior
    InitializeMissionType(CurrentMission.MissionType);
    
    // Spawn NPCs for the mission
    SpawnMissionNPCs(CurrentMission.TargetNPCCount, CurrentMission.CentralLocation, CurrentMission.MissionRadius);
    
    UE_LOG(LogTemp, Log, TEXT("Started crowd mission: %s with %d NPCs"), 
        *CurrentMission.MissionName, CurrentMission.TargetNPCCount);
}

void AQuest_CrowdMissionController::CompleteMission()
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    CurrentMission.bIsActive = false;
    CurrentMission.CompletionPercentage = 100.0f;
    
    CleanupMission();
    
    UE_LOG(LogTemp, Log, TEXT("Completed mission: %s"), *CurrentMission.MissionName);
    
    // Auto-start next mission if enabled
    if (bAutoStartNextMission && MissionQueue.Num() > 0)
    {
        ProcessMissionQueue();
    }
}

void AQuest_CrowdMissionController::AbortMission()
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    CurrentMission.bIsActive = false;
    CurrentMission.CompletionPercentage = 0.0f;
    
    CleanupMission();
    
    UE_LOG(LogTemp, Warning, TEXT("Aborted mission: %s"), *CurrentMission.MissionName);
}

FQuest_CrowdMissionData AQuest_CrowdMissionController::GenerateRandomMission()
{
    FQuest_CrowdMissionData NewMission;
    
    // Randomly select mission type based on weights
    TArray<EQuest_CrowdMissionType> MissionTypes;
    TArray<float> Weights;
    
    for (const auto& Pair : MissionTypeWeights)
    {
        MissionTypes.Add(Pair.Key);
        Weights.Add(Pair.Value);
    }
    
    int32 RandomIndex = FMath::RandRange(0, MissionTypes.Num() - 1);
    NewMission.MissionType = MissionTypes[RandomIndex];
    
    // Set random parameters
    NewMission.CrowdDensity = static_cast<EQuest_CrowdDensity>(FMath::RandRange(0, 4));
    NewMission.CentralLocation = GatheringPoints[FMath::RandRange(0, GatheringPoints.Num() - 1)];
    NewMission.MissionRadius = FMath::RandRange(300.0f, 1500.0f);
    NewMission.MissionDuration = FMath::RandRange(120.0f, 600.0f);
    
    // Set NPC count based on density
    switch (NewMission.CrowdDensity)
    {
        case EQuest_CrowdDensity::Sparse:
            NewMission.TargetNPCCount = FMath::RandRange(5, 15);
            break;
        case EQuest_CrowdDensity::Moderate:
            NewMission.TargetNPCCount = FMath::RandRange(16, 50);
            break;
        case EQuest_CrowdDensity::Dense:
            NewMission.TargetNPCCount = FMath::RandRange(51, 150);
            break;
        case EQuest_CrowdDensity::Massive:
            NewMission.TargetNPCCount = FMath::RandRange(151, 500);
            break;
        case EQuest_CrowdDensity::Extreme:
            NewMission.TargetNPCCount = FMath::RandRange(500, 1000);
            break;
    }
    
    // Generate mission name
    FString MissionTypeName = UEnum::GetValueAsString(NewMission.MissionType);
    NewMission.MissionName = FString::Printf(TEXT("Random %s Mission"), *MissionTypeName);
    
    return NewMission;
}

void AQuest_CrowdMissionController::SpawnMissionNPCs(int32 NPCCount, const FVector& CenterLocation, float Radius)
{
    if (!CrowdManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn NPCs - CrowdManager is null"));
        return;
    }
    
    ParticipatingNPCs.Empty();
    CurrentNPCCount = NPCCount;
    
    // Spawn NPCs in circular pattern around center location
    for (int32 i = 0; i < NPCCount; i++)
    {
        float Angle = (2.0f * PI * i) / NPCCount;
        float Distance = FMath::RandRange(Radius * 0.3f, Radius * 0.8f);
        
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Add random offset
        SpawnLocation += FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        );
        
        // Note: Actual NPC spawning would be handled by CrowdManager
        // For now, we track the intended spawn locations
        UE_LOG(LogTemp, Log, TEXT("NPC %d spawn location: %s"), i, *SpawnLocation.ToString());
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d NPCs for mission"), NPCCount);
}

void AQuest_CrowdMissionController::UpdateMissionProgress(float DeltaTime)
{
    if (!CurrentMission.bIsActive || !bMissionInitialized)
    {
        return;
    }
    
    float ElapsedTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
    float ProgressByTime = FMath::Clamp(ElapsedTime / CurrentMission.MissionDuration, 0.0f, 1.0f);
    
    // Update progress based on mission type
    switch (CurrentMission.MissionType)
    {
        case EQuest_CrowdMissionType::TribalGathering:
            // Progress based on NPCs reaching gathering points
            CurrentMission.CompletionPercentage = ProgressByTime * 100.0f;
            break;
            
        case EQuest_CrowdMissionType::MigrationEscort:
            // Progress based on distance traveled
            CurrentMission.CompletionPercentage = ProgressByTime * 100.0f;
            break;
            
        case EQuest_CrowdMissionType::CrowdControl:
            // Progress based on crowd density management
            CurrentMission.CompletionPercentage = ProgressByTime * 100.0f;
            break;
            
        default:
            CurrentMission.CompletionPercentage = ProgressByTime * 100.0f;
            break;
    }
    
    // Check for mission completion
    if (IsMissionComplete())
    {
        CompleteMission();
    }
}

bool AQuest_CrowdMissionController::IsMissionComplete() const
{
    return CurrentMission.CompletionPercentage >= 100.0f;
}

float AQuest_CrowdMissionController::GetMissionTimeRemaining() const
{
    if (!CurrentMission.bIsActive)
    {
        return 0.0f;
    }
    
    float ElapsedTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
    return FMath::Max(0.0f, CurrentMission.MissionDuration - ElapsedTime);
}

void AQuest_CrowdMissionController::AddObjectiveToCurrentMission(const FString& ObjectiveDescription)
{
    CurrentMission.ObjectiveDescriptions.Add(ObjectiveDescription);
    UE_LOG(LogTemp, Log, TEXT("Added objective: %s"), *ObjectiveDescription);
}

void AQuest_CrowdMissionController::SetCrowdDensityTarget(EQuest_CrowdDensity NewDensity)
{
    CurrentMission.CrowdDensity = NewDensity;
    
    // Adjust target NPC count based on new density
    switch (NewDensity)
    {
        case EQuest_CrowdDensity::Sparse:
            CurrentMission.TargetNPCCount = FMath::RandRange(5, 15);
            break;
        case EQuest_CrowdDensity::Moderate:
            CurrentMission.TargetNPCCount = FMath::RandRange(16, 50);
            break;
        case EQuest_CrowdDensity::Dense:
            CurrentMission.TargetNPCCount = FMath::RandRange(51, 150);
            break;
        case EQuest_CrowdDensity::Massive:
            CurrentMission.TargetNPCCount = FMath::RandRange(151, 500);
            break;
        case EQuest_CrowdDensity::Extreme:
            CurrentMission.TargetNPCCount = FMath::RandRange(500, 1000);
            break;
    }
}

TArray<AActor*> AQuest_CrowdMissionController::GetNPCsInMissionRadius() const
{
    TArray<AActor*> NPCsInRadius;
    
    if (!GetWorld())
    {
        return NPCsInRadius;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), CurrentMission.CentralLocation);
            if (Distance <= CurrentMission.MissionRadius)
            {
                NPCsInRadius.Add(Actor);
            }
        }
    }
    
    return NPCsInRadius;
}

void AQuest_CrowdMissionController::ConfigureMissionBehavior(EQuest_CrowdMissionType MissionType)
{
    InitializeMissionType(MissionType);
}

void AQuest_CrowdMissionController::QueueMission(const FQuest_CrowdMissionData& MissionData)
{
    MissionQueue.Add(MissionData);
    UE_LOG(LogTemp, Log, TEXT("Queued mission: %s"), *MissionData.MissionName);
}

void AQuest_CrowdMissionController::ProcessMissionQueue()
{
    if (CurrentMission.bIsActive || MissionQueue.Num() == 0)
    {
        return;
    }
    
    FQuest_CrowdMissionData NextMission = MissionQueue[0];
    MissionQueue.RemoveAt(0);
    
    StartCrowdMission(NextMission);
}

void AQuest_CrowdMissionController::InitializeMissionType(EQuest_CrowdMissionType MissionType)
{
    switch (MissionType)
    {
        case EQuest_CrowdMissionType::TribalGathering:
            SetupTribalGathering();
            break;
        case EQuest_CrowdMissionType::MigrationEscort:
            SetupMigrationEscort();
            break;
        case EQuest_CrowdMissionType::CrowdControl:
            SetupCrowdControl();
            break;
        case EQuest_CrowdMissionType::SocialMediation:
            SetupSocialMediation();
            break;
        case EQuest_CrowdMissionType::GroupHunt:
            SetupGroupHunt();
            break;
        case EQuest_CrowdMissionType::CommunityDefense:
            SetupCommunityDefense();
            break;
        case EQuest_CrowdMissionType::ResourceSharing:
            SetupResourceSharing();
            break;
        case EQuest_CrowdMissionType::CulturalExchange:
            SetupCulturalExchange();
            break;
    }
    
    bMissionInitialized = true;
}

void AQuest_CrowdMissionController::SetupTribalGathering()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Gather tribal members at the central fire pit"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Maintain peaceful gathering atmosphere"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Ensure all members participate in the ritual"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Tribal Gathering mission"));
}

void AQuest_CrowdMissionController::SetupMigrationEscort()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Guide the nomadic tribe safely across the territory"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Protect the group from predators"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Maintain group cohesion during travel"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Migration Escort mission"));
}

void AQuest_CrowdMissionController::SetupCrowdControl()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Manage crowd density in the settlement"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Prevent overcrowding at resource points"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Direct traffic flow through the camp"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Crowd Control mission"));
}

void AQuest_CrowdMissionController::SetupSocialMediation()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Mediate disputes between tribal groups"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Facilitate peaceful negotiations"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Restore harmony to the community"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Social Mediation mission"));
}

void AQuest_CrowdMissionController::SetupGroupHunt()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Coordinate hunting party formation"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Lead the group to hunting grounds"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Ensure successful prey capture"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Group Hunt mission"));
}

void AQuest_CrowdMissionController::SetupCommunityDefense()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Organize defensive positions"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Coordinate group response to threats"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Protect vulnerable community members"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Community Defense mission"));
}

void AQuest_CrowdMissionController::SetupResourceSharing()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Facilitate equitable resource distribution"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Manage sharing ceremonies"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Ensure all families receive provisions"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Resource Sharing mission"));
}

void AQuest_CrowdMissionController::SetupCulturalExchange()
{
    CurrentMission.ObjectiveDescriptions.Empty();
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Organize cultural exchange between tribes"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Facilitate knowledge sharing"));
    CurrentMission.ObjectiveDescriptions.Add(TEXT("Promote inter-tribal cooperation"));
    
    UE_LOG(LogTemp, Log, TEXT("Configured Cultural Exchange mission"));
}

void AQuest_CrowdMissionController::CleanupMission()
{
    // Clear participating NPCs
    ParticipatingNPCs.Empty();
    CurrentNPCCount = 0;
    bMissionInitialized = false;
    
    // Reset mission timer
    MissionStartTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Mission cleanup completed"));
}

void AQuest_CrowdMissionController::ValidateMissionParameters()
{
    // Ensure mission parameters are within valid ranges
    CurrentMission.MissionRadius = FMath::Clamp(CurrentMission.MissionRadius, 100.0f, 5000.0f);
    CurrentMission.TargetNPCCount = FMath::Clamp(CurrentMission.TargetNPCCount, 1, 1000);
    CurrentMission.MissionDuration = FMath::Clamp(CurrentMission.MissionDuration, 30.0f, 1800.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Mission parameters validated"));
}