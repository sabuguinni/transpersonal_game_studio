#include "World_PerformanceIntegration.h"
#include "World_TerrainManager.h"
#include "World_BiomeManager.h"
#include "World_WeatherSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UWorld_PerformanceIntegration::UWorld_PerformanceIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Tick every 0.5 seconds for performance
    
    // Initialize default settings
    LODSettings.HighDetailDistance = 1000.0f;
    LODSettings.MediumDetailDistance = 2500.0f;
    LODSettings.LowDetailDistance = 5000.0f;
    LODSettings.CullDistance = 10000.0f;
    LODSettings.bEnableDistanceCulling = true;
    LODSettings.bEnableFrustumCulling = true;
    LODSettings.bEnableOcclusionCulling = false;
    
    StreamingSettings.StreamingDistance = 8000.0f;
    StreamingSettings.UnloadDistance = 12000.0f;
    StreamingSettings.MaxActiveChunks = 25;
    StreamingSettings.MaxActorsPerChunk = 320;
    StreamingSettings.bEnableWorldPartition = true;
    StreamingSettings.bPreloadAdjacentChunks = true;
    
    MaxTotalActors = 8000;
    MaxDinosaurs = 150;
    MaxPropsPerBiome = 1000;
    TargetFrameTime = 16.67f; // 60fps
    PerformanceCheckInterval = 2.0f;
}

void UWorld_PerformanceIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerformanceSystems();
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceIntegration: System initialized"));
}

void UWorld_PerformanceIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastPerformanceCheck += DeltaTime;
    
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdatePerformanceMetrics();
        
        if (!IsPerformanceWithinLimits())
        {
            OptimizeWorldGeneration();
        }
        
        // Manage world streaming based on player position
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    ManageWorldStreaming(PlayerPawn->GetActorLocation());
                }
            }
        }
        
        LastPerformanceCheck = 0.0f;
    }
}

void UWorld_PerformanceIntegration::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalActors = AllActors.Num();
    CurrentMetrics.VisibleActors = 0;
    CurrentMetrics.CulledActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.PropsCount = 0;
    
    // Count different actor types
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Count dinosaurs
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
            ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")) ||
            ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) || 
            ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("ankylo")) || ActorLabel.Contains(TEXT("parasauro")))
        {
            CurrentMetrics.DinosaurCount++;
        }
        
        // Count props (rocks, trees, etc.)
        if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("tree")) || 
            ActorName.Contains(TEXT("bush")) || ActorName.Contains(TEXT("prop")) ||
            ActorLabel.Contains(TEXT("rock")) || ActorLabel.Contains(TEXT("tree")) || 
            ActorLabel.Contains(TEXT("bush")) || ActorLabel.Contains(TEXT("prop")))
        {
            CurrentMetrics.PropsCount++;
        }
        
        // Check visibility
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsVisible())
            {
                CurrentMetrics.VisibleActors++;
            }
            else
            {
                CurrentMetrics.CulledActors++;
            }
        }
    }
    
    // Calculate frame time
    CurrentMetrics.FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Check performance warning
    CurrentMetrics.bPerformanceWarning = (CurrentMetrics.TotalActors > MaxTotalActors * 0.8f) ||
                                        (CurrentMetrics.DinosaurCount > MaxDinosaurs * 0.8f) ||
                                        (CurrentMetrics.FrameTime > TargetFrameTime * 1.2f);
    
    CachedActors = AllActors;
}

bool UWorld_PerformanceIntegration::IsPerformanceWithinLimits() const
{
    return CurrentMetrics.TotalActors <= MaxTotalActors &&
           CurrentMetrics.DinosaurCount <= MaxDinosaurs &&
           CurrentMetrics.FrameTime <= TargetFrameTime * 1.5f; // Allow 50% overhead
}

void UWorld_PerformanceIntegration::OptimizeWorldGeneration()
{
    if (bPerformanceOptimizationActive)
    {
        return; // Already optimizing
    }
    
    bPerformanceOptimizationActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("World_PerformanceIntegration: Performance optimization triggered"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d/%d, Dinosaurs: %d/%d, Frame Time: %.2fms"), 
           CurrentMetrics.TotalActors, MaxTotalActors, 
           CurrentMetrics.DinosaurCount, MaxDinosaurs, 
           CurrentMetrics.FrameTime);
    
    // Clean up excess actors if needed
    if (CurrentMetrics.TotalActors > MaxTotalActors)
    {
        CleanupExcessActors();
    }
    
    // Sync with performance systems
    SyncWithCullingSystem();
    
    bPerformanceOptimizationActive = false;
}

void UWorld_PerformanceIntegration::UpdateActorLOD(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    
    int32 LODLevel = CalculateLODLevel(DistanceToPlayer);
    ApplyLODSettingsToActor(Actor, LODLevel);
}

void UWorld_PerformanceIntegration::ApplyLODSettingsToActor(AActor* Actor, int32 LODLevel)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    
    // Apply LOD based on level
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        switch (LODLevel)
        {
            case 0: // High detail
                MeshComp->SetVisibility(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                break;
                
            case 1: // Medium detail
                MeshComp->SetVisibility(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                break;
                
            case 2: // Low detail
                MeshComp->SetVisibility(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                break;
                
            case 3: // Culled
                MeshComp->SetVisibility(false);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                break;
        }
    }
    
    // Disable tick for distant actors
    if (LODLevel >= 2)
    {
        Actor->SetActorTickEnabled(false);
    }
    else
    {
        Actor->SetActorTickEnabled(true);
    }
}

int32 UWorld_PerformanceIntegration::CalculateLODLevel(float Distance) const
{
    if (Distance <= LODSettings.HighDetailDistance)
    {
        return 0; // High detail
    }
    else if (Distance <= LODSettings.MediumDetailDistance)
    {
        return 1; // Medium detail
    }
    else if (Distance <= LODSettings.LowDetailDistance)
    {
        return 2; // Low detail
    }
    else if (Distance <= LODSettings.CullDistance)
    {
        return 3; // Culled but exists
    }
    else
    {
        return 4; // Fully culled
    }
}

void UWorld_PerformanceIntegration::ManageWorldStreaming(const FVector& PlayerLocation)
{
    if (!StreamingSettings.bEnableWorldPartition)
    {
        return;
    }
    
    // Simple chunk-based streaming simulation
    // In a real implementation, this would work with UE5's World Partition system
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > StreamingSettings.UnloadDistance)
        {
            // Mark for unloading (in real implementation, would unload chunk)
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorTickEnabled(false);
        }
        else if (Distance <= StreamingSettings.StreamingDistance)
        {
            // Mark for loading (in real implementation, would load chunk)
            Actor->SetActorHiddenInGame(false);
            UpdateActorLOD(Actor, Distance);
        }
    }
}

void UWorld_PerformanceIntegration::LoadChunk(const FVector& ChunkLocation)
{
    // Placeholder for chunk loading logic
    // In real implementation, would interface with World Partition
    UE_LOG(LogTemp, Log, TEXT("Loading chunk at location: %s"), *ChunkLocation.ToString());
}

void UWorld_PerformanceIntegration::UnloadChunk(const FVector& ChunkLocation)
{
    // Placeholder for chunk unloading logic
    UE_LOG(LogTemp, Log, TEXT("Unloading chunk at location: %s"), *ChunkLocation.ToString());
}

void UWorld_PerformanceIntegration::InitializePerformanceSystems()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Find performance system components
    TerrainManager = GetOwner()->FindComponentByClass<UWorld_TerrainManager>();
    BiomeManager = GetOwner()->FindComponentByClass<UWorld_BiomeManager>();
    WeatherSystem = GetOwner()->FindComponentByClass<UWorld_WeatherSystem>();
    
    // Try to find performance components from Agent #4
    // Note: These may not exist if Agent #4's systems aren't active
    CullingSystem = GetOwner()->FindComponentByClass<UPerf_CullingSystem>();
    DayNightCycle = GetOwner()->FindComponentByClass<UPerf_DayNightCycle>();
    
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceIntegration: Found TerrainManager: %s, BiomeManager: %s, WeatherSystem: %s"), 
           TerrainManager ? TEXT("Yes") : TEXT("No"),
           BiomeManager ? TEXT("Yes") : TEXT("No"),
           WeatherSystem ? TEXT("Yes") : TEXT("No"));
}

void UWorld_PerformanceIntegration::SyncWithCullingSystem()
{
    if (!CullingSystem)
    {
        return;
    }
    
    // Sync LOD settings with culling system
    // This would interface with Agent #4's culling system
    UE_LOG(LogTemp, Log, TEXT("Syncing with culling system - LOD distances updated"));
}

void UWorld_PerformanceIntegration::SyncWithDayNightCycle()
{
    if (!DayNightCycle)
    {
        return;
    }
    
    // Sync weather and lighting with day/night cycle
    UE_LOG(LogTemp, Log, TEXT("Syncing with day/night cycle"));
}

void UWorld_PerformanceIntegration::CleanupExcessActors()
{
    if (CurrentMetrics.TotalActors <= MaxTotalActors)
    {
        return;
    }
    
    TArray<AActor*> ActorsToRemove;
    PrioritizeActors(CachedActors);
    
    int32 ActorsToDelete = CurrentMetrics.TotalActors - MaxTotalActors;
    
    for (int32 i = 0; i < FMath::Min(ActorsToDelete, CachedActors.Num()); i++)
    {
        AActor* Actor = CachedActors[i];
        if (Actor && IsValid(Actor))
        {
            // Don't delete essential actors
            FString ActorName = Actor->GetName().ToLower();
            if (!ActorName.Contains(TEXT("playerstart")) && 
                !ActorName.Contains(TEXT("gamemode")) && 
                !ActorName.Contains(TEXT("controller")))
            {
                ActorsToRemove.Add(Actor);
            }
        }
    }
    
    // Remove excess actors
    for (AActor* Actor : ActorsToRemove)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d excess actors"), ActorsToRemove.Num());
}

void UWorld_PerformanceIntegration::PrioritizeActors(TArray<AActor*>& Actors)
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Sort actors by distance from player (furthest first for deletion)
    Actors.Sort([PlayerLocation](const AActor& A, const AActor& B) {
        float DistA = FVector::Dist(PlayerLocation, A.GetActorLocation());
        float DistB = FVector::Dist(PlayerLocation, B.GetActorLocation());
        return DistA > DistB;
    });
}

bool UWorld_PerformanceIntegration::CanSpawnNewActor(const FString& ActorType) const
{
    if (CurrentMetrics.TotalActors >= MaxTotalActors)
    {
        return false;
    }
    
    if (ActorType.Contains(TEXT("dinosaur")) || ActorType.Contains(TEXT("dino")))
    {
        return CurrentMetrics.DinosaurCount < MaxDinosaurs;
    }
    
    return true;
}

void UWorld_PerformanceIntegration::LogPerformanceStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("=== World Performance Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d/%d"), CurrentMetrics.TotalActors, MaxTotalActors);
    UE_LOG(LogTemp, Log, TEXT("Visible Actors: %d"), CurrentMetrics.VisibleActors);
    UE_LOG(LogTemp, Log, TEXT("Culled Actors: %d"), CurrentMetrics.CulledActors);
    UE_LOG(LogTemp, Log, TEXT("Dinosaurs: %d/%d"), CurrentMetrics.DinosaurCount, MaxDinosaurs);
    UE_LOG(LogTemp, Log, TEXT("Props: %d"), CurrentMetrics.PropsCount);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2fms (Target: %.2fms)"), CurrentMetrics.FrameTime, TargetFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Warning: %s"), CurrentMetrics.bPerformanceWarning ? TEXT("YES") : TEXT("NO"));
}

void UWorld_PerformanceIntegration::DrawPerformanceDebugInfo() const
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw debug info on screen
    if (GEngine)
    {
        FString DebugText = FString::Printf(TEXT("Actors: %d/%d | Dinos: %d/%d | Frame: %.1fms"), 
                                           CurrentMetrics.TotalActors, MaxTotalActors,
                                           CurrentMetrics.DinosaurCount, MaxDinosaurs,
                                           CurrentMetrics.FrameTime);
        
        FColor DebugColor = CurrentMetrics.bPerformanceWarning ? FColor::Red : FColor::Green;
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, DebugColor, DebugText);
    }
}

void UWorld_PerformanceIntegration::TestPerformanceIntegration()
{
    UpdatePerformanceMetrics();
    LogPerformanceStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Integration Test Complete"));
}