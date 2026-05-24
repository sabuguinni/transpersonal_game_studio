#include "Quest_CrowdIntegrationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// AQuest_CrowdObservationMarker Implementation
AQuest_CrowdObservationMarker::AQuest_CrowdObservationMarker()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create marker mesh component
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    MarkerMesh->SetupAttachment(RootComponent);

    // Set default values
    bIsActive = false;
    DetectionRadius = 500.0f;

    // Initialize observation data with defaults
    ObservationData.MissionType = EQuest_CrowdMissionType::HerdObservation;
    ObservationData.ObservationRadius = 1000.0f;
    ObservationData.RequiredObservationTime = 300;
    ObservationData.MinimumCrowdSize = 10;
    ObservationData.Status = EQuest_ObservationStatus::NotStarted;
    ObservationData.ProgressPercentage = 0.0f;

    // Initialize mission reward
    MissionReward.ExperiencePoints = 150;
    MissionReward.bUnlocksNewArea = false;
    MissionReward.RewardDescription = TEXT("Knowledge about prehistoric creature behavior");
}

void AQuest_CrowdObservationMarker::BeginPlay()
{
    Super::BeginPlay();

    // Set marker mesh properties
    if (MarkerMesh)
    {
        MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        MarkerMesh->SetCastShadow(false);
    }
}

void AQuest_CrowdObservationMarker::ActivateObservationMission()
{
    if (!bIsActive)
    {
        bIsActive = true;
        ObservationData.Status = EQuest_ObservationStatus::InProgress;
        ObservationData.ProgressPercentage = 0.0f;
        ObservationData.ObservationLocation = GetActorLocation();

        OnMissionStarted();

        UE_LOG(LogTemp, Log, TEXT("Observation mission activated at location: %s"), 
               *GetActorLocation().ToString());
    }
}

void AQuest_CrowdObservationMarker::UpdateObservationProgress(float DeltaTime)
{
    if (!bIsActive || ObservationData.Status != EQuest_ObservationStatus::InProgress)
    {
        return;
    }

    // Check if player is in observation range
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (IsPlayerInObservationRange(PlayerPawn))
    {
        // Update progress based on time spent observing
        float ProgressIncrement = (DeltaTime / ObservationData.RequiredObservationTime) * 100.0f;
        ObservationData.ProgressPercentage = FMath::Clamp(
            ObservationData.ProgressPercentage + ProgressIncrement, 
            0.0f, 100.0f
        );

        OnProgressUpdated(ObservationData.ProgressPercentage);

        // Check if mission is complete
        if (ObservationData.ProgressPercentage >= 100.0f)
        {
            CompleteMission();
        }
    }
}

bool AQuest_CrowdObservationMarker::IsPlayerInObservationRange(APawn* Player)
{
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= DetectionRadius;
}

void AQuest_CrowdObservationMarker::CompleteMission()
{
    if (bIsActive && ObservationData.Status == EQuest_ObservationStatus::InProgress)
    {
        ObservationData.Status = EQuest_ObservationStatus::Completed;
        ObservationData.ProgressPercentage = 100.0f;
        bIsActive = false;

        OnMissionCompleted();

        UE_LOG(LogTemp, Log, TEXT("Observation mission completed! Reward: %d XP"), 
               MissionReward.ExperiencePoints);
    }
}

// UQuest_CrowdIntegrationSystem Implementation
UQuest_CrowdIntegrationSystem::UQuest_CrowdIntegrationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;

    MaxSimultaneousMissions = 3;
    MissionGenerationInterval = 120.0f; // 2 minutes
    LastMissionGenerationTime = 0.0f;
    CompletedMissionCount = 0;
    CachedWorld = nullptr;
}

void UQuest_CrowdIntegrationSystem::BeginPlay()
{
    Super::BeginPlay();

    CachedWorld = GetWorld();
    InitializeCrowdMissions();
}

void UQuest_CrowdIntegrationSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateActiveMissions(DeltaTime);

    // Generate new missions periodically
    if (CachedWorld)
    {
        float CurrentTime = CachedWorld->GetTimeSeconds();
        if (CurrentTime - LastMissionGenerationTime >= MissionGenerationInterval)
        {
            if (ActiveObservationMissions.Num() < MaxSimultaneousMissions)
            {
                GenerateNewObservationMission();
                LastMissionGenerationTime = CurrentTime;
            }
        }
    }
}

void UQuest_CrowdIntegrationSystem::InitializeCrowdMissions()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing crowd integration mission system"));

    // Clear existing missions
    ActiveObservationMissions.Empty();
    ObservationMarkers.Empty();

    // Generate initial set of missions
    for (int32 i = 0; i < 2; ++i)
    {
        GenerateNewObservationMission();
    }
}

void UQuest_CrowdIntegrationSystem::GenerateNewObservationMission()
{
    if (!CachedWorld || ActiveObservationMissions.Num() >= MaxSimultaneousMissions)
    {
        return;
    }

    // Randomly select mission type
    TArray<EQuest_CrowdMissionType> MissionTypes = {
        EQuest_CrowdMissionType::HerdObservation,
        EQuest_CrowdMissionType::PredatorTracking,
        EQuest_CrowdMissionType::MigrationMapping,
        EQuest_CrowdMissionType::TerritorialStudy,
        EQuest_CrowdMissionType::PackBehaviorAnalysis
    };

    EQuest_CrowdMissionType SelectedType = MissionTypes[FMath::RandRange(0, MissionTypes.Num() - 1)];
    FVector MissionLocation = FindSuitableObservationLocation(SelectedType);

    if (StartObservationMission(SelectedType, MissionLocation))
    {
        UE_LOG(LogTemp, Log, TEXT("Generated new observation mission of type: %d at location: %s"), 
               (int32)SelectedType, *MissionLocation.ToString());
    }
}

void UQuest_CrowdIntegrationSystem::UpdateActiveMissions(float DeltaTime)
{
    // Update all active observation markers
    for (AQuest_CrowdObservationMarker* Marker : ObservationMarkers)
    {
        if (Marker && IsValid(Marker))
        {
            Marker->UpdateObservationProgress(DeltaTime);
        }
    }

    // Clean up completed missions
    for (int32 i = ActiveObservationMissions.Num() - 1; i >= 0; --i)
    {
        if (ActiveObservationMissions[i].Status == EQuest_ObservationStatus::Completed)
        {
            CompleteObservationMission(i);
        }
    }
}

bool UQuest_CrowdIntegrationSystem::StartObservationMission(EQuest_CrowdMissionType MissionType, FVector Location)
{
    if (!CachedWorld || ActiveObservationMissions.Num() >= MaxSimultaneousMissions)
    {
        return false;
    }

    // Create mission data
    FQuest_CrowdObservationData NewMission;
    NewMission.MissionType = MissionType;
    NewMission.ObservationLocation = Location;
    NewMission.Status = EQuest_ObservationStatus::NotStarted;

    // Set mission-specific parameters
    switch (MissionType)
    {
        case EQuest_CrowdMissionType::HerdObservation:
            NewMission.ObservationRadius = 1500.0f;
            NewMission.RequiredObservationTime = 240;
            NewMission.MinimumCrowdSize = 15;
            NewMission.BehaviorPatterns.Add(TEXT("Grazing patterns"));
            NewMission.BehaviorPatterns.Add(TEXT("Herd movement"));
            break;

        case EQuest_CrowdMissionType::PredatorTracking:
            NewMission.ObservationRadius = 2000.0f;
            NewMission.RequiredObservationTime = 180;
            NewMission.MinimumCrowdSize = 3;
            NewMission.BehaviorPatterns.Add(TEXT("Hunting strategies"));
            NewMission.BehaviorPatterns.Add(TEXT("Pack coordination"));
            break;

        case EQuest_CrowdMissionType::MigrationMapping:
            NewMission.ObservationRadius = 2500.0f;
            NewMission.RequiredObservationTime = 300;
            NewMission.MinimumCrowdSize = 20;
            NewMission.BehaviorPatterns.Add(TEXT("Migration routes"));
            NewMission.BehaviorPatterns.Add(TEXT("Seasonal patterns"));
            break;

        default:
            NewMission.ObservationRadius = 1000.0f;
            NewMission.RequiredObservationTime = 200;
            NewMission.MinimumCrowdSize = 10;
            break;
    }

    // Add to active missions
    ActiveObservationMissions.Add(NewMission);

    // Spawn observation marker
    SpawnObservationMarker(NewMission);

    OnNewMissionGenerated(NewMission);

    return true;
}

void UQuest_CrowdIntegrationSystem::CompleteObservationMission(int32 MissionIndex)
{
    if (MissionIndex >= 0 && MissionIndex < ActiveObservationMissions.Num())
    {
        FQuest_CrowdObservationData CompletedMission = ActiveObservationMissions[MissionIndex];
        ActiveObservationMissions.RemoveAt(MissionIndex);
        CompletedMissionCount++;

        OnMissionCompleted(CompletedMission);

        UE_LOG(LogTemp, Log, TEXT("Mission completed! Total completed missions: %d"), CompletedMissionCount);
    }
}

TArray<FQuest_CrowdObservationData> UQuest_CrowdIntegrationSystem::GetActiveMissions() const
{
    return ActiveObservationMissions;
}

int32 UQuest_CrowdIntegrationSystem::GetCompletedMissionCount() const
{
    return CompletedMissionCount;
}

void UQuest_CrowdIntegrationSystem::SpawnObservationMarker(const FQuest_CrowdObservationData& MissionData)
{
    if (!CachedWorld)
    {
        return;
    }

    // Spawn observation marker actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AQuest_CrowdObservationMarker* NewMarker = CachedWorld->SpawnActor<AQuest_CrowdObservationMarker>(
        AQuest_CrowdObservationMarker::StaticClass(),
        MissionData.ObservationLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (NewMarker)
    {
        NewMarker->ObservationData = MissionData;
        NewMarker->DetectionRadius = MissionData.ObservationRadius * 0.5f;
        ObservationMarkers.Add(NewMarker);

        UE_LOG(LogTemp, Log, TEXT("Spawned observation marker at: %s"), 
               *MissionData.ObservationLocation.ToString());
    }
}

FVector UQuest_CrowdIntegrationSystem::FindSuitableObservationLocation(EQuest_CrowdMissionType MissionType)
{
    // Generate locations based on mission type
    FVector BaseLocation = FVector::ZeroVector;

    switch (MissionType)
    {
        case EQuest_CrowdMissionType::HerdObservation:
            // Open grassland areas
            BaseLocation = FVector(
                FMath::RandRange(-2000.0f, 2000.0f),
                FMath::RandRange(-2000.0f, 2000.0f),
                200.0f
            );
            break;

        case EQuest_CrowdMissionType::PredatorTracking:
            // Forest edges and rocky areas
            BaseLocation = FVector(
                FMath::RandRange(-3000.0f, 3000.0f),
                FMath::RandRange(-1500.0f, 1500.0f),
                300.0f
            );
            break;

        case EQuest_CrowdMissionType::MigrationMapping:
            // River valleys and migration corridors
            BaseLocation = FVector(
                FMath::RandRange(-1000.0f, 1000.0f),
                FMath::RandRange(-3000.0f, 3000.0f),
                150.0f
            );
            break;

        default:
            BaseLocation = FVector(
                FMath::RandRange(-1500.0f, 1500.0f),
                FMath::RandRange(-1500.0f, 1500.0f),
                200.0f
            );
            break;
    }

    return BaseLocation;
}

bool UQuest_CrowdIntegrationSystem::ValidateObservationLocation(FVector Location, float Radius)
{
    // Basic validation - check if location is not too close to existing missions
    for (const FQuest_CrowdObservationData& Mission : ActiveObservationMissions)
    {
        float Distance = FVector::Dist(Location, Mission.ObservationLocation);
        if (Distance < Radius + Mission.ObservationRadius)
        {
            return false; // Too close to existing mission
        }
    }

    return true;
}