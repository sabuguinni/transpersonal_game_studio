#include "Dir_ProductionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    MaxTotalActors = 8000;
    MaxDinosaurs = 150;
    
    InitializeBiomeData();
}

void UDir_ProductionManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    UpdateProductionMetrics();
}

void UDir_ProductionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionMetrics();
    
    // Auto-cleanup if limits exceeded
    if (ProductionMetrics.TotalActors > MaxTotalActors)
    {
        CleanupExcessActors();
    }
}

void UDir_ProductionManager::InitializeBiomeData()
{
    BiomeData.Empty();
    
    // Initialize the 5 biomes with their center coordinates
    FDir_BiomeData Savana;
    Savana.BiomeType = EDir_BiomeType::Savana;
    Savana.CenterLocation = FVector(0, 0, 100);
    Savana.MaxActors = 1600; // 20% of 8000
    BiomeData.Add(Savana);
    
    FDir_BiomeData Pantano;
    Pantano.BiomeType = EDir_BiomeType::Pantano;
    Pantano.CenterLocation = FVector(-50000, -45000, 100);
    Pantano.MaxActors = 1600;
    BiomeData.Add(Pantano);
    
    FDir_BiomeData Floresta;
    Floresta.BiomeType = EDir_BiomeType::Floresta;
    Floresta.CenterLocation = FVector(-45000, 40000, 100);
    Floresta.MaxActors = 1600;
    BiomeData.Add(Floresta);
    
    FDir_BiomeData Deserto;
    Deserto.BiomeType = EDir_BiomeType::Deserto;
    Deserto.CenterLocation = FVector(55000, 0, 100);
    Deserto.MaxActors = 1600;
    BiomeData.Add(Deserto);
    
    FDir_BiomeData Montanha;
    Montanha.BiomeType = EDir_BiomeType::Montanha;
    Montanha.CenterLocation = FVector(40000, 50000, 100);
    Montanha.MaxActors = 1600;
    BiomeData.Add(Montanha);
}

void UDir_ProductionManager::UpdateProductionMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count all actors
    int32 TotalActorCount = 0;
    int32 DinosaurCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            TotalActorCount++;
            
            // Check if it's a dinosaur
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("veloci")) || 
                ActorName.Contains(TEXT("tricera")) || 
                ActorName.Contains(TEXT("brachi")) || 
                ActorName.Contains(TEXT("ankylo")) || 
                ActorName.Contains(TEXT("parasauro")))
            {
                DinosaurCount++;
            }
        }
    }
    
    ProductionMetrics.TotalActors = TotalActorCount;
    ProductionMetrics.TotalDinosaurs = DinosaurCount;
    
    // Update biome counts
    CountActorsInBiomes();
    
    // Calculate performance score based on limits
    float ActorRatio = (float)TotalActorCount / (float)MaxTotalActors;
    float DinoRatio = (float)DinosaurCount / (float)MaxDinosaurs;
    ProductionMetrics.PerformanceScore = FMath::Max(0.0f, 100.0f - (ActorRatio * 50.0f) - (DinoRatio * 30.0f));
    
    // Determine production phase
    if (TotalActorCount < 1000)
    {
        ProductionMetrics.CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
    else if (TotalActorCount < 3000)
    {
        ProductionMetrics.CurrentPhase = EDir_ProductionPhase::PrototypeDevelopment;
    }
    else if (TotalActorCount < 5000)
    {
        ProductionMetrics.CurrentPhase = EDir_ProductionPhase::VerticalSlice;
    }
    else
    {
        ProductionMetrics.CurrentPhase = EDir_ProductionPhase::Production;
    }
}

void UDir_ProductionManager::CountActorsInBiomes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Reset counts
    for (FDir_BiomeData& Biome : BiomeData)
    {
        Biome.ActorCount = 0;
        Biome.DinosaurCount = 0;
    }
    
    // Count actors in each biome
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            for (FDir_BiomeData& Biome : BiomeData)
            {
                if (IsActorInBiome(Actor, Biome))
                {
                    Biome.ActorCount++;
                    
                    // Check if it's a dinosaur
                    FString ActorName = Actor->GetName().ToLower();
                    if (ActorName.Contains(TEXT("trex")) || 
                        ActorName.Contains(TEXT("veloci")) || 
                        ActorName.Contains(TEXT("tricera")) || 
                        ActorName.Contains(TEXT("brachi")) || 
                        ActorName.Contains(TEXT("ankylo")) || 
                        ActorName.Contains(TEXT("parasauro")))
                    {
                        Biome.DinosaurCount++;
                    }
                    break; // Actor can only be in one biome
                }
            }
        }
    }
}

bool UDir_ProductionManager::IsActorInBiome(AActor* Actor, const FDir_BiomeData& Biome)
{
    if (!Actor)
    {
        return false;
    }
    
    FVector ActorLocation = Actor->GetActorLocation();
    float Distance = FVector::Dist2D(ActorLocation, Biome.CenterLocation);
    
    // 20km radius for each biome
    return Distance <= 20000.0f;
}

bool UDir_ProductionManager::CanSpawnActorInBiome(EDir_BiomeType BiomeType)
{
    for (const FDir_BiomeData& Biome : BiomeData)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome.ActorCount < Biome.MaxActors;
        }
    }
    return false;
}

FVector UDir_ProductionManager::GetRandomLocationInBiome(EDir_BiomeType BiomeType)
{
    for (const FDir_BiomeData& Biome : BiomeData)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float RandomX = FMath::RandRange(-15000.0f, 15000.0f);
            float RandomY = FMath::RandRange(-15000.0f, 15000.0f);
            
            return FVector(
                Biome.CenterLocation.X + RandomX,
                Biome.CenterLocation.Y + RandomY,
                100.0f
            );
        }
    }
    return FVector::ZeroVector;
}

void UDir_ProductionManager::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> ActorsToRemove;
    int32 ActorsToRemoveCount = ProductionMetrics.TotalActors - MaxTotalActors;
    
    if (ActorsToRemoveCount <= 0)
    {
        return;
    }
    
    // Collect non-essential actors for removal
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Don't remove essential actors
            if (ActorName.Contains(TEXT("playerstart")) || 
                ActorName.Contains(TEXT("gamemode")) || 
                ActorName.Contains(TEXT("controller")) ||
                ActorName.Contains(TEXT("pawn")) ||
                ActorName.Contains(TEXT("character")))
            {
                continue;
            }
            
            ActorsToRemove.Add(Actor);
            
            if (ActorsToRemove.Num() >= ActorsToRemoveCount)
            {
                break;
            }
        }
    }
    
    // Remove excess actors
    for (AActor* Actor : ActorsToRemove)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Manager: Removed %d excess actors"), ActorsToRemove.Num());
}

void UDir_ProductionManager::InitializeBiomes()
{
    InitializeBiomeData();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Production Manager: Initialized %d biomes"), BiomeData.Num());
}

void UDir_ProductionManager::ValidateActorLimits()
{
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d / %d"), ProductionMetrics.TotalActors, MaxTotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Total Dinosaurs: %d / %d"), ProductionMetrics.TotalDinosaurs, MaxDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.1f"), ProductionMetrics.PerformanceScore);
    
    for (const FDir_BiomeData& Biome : BiomeData)
    {
        FString BiomeName;
        switch (Biome.BiomeType)
        {
            case EDir_BiomeType::Savana: BiomeName = TEXT("Savana"); break;
            case EDir_BiomeType::Pantano: BiomeName = TEXT("Pantano"); break;
            case EDir_BiomeType::Floresta: BiomeName = TEXT("Floresta"); break;
            case EDir_BiomeType::Deserto: BiomeName = TEXT("Deserto"); break;
            case EDir_BiomeType::Montanha: BiomeName = TEXT("Montanha"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %d actors, %d dinosaurs"), *BiomeName, Biome.ActorCount, Biome.DinosaurCount);
    }
    
    if (ProductionMetrics.TotalActors > MaxTotalActors)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Actor limit exceeded! Cleanup required."));
    }
}