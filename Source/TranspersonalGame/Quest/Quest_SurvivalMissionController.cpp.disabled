#include "Quest_SurvivalMissionController.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AQuest_SurvivalMissionController::AQuest_SurvivalMissionController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create detection sphere component
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(500.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create mission marker mesh
    MissionMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissionMarker"));
    MissionMarker->SetupAttachment(RootComponent);
    MissionMarker->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    MissionMarker->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

    // Initialize mission properties
    CurrentMissionType = EQuest_SurvivalMissionType::WaterGathering;
    MissionRadius = 500.0f;
    bMissionActive = false;
    MissionTitle = TEXT("Survival Mission");
    MissionDescription = TEXT("Complete survival objectives to stay alive");
    MissionStartTime = 0.0f;
    bPlayerInRange = false;

    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_SurvivalMissionController::OnDetectionSphereBeginOverlap);
}

void AQuest_SurvivalMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with a default water gathering mission
    StartMission(EQuest_SurvivalMissionType::WaterGathering);
}

void AQuest_SurvivalMissionController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bMissionActive)
    {
        UpdateMissionProgress();

        // Check for mission timeout
        float CurrentTime = GetWorld()->GetTimeSeconds();
        for (const FQuest_SurvivalObjective& Objective : ActiveObjectives)
        {
            if (!Objective.bIsCompleted && Objective.TimeLimit > 0.0f)
            {
                float ElapsedTime = CurrentTime - MissionStartTime;
                if (ElapsedTime > Objective.TimeLimit)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Mission objective timed out: %s"), *Objective.ObjectiveDescription);
                }
            }
        }
    }
}

void AQuest_SurvivalMissionController::StartMission(EQuest_SurvivalMissionType MissionType)
{
    CurrentMissionType = MissionType;
    bMissionActive = true;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    ActiveObjectives.Empty();

    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::WaterGathering:
            InitializeWaterGatheringMission();
            break;
        case EQuest_SurvivalMissionType::FoodForaging:
            InitializeFoodForagingMission();
            break;
        case EQuest_SurvivalMissionType::ShelterBuilding:
            InitializeShelterBuildingMission();
            break;
        case EQuest_SurvivalMissionType::ToolCrafting:
            InitializeToolCraftingMission();
            break;
        case EQuest_SurvivalMissionType::TerritoryMapping:
            InitializeTerritoryMappingMission();
            break;
        case EQuest_SurvivalMissionType::PredatorAvoidance:
            InitializePredatorAvoidanceMission();
            break;
    }

    OnMissionStarted(MissionType);
    UE_LOG(LogTemp, Log, TEXT("Started mission: %s"), *MissionTitle);
}

void AQuest_SurvivalMissionController::CompleteMission()
{
    bMissionActive = false;
    OnMissionCompleted();
    UE_LOG(LogTemp, Log, TEXT("Completed mission: %s"), *MissionTitle);
}

void AQuest_SurvivalMissionController::AddObjective(const FQuest_SurvivalObjective& NewObjective)
{
    ActiveObjectives.Add(NewObjective);
    UE_LOG(LogTemp, Log, TEXT("Added objective: %s"), *NewObjective.ObjectiveDescription);
}

void AQuest_SurvivalMissionController::CompleteObjective(int32 ObjectiveIndex)
{
    if (ActiveObjectives.IsValidIndex(ObjectiveIndex))
    {
        ActiveObjectives[ObjectiveIndex].bIsCompleted = true;
        OnObjectiveCompleted(ObjectiveIndex);
        UE_LOG(LogTemp, Log, TEXT("Completed objective: %s"), *ActiveObjectives[ObjectiveIndex].ObjectiveDescription);

        // Check if all objectives are completed
        bool bAllCompleted = true;
        for (const FQuest_SurvivalObjective& Objective : ActiveObjectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllCompleted = false;
                break;
            }
        }

        if (bAllCompleted)
        {
            CompleteMission();
        }
    }
}

bool AQuest_SurvivalMissionController::CheckObjectiveCompletion(const FQuest_SurvivalObjective& Objective, const FVector& PlayerLocation)
{
    if (Objective.bIsCompleted)
    {
        return true;
    }

    float DistanceToTarget = FVector::Dist(PlayerLocation, Objective.TargetLocation);
    return DistanceToTarget <= Objective.CompletionRadius;
}

void AQuest_SurvivalMissionController::UpdateMissionProgress()
{
    if (!bMissionActive || !bPlayerInRange)
    {
        return;
    }

    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Check each objective for completion
    for (int32 i = 0; i < ActiveObjectives.Num(); ++i)
    {
        if (!ActiveObjectives[i].bIsCompleted)
        {
            if (CheckObjectiveCompletion(ActiveObjectives[i], PlayerLocation))
            {
                CompleteObjective(i);
            }
        }
    }
}

float AQuest_SurvivalMissionController::GetMissionCompletionPercentage() const
{
    if (ActiveObjectives.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedCount = 0;
    for (const FQuest_SurvivalObjective& Objective : ActiveObjectives)
    {
        if (Objective.bIsCompleted)
        {
            CompletedCount++;
        }
    }

    return static_cast<float>(CompletedCount) / static_cast<float>(ActiveObjectives.Num()) * 100.0f;
}

void AQuest_SurvivalMissionController::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        bPlayerInRange = true;
        UE_LOG(LogTemp, Log, TEXT("Player entered mission area: %s"), *MissionTitle);
    }
}

void AQuest_SurvivalMissionController::InitializeWaterGatheringMission()
{
    MissionTitle = TEXT("Find Clean Water");
    MissionDescription = TEXT("Locate and gather clean water before dehydration becomes critical");
    VoiceLineURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777644656541_SurvivalGuide_Elder.mp3");

    FQuest_SurvivalObjective WaterObjective;
    WaterObjective.ObjectiveDescription = TEXT("Reach the river and collect water");
    WaterObjective.MissionType = EQuest_SurvivalMissionType::WaterGathering;
    WaterObjective.Priority = EQuest_MissionPriority::Critical;
    WaterObjective.TimeLimit = 600.0f; // 10 minutes
    WaterObjective.TargetLocation = FVector(1500.0f, 0.0f, 50.0f); // River location
    WaterObjective.CompletionRadius = 300.0f;

    AddObjective(WaterObjective);
}

void AQuest_SurvivalMissionController::InitializeFoodForagingMission()
{
    MissionTitle = TEXT("Forage for Food");
    MissionDescription = TEXT("Find edible plants and berries to sustain yourself");
    VoiceLineURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777644664282_Hunter_Tracker.mp3");

    FQuest_SurvivalObjective ForagingObjective;
    ForagingObjective.ObjectiveDescription = TEXT("Gather edible plants from the forest");
    ForagingObjective.MissionType = EQuest_SurvivalMissionType::FoodForaging;
    ForagingObjective.Priority = EQuest_MissionPriority::High;
    ForagingObjective.TimeLimit = 900.0f; // 15 minutes
    ForagingObjective.TargetLocation = FVector(-800.0f, 1200.0f, 100.0f); // Forest area
    ForagingObjective.CompletionRadius = 400.0f;

    AddObjective(ForagingObjective);
}

void AQuest_SurvivalMissionController::InitializeShelterBuildingMission()
{
    MissionTitle = TEXT("Build Shelter");
    MissionDescription = TEXT("Construct a basic shelter to protect from predators and weather");

    FQuest_SurvivalObjective ShelterObjective;
    ShelterObjective.ObjectiveDescription = TEXT("Find a safe location and build shelter");
    ShelterObjective.MissionType = EQuest_SurvivalMissionType::ShelterBuilding;
    ShelterObjective.Priority = EQuest_MissionPriority::High;
    ShelterObjective.TimeLimit = 1200.0f; // 20 minutes
    ShelterObjective.TargetLocation = FVector(0.0f, -1000.0f, 150.0f); // Cave area
    ShelterObjective.CompletionRadius = 250.0f;

    AddObjective(ShelterObjective);
}

void AQuest_SurvivalMissionController::InitializeToolCraftingMission()
{
    MissionTitle = TEXT("Craft Basic Tools");
    MissionDescription = TEXT("Create essential tools for survival and hunting");

    FQuest_SurvivalObjective ToolObjective;
    ToolObjective.ObjectiveDescription = TEXT("Gather materials and craft a stone axe");
    ToolObjective.MissionType = EQuest_SurvivalMissionType::ToolCrafting;
    ToolObjective.Priority = EQuest_MissionPriority::Medium;
    ToolObjective.TimeLimit = 800.0f; // 13 minutes
    ToolObjective.TargetLocation = FVector(600.0f, 600.0f, 75.0f); // Rocky area
    ToolObjective.CompletionRadius = 350.0f;

    AddObjective(ToolObjective);
}

void AQuest_SurvivalMissionController::InitializeTerritoryMappingMission()
{
    MissionTitle = TEXT("Scout Territory");
    MissionDescription = TEXT("Explore and map the surrounding area for resources and dangers");

    FQuest_SurvivalObjective ScoutObjective;
    ScoutObjective.ObjectiveDescription = TEXT("Explore three key locations and identify threats");
    ScoutObjective.MissionType = EQuest_SurvivalMissionType::TerritoryMapping;
    ScoutObjective.Priority = EQuest_MissionPriority::Medium;
    ScoutObjective.TimeLimit = 1500.0f; // 25 minutes
    ScoutObjective.TargetLocation = FVector(0.0f, 0.0f, 200.0f); // High vantage point
    ScoutObjective.CompletionRadius = 500.0f;

    AddObjective(ScoutObjective);
}

void AQuest_SurvivalMissionController::InitializePredatorAvoidanceMission()
{
    MissionTitle = TEXT("Avoid Predators");
    MissionDescription = TEXT("Navigate safely through dangerous territory without alerting carnivores");

    FQuest_SurvivalObjective AvoidanceObjective;
    AvoidanceObjective.ObjectiveDescription = TEXT("Cross predator territory undetected");
    AvoidanceObjective.MissionType = EQuest_SurvivalMissionType::PredatorAvoidance;
    AvoidanceObjective.Priority = EQuest_MissionPriority::Critical;
    AvoidanceObjective.TimeLimit = 450.0f; // 7.5 minutes
    AvoidanceObjective.TargetLocation = FVector(-1200.0f, -800.0f, 80.0f); // Dangerous area
    AvoidanceObjective.CompletionRadius = 200.0f;

    AddObjective(AvoidanceObjective);
}