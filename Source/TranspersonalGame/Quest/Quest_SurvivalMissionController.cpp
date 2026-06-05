#include "Quest_SurvivalMissionController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AQuest_SurvivalMissionController::AQuest_SurvivalMissionController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mission marker mesh
    MissionMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissionMarkerMesh"));
    MissionMarkerMesh->SetupAttachment(RootComponent);
    MissionMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default mission
    CurrentMission.MissionType = EQuest_SurvivalMissionType::Hunt_Raptor;
    CurrentMission.Difficulty = EQuest_MissionDifficulty::Beginner;
    CurrentMission.MissionTitle = TEXT("First Hunt");
    CurrentMission.MissionDescription = TEXT("Hunt a raptor to prove your survival skills");
    CurrentMission.RequiredQuantity = 1;
    CurrentMission.TimeLimit = 600.0f; // 10 minutes
    CurrentMission.ExperienceReward = 150;
}

void AQuest_SurvivalMissionController::BeginPlay()
{
    Super::BeginPlay();

    InitializeMissionDatabase();
    SetupMissionMarker();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_SurvivalMissionController::OnInteractionSphereBeginOverlap);
    }

    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController initialized with %d available missions"), AvailableMissions.Num());
}

void AQuest_SurvivalMissionController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bMissionActive)
    {
        float ElapsedTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
        
        // Check for mission timeout
        if (ElapsedTime >= CurrentMission.TimeLimit)
        {
            FailMission();
        }

        // Update visual indicators
        UpdateMissionMarkerVisual();
    }
}

void AQuest_SurvivalMissionController::StartMission(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty)
{
    if (bMissionActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new mission - mission already active"));
        return;
    }

    CurrentMission = CreateMissionData(MissionType, Difficulty);
    bMissionActive = true;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    CurrentProgress = 0;

    UE_LOG(LogTemp, Warning, TEXT("Mission Started: %s"), *CurrentMission.MissionTitle);

    // Notify game systems
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Mission Started: %s"), *CurrentMission.MissionTitle));
    }

    UpdateMissionMarkerVisual();
}

void AQuest_SurvivalMissionController::CompleteMission()
{
    if (!bMissionActive)
    {
        return;
    }

    bMissionActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Mission Completed: %s - Reward: %d XP"), 
        *CurrentMission.MissionTitle, CurrentMission.ExperienceReward);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Mission Complete! +%d XP"), CurrentMission.ExperienceReward));
    }

    // Auto-generate next mission after completion
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        GenerateRandomMission();
    }, 3.0f, false);

    UpdateMissionMarkerVisual();
}

void AQuest_SurvivalMissionController::FailMission()
{
    if (!bMissionActive)
    {
        return;
    }

    bMissionActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Mission Failed: %s"), *CurrentMission.MissionTitle);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            FString::Printf(TEXT("Mission Failed: %s"), *CurrentMission.MissionTitle));
    }

    // Generate easier mission after failure
    EQuest_MissionDifficulty NewDifficulty = CurrentMission.Difficulty;
    if (NewDifficulty > EQuest_MissionDifficulty::Beginner)
    {
        NewDifficulty = static_cast<EQuest_MissionDifficulty>(static_cast<uint8>(NewDifficulty) - 1);
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, NewDifficulty]()
    {
        GenerateDifficultyBasedMission(NewDifficulty);
    }, 5.0f, false);

    UpdateMissionMarkerVisual();
}

void AQuest_SurvivalMissionController::UpdateProgress(int32 ProgressAmount)
{
    if (!bMissionActive)
    {
        return;
    }

    CurrentProgress += ProgressAmount;
    
    UE_LOG(LogTemp, Warning, TEXT("Mission Progress: %d/%d"), CurrentProgress, CurrentMission.RequiredQuantity);

    if (IsMissionComplete())
    {
        CompleteMission();
    }
}

bool AQuest_SurvivalMissionController::IsMissionComplete() const
{
    return bMissionActive && (CurrentProgress >= CurrentMission.RequiredQuantity);
}

float AQuest_SurvivalMissionController::GetRemainingTime() const
{
    if (!bMissionActive)
    {
        return 0.0f;
    }

    float ElapsedTime = GetWorld()->GetTimeSeconds() - MissionStartTime;
    return FMath::Max(0.0f, CurrentMission.TimeLimit - ElapsedTime);
}

FString AQuest_SurvivalMissionController::GetMissionStatusText() const
{
    if (!bMissionActive)
    {
        return TEXT("No Active Mission");
    }

    float RemainingTime = GetRemainingTime();
    int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
    int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;

    return FString::Printf(TEXT("%s - Progress: %d/%d - Time: %02d:%02d"), 
        *CurrentMission.MissionTitle, CurrentProgress, CurrentMission.RequiredQuantity, Minutes, Seconds);
}

void AQuest_SurvivalMissionController::GenerateRandomMission()
{
    if (AvailableMissions.Num() == 0)
    {
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, AvailableMissions.Num() - 1);
    FQuest_SurvivalMissionData SelectedMission = AvailableMissions[RandomIndex];

    StartMission(SelectedMission.MissionType, SelectedMission.Difficulty);
}

void AQuest_SurvivalMissionController::GenerateDifficultyBasedMission(EQuest_MissionDifficulty TargetDifficulty)
{
    TArray<FQuest_SurvivalMissionData> FilteredMissions;
    
    for (const FQuest_SurvivalMissionData& Mission : AvailableMissions)
    {
        if (Mission.Difficulty == TargetDifficulty)
        {
            FilteredMissions.Add(Mission);
        }
    }

    if (FilteredMissions.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, FilteredMissions.Num() - 1);
        FQuest_SurvivalMissionData SelectedMission = FilteredMissions[RandomIndex];
        StartMission(SelectedMission.MissionType, SelectedMission.Difficulty);
    }
    else
    {
        GenerateRandomMission();
    }
}

void AQuest_SurvivalMissionController::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Check if it's the player character
    if (OtherActor->IsA<APawn>())
    {
        if (!bMissionActive)
        {
            GenerateRandomMission();
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, GetMissionStatusText());
            }
        }
    }
}

void AQuest_SurvivalMissionController::InitializeMissionDatabase()
{
    AvailableMissions.Empty();

    // Hunt missions
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Hunt_Raptor, EQuest_MissionDifficulty::Beginner));
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Hunt_Raptor, EQuest_MissionDifficulty::Intermediate));
    
    // Gather missions
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Gather_Resources, EQuest_MissionDifficulty::Beginner));
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Gather_Resources, EQuest_MissionDifficulty::Novice));
    
    // Build missions
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Build_Shelter, EQuest_MissionDifficulty::Novice));
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Build_Shelter, EQuest_MissionDifficulty::Advanced));
    
    // Exploration missions
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Explore_Cave, EQuest_MissionDifficulty::Intermediate));
    AvailableMissions.Add(CreateMissionData(EQuest_SurvivalMissionType::Find_Water, EQuest_MissionDifficulty::Beginner));

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d survival missions"), AvailableMissions.Num());
}

void AQuest_SurvivalMissionController::SetupMissionMarker()
{
    if (MissionMarkerMesh)
    {
        // Set default scale and visibility
        MissionMarkerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 3.0f));
        MissionMarkerMesh->SetVisibility(true);
    }
}

void AQuest_SurvivalMissionController::UpdateMissionMarkerVisual()
{
    if (!MissionMarkerMesh)
    {
        return;
    }

    if (bMissionActive)
    {
        // Pulsing effect for active missions
        float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 0.3f + 1.0f;
        MissionMarkerMesh->SetWorldScale3D(FVector(2.0f * PulseValue, 2.0f * PulseValue, 3.0f));
    }
    else
    {
        // Static scale for inactive missions
        MissionMarkerMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 3.0f));
    }
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionController::CreateMissionData(EQuest_SurvivalMissionType Type, EQuest_MissionDifficulty Difficulty)
{
    FQuest_SurvivalMissionData MissionData;
    MissionData.MissionType = Type;
    MissionData.Difficulty = Difficulty;

    // Configure mission based on type and difficulty
    switch (Type)
    {
        case EQuest_SurvivalMissionType::Hunt_Raptor:
            MissionData.MissionTitle = TEXT("Raptor Hunt");
            MissionData.MissionDescription = TEXT("Hunt and kill raptors to secure the area");
            MissionData.RequiredQuantity = (Difficulty == EQuest_MissionDifficulty::Beginner) ? 1 : 3;
            MissionData.TimeLimit = 600.0f + (static_cast<int32>(Difficulty) * 300.0f);
            MissionData.ExperienceReward = 100 + (static_cast<int32>(Difficulty) * 50);
            break;

        case EQuest_SurvivalMissionType::Gather_Resources:
            MissionData.MissionTitle = TEXT("Resource Gathering");
            MissionData.MissionDescription = TEXT("Collect essential survival resources");
            MissionData.RequiredQuantity = 5 + (static_cast<int32>(Difficulty) * 3);
            MissionData.TimeLimit = 300.0f + (static_cast<int32>(Difficulty) * 120.0f);
            MissionData.ExperienceReward = 75 + (static_cast<int32>(Difficulty) * 25);
            MissionData.RequiredItems.Add(TEXT("Stone"));
            MissionData.RequiredItems.Add(TEXT("Wood"));
            break;

        case EQuest_SurvivalMissionType::Build_Shelter:
            MissionData.MissionTitle = TEXT("Shelter Construction");
            MissionData.MissionDescription = TEXT("Build a shelter to protect from the elements");
            MissionData.RequiredQuantity = 1;
            MissionData.TimeLimit = 900.0f + (static_cast<int32>(Difficulty) * 300.0f);
            MissionData.ExperienceReward = 200 + (static_cast<int32>(Difficulty) * 100);
            break;

        case EQuest_SurvivalMissionType::Find_Water:
            MissionData.MissionTitle = TEXT("Water Source");
            MissionData.MissionDescription = TEXT("Locate a clean water source for survival");
            MissionData.RequiredQuantity = 1;
            MissionData.TimeLimit = 450.0f;
            MissionData.ExperienceReward = 125;
            break;

        case EQuest_SurvivalMissionType::Craft_Tools:
            MissionData.MissionTitle = TEXT("Tool Crafting");
            MissionData.MissionDescription = TEXT("Craft essential tools for survival");
            MissionData.RequiredQuantity = 2 + static_cast<int32>(Difficulty);
            MissionData.TimeLimit = 600.0f + (static_cast<int32>(Difficulty) * 180.0f);
            MissionData.ExperienceReward = 150 + (static_cast<int32>(Difficulty) * 50);
            break;

        case EQuest_SurvivalMissionType::Escape_Predator:
            MissionData.MissionTitle = TEXT("Predator Escape");
            MissionData.MissionDescription = TEXT("Survive and escape from dangerous predators");
            MissionData.RequiredQuantity = 1;
            MissionData.TimeLimit = 180.0f;
            MissionData.ExperienceReward = 300;
            break;

        case EQuest_SurvivalMissionType::Explore_Cave:
            MissionData.MissionTitle = TEXT("Cave Exploration");
            MissionData.MissionDescription = TEXT("Explore dangerous caves for valuable resources");
            MissionData.RequiredQuantity = 1;
            MissionData.TimeLimit = 720.0f + (static_cast<int32>(Difficulty) * 240.0f);
            MissionData.ExperienceReward = 250 + (static_cast<int32>(Difficulty) * 75);
            break;
    }

    return MissionData;
}