#include "World_BiomeOptimizationManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

AWorld_BiomeOptimizationManager::AWorld_BiomeOptimizationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create optimization zone component
    OptimizationZone = CreateDefaultSubobject<USphereComponent>(TEXT("OptimizationZone"));
    RootComponent = OptimizationZone;
    OptimizationZone->SetSphereRadius(2500.0f);
    OptimizationZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OptimizationZone->SetCollisionResponseToAllChannels(ECR_Ignore);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetVisibility(false); // Hidden by default

    // Initialize optimization data
    OptimizationData.BiomeType = EBiomeType::Forest;
    OptimizationData.MaxLODLevel = 4;
    OptimizationData.CullingDistance = 5000.0f;
    OptimizationData.DetailCullingDistance = 2000.0f;
    OptimizationData.MaxInstanceCount = 1000;
    OptimizationData.PerformanceThreshold = 30.0f;

    // Performance settings
    PerformanceCheckInterval = 1.0f;
    bEnableAdaptiveLOD = true;
    bEnableCulling = true;
    bEnableInstancePooling = true;

    // Initialize performance metrics
    CurrentFPS = 60.0f;
    CurrentInstanceCount = 0;
    ActiveLODLevel = 0;
    CullingEfficiency = 1.0f;
    LastPerformanceCheck = 0.0f;
}

void AWorld_BiomeOptimizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeOptimization();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeOptimizationManager: Initialized for biome type %d"), (int32)OptimizationData.BiomeType);
}

void AWorld_BiomeOptimizationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastPerformanceCheck += DeltaTime;
    
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        CheckPerformanceMetrics();
        
        if (bEnableAdaptiveLOD)
        {
            UpdateLODLevels();
        }
        
        if (bEnableCulling)
        {
            PerformCullingPass();
        }
        
        if (bEnableInstancePooling)
        {
            OptimizeInstanceCounts();
        }
        
        LastPerformanceCheck = 0.0f;
    }
}

void AWorld_BiomeOptimizationManager::InitializeOptimization()
{
    // Set optimization zone radius based on biome type
    float ZoneRadius = 2500.0f;
    switch (OptimizationData.BiomeType)
    {
        case EBiomeType::Forest:
            ZoneRadius = 2000.0f;
            break;
        case EBiomeType::Plains:
            ZoneRadius = 3000.0f;
            break;
        case EBiomeType::Mountains:
            ZoneRadius = 2500.0f;
            break;
        case EBiomeType::Desert:
            ZoneRadius = 3500.0f;
            break;
        case EBiomeType::Tundra:
            ZoneRadius = 2800.0f;
            break;
        case EBiomeType::Swamp:
            ZoneRadius = 1800.0f;
            break;
        default:
            ZoneRadius = 2500.0f;
            break;
    }
    
    OptimizationZone->SetSphereRadius(ZoneRadius);
    
    // Initialize LOD level instances
    LODLevelInstances.Empty();
    for (int32 i = 0; i <= OptimizationData.MaxLODLevel; i++)
    {
        LODLevelInstances.Add(i, TArray<UStaticMeshComponent*>());
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Initialized with radius %.1f for biome %d"), 
           ZoneRadius, (int32)OptimizationData.BiomeType);
}

void AWorld_BiomeOptimizationManager::UpdateLODLevels()
{
    int32 OptimalLOD = CalculateOptimalLODLevel();
    
    if (OptimalLOD != ActiveLODLevel)
    {
        SetBiomeLODLevel(OptimalLOD);
        UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Updated LOD level from %d to %d"), 
               ActiveLODLevel, OptimalLOD);
    }
}

void AWorld_BiomeOptimizationManager::PerformCullingPass()
{
    if (!GetWorld())
        return;

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
        return;

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    float DistanceToPlayer = FVector::Dist(PlayerLocation, MyLocation);

    // Distance-based culling
    bool bShouldCull = DistanceToPlayer > OptimizationData.CullingDistance;
    
    // Update culling efficiency
    if (bShouldCull)
    {
        CullingEfficiency = FMath::Max(0.1f, CullingEfficiency - 0.1f);
    }
    else
    {
        CullingEfficiency = FMath::Min(1.0f, CullingEfficiency + 0.05f);
    }

    // Apply culling to managed actors
    for (AActor* Actor : ManagedActors)
    {
        if (IsValid(Actor))
        {
            float ActorDistance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            bool bActorShouldCull = ActorDistance > OptimizationData.CullingDistance;
            
            Actor->SetActorHiddenInGame(bActorShouldCull);
            Actor->SetActorTickEnabled(!bActorShouldCull);
        }
    }
}

void AWorld_BiomeOptimizationManager::OptimizeInstanceCounts()
{
    // Count current visible instances
    int32 VisibleInstances = 0;
    for (AActor* Actor : ManagedActors)
    {
        if (IsValid(Actor) && !Actor->IsHidden())
        {
            VisibleInstances++;
        }
    }
    
    CurrentInstanceCount = VisibleInstances;
    
    // If we exceed the maximum, start pooling
    if (CurrentInstanceCount > OptimizationData.MaxInstanceCount)
    {
        PoolUnusedInstances();
    }
}

void AWorld_BiomeOptimizationManager::CheckPerformanceMetrics()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        // Get current FPS (simplified)
        CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        CurrentFPS = FMath::Clamp(CurrentFPS, 1.0f, 120.0f);
    }
}

float AWorld_BiomeOptimizationManager::GetCurrentPerformanceScore() const
{
    float FPSScore = CurrentFPS / 60.0f; // Normalize to 60 FPS
    float InstanceScore = 1.0f - (float(CurrentInstanceCount) / float(OptimizationData.MaxInstanceCount));
    float CullingScore = CullingEfficiency;
    
    return (FPSScore + InstanceScore + CullingScore) / 3.0f;
}

void AWorld_BiomeOptimizationManager::SetBiomeLODLevel(int32 NewLODLevel)
{
    ActiveLODLevel = FMath::Clamp(NewLODLevel, 0, OptimizationData.MaxLODLevel);
    
    // Apply LOD level to managed actors
    for (AActor* Actor : ManagedActors)
    {
        if (IsValid(Actor))
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    MeshComp->SetForcedLodModel(ActiveLODLevel + 1);
                }
            }
        }
    }
}

int32 AWorld_BiomeOptimizationManager::CalculateOptimalLODLevel() const
{
    float PerformanceScore = GetCurrentPerformanceScore();
    
    if (PerformanceScore > 0.8f)
    {
        return 0; // Highest quality
    }
    else if (PerformanceScore > 0.6f)
    {
        return 1;
    }
    else if (PerformanceScore > 0.4f)
    {
        return 2;
    }
    else if (PerformanceScore > 0.2f)
    {
        return 3;
    }
    else
    {
        return OptimizationData.MaxLODLevel; // Lowest quality
    }
}

void AWorld_BiomeOptimizationManager::CreateLODTransitionZone(const FWorld_LODTransitionZone& TransitionZone)
{
    TransitionZones.Add(TransitionZone);
    UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Created LOD transition zone at %s"), 
           *TransitionZone.ZoneCenter.ToString());
}

void AWorld_BiomeOptimizationManager::EnableDistanceCulling(float MaxDistance)
{
    OptimizationData.CullingDistance = MaxDistance;
    bEnableCulling = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Enabled distance culling at %.1f units"), MaxDistance);
}

void AWorld_BiomeOptimizationManager::EnableFrustumCulling()
{
    // Frustum culling is typically handled by the engine
    UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Frustum culling enabled"));
}

void AWorld_BiomeOptimizationManager::EnableOcclusionCulling()
{
    // Occlusion culling setup
    UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Occlusion culling enabled"));
}

void AWorld_BiomeOptimizationManager::RegisterManagedActor(AActor* Actor)
{
    if (IsValid(Actor) && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
        UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Registered actor %s"), 
               *Actor->GetName());
    }
}

void AWorld_BiomeOptimizationManager::UnregisterManagedActor(AActor* Actor)
{
    if (IsValid(Actor))
    {
        ManagedActors.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Unregistered actor %s"), 
               *Actor->GetName());
    }
}

void AWorld_BiomeOptimizationManager::PoolUnusedInstances()
{
    if (!GetWorld())
        return;

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
        return;

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Sort actors by distance and hide the furthest ones
    ManagedActors.Sort([PlayerLocation](const AActor& A, const AActor& B) {
        float DistA = FVector::Dist(PlayerLocation, A.GetActorLocation());
        float DistB = FVector::Dist(PlayerLocation, B.GetActorLocation());
        return DistA < DistB;
    });
    
    // Hide actors beyond the maximum count
    for (int32 i = OptimizationData.MaxInstanceCount; i < ManagedActors.Num(); i++)
    {
        if (IsValid(ManagedActors[i]))
        {
            ManagedActors[i]->SetActorHiddenInGame(true);
            ManagedActors[i]->SetActorTickEnabled(false);
        }
    }
}

void AWorld_BiomeOptimizationManager::SetOptimizationRadius(float NewRadius)
{
    OptimizationZone->SetSphereRadius(NewRadius);
    UE_LOG(LogTemp, Log, TEXT("BiomeOptimizationManager: Set optimization radius to %.1f"), NewRadius);
}

void AWorld_BiomeOptimizationManager::SetBiomeType(EBiomeType NewBiomeType)
{
    OptimizationData.BiomeType = NewBiomeType;
    InitializeOptimization(); // Reinitialize with new biome settings
}

void AWorld_BiomeOptimizationManager::DebugOptimizationState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME OPTIMIZATION DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Biome Type: %d"), (int32)OptimizationData.BiomeType);
    UE_LOG(LogTemp, Warning, TEXT("Active LOD Level: %d"), ActiveLODLevel);
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Instance Count: %d / %d"), CurrentInstanceCount, OptimizationData.MaxInstanceCount);
    UE_LOG(LogTemp, Warning, TEXT("Culling Efficiency: %.2f"), CullingEfficiency);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), GetCurrentPerformanceScore());
    UE_LOG(LogTemp, Warning, TEXT("Managed Actors: %d"), ManagedActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Transition Zones: %d"), TransitionZones.Num());
}

void AWorld_BiomeOptimizationManager::ResetOptimization()
{
    // Reset all optimization settings to defaults
    ActiveLODLevel = 0;
    CurrentInstanceCount = 0;
    CullingEfficiency = 1.0f;
    LastPerformanceCheck = 0.0f;
    
    // Show all managed actors
    for (AActor* Actor : ManagedActors)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorTickEnabled(true);
        }
    }
    
    // Clear LOD level instances
    LODLevelInstances.Empty();
    TransitionZones.Empty();
    
    InitializeOptimization();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeOptimizationManager: Reset optimization state"));
}