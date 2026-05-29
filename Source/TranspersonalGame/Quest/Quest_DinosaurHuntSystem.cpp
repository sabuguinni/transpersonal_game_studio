#include "Quest_DinosaurHuntSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

UQuest_DinosaurHuntSystem::UQuest_DinosaurHuntSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize hunt configuration
    MaxHuntDistance = 50000.0f;
    TrackingUpdateInterval = 5.0f;
    bShowHuntMarkers = true;

    // Initialize reward multipliers
    MeatMultiplier = 1.0f;
    BoneMultiplier = 1.0f;
    HideMultiplier = 1.0f;

    // Initialize calculated rewards
    CalculatedMeatReward = 0;
    CalculatedBoneReward = 0;
    CalculatedHideReward = 0;

    TrackingTimer = 0.0f;
}

void UQuest_DinosaurHuntSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Hunt system initialized"));
    
    // Initialize current mission with default values
    CurrentMission = FQuest_HuntMission();
    CurrentMission.MissionName = TEXT("No Active Hunt");
    CurrentMission.bIsActive = false;
    CurrentMission.bIsCompleted = false;
}

void UQuest_DinosaurHuntSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentMission.bIsActive && !CurrentMission.bIsCompleted)
    {
        UpdateHuntTimer(DeltaTime);
        
        TrackingTimer += DeltaTime;
        if (TrackingTimer >= TrackingUpdateInterval)
        {
            TrackingTimer = 0.0f;
            // Update target tracking logic here
        }
    }
}

void UQuest_DinosaurHuntSystem::StartHuntMission(EQuest_DinosaurSpecies Species, EQuest_HuntDifficulty Difficulty, FVector TargetLocation)
{
    if (CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurHuntSystem: Cannot start new hunt - mission already active"));
        return;
    }

    // Initialize new hunt mission
    CurrentMission = FQuest_HuntMission();
    CurrentMission.MissionName = FString::Printf(TEXT("Hunt %s"), *GetSpeciesDisplayName(Species));
    CurrentMission.Target.Species = Species;
    CurrentMission.Target.LastKnownLocation = TargetLocation;
    CurrentMission.Difficulty = Difficulty;
    CurrentMission.StartLocation = GetOwner()->GetActorLocation();
    CurrentMission.bIsActive = true;
    CurrentMission.bIsCompleted = false;

    // Scale difficulty based on species
    ScaleDifficultyBySpecies(Species);

    // Set required tools based on difficulty
    CurrentMission.RequiredTools.Empty();
    switch (Difficulty)
    {
        case EQuest_HuntDifficulty::Scavenge:
            CurrentMission.RequiredTools.Add(TEXT("Stone Knife"));
            CurrentMission.TimeLimit = 900.0f; // 15 minutes
            break;
        case EQuest_HuntDifficulty::Ambush:
            CurrentMission.RequiredTools.Add(TEXT("Spear"));
            CurrentMission.RequiredTools.Add(TEXT("Stone Axe"));
            CurrentMission.TimeLimit = 1200.0f; // 20 minutes
            break;
        case EQuest_HuntDifficulty::Hunt:
            CurrentMission.RequiredTools.Add(TEXT("Spear"));
            CurrentMission.RequiredTools.Add(TEXT("Bow"));
            CurrentMission.RequiredTools.Add(TEXT("Stone Arrows"));
            CurrentMission.TimeLimit = 1800.0f; // 30 minutes
            break;
        case EQuest_HuntDifficulty::PackHunt:
            CurrentMission.RequiredTools.Add(TEXT("Spear"));
            CurrentMission.RequiredTools.Add(TEXT("Bow"));
            CurrentMission.RequiredTools.Add(TEXT("Stone Arrows"));
            CurrentMission.RequiredTools.Add(TEXT("Trap"));
            CurrentMission.TimeLimit = 2400.0f; // 40 minutes
            break;
        case EQuest_HuntDifficulty::ApexPredator:
            CurrentMission.RequiredTools.Add(TEXT("Reinforced Spear"));
            CurrentMission.RequiredTools.Add(TEXT("Composite Bow"));
            CurrentMission.RequiredTools.Add(TEXT("Poison Arrows"));
            CurrentMission.RequiredTools.Add(TEXT("Pit Trap"));
            CurrentMission.TimeLimit = 3600.0f; // 60 minutes
            break;
    }

    CurrentMission.RemainingTime = CurrentMission.TimeLimit;

    UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Started hunt mission - %s"), *CurrentMission.MissionName);
    
    // Calculate potential rewards
    CalculateHuntRewards();
}

void UQuest_DinosaurHuntSystem::CompleteHuntMission()
{
    if (!CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurHuntSystem: No active hunt to complete"));
        return;
    }

    CurrentMission.bIsCompleted = true;
    CurrentMission.bIsActive = false;

    UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Hunt mission completed - %s"), *CurrentMission.MissionName);
    UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Rewards - Meat: %d, Bone: %d, Hide: %d"), 
           CalculatedMeatReward, CalculatedBoneReward, CalculatedHideReward);
}

void UQuest_DinosaurHuntSystem::CancelHuntMission()
{
    if (!CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurHuntSystem: No active hunt to cancel"));
        return;
    }

    CurrentMission.bIsActive = false;
    CurrentMission.bIsCompleted = false;
    CurrentMission.MissionName = TEXT("Hunt Cancelled");

    UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Hunt mission cancelled"));
}

bool UQuest_DinosaurHuntSystem::IsHuntActive() const
{
    return CurrentMission.bIsActive;
}

float UQuest_DinosaurHuntSystem::GetRemainingTime() const
{
    return CurrentMission.RemainingTime;
}

void UQuest_DinosaurHuntSystem::UpdateTargetLocation(FVector NewLocation)
{
    if (CurrentMission.bIsActive)
    {
        CurrentMission.Target.LastKnownLocation = NewLocation;
        UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Target location updated"));
    }
}

FVector UQuest_DinosaurHuntSystem::GetTargetLocation() const
{
    return CurrentMission.Target.LastKnownLocation;
}

float UQuest_DinosaurHuntSystem::GetDistanceToTarget() const
{
    if (!GetOwner())
    {
        return -1.0f;
    }

    FVector PlayerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = CurrentMission.Target.LastKnownLocation;
    
    return FVector::Dist(PlayerLocation, TargetLocation);
}

bool UQuest_DinosaurHuntSystem::ValidateHuntSuccess(AActor* KilledDinosaur)
{
    if (!CurrentMission.bIsActive || !KilledDinosaur)
    {
        return false;
    }

    // Check if the killed dinosaur matches the target species
    FString DinosaurName = KilledDinosaur->GetName();
    FString TargetSpecies = GetSpeciesDisplayName(CurrentMission.Target.Species);

    // Simple name matching - in a real implementation, you'd check the dinosaur's species component
    bool bSpeciesMatch = DinosaurName.Contains(TargetSpecies);

    if (bSpeciesMatch)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurHuntSystem: Hunt target validated - %s"), *DinosaurName);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurHuntSystem: Hunt target mismatch - killed %s, needed %s"), 
           *DinosaurName, *TargetSpecies);
    return false;
}

bool UQuest_DinosaurHuntSystem::CheckRequiredTools()
{
    // In a real implementation, this would check the player's inventory
    // For now, we'll assume the player has the tools if they started the hunt
    return true;
}

void UQuest_DinosaurHuntSystem::ScaleDifficultyBySpecies(EQuest_DinosaurSpecies Species)
{
    switch (Species)
    {
        case EQuest_DinosaurSpecies::TRex:
            CurrentMission.Target.ThreatLevel = 10.0f;
            CurrentMission.Target.PackSize = 1;
            CurrentMission.Target.TrackingRadius = 8000.0f;
            break;
        case EQuest_DinosaurSpecies::Velociraptor:
            CurrentMission.Target.ThreatLevel = 7.0f;
            CurrentMission.Target.PackSize = 3;
            CurrentMission.Target.TrackingRadius = 6000.0f;
            break;
        case EQuest_DinosaurSpecies::Triceratops:
            CurrentMission.Target.ThreatLevel = 6.0f;
            CurrentMission.Target.PackSize = 2;
            CurrentMission.Target.TrackingRadius = 5000.0f;
            break;
        case EQuest_DinosaurSpecies::Brachiosaurus:
            CurrentMission.Target.ThreatLevel = 4.0f;
            CurrentMission.Target.PackSize = 1;
            CurrentMission.Target.TrackingRadius = 10000.0f;
            break;
        case EQuest_DinosaurSpecies::Ankylosaurus:
            CurrentMission.Target.ThreatLevel = 5.0f;
            CurrentMission.Target.PackSize = 1;
            CurrentMission.Target.TrackingRadius = 4000.0f;
            break;
        case EQuest_DinosaurSpecies::Parasaurolophus:
            CurrentMission.Target.ThreatLevel = 3.0f;
            CurrentMission.Target.PackSize = 4;
            CurrentMission.Target.TrackingRadius = 7000.0f;
            break;
        default:
            CurrentMission.Target.ThreatLevel = 5.0f;
            CurrentMission.Target.PackSize = 1;
            CurrentMission.Target.TrackingRadius = 5000.0f;
            break;
    }
}

FString UQuest_DinosaurHuntSystem::GetHuntInstructions() const
{
    if (!CurrentMission.bIsActive)
    {
        return TEXT("No active hunt mission");
    }

    FString Instructions = FString::Printf(TEXT("Hunt Mission: %s\n"), *CurrentMission.MissionName);
    Instructions += FString::Printf(TEXT("Target: %s\n"), *GetSpeciesDisplayName(CurrentMission.Target.Species));
    Instructions += FString::Printf(TEXT("Difficulty: %s\n"), *GetDifficultyDisplayName(CurrentMission.Difficulty));
    Instructions += FString::Printf(TEXT("Time Remaining: %.0f seconds\n"), CurrentMission.RemainingTime);
    Instructions += FString::Printf(TEXT("Distance to Target: %.0fm\n"), GetDistanceToTarget());
    
    Instructions += TEXT("Required Tools:\n");
    for (const FString& Tool : CurrentMission.RequiredTools)
    {
        Instructions += FString::Printf(TEXT("- %s\n"), *Tool);
    }

    return Instructions;
}

void UQuest_DinosaurHuntSystem::CalculateHuntRewards()
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }

    // Base rewards by species
    int32 BaseMeat = 10;
    int32 BaseBone = 5;
    int32 BaseHide = 3;

    switch (CurrentMission.Target.Species)
    {
        case EQuest_DinosaurSpecies::TRex:
            BaseMeat = 100;
            BaseBone = 50;
            BaseHide = 25;
            break;
        case EQuest_DinosaurSpecies::Velociraptor:
            BaseMeat = 30;
            BaseBone = 15;
            BaseHide = 10;
            break;
        case EQuest_DinosaurSpecies::Triceratops:
            BaseMeat = 80;
            BaseBone = 40;
            BaseHide = 20;
            break;
        case EQuest_DinosaurSpecies::Brachiosaurus:
            BaseMeat = 150;
            BaseBone = 75;
            BaseHide = 35;
            break;
        case EQuest_DinosaurSpecies::Ankylosaurus:
            BaseMeat = 60;
            BaseBone = 30;
            BaseHide = 15;
            break;
        case EQuest_DinosaurSpecies::Parasaurolophus:
            BaseMeat = 40;
            BaseBone = 20;
            BaseHide = 12;
            break;
    }

    // Apply difficulty multiplier
    float DifficultyMultiplier = 1.0f;
    switch (CurrentMission.Difficulty)
    {
        case EQuest_HuntDifficulty::Scavenge:
            DifficultyMultiplier = 0.5f;
            break;
        case EQuest_HuntDifficulty::Ambush:
            DifficultyMultiplier = 0.8f;
            break;
        case EQuest_HuntDifficulty::Hunt:
            DifficultyMultiplier = 1.0f;
            break;
        case EQuest_HuntDifficulty::PackHunt:
            DifficultyMultiplier = 1.5f;
            break;
        case EQuest_HuntDifficulty::ApexPredator:
            DifficultyMultiplier = 2.0f;
            break;
    }

    // Calculate final rewards
    CalculatedMeatReward = FMath::RoundToInt(BaseMeat * DifficultyMultiplier * MeatMultiplier);
    CalculatedBoneReward = FMath::RoundToInt(BaseBone * DifficultyMultiplier * BoneMultiplier);
    CalculatedHideReward = FMath::RoundToInt(BaseHide * DifficultyMultiplier * HideMultiplier);
}

int32 UQuest_DinosaurHuntSystem::GetMeatReward() const
{
    return CalculatedMeatReward;
}

int32 UQuest_DinosaurHuntSystem::GetBoneReward() const
{
    return CalculatedBoneReward;
}

int32 UQuest_DinosaurHuntSystem::GetHideReward() const
{
    return CalculatedHideReward;
}

void UQuest_DinosaurHuntSystem::UpdateHuntTimer(float DeltaTime)
{
    if (CurrentMission.bIsActive && !CurrentMission.bIsCompleted)
    {
        CurrentMission.RemainingTime -= DeltaTime;
        
        if (CurrentMission.RemainingTime <= 0.0f)
        {
            CheckHuntTimeout();
        }
    }
}

void UQuest_DinosaurHuntSystem::CheckHuntTimeout()
{
    if (CurrentMission.RemainingTime <= 0.0f)
    {
        CurrentMission.bIsActive = false;
        CurrentMission.bIsCompleted = false;
        CurrentMission.MissionName = TEXT("Hunt Failed - Time Expired");
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurHuntSystem: Hunt mission failed - time expired"));
    }
}

FString UQuest_DinosaurHuntSystem::GetSpeciesDisplayName(EQuest_DinosaurSpecies Species) const
{
    switch (Species)
    {
        case EQuest_DinosaurSpecies::TRex:
            return TEXT("T-Rex");
        case EQuest_DinosaurSpecies::Velociraptor:
            return TEXT("Velociraptor");
        case EQuest_DinosaurSpecies::Triceratops:
            return TEXT("Triceratops");
        case EQuest_DinosaurSpecies::Brachiosaurus:
            return TEXT("Brachiosaurus");
        case EQuest_DinosaurSpecies::Ankylosaurus:
            return TEXT("Ankylosaurus");
        case EQuest_DinosaurSpecies::Parasaurolophus:
            return TEXT("Parasaurolophus");
        default:
            return TEXT("Unknown");
    }
}

FString UQuest_DinosaurHuntSystem::GetDifficultyDisplayName(EQuest_HuntDifficulty Difficulty) const
{
    switch (Difficulty)
    {
        case EQuest_HuntDifficulty::Scavenge:
            return TEXT("Scavenge");
        case EQuest_HuntDifficulty::Ambush:
            return TEXT("Ambush");
        case EQuest_HuntDifficulty::Hunt:
            return TEXT("Hunt");
        case EQuest_HuntDifficulty::PackHunt:
            return TEXT("Pack Hunt");
        case EQuest_HuntDifficulty::ApexPredator:
            return TEXT("Apex Predator");
        default:
            return TEXT("Unknown");
    }
}