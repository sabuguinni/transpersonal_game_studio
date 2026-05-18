#include "Quest_DinosaurEncounterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AQuest_DinosaurEncounterSystem::AQuest_DinosaurEncounterSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize encounter system parameters
    EncounterCheckRadius = 2000.0f;
    EncounterSpawnRate = 30.0f;
    MaxActiveEncounters = 5;
    EncounterTimer = 0.0f;
    CurrentDifficulty = 1.0f;
    EncounterCounter = 0;

    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AQuest_DinosaurEncounterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEncounterSystem();
    LoadEncounterData();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: System initialized with %d max encounters"), MaxActiveEncounters);
}

void AQuest_DinosaurEncounterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    EncounterTimer += DeltaTime;
    
    UpdateEncounterStates(DeltaTime);
    CheckEncounterTriggers();
    
    // Spawn new encounters based on timer
    if (EncounterTimer >= EncounterSpawnRate && ActiveEncounters.Num() < MaxActiveEncounters)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            TriggerRandomEncounter(PlayerLocation);
        }
        EncounterTimer = 0.0f;
    }
    
    // Clean up completed encounters every 60 seconds
    if (FMath::Fmod(GetWorld()->GetTimeSeconds(), 60.0f) < DeltaTime)
    {
        CleanupCompletedEncounters();
    }
}

void AQuest_DinosaurEncounterSystem::SpawnDinosaurEncounter(const FString& Species, EQuest_EncounterType Type, const FVector& Location)
{
    if (ActiveEncounters.Num() >= MaxActiveEncounters)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Maximum encounters reached, cannot spawn new encounter"));
        return;
    }

    FQuest_DinosaurEncounter NewEncounter;
    NewEncounter.DinosaurSpecies = Species;
    NewEncounter.EncounterType = Type;
    NewEncounter.SpawnLocation = Location;
    NewEncounter.bIsActiveEncounter = true;
    
    // Set encounter parameters based on species and type
    if (Species == TEXT("TRex"))
    {
        NewEncounter.DinosaurCount = 1;
        NewEncounter.ThreatLevel = 8.0f * CurrentDifficulty;
        NewEncounter.RewardPoints = 200.0f * CurrentDifficulty;
    }
    else if (Species == TEXT("Velociraptor"))
    {
        NewEncounter.DinosaurCount = FMath::RandRange(2, 4);
        NewEncounter.ThreatLevel = 6.0f * CurrentDifficulty;
        NewEncounter.RewardPoints = 150.0f * CurrentDifficulty;
    }
    else if (Species == TEXT("Triceratops"))
    {
        NewEncounter.DinosaurCount = FMath::RandRange(1, 2);
        NewEncounter.ThreatLevel = 5.0f * CurrentDifficulty;
        NewEncounter.RewardPoints = 120.0f * CurrentDifficulty;
    }
    else
    {
        NewEncounter.DinosaurCount = 1;
        NewEncounter.ThreatLevel = 4.0f * CurrentDifficulty;
        NewEncounter.RewardPoints = 100.0f * CurrentDifficulty;
    }

    ActiveEncounters.Add(NewEncounter);
    
    // Set up reward for this encounter
    FQuest_EncounterReward Reward;
    Reward.ExperiencePoints = FMath::RoundToInt(NewEncounter.RewardPoints * 0.5f);
    Reward.SurvivalPoints = NewEncounter.RewardPoints * 0.3f;
    
    if (Type == EQuest_EncounterType::Hunt)
    {
        Reward.ItemRewards.Add(TEXT("Dinosaur_Meat"));
        Reward.ItemRewards.Add(TEXT("Dinosaur_Hide"));
    }
    else if (Type == EQuest_EncounterType::Observe)
    {
        Reward.ItemRewards.Add(TEXT("Knowledge_Points"));
    }
    
    FString EncounterId = GenerateEncounterId();
    EncounterRewards.Add(EncounterId, Reward);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Spawned %s encounter at %s"), *Species, *Location.ToString());
}

void AQuest_DinosaurEncounterSystem::CompleteEncounter(const FString& EncounterId)
{
    // Find and remove the encounter
    for (int32 i = ActiveEncounters.Num() - 1; i >= 0; i--)
    {
        if (ActiveEncounters[i].bIsActiveEncounter)
        {
            ActiveEncounters[i].bIsActiveEncounter = false;
            
            // Award rewards
            if (EncounterRewards.Contains(EncounterId))
            {
                FQuest_EncounterReward Reward = EncounterRewards[EncounterId];
                UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Encounter completed! Awarded %d XP, %.1f survival points"), 
                       Reward.ExperiencePoints, Reward.SurvivalPoints);
                
                // Here you would integrate with player progression system
                // For now, just log the rewards
                for (const FString& Item : Reward.ItemRewards)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Awarded item: %s"), *Item);
                }
            }
            break;
        }
    }
    
    SaveEncounterProgress();
}

bool AQuest_DinosaurEncounterSystem::CheckPlayerNearEncounter(const FVector& PlayerLocation, float CheckDistance)
{
    for (const FQuest_DinosaurEncounter& Encounter : ActiveEncounters)
    {
        if (Encounter.bIsActiveEncounter)
        {
            float Distance = FVector::Dist(PlayerLocation, Encounter.SpawnLocation);
            if (Distance <= CheckDistance)
            {
                return true;
            }
        }
    }
    return false;
}

TArray<FQuest_DinosaurEncounter> AQuest_DinosaurEncounterSystem::GetActiveEncounters() const
{
    TArray<FQuest_DinosaurEncounter> Active;
    for (const FQuest_DinosaurEncounter& Encounter : ActiveEncounters)
    {
        if (Encounter.bIsActiveEncounter)
        {
            Active.Add(Encounter);
        }
    }
    return Active;
}

void AQuest_DinosaurEncounterSystem::TriggerRandomEncounter(const FVector& PlayerLocation)
{
    // Define possible encounter types and species
    TArray<FString> DinosaurSpecies = {TEXT("TRex"), TEXT("Velociraptor"), TEXT("Triceratops"), TEXT("Parasaurolophus")};
    TArray<EQuest_EncounterType> EncounterTypes = {EQuest_EncounterType::Hunt, EQuest_EncounterType::Observe, EQuest_EncounterType::Escape};
    
    // Select random species and type
    FString SelectedSpecies = DinosaurSpecies[FMath::RandRange(0, DinosaurSpecies.Num() - 1)];
    EQuest_EncounterType SelectedType = EncounterTypes[FMath::RandRange(0, EncounterTypes.Num() - 1)];
    
    // Generate spawn location around player (500-1500 units away)
    float SpawnDistance = FMath::RandRange(500.0f, 1500.0f);
    float SpawnAngle = FMath::RandRange(0.0f, 360.0f);
    
    FVector SpawnOffset = FVector(
        FMath::Cos(FMath::DegreesToRadians(SpawnAngle)) * SpawnDistance,
        FMath::Sin(FMath::DegreesToRadians(SpawnAngle)) * SpawnDistance,
        0.0f
    );
    
    FVector SpawnLocation = PlayerLocation + SpawnOffset;
    SpawnLocation.Z = 100.0f; // Ensure spawn above ground
    
    SpawnDinosaurEncounter(SelectedSpecies, SelectedType, SpawnLocation);
}

void AQuest_DinosaurEncounterSystem::SetEncounterDifficulty(float NewDifficulty)
{
    CurrentDifficulty = FMath::Clamp(NewDifficulty, 0.5f, 3.0f);
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Difficulty set to %.2f"), CurrentDifficulty);
}

FQuest_EncounterReward AQuest_DinosaurEncounterSystem::GetEncounterReward(const FString& EncounterId) const
{
    if (EncounterRewards.Contains(EncounterId))
    {
        return EncounterRewards[EncounterId];
    }
    return FQuest_EncounterReward();
}

void AQuest_DinosaurEncounterSystem::InitializeEncounterSystem()
{
    ActiveEncounters.Empty();
    EncounterRewards.Empty();
    EncounterTimer = 0.0f;
    EncounterCounter = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Encounter system initialized"));
}

void AQuest_DinosaurEncounterSystem::CleanupCompletedEncounters()
{
    int32 RemovedCount = 0;
    
    // Remove inactive encounters
    for (int32 i = ActiveEncounters.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEncounters[i].bIsActiveEncounter)
        {
            ActiveEncounters.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Cleaned up %d completed encounters"), RemovedCount);
    }
}

void AQuest_DinosaurEncounterSystem::UpdateEncounterStates(float DeltaTime)
{
    // Update encounter logic here
    // For now, just ensure encounters stay active
    for (FQuest_DinosaurEncounter& Encounter : ActiveEncounters)
    {
        if (Encounter.bIsActiveEncounter)
        {
            // Update encounter state based on player proximity, time, etc.
            // This is where you'd add more complex encounter behavior
        }
    }
}

void AQuest_DinosaurEncounterSystem::CheckEncounterTriggers()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // Check if player is near any encounters
    for (FQuest_DinosaurEncounter& Encounter : ActiveEncounters)
    {
        if (Encounter.bIsActiveEncounter)
        {
            float Distance = FVector::Dist(PlayerLocation, Encounter.SpawnLocation);
            if (Distance <= EncounterCheckRadius)
            {
                // Trigger encounter interaction
                UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Player near %s encounter at distance %.1f"), 
                       *Encounter.DinosaurSpecies, Distance);
            }
        }
    }
}

FString AQuest_DinosaurEncounterSystem::GenerateEncounterId() const
{
    return FString::Printf(TEXT("Encounter_%d_%d"), EncounterCounter, FMath::RandRange(1000, 9999));
}

void AQuest_DinosaurEncounterSystem::LoadEncounterData()
{
    // Load encounter configuration from data tables or config files
    // For now, use default values
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Encounter data loaded"));
}

void AQuest_DinosaurEncounterSystem::SaveEncounterProgress()
{
    // Save encounter progress to persistent storage
    // For now, just log the save operation
    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurEncounterSystem: Encounter progress saved"));
}