#include "Perf_BiomePerformanceOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"

UPerf_BiomePerformanceOptimizer::UPerf_BiomePerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    PerformanceSettings = FPerf_BiomePerformanceSettings();
    TargetFPS = 60.0f;
    CurrentFPS = 60.0f;
}

void UPerf_BiomePerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeStats();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Biome Performance Optimizer initialized"));
}

void UPerf_BiomePerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= PerformanceSettings.BiomeUpdateInterval)
    {
        UpdatePerformanceMetrics();
        UpdateBiomeStats();
        OptimizeBiomePerformance();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_BiomePerformanceOptimizer::InitializeBiomeStats()
{
    BiomeStats.Empty();
    
    // Initialize stats for all 5 biomes based on brain memory coordinates
    TArray<TPair<FString, FVector>> BiomeLocations = {
        {TEXT("Savana"), FVector(0, 0, 0)},
        {TEXT("Pantano"), FVector(-50000, -45000, 0)},
        {TEXT("Floresta"), FVector(-45000, 40000, 0)},
        {TEXT("Deserto"), FVector(55000, 0, 0)},
        {TEXT("Montanha"), FVector(40000, 50000, 0)}
    };
    
    for (const auto& BiomeData : BiomeLocations)
    {
        FPerf_BiomeStats NewStats;
        NewStats.BiomeName = BiomeData.Key;
        NewStats.BiomeCenter = BiomeData.Value;
        NewStats.ActiveDinosaurs = 0;
        NewStats.TotalActors = 0;
        NewStats.FrameTimeMs = 16.67f; // 60 FPS baseline
        NewStats.bIsPlayerInBiome = false;
        
        BiomeStats.Add(NewStats);
    }
}

void UPerf_BiomePerformanceOptimizer::UpdatePerformanceMetrics()
{
    // Get current FPS from engine
    CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    if (CurrentFPS > 200.0f) CurrentFPS = 60.0f; // Cap unrealistic values
    
    // Update frame time for each biome
    for (FPerf_BiomeStats& Stats : BiomeStats)
    {
        Stats.FrameTimeMs = (1000.0f / CurrentFPS);
    }
}

void UPerf_BiomePerformanceOptimizer::UpdateBiomeStats()
{
    if (!GetWorld()) return;
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Update stats for each biome
    for (FPerf_BiomeStats& Stats : BiomeStats)
    {
        Stats.ActiveDinosaurs = 0;
        Stats.TotalActors = 0;
        Stats.bIsPlayerInBiome = false;
        
        // Check if player is in this biome (within 20km radius)
        float DistanceToPlayer = FVector::Dist(PlayerLocation, Stats.BiomeCenter);
        Stats.bIsPlayerInBiome = (DistanceToPlayer <= 20000.0f);
        
        // Count actors in biome area
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (!Actor) continue;
            
            float DistanceToBiome = FVector::Dist(Actor->GetActorLocation(), Stats.BiomeCenter);
            if (DistanceToBiome <= 15000.0f) // Within biome radius
            {
                Stats.TotalActors++;
                
                // Check if it's a dinosaur (contains "rex", "raptor", "tricera", etc.)
                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("rex")) || 
                    ActorName.Contains(TEXT("raptor")) || 
                    ActorName.Contains(TEXT("tricera")) ||
                    ActorName.Contains(TEXT("brachio")) ||
                    ActorName.Contains(TEXT("ankyl")))
                {
                    Stats.ActiveDinosaurs++;
                }
            }
        }
    }
}

void UPerf_BiomePerformanceOptimizer::OptimizeBiomePerformance()
{
    if (CurrentFPS < TargetFPS - 5.0f) // Performance is below target
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below target (%.1f FPS), optimizing biomes"), CurrentFPS);
        
        CullDistantBiomeActors();
        BalanceDinosaurDistribution();
        
        // Reduce LOD for distant biomes
        for (const FPerf_BiomeStats& Stats : BiomeStats)
        {
            if (!Stats.bIsPlayerInBiome && Stats.TotalActors > PerformanceSettings.MaxActiveActorsPerBiome)
            {
                OptimizeBiomeActors(GetBiomeTypeForLocation(Stats.BiomeCenter), Stats.BiomeCenter);
            }
        }
    }
}

FPerf_BiomeStats UPerf_BiomePerformanceOptimizer::GetBiomeStatsForLocation(const FVector& Location)
{
    float ClosestDistance = FLT_MAX;
    FPerf_BiomeStats ClosestStats;
    
    for (const FPerf_BiomeStats& Stats : BiomeStats)
    {
        float Distance = FVector::Dist(Location, Stats.BiomeCenter);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestStats = Stats;
        }
    }
    
    return ClosestStats;
}

void UPerf_BiomePerformanceOptimizer::SetBiomeActorLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor) return;
    
    // Set LOD for static mesh components
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        MeshComp->SetForcedLodModel(FMath::Clamp(LODLevel, 0, 3));
    }
    
    // Set LOD for skeletal mesh components (dinosaurs)
    if (USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkelMeshComp->SetForcedLOD(FMath::Clamp(LODLevel, 0, 3));
    }
}

void UPerf_BiomePerformanceOptimizer::CullDistantBiomeActors()
{
    if (!PerformanceSettings.bEnableBiomeCulling) return;
    
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (ShouldCullActor(Actor, PlayerLocation))
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
        }
        else
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorEnableCollision(true);
        }
    }
}

void UPerf_BiomePerformanceOptimizer::BalanceDinosaurDistribution()
{
    // Count dinosaurs per biome and redistribute if needed
    for (const FPerf_BiomeStats& Stats : BiomeStats)
    {
        if (Stats.ActiveDinosaurs > PerformanceSettings.MaxDinosaursPerBiome)
        {
            UE_LOG(LogTemp, Warning, TEXT("Biome %s has %d dinosaurs (max: %d), balancing"), 
                   *Stats.BiomeName, Stats.ActiveDinosaurs, PerformanceSettings.MaxDinosaursPerBiome);
            
            // Find excess dinosaurs and hide them temporarily
            TArray<AActor*> BiomeDinosaurs;
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
            
            for (AActor* Actor : AllActors)
            {
                if (!Actor) continue;
                
                float DistanceToBiome = FVector::Dist(Actor->GetActorLocation(), Stats.BiomeCenter);
                if (DistanceToBiome <= 15000.0f)
                {
                    FString ActorName = Actor->GetName().ToLower();
                    if (ActorName.Contains(TEXT("rex")) || ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("tricera")))
                    {
                        BiomeDinosaurs.Add(Actor);
                    }
                }
            }
            
            // Hide excess dinosaurs
            int32 ExcessCount = BiomeDinosaurs.Num() - PerformanceSettings.MaxDinosaursPerBiome;
            for (int32 i = 0; i < ExcessCount && i < BiomeDinosaurs.Num(); i++)
            {
                BiomeDinosaurs[i]->SetActorHiddenInGame(true);
                SetBiomeActorLOD(BiomeDinosaurs[i], 3); // Lowest LOD
            }
        }
    }
}

EBiomeType UPerf_BiomePerformanceOptimizer::GetBiomeTypeForLocation(const FVector& Location)
{
    // Determine biome based on location coordinates from brain memory
    if (FVector::Dist(Location, FVector(0, 0, 0)) <= 20000.0f)
        return EBiomeType::Savanna;
    else if (FVector::Dist(Location, FVector(-50000, -45000, 0)) <= 20000.0f)
        return EBiomeType::Swamp;
    else if (FVector::Dist(Location, FVector(-45000, 40000, 0)) <= 20000.0f)
        return EBiomeType::Forest;
    else if (FVector::Dist(Location, FVector(55000, 0, 0)) <= 20000.0f)
        return EBiomeType::Desert;
    else if (FVector::Dist(Location, FVector(40000, 50000, 0)) <= 20000.0f)
        return EBiomeType::Mountain;
    
    return EBiomeType::Savanna; // Default
}

bool UPerf_BiomePerformanceOptimizer::ShouldCullActor(AActor* Actor, const FVector& PlayerLocation)
{
    if (!Actor) return false;
    
    float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
    
    // Don't cull essential actors
    FString ActorName = Actor->GetName().ToLower();
    if (ActorName.Contains(TEXT("player")) || 
        ActorName.Contains(TEXT("gamemode")) || 
        ActorName.Contains(TEXT("controller")))
    {
        return false;
    }
    
    // Cull actors beyond max draw distance
    return (DistanceToPlayer > PerformanceSettings.MaxDrawDistance);
}

void UPerf_BiomePerformanceOptimizer::OptimizeBiomeActors(EBiomeType BiomeType, const FVector& BiomeCenter)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        float DistanceToBiome = FVector::Dist(Actor->GetActorLocation(), BiomeCenter);
        if (DistanceToBiome <= 15000.0f) // Within biome
        {
            // Set appropriate LOD based on distance from biome center
            int32 LODLevel = 0;
            if (DistanceToBiome > 10000.0f) LODLevel = 2;
            else if (DistanceToBiome > 5000.0f) LODLevel = 1;
            
            SetBiomeActorLOD(Actor, LODLevel);
        }
    }
}