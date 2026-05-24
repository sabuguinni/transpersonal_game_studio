#include "Quest_BiomeExplorationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeExploration, Log, All);

UQuest_BiomeExplorationSystem::UQuest_BiomeExplorationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f; // Check every 2 seconds
    
    // Initialize biome discovery states
    BiomeDiscoveryStates.Add(EBiomeType::Swamp, false);
    BiomeDiscoveryStates.Add(EBiomeType::Forest, false);
    BiomeDiscoveryStates.Add(EBiomeType::Savanna, false);
    BiomeDiscoveryStates.Add(EBiomeType::Desert, false);
    BiomeDiscoveryStates.Add(EBiomeType::Mountain, false);
    
    // Define biome boundaries
    BiomeBoundaries.Add(EBiomeType::Swamp, FBiomeBounds{
        FVector(-77500, -76500, -500), FVector(-25000, -15000, 1000)
    });
    BiomeBoundaries.Add(EBiomeType::Forest, FBiomeBounds{
        FVector(-77500, 15000, -500), FVector(-15000, 76500, 1000)
    });
    BiomeBoundaries.Add(EBiomeType::Savanna, FBiomeBounds{
        FVector(-20000, -20000, -500), FVector(20000, 20000, 1000)
    });
    BiomeBoundaries.Add(EBiomeType::Desert, FBiomeBounds{
        FVector(25000, -30000, -500), FVector(79500, 30000, 1000)
    });
    BiomeBoundaries.Add(EBiomeType::Mountain, FBiomeBounds{
        FVector(15000, 20000, 0), FVector(79500, 76500, 2000)
    });
    
    CurrentPlayerBiome = EBiomeType::Savanna; // Start in Savanna
    ExplorationRadius = 5000.0f;
    BiomesDiscovered = 0;
    TotalExplorationDistance = 0.0f;
    LastPlayerLocation = FVector::ZeroVector;
}

void UQuest_BiomeExplorationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBiomeExploration, Log, TEXT("Biome Exploration System initialized"));
    
    // Get initial player location
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        LastPlayerLocation = PlayerPawn->GetActorLocation();
        CurrentPlayerBiome = DetermineBiomeFromLocation(LastPlayerLocation);
        UE_LOG(LogBiomeExploration, Log, TEXT("Player starting biome: %s"), 
               *UEnum::GetValueAsString(CurrentPlayerBiome));
    }
    
    // Create exploration markers for each biome
    CreateBiomeExplorationMarkers();
}

void UQuest_BiomeExplorationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
        return;
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;
    
    FVector CurrentLocation = PlayerPawn->GetActorLocation();
    
    // Calculate distance traveled
    if (LastPlayerLocation != FVector::ZeroVector)
    {
        float DistanceTraveled = FVector::Dist(CurrentLocation, LastPlayerLocation);
        TotalExplorationDistance += DistanceTraveled;
    }
    
    // Check current biome
    EBiomeType NewBiome = DetermineBiomeFromLocation(CurrentLocation);
    if (NewBiome != CurrentPlayerBiome)
    {
        OnBiomeEntered(NewBiome);
        CurrentPlayerBiome = NewBiome;
    }
    
    // Check for biome discovery
    CheckBiomeDiscovery(CurrentLocation);
    
    LastPlayerLocation = CurrentLocation;
}

EBiomeType UQuest_BiomeExplorationSystem::DetermineBiomeFromLocation(const FVector& Location) const
{
    for (const auto& BiomePair : BiomeBoundaries)
    {
        const FBiomeBounds& Bounds = BiomePair.Value;
        if (Location.X >= Bounds.Min.X && Location.X <= Bounds.Max.X &&
            Location.Y >= Bounds.Min.Y && Location.Y <= Bounds.Max.Y &&
            Location.Z >= Bounds.Min.Z && Location.Z <= Bounds.Max.Z)
        {
            return BiomePair.Key;
        }
    }
    
    // Default to Savanna if no biome matches
    return EBiomeType::Savanna;
}

void UQuest_BiomeExplorationSystem::CheckBiomeDiscovery(const FVector& PlayerLocation)
{
    EBiomeType PlayerBiome = DetermineBiomeFromLocation(PlayerLocation);
    
    if (!BiomeDiscoveryStates[PlayerBiome])
    {
        // Mark biome as discovered
        BiomeDiscoveryStates[PlayerBiome] = true;
        BiomesDiscovered++;
        
        UE_LOG(LogBiomeExploration, Log, TEXT("Biome discovered: %s (Total: %d/5)"), 
               *UEnum::GetValueAsString(PlayerBiome), BiomesDiscovered);
        
        // Trigger discovery event
        OnBiomeDiscovered(PlayerBiome);
        
        // Check if all biomes discovered
        if (BiomesDiscovered >= 5)
        {
            OnAllBiomesDiscovered();
        }
    }
}

void UQuest_BiomeExplorationSystem::OnBiomeEntered(EBiomeType NewBiome)
{
    UE_LOG(LogBiomeExploration, Log, TEXT("Player entered biome: %s"), 
           *UEnum::GetValueAsString(NewBiome));
    
    // Trigger biome-specific events
    switch (NewBiome)
    {
        case EBiomeType::Swamp:
            TriggerSwampExplorationQuest();
            break;
        case EBiomeType::Forest:
            TriggerForestExplorationQuest();
            break;
        case EBiomeType::Desert:
            TriggerDesertExplorationQuest();
            break;
        case EBiomeType::Mountain:
            TriggerMountainExplorationQuest();
            break;
        default:
            break;
    }
}

void UQuest_BiomeExplorationSystem::OnBiomeDiscovered(EBiomeType DiscoveredBiome)
{
    FString BiomeName = UEnum::GetValueAsString(DiscoveredBiome);
    UE_LOG(LogBiomeExploration, Warning, TEXT("NEW BIOME DISCOVERED: %s!"), *BiomeName);
    
    // Award exploration experience
    int32 ExplorationXP = 100;
    AwardExplorationExperience(ExplorationXP);
    
    // Create discovery notification
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Biome Discovered: %s (+%d XP)"), *BiomeName, ExplorationXP);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
    }
}

void UQuest_BiomeExplorationSystem::OnAllBiomesDiscovered()
{
    UE_LOG(LogBiomeExploration, Warning, TEXT("ALL BIOMES DISCOVERED! Master Explorer achievement unlocked!"));
    
    // Award master explorer bonus
    int32 MasterExplorerXP = 500;
    AwardExplorationExperience(MasterExplorerXP);
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("MASTER EXPLORER! All biomes discovered! (+%d XP)"), MasterExplorerXP);
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Gold, Message);
    }
}

void UQuest_BiomeExplorationSystem::TriggerSwampExplorationQuest()
{
    UE_LOG(LogBiomeExploration, Log, TEXT("Triggering Swamp exploration quest"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("Quest: Explore the dangerous swamplands. Beware of predators lurking in the murky waters."));
    }
}

void UQuest_BiomeExplorationSystem::TriggerForestExplorationQuest()
{
    UE_LOG(LogBiomeExploration, Log, TEXT("Triggering Forest exploration quest"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Quest: Navigate the dense forest. Look for useful resources among the ancient trees."));
    }
}

void UQuest_BiomeExplorationSystem::TriggerDesertExplorationQuest()
{
    UE_LOG(LogBiomeExploration, Log, TEXT("Triggering Desert exploration quest"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            TEXT("Quest: Survive the harsh desert. Find water sources and shelter from the heat."));
    }
}

void UQuest_BiomeExplorationSystem::TriggerMountainExplorationQuest()
{
    UE_LOG(LogBiomeExploration, Log, TEXT("Triggering Mountain exploration quest"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
            TEXT("Quest: Climb the treacherous mountains. Watch for avalanches and extreme cold."));
    }
}

void UQuest_BiomeExplorationSystem::AwardExplorationExperience(int32 XPAmount)
{
    // TODO: Integrate with character progression system
    UE_LOG(LogBiomeExploration, Log, TEXT("Awarded %d exploration XP"), XPAmount);
}

void UQuest_BiomeExplorationSystem::CreateBiomeExplorationMarkers()
{
    if (!GetWorld())
        return;
    
    // Create visual markers for each biome center
    TArray<TPair<EBiomeType, FVector>> BiomeCenters = {
        {EBiomeType::Swamp, FVector(-50000, -45000, 0)},
        {EBiomeType::Forest, FVector(-45000, 40000, 0)},
        {EBiomeType::Savanna, FVector(0, 0, 0)},
        {EBiomeType::Desert, FVector(55000, 0, 0)},
        {EBiomeType::Mountain, FVector(40000, 50000, 500)}
    };
    
    for (const auto& BiomeCenter : BiomeCenters)
    {
        // Spawn exploration marker
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AStaticMeshActor* Marker = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            BiomeCenter.Value + FVector(0, 0, 200), // Slightly elevated
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (Marker)
        {
            FString BiomeName = UEnum::GetValueAsString(BiomeCenter.Key);
            Marker->SetActorLabel(FString::Printf(TEXT("ExplorationMarker_%s"), *BiomeName));
            
            UE_LOG(LogBiomeExploration, Log, TEXT("Created exploration marker for %s"), *BiomeName);
        }
    }
}

bool UQuest_BiomeExplorationSystem::IsBiomeDiscovered(EBiomeType Biome) const
{
    const bool* DiscoveryState = BiomeDiscoveryStates.Find(Biome);
    return DiscoveryState ? *DiscoveryState : false;
}

float UQuest_BiomeExplorationSystem::GetExplorationProgress() const
{
    return BiomesDiscovered / 5.0f;
}