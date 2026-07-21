#include "Quest_PrehistoricSurvivalMissions.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

AQuest_PrehistoricSurvivalMissions::AQuest_PrehistoricSurvivalMissions()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create mission trigger sphere
    MissionTriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MissionTriggerSphere"));
    RootComponent = MissionTriggerSphere;
    MissionTriggerSphere->SetSphereRadius(500.0f);
    MissionTriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MissionTriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    MissionTriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create mission marker mesh
    MissionMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissionMarker"));
    MissionMarker->SetupAttachment(RootComponent);
    MissionMarker->SetRelativeLocation(FVector(0, 0, 100));
    MissionMarker->SetRelativeScale3D(FVector(2.0f, 2.0f, 3.0f));

    // Initialize mission properties
    MissionType = EQuest_SurvivalMissionType::WaterSource;
    Difficulty = EQuest_SurvivalDifficulty::Novice;
    MissionTitle = TEXT("Survival Challenge");
    MissionDescription = TEXT("Complete this survival task to progress");
    MissionTimeLimit = 600.0f; // 10 minutes default
    bIsRepeatable = true;
    MaxCompletions = 5;

    // Initialize mission state
    bIsMissionActive = false;
    bIsMissionCompleted = false;
    MissionStartTime = 0.0f;
    RemainingTime = 0.0f;
    CompletionCount = 0;
    CurrentPlayer = nullptr;
    bMissionDataInitialized = false;
    LastValidationTime = 0.0f;

    // Bind trigger events
    MissionTriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_PrehistoricSurvivalMissions::OnMissionTriggerBeginOverlap);
    MissionTriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_PrehistoricSurvivalMissions::OnMissionTriggerEndOverlap);
}

void AQuest_PrehistoricSurvivalMissions::BeginPlay()
{
    Super::BeginPlay();
    InitializeMissionData();
    SetupMissionMarker();
}

void AQuest_PrehistoricSurvivalMissions::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMissionActive && CurrentPlayer)
    {
        UpdateMissionTimer();
        CheckMissionCompletion();
        
        // Validate player every 5 seconds
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastValidationTime > 5.0f)
        {
            ValidatePlayerSurvivalStats(CurrentPlayer);
            LastValidationTime = CurrentTime;
        }
    }
}

void AQuest_PrehistoricSurvivalMissions::InitializeMissionData()
{
    if (bMissionDataInitialized) return;

    // Set mission-specific data based on type
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::WaterSource:
            MissionTitle = TEXT("Find Clean Water");
            MissionDescription = TEXT("Locate and secure a reliable water source for survival");
            Requirements.RequiredItems.Add(TEXT("Empty Container"));
            Requirements.MinimumHealthPercentage = 30.0f;
            Rewards.ExperiencePoints = 150;
            Rewards.ItemRewards.Add(TEXT("Water Container"));
            break;

        case EQuest_SurvivalMissionType::ShelterBuilding:
            MissionTitle = TEXT("Build Shelter");
            MissionDescription = TEXT("Construct a basic shelter to protect from elements");
            Requirements.RequiredItems.Add(TEXT("Wood"));
            Requirements.RequiredItems.Add(TEXT("Stones"));
            Requirements.MinimumStaminaPercentage = 50.0f;
            Rewards.ExperiencePoints = 200;
            Rewards.UnlockedRecipe = TEXT("Advanced Shelter");
            break;

        case EQuest_SurvivalMissionType::FireCrafting:
            MissionTitle = TEXT("Create Fire");
            MissionDescription = TEXT("Master the art of fire-making for warmth and cooking");
            Requirements.RequiredItems.Add(TEXT("Dry Wood"));
            Requirements.RequiredItems.Add(TEXT("Tinder"));
            Requirements.bRequiresDaylight = false;
            Rewards.ExperiencePoints = 120;
            Rewards.UnlockedRecipe = TEXT("Cooking Fire");
            break;

        case EQuest_SurvivalMissionType::ToolMaking:
            MissionTitle = TEXT("Craft Essential Tools");
            MissionDescription = TEXT("Create basic tools necessary for survival");
            Requirements.RequiredItems.Add(TEXT("Stone"));
            Requirements.RequiredItems.Add(TEXT("Stick"));
            Requirements.RequiredSkillLevel = 2;
            Rewards.ExperiencePoints = 180;
            Rewards.ItemRewards.Add(TEXT("Stone Axe"));
            break;

        case EQuest_SurvivalMissionType::FoodGathering:
            MissionTitle = TEXT("Gather Food");
            MissionDescription = TEXT("Collect edible plants and hunt small game");
            Requirements.MinimumStaminaPercentage = 40.0f;
            Requirements.bRequiresDaylight = true;
            Rewards.ExperiencePoints = 140;
            Rewards.HealthRestoration = 50.0f;
            break;

        case EQuest_SurvivalMissionType::HuntingPreparation:
            MissionTitle = TEXT("Prepare for the Hunt");
            MissionDescription = TEXT("Ready weapons and plan strategy for hunting large prey");
            Requirements.RequiredItems.Add(TEXT("Spear"));
            Requirements.RequiredItems.Add(TEXT("Rope"));
            Requirements.RequiredSkillLevel = 3;
            Rewards.ExperiencePoints = 250;
            Rewards.UnlockedRecipe = TEXT("Hunting Trap"));
            break;

        case EQuest_SurvivalMissionType::TerritoryMapping:
            MissionTitle = TEXT("Map the Territory");
            MissionDescription = TEXT("Explore and document the surrounding area");
            Requirements.MinimumHealthPercentage = 60.0f;
            Requirements.MaximumDistanceFromShelter = 10000.0f;
            Rewards.ExperiencePoints = 300;
            Rewards.SkillPointsAwarded = 2;
            break;

        default:
            break;
    }

    // Adjust rewards based on difficulty
    float DifficultyMultiplier = 1.0f + (static_cast<int32>(Difficulty) * 0.25f);
    Rewards.ExperiencePoints = FMath::RoundToInt(Rewards.ExperiencePoints * DifficultyMultiplier);
    Rewards.SkillPointsAwarded = FMath::Max(1, FMath::RoundToInt(Rewards.SkillPointsAwarded * DifficultyMultiplier));

    bMissionDataInitialized = true;
}

void AQuest_PrehistoricSurvivalMissions::SetupMissionMarker()
{
    if (!MissionMarker) return;

    // Set marker color based on mission type
    FLinearColor MarkerColor = FLinearColor::Blue;
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::WaterSource:
            MarkerColor = FLinearColor::Blue;
            break;
        case EQuest_SurvivalMissionType::ShelterBuilding:
            MarkerColor = FLinearColor::Brown;
            break;
        case EQuest_SurvivalMissionType::FireCrafting:
            MarkerColor = FLinearColor::Red;
            break;
        case EQuest_SurvivalMissionType::ToolMaking:
            MarkerColor = FLinearColor::Gray;
            break;
        case EQuest_SurvivalMissionType::FoodGathering:
            MarkerColor = FLinearColor::Green;
            break;
        default:
            MarkerColor = FLinearColor::Yellow;
            break;
    }

    // Create dynamic material for marker
    if (UMaterialInterface* BaseMaterial = MissionMarker->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), MarkerColor);
            MissionMarker->SetMaterial(0, DynamicMaterial);
        }
    }
}

bool AQuest_PrehistoricSurvivalMissions::CanStartMission(APawn* Player)
{
    if (!Player || bIsMissionActive) return false;
    if (bIsMissionCompleted && !bIsRepeatable) return false;
    if (CompletionCount >= MaxCompletions && MaxCompletions > 0) return false;

    return CheckMissionRequirements(Player);
}

bool AQuest_PrehistoricSurvivalMissions::StartSurvivalMission(APawn* Player)
{
    if (!CanStartMission(Player)) return false;

    CurrentPlayer = Player;
    bIsMissionActive = true;
    bIsMissionCompleted = false;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    RemainingTime = MissionTimeLimit;

    // Setup mission timer
    GetWorld()->GetTimerManager().SetTimer(
        MissionTimerHandle,
        this,
        &AQuest_PrehistoricSurvivalMissions::UpdateMissionTimer,
        1.0f,
        true
    );

    // Initialize objectives based on mission type
    UpdateMissionObjectives();

    UE_LOG(LogTemp, Warning, TEXT("Started survival mission: %s"), *MissionTitle);
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::CompleteSurvivalMission()
{
    if (!bIsMissionActive || !CurrentPlayer) return false;

    bIsMissionActive = false;
    bIsMissionCompleted = true;
    CompletionCount++;

    // Clear timer
    GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);

    // Award rewards
    AwardMissionRewards(CurrentPlayer);

    UE_LOG(LogTemp, Warning, TEXT("Completed survival mission: %s"), *MissionTitle);
    return true;
}

void AQuest_PrehistoricSurvivalMissions::CancelSurvivalMission()
{
    if (!bIsMissionActive) return;

    bIsMissionActive = false;
    CurrentPlayer = nullptr;
    GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);

    UE_LOG(LogTemp, Warning, TEXT("Cancelled survival mission: %s"), *MissionTitle);
}

float AQuest_PrehistoricSurvivalMissions::GetMissionProgress()
{
    if (!bIsMissionActive) return 0.0f;

    // Calculate progress based on completed objectives
    int32 TotalObjectives = CompletedObjectives.Num() + RemainingObjectives.Num();
    if (TotalObjectives == 0) return 0.0f;

    return static_cast<float>(CompletedObjectives.Num()) / static_cast<float>(TotalObjectives);
}

bool AQuest_PrehistoricSurvivalMissions::CheckMissionRequirements(APawn* Player)
{
    if (!Player) return false;

    // Check basic requirements
    // Note: In a full implementation, you would check actual player stats
    // For now, we'll assume requirements are met
    
    return true;
}

void AQuest_PrehistoricSurvivalMissions::AwardMissionRewards(APawn* Player)
{
    if (!Player) return;

    // Award experience points
    UE_LOG(LogTemp, Warning, TEXT("Awarded %d experience points"), Rewards.ExperiencePoints);

    // Award items
    for (const FString& Item : Rewards.ItemRewards)
    {
        UE_LOG(LogTemp, Warning, TEXT("Awarded item: %s"), *Item);
    }

    // Restore health/stamina
    if (Rewards.HealthRestoration > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Restored %.1f health"), Rewards.HealthRestoration);
    }

    // Award skill points
    if (Rewards.SkillPointsAwarded > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Awarded %d skill points"), Rewards.SkillPointsAwarded);
    }

    // Unlock recipe
    if (!Rewards.UnlockedRecipe.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Unlocked recipe: %s"), *Rewards.UnlockedRecipe);
    }
}

FString AQuest_PrehistoricSurvivalMissions::GetMissionStatusText()
{
    if (!bIsMissionActive)
    {
        if (bIsMissionCompleted)
        {
            return FString::Printf(TEXT("Mission Completed (%d/%d)"), CompletionCount, MaxCompletions);
        }
        else
        {
            return TEXT("Mission Available");
        }
    }

    int32 MinutesRemaining = FMath::FloorToInt(RemainingTime / 60.0f);
    int32 SecondsRemaining = FMath::FloorToInt(RemainingTime) % 60;
    
    return FString::Printf(TEXT("Active - %02d:%02d remaining"), MinutesRemaining, SecondsRemaining);
}

void AQuest_PrehistoricSurvivalMissions::UpdateMissionObjectives()
{
    CompletedObjectives.Empty();
    RemainingObjectives.Empty();

    // Add objectives based on mission type
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::WaterSource:
            RemainingObjectives.Add(TEXT("Locate water source"));
            RemainingObjectives.Add(TEXT("Test water quality"));
            RemainingObjectives.Add(TEXT("Fill container"));
            break;

        case EQuest_SurvivalMissionType::ShelterBuilding:
            RemainingObjectives.Add(TEXT("Gather building materials"));
            RemainingObjectives.Add(TEXT("Choose shelter location"));
            RemainingObjectives.Add(TEXT("Construct basic frame"));
            RemainingObjectives.Add(TEXT("Add weatherproofing"));
            break;

        case EQuest_SurvivalMissionType::FireCrafting:
            RemainingObjectives.Add(TEXT("Gather dry tinder"));
            RemainingObjectives.Add(TEXT("Prepare kindling"));
            RemainingObjectives.Add(TEXT("Create spark"));
            RemainingObjectives.Add(TEXT("Maintain flame"));
            break;

        default:
            RemainingObjectives.Add(TEXT("Complete mission requirements"));
            break;
    }
}

bool AQuest_PrehistoricSurvivalMissions::IsPlayerInMissionArea(APawn* Player)
{
    if (!Player || !MissionTriggerSphere) return false;

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= MissionTriggerSphere->GetScaledSphereRadius();
}

void AQuest_PrehistoricSurvivalMissions::UpdateMissionTimer()
{
    if (!bIsMissionActive) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    RemainingTime = MissionTimeLimit - (CurrentTime - MissionStartTime);

    if (RemainingTime <= 0.0f)
    {
        HandleMissionTimeout();
    }
}

void AQuest_PrehistoricSurvivalMissions::CheckMissionCompletion()
{
    if (!bIsMissionActive) return;

    // Check if all objectives are completed
    if (RemainingObjectives.Num() == 0 && CompletedObjectives.Num() > 0)
    {
        CompleteSurvivalMission();
    }
}

void AQuest_PrehistoricSurvivalMissions::HandleMissionTimeout()
{
    UE_LOG(LogTemp, Warning, TEXT("Mission timeout: %s"), *MissionTitle);
    CancelSurvivalMission();
}

void AQuest_PrehistoricSurvivalMissions::ValidatePlayerSurvivalStats(APawn* Player)
{
    if (!Player) return;

    // In a full implementation, check player health, stamina, etc.
    // For now, just log validation
    UE_LOG(LogTemp, Log, TEXT("Validating player survival stats for mission: %s"), *MissionTitle);
}

// Specialized mission processors
bool AQuest_PrehistoricSurvivalMissions::ProcessWaterSourceMission()
{
    // Implement water source mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing water source mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessShelterBuildingMission()
{
    // Implement shelter building mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing shelter building mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessFireCraftingMission()
{
    // Implement fire crafting mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing fire crafting mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessToolMakingMission()
{
    // Implement tool making mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing tool making mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessFoodGatheringMission()
{
    // Implement food gathering mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing food gathering mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessHuntingPreparationMission()
{
    // Implement hunting preparation mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing hunting preparation mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessTerritoryMappingMission()
{
    // Implement territory mapping mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing territory mapping mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessResourceStockpilingMission()
{
    // Implement resource stockpiling mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing resource stockpiling mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessWeatherSurvivalMission()
{
    // Implement weather survival mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing weather survival mission"));
    return true;
}

bool AQuest_PrehistoricSurvivalMissions::ProcessInjuryTreatmentMission()
{
    // Implement injury treatment mission logic
    UE_LOG(LogTemp, Warning, TEXT("Processing injury treatment mission"));
    return true;
}

void AQuest_PrehistoricSurvivalMissions::OnMissionTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* Player = Cast<APawn>(OtherActor))
    {
        if (CanStartMission(Player))
        {
            UE_LOG(LogTemp, Warning, TEXT("Player entered mission area: %s"), *MissionTitle);
            // Could auto-start mission or show UI prompt here
        }
    }
}

void AQuest_PrehistoricSurvivalMissions::OnMissionTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (APawn* Player = Cast<APawn>(OtherActor))
    {
        if (Player == CurrentPlayer && bIsMissionActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player left mission area: %s"), *MissionTitle);
            // Could warn player or pause mission
        }
    }
}