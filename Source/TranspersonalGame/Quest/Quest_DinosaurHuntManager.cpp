#include "Quest_DinosaurHuntManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMeshActor.h"

AQuest_DinosaurHuntManager::AQuest_DinosaurHuntManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize hunt management settings
    MaxActiveHunts = 3;
    HuntDetectionRadius = 5000.0f;
    PlayerTrackingUpdateInterval = 2.0f;
    PlayerTrackingTimer = 0.0f;
    
    // Initialize progression
    TotalHuntsCompleted = 0;
    HuntDifficultyMultiplier = 1.0f;
    
    // Initialize biome hunting zones (coordinates from memory)
    BiomeHuntingZones.Add(TEXT("Savana"), FVector(0.0f, 0.0f, 100.0f));
    BiomeHuntingZones.Add(TEXT("Forest"), FVector(-45000.0f, 40000.0f, 100.0f));
    BiomeHuntingZones.Add(TEXT("Desert"), FVector(55000.0f, 0.0f, 100.0f));
    BiomeHuntingZones.Add(TEXT("Swamp"), FVector(-50000.0f, -45000.0f, 100.0f));
    BiomeHuntingZones.Add(TEXT("Mountain"), FVector(40000.0f, 50000.0f, 100.0f));
    
    // Initialize biome dinosaur species
    TArray<FString> SavanaDinos;
    SavanaDinos.Add(TEXT("TRex"));
    SavanaDinos.Add(TEXT("Triceratops"));
    SavanaDinos.Add(TEXT("Velociraptor"));
    BiomeDinosaurSpecies.Add(TEXT("Savana"), SavanaDinos);
    
    TArray<FString> ForestDinos;
    ForestDinos.Add(TEXT("Brachiosaurus"));
    ForestDinos.Add(TEXT("Parasaurolophus"));
    ForestDinos.Add(TEXT("Protoceratops"));
    BiomeDinosaurSpecies.Add(TEXT("Forest"), ForestDinos);
    
    TArray<FString> DesertDinos;
    DesertDinos.Add(TEXT("Ankylosaurus"));
    DesertDinos.Add(TEXT("Pachycephalo"));
    BiomeDinosaurSpecies.Add(TEXT("Desert"), DesertDinos);
    
    TArray<FString> SwampDinos;
    SwampDinos.Add(TEXT("Tsintaosaurus"));
    SwampDinos.Add(TEXT("Parasaurolophus"));
    BiomeDinosaurSpecies.Add(TEXT("Swamp"), SwampDinos);
    
    TArray<FString> MountainDinos;
    MountainDinos.Add(TEXT("Triceratops"));
    MountainDinos.Add(TEXT("Ankylosaurus"));
    BiomeDinosaurSpecies.Add(TEXT("Mountain"), MountainDinos);
}

void AQuest_DinosaurHuntManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character
    PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        LastPlayerLocation = PlayerCharacter->GetActorLocation();
        UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Player character found at %s"), *LastPlayerLocation.ToString());
    }
    
    // Initialize biome hunts
    InitializeBiomeHunts();
    
    // Start with basic hunts
    StartNewHunt(TEXT("Velociraptor"), BiomeHuntingZones[TEXT("Savana")], 2);
    StartNewHunt(TEXT("Protoceratops"), BiomeHuntingZones[TEXT("Forest")], 1);
}

void AQuest_DinosaurHuntManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    PlayerTrackingTimer += DeltaTime;
    if (PlayerTrackingTimer >= PlayerTrackingUpdateInterval)
    {
        UpdatePlayerLocation();
        PlayerTrackingTimer = 0.0f;
    }
    
    UpdateHuntProgress(DeltaTime);
    CheckHuntCompletion();
}

void AQuest_DinosaurHuntManager::StartNewHunt(const FString& DinosaurSpecies, const FVector& TargetLocation, int32 RequiredKills)
{
    if (ActiveHuntTargets.Num() >= MaxActiveHunts)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Cannot start new hunt - maximum active hunts reached"));
        return;
    }
    
    FQuest_HuntTarget NewHunt;
    NewHunt.DinosaurSpecies = DinosaurSpecies;
    NewHunt.TargetLocation = TargetLocation;
    NewHunt.RequiredKills = RequiredKills;
    NewHunt.CurrentKills = 0;
    NewHunt.bIsCompleted = false;
    
    CalculateHuntDifficulty(NewHunt);
    
    ActiveHuntTargets.Add(NewHunt);
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Started new hunt for %s at %s (Difficulty: %.2f)"), 
           *DinosaurSpecies, *TargetLocation.ToString(), NewHunt.DifficultyLevel);
}

void AQuest_DinosaurHuntManager::RegisterDinosaurKill(const FString& DinosaurSpecies, const FVector& KillLocation)
{
    for (FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        if (Hunt.DinosaurSpecies == DinosaurSpecies && !Hunt.bIsCompleted)
        {
            // Check if kill is within hunt area
            float Distance = FVector::Dist(Hunt.TargetLocation, KillLocation);
            if (Distance <= HuntDetectionRadius)
            {
                Hunt.CurrentKills++;
                UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Kill registered for %s (%d/%d)"), 
                       *DinosaurSpecies, Hunt.CurrentKills, Hunt.RequiredKills);
                
                if (Hunt.CurrentKills >= Hunt.RequiredKills)
                {
                    CompleteHunt(DinosaurSpecies);
                }
                break;
            }
        }
    }
}

bool AQuest_DinosaurHuntManager::IsHuntCompleted(const FString& DinosaurSpecies)
{
    for (const FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        if (Hunt.DinosaurSpecies == DinosaurSpecies)
        {
            return Hunt.bIsCompleted;
        }
    }
    return false;
}

void AQuest_DinosaurHuntManager::CompleteHunt(const FString& DinosaurSpecies)
{
    for (int32 i = 0; i < ActiveHuntTargets.Num(); i++)
    {
        if (ActiveHuntTargets[i].DinosaurSpecies == DinosaurSpecies)
        {
            ActiveHuntTargets[i].bIsCompleted = true;
            
            // Grant reward
            GrantHuntReward(ActiveHuntTargets[i]);
            
            // Move to completed hunts
            CompletedHuntTargets.Add(ActiveHuntTargets[i]);
            ActiveHuntTargets.RemoveAt(i);
            
            TotalHuntsCompleted++;
            HuntDifficultyMultiplier += 0.1f;
            
            UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Hunt completed for %s! Total completed: %d"), 
                   *DinosaurSpecies, TotalHuntsCompleted);
            break;
        }
    }
}

void AQuest_DinosaurHuntManager::InitializeBiomeHunts()
{
    // Create default hunt rewards
    FQuest_HuntReward BasicReward;
    BasicReward.RewardType = TEXT("Basic Hunt");
    BasicReward.ExperiencePoints = 100;
    BasicReward.CraftingMaterials.Add(TEXT("Bone"));
    BasicReward.CraftingMaterials.Add(TEXT("Hide"));
    BasicReward.SurvivalBonus = 0.1f;
    HuntRewards.Add(BasicReward);
    
    FQuest_HuntReward AdvancedReward;
    AdvancedReward.RewardType = TEXT("Advanced Hunt");
    AdvancedReward.ExperiencePoints = 250;
    AdvancedReward.CraftingMaterials.Add(TEXT("Rare Bone"));
    AdvancedReward.CraftingMaterials.Add(TEXT("Thick Hide"));
    AdvancedReward.CraftingMaterials.Add(TEXT("Claw"));
    AdvancedReward.SurvivalBonus = 0.25f;
    HuntRewards.Add(AdvancedReward);
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Biome hunts initialized with %d biomes"), BiomeHuntingZones.Num());
}

TArray<FString> AQuest_DinosaurHuntManager::GetAvailableHuntsInBiome(const FString& BiomeName)
{
    TArray<FString> AvailableHunts;
    
    if (BiomeDinosaurSpecies.Contains(BiomeName))
    {
        TArray<FString> BiomeDinos = BiomeDinosaurSpecies[BiomeName];
        
        // Filter out species that already have active hunts
        for (const FString& Species : BiomeDinos)
        {
            bool bHasActiveHunt = false;
            for (const FQuest_HuntTarget& Hunt : ActiveHuntTargets)
            {
                if (Hunt.DinosaurSpecies == Species)
                {
                    bHasActiveHunt = true;
                    break;
                }
            }
            
            if (!bHasActiveHunt)
            {
                AvailableHunts.Add(Species);
            }
        }
    }
    
    return AvailableHunts;
}

void AQuest_DinosaurHuntManager::StartBiomeHunt(const FString& BiomeName, const FString& DinosaurSpecies)
{
    if (BiomeHuntingZones.Contains(BiomeName))
    {
        FVector BiomeLocation = BiomeHuntingZones[BiomeName];
        int32 RequiredKills = (DinosaurSpecies == TEXT("TRex")) ? 1 : 2; // T-Rex is harder
        StartNewHunt(DinosaurSpecies, BiomeLocation, RequiredKills);
    }
}

void AQuest_DinosaurHuntManager::UpdatePlayerLocation()
{
    if (PlayerCharacter)
    {
        FVector CurrentLocation = PlayerCharacter->GetActorLocation();
        LastPlayerLocation = CurrentLocation;
        
        // Check if player entered a new biome
        FString CurrentBiome = GetNearestBiome(CurrentLocation);
        UE_LOG(LogTemp, Log, TEXT("Hunt Manager: Player in %s biome at %s"), *CurrentBiome, *CurrentLocation.ToString());
    }
}

FString AQuest_DinosaurHuntManager::GetNearestBiome(const FVector& PlayerLocation)
{
    FString NearestBiome = TEXT("Unknown");
    float MinDistance = MAX_FLT;
    
    for (const auto& BiomePair : BiomeHuntingZones)
    {
        float Distance = FVector::Dist(PlayerLocation, BiomePair.Value);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestBiome = BiomePair.Key;
        }
    }
    
    return NearestBiome;
}

TArray<FQuest_HuntTarget> AQuest_DinosaurHuntManager::GetActiveHuntsNearPlayer(float SearchRadius)
{
    TArray<FQuest_HuntTarget> NearbyHunts;
    
    if (PlayerCharacter)
    {
        FVector PlayerLocation = PlayerCharacter->GetActorLocation();
        
        for (const FQuest_HuntTarget& Hunt : ActiveHuntTargets)
        {
            float Distance = FVector::Dist(PlayerLocation, Hunt.TargetLocation);
            if (Distance <= SearchRadius)
            {
                NearbyHunts.Add(Hunt);
            }
        }
    }
    
    return NearbyHunts;
}

void AQuest_DinosaurHuntManager::GrantHuntReward(const FQuest_HuntTarget& CompletedHunt)
{
    if (HuntRewards.Num() > 0)
    {
        int32 RewardIndex = (CompletedHunt.DifficultyLevel > 2.0f) ? 1 : 0;
        RewardIndex = FMath::Clamp(RewardIndex, 0, HuntRewards.Num() - 1);
        
        FQuest_HuntReward Reward = HuntRewards[RewardIndex];
        
        UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Granted %s reward - %d XP, %.2f survival bonus"), 
               *Reward.RewardType, Reward.ExperiencePoints, Reward.SurvivalBonus);
    }
}

void AQuest_DinosaurHuntManager::CalculateHuntDifficulty(FQuest_HuntTarget& HuntTarget)
{
    float BaseDifficulty = 1.0f;
    
    // Adjust difficulty based on species
    if (HuntTarget.DinosaurSpecies == TEXT("TRex"))
    {
        BaseDifficulty = 5.0f;
    }
    else if (HuntTarget.DinosaurSpecies == TEXT("Velociraptor"))
    {
        BaseDifficulty = 3.0f;
    }
    else if (HuntTarget.DinosaurSpecies == TEXT("Triceratops"))
    {
        BaseDifficulty = 2.5f;
    }
    else if (HuntTarget.DinosaurSpecies == TEXT("Ankylosaurus"))
    {
        BaseDifficulty = 2.0f;
    }
    else
    {
        BaseDifficulty = 1.5f; // Herbivores
    }
    
    // Apply global difficulty multiplier
    HuntTarget.DifficultyLevel = BaseDifficulty * HuntDifficultyMultiplier;
}

void AQuest_DinosaurHuntManager::UpdateHuntProgress(float DeltaTime)
{
    // Update hunt timers and check for dynamic events
    for (FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        // Could add time-based hunt mechanics here
    }
}

void AQuest_DinosaurHuntManager::CheckHuntCompletion()
{
    // Check for auto-completion conditions
    TArray<FString> CompletedHunts;
    
    for (const FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        if (Hunt.CurrentKills >= Hunt.RequiredKills && !Hunt.bIsCompleted)
        {
            CompletedHunts.Add(Hunt.DinosaurSpecies);
        }
    }
    
    for (const FString& Species : CompletedHunts)
    {
        CompleteHunt(Species);
    }
}

void AQuest_DinosaurHuntManager::SpawnHuntMarkers()
{
    // Could spawn visual markers for active hunts
}

void AQuest_DinosaurHuntManager::UpdateBiomeHuntAvailability()
{
    // Update which hunts are available based on player progression
}

void AQuest_DinosaurHuntManager::DebugSpawnHuntTargets()
{
    UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Debug spawning hunt targets"));
    
    for (const auto& BiomePair : BiomeHuntingZones)
    {
        TArray<FString> BiomeDinos = GetAvailableHuntsInBiome(BiomePair.Key);
        if (BiomeDinos.Num() > 0)
        {
            StartBiomeHunt(BiomePair.Key, BiomeDinos[0]);
        }
    }
}

void AQuest_DinosaurHuntManager::DebugPrintActiveHunts()
{
    UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: Active hunts (%d):"), ActiveHuntTargets.Num());
    
    for (const FQuest_HuntTarget& Hunt : ActiveHuntTargets)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s: %d/%d kills (Difficulty: %.2f)"), 
               *Hunt.DinosaurSpecies, Hunt.CurrentKills, Hunt.RequiredKills, Hunt.DifficultyLevel);
    }
}

void AQuest_DinosaurHuntManager::DebugClearAllHunts()
{
    ActiveHuntTargets.Empty();
    CompletedHuntTargets.Empty();
    TotalHuntsCompleted = 0;
    HuntDifficultyMultiplier = 1.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt Manager: All hunts cleared"));
}