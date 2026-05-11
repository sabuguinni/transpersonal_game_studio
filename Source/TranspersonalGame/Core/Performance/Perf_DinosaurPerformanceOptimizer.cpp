#include "Perf_DinosaurPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

APerf_DinosaurPerformanceOptimizer::APerf_DinosaurPerformanceOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create optimization radius component
    OptimizationRadius = CreateDefaultSubobject<USphereComponent>(TEXT("OptimizationRadius"));
    OptimizationRadius->SetupAttachment(RootComponent);
    OptimizationRadius->SetSphereRadius(10000.0f); // 10km optimization radius
    OptimizationRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default settings
    OptimizationUpdateInterval = 0.5f;
    bEnableAggressiveOptimization = false;
    TargetFrameRate = 60.0f;

    // Initialize runtime data
    CurrentFrameRate = 60.0f;
    ActiveDinosaurCount = 0;
    TotalPerformanceImpact = 0.0f;
    LastFrameTime = 0.0f;
    AverageFrameTime = 16.67f; // 60 FPS baseline
    FrameTimesSampled = 0;

    // Initialize LOD settings with realistic values for dinosaur game
    LODSettings.HighDetailDistance = 2000.0f;   // 20 meters - full detail
    LODSettings.MediumDetailDistance = 5000.0f; // 50 meters - reduced detail
    LODSettings.LowDetailDistance = 10000.0f;   // 100 meters - minimal detail
    LODSettings.CullingDistance = 15000.0f;     // 150 meters - cull completely
    LODSettings.MaxVisibleDinosaurs = 30;       // Maximum dinosaurs rendered simultaneously
    LODSettings.AIUpdateFrequencyMultiplier = 1.0f;
}

void APerf_DinosaurPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();

    // Start optimization timer
    GetWorldTimerManager().SetTimer(
        OptimizationTimerHandle,
        this,
        &APerf_DinosaurPerformanceOptimizer::OptimizeDinosaurPerformance,
        OptimizationUpdateInterval,
        true
    );

    // Start performance analysis timer
    GetWorldTimerManager().SetTimer(
        PerformanceAnalysisTimerHandle,
        this,
        &APerf_DinosaurPerformanceOptimizer::AnalyzeDinosaurPerformanceImpact,
        1.0f, // Analyze every second
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Dinosaur Performance Optimizer initialized with target FPS: %.1f"), TargetFrameRate);
}

void APerf_DinosaurPerformanceOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track frame time for performance analysis
    LastFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    RecentFrameTimes.Add(LastFrameTime);

    // Keep only last 60 frame times (1 second at 60fps)
    if (RecentFrameTimes.Num() > 60)
    {
        RecentFrameTimes.RemoveAt(0);
    }

    // Calculate average frame time
    if (RecentFrameTimes.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float FrameTime : RecentFrameTimes)
        {
            TotalTime += FrameTime;
        }
        AverageFrameTime = TotalTime / RecentFrameTimes.Num();
        CurrentFrameRate = 1000.0f / AverageFrameTime;
    }

    // Update optimization data
    UpdateOptimizationData();
}

void APerf_DinosaurPerformanceOptimizer::OptimizeDinosaurPerformance()
{
    if (!GetWorld())
    {
        return;
    }

    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    // Find player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    // Clear previous tracking data
    TrackedDinosaurs.Empty();
    ActiveDinosaurCount = 0;
    TotalPerformanceImpact = 0.0f;

    // Process potential dinosaur actors
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
        {
            continue;
        }

        // Check if this could be a dinosaur (has mesh component and specific naming)
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (!MeshComp)
        {
            continue;
        }

        FString ActorName = Actor->GetName();
        bool bIsDinosaur = ActorName.Contains(TEXT("Dinosaur")) || 
                          ActorName.Contains(TEXT("TRex")) || 
                          ActorName.Contains(TEXT("Raptor")) || 
                          ActorName.Contains(TEXT("Brachio")) ||
                          ActorName.Contains(TEXT("Triceratops"));

        if (bIsDinosaur)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            // Create optimization data for this dinosaur
            FPerf_DinosaurOptimizationData OptData;
            OptData.DistanceFromPlayer = Distance;
            OptData.CreatureSize = ECreatureSize::Large; // Default to large for now
            
            // Determine performance impact based on size and distance
            float SizeMultiplier = (OptData.CreatureSize == ECreatureSize::Large) ? 3.0f : 
                                  (OptData.CreatureSize == ECreatureSize::Medium) ? 2.0f : 1.0f;
            OptData.PerformanceImpactScore = SizeMultiplier / FMath::Max(Distance / 1000.0f, 0.1f);

            // Apply LOD optimization
            UpdateDinosaurLOD(Actor, Distance);
            
            TrackedDinosaurs.Add(OptData);
            ActiveDinosaurCount++;
            TotalPerformanceImpact += OptData.PerformanceImpactScore;
        }
    }

    // Apply global optimizations if needed
    if (CurrentFrameRate < TargetFrameRate * 0.8f) // If below 80% of target
    {
        HandlePerformanceCriticalSituation();
    }

    // Cull distant dinosaurs if we have too many
    if (ActiveDinosaurCount > LODSettings.MaxVisibleDinosaurs)
    {
        CullDistantDinosaurs();
    }

    UE_LOG(LogTemp, Log, TEXT("Dinosaur Performance Optimization: %d active dinosaurs, %.1f FPS, %.2f total impact"), 
           ActiveDinosaurCount, CurrentFrameRate, TotalPerformanceImpact);
}

void APerf_DinosaurPerformanceOptimizer::UpdateDinosaurLOD(AActor* DinosaurActor, float DistanceFromPlayer)
{
    if (!DinosaurActor)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = DinosaurActor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
    {
        return;
    }

    int32 NewLODLevel = 0;
    bool bShouldSimulatePhysics = true;
    bool bShouldEnableAI = true;

    // Determine LOD level based on distance
    if (DistanceFromPlayer > LODSettings.CullingDistance)
    {
        // Cull completely - make invisible
        MeshComp->SetVisibility(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        bShouldSimulatePhysics = false;
        bShouldEnableAI = false;
        NewLODLevel = 4; // Culled
    }
    else if (DistanceFromPlayer > LODSettings.LowDetailDistance)
    {
        // Low detail
        MeshComp->SetVisibility(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        bShouldSimulatePhysics = false;
        bShouldEnableAI = false;
        NewLODLevel = 3;
    }
    else if (DistanceFromPlayer > LODSettings.MediumDetailDistance)
    {
        // Medium detail
        MeshComp->SetVisibility(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        bShouldSimulatePhysics = true;
        bShouldEnableAI = false; // Disable AI for medium distance
        NewLODLevel = 2;
    }
    else if (DistanceFromPlayer > LODSettings.HighDetailDistance)
    {
        // High detail
        MeshComp->SetVisibility(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        bShouldSimulatePhysics = true;
        bShouldEnableAI = true;
        NewLODLevel = 1;
    }
    else
    {
        // Maximum detail
        MeshComp->SetVisibility(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        bShouldSimulatePhysics = true;
        bShouldEnableAI = true;
        NewLODLevel = 0;
    }

    // Apply physics optimization
    if (MeshComp->IsSimulatingPhysics() != bShouldSimulatePhysics)
    {
        MeshComp->SetSimulatePhysics(bShouldSimulatePhysics);
    }

    // Update actor label for debugging
    FString LODLabel = FString::Printf(TEXT("%s_LOD%d"), *DinosaurActor->GetName(), NewLODLevel);
    DinosaurActor->SetActorLabel(LODLabel);
}

void APerf_DinosaurPerformanceOptimizer::SetDinosaurAIComplexity(AActor* DinosaurActor, float ComplexityLevel)
{
    if (!DinosaurActor)
    {
        return;
    }

    // This would interface with the AI system when it's implemented
    // For now, we'll just log the complexity change
    UE_LOG(LogTemp, Log, TEXT("Setting AI complexity for %s to %.2f"), *DinosaurActor->GetName(), ComplexityLevel);
}

void APerf_DinosaurPerformanceOptimizer::CullDistantDinosaurs()
{
    if (!GetWorld())
    {
        return;
    }

    // Get player location
    FVector PlayerLocation = FVector::ZeroVector;
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    // Find all actors and sort by distance
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    TArray<TPair<AActor*, float>> DinosaurDistances;

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
        {
            continue;
        }

        FString ActorName = Actor->GetName();
        bool bIsDinosaur = ActorName.Contains(TEXT("Dinosaur")) || 
                          ActorName.Contains(TEXT("TRex")) || 
                          ActorName.Contains(TEXT("Raptor")) || 
                          ActorName.Contains(TEXT("Brachio"));

        if (bIsDinosaur)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            DinosaurDistances.Add(TPair<AActor*, float>(Actor, Distance));
        }
    }

    // Sort by distance (closest first)
    DinosaurDistances.Sort([](const TPair<AActor*, float>& A, const TPair<AActor*, float>& B) {
        return A.Value < B.Value;
    });

    // Keep only the closest MaxVisibleDinosaurs, cull the rest
    for (int32 i = 0; i < DinosaurDistances.Num(); i++)
    {
        AActor* DinosaurActor = DinosaurDistances[i].Key;
        
        if (i >= LODSettings.MaxVisibleDinosaurs)
        {
            // Cull this dinosaur
            UStaticMeshComponent* MeshComp = DinosaurActor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                MeshComp->SetVisibility(false);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                DinosaurActor->SetActorLabel(FString::Printf(TEXT("%s_CULLED"), *DinosaurActor->GetName()));
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Culled %d distant dinosaurs, keeping %d visible"), 
           FMath::Max(0, DinosaurDistances.Num() - LODSettings.MaxVisibleDinosaurs), 
           FMath::Min(DinosaurDistances.Num(), LODSettings.MaxVisibleDinosaurs));
}

void APerf_DinosaurPerformanceOptimizer::AnalyzeDinosaurPerformanceImpact()
{
    if (TrackedDinosaurs.Num() == 0)
    {
        return;
    }

    // Calculate performance metrics
    float HighDetailCount = 0;
    float MediumDetailCount = 0;
    float LowDetailCount = 0;
    float CulledCount = 0;

    for (const FPerf_DinosaurOptimizationData& Data : TrackedDinosaurs)
    {
        if (Data.DistanceFromPlayer <= LODSettings.HighDetailDistance)
        {
            HighDetailCount++;
        }
        else if (Data.DistanceFromPlayer <= LODSettings.MediumDetailDistance)
        {
            MediumDetailCount++;
        }
        else if (Data.DistanceFromPlayer <= LODSettings.LowDetailDistance)
        {
            LowDetailCount++;
        }
        else
        {
            CulledCount++;
        }
    }

    // Log performance analysis
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Performance Analysis: High=%d, Medium=%d, Low=%d, Culled=%d, FPS=%.1f"), 
           (int32)HighDetailCount, (int32)MediumDetailCount, (int32)LowDetailCount, (int32)CulledCount, CurrentFrameRate);
}

float APerf_DinosaurPerformanceOptimizer::GetCurrentPerformanceScore() const
{
    // Performance score based on frame rate vs target
    return FMath::Clamp(CurrentFrameRate / TargetFrameRate, 0.0f, 2.0f);
}

int32 APerf_DinosaurPerformanceOptimizer::GetOptimalDinosaurCount() const
{
    // Calculate optimal dinosaur count based on current performance
    float PerformanceRatio = CurrentFrameRate / TargetFrameRate;
    
    if (PerformanceRatio > 1.2f)
    {
        // Performance is good, can handle more dinosaurs
        return LODSettings.MaxVisibleDinosaurs + 10;
    }
    else if (PerformanceRatio < 0.8f)
    {
        // Performance is poor, reduce dinosaur count
        return FMath::Max(5, LODSettings.MaxVisibleDinosaurs - 10);
    }
    
    return LODSettings.MaxVisibleDinosaurs;
}

void APerf_DinosaurPerformanceOptimizer::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DINOSAUR PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f (Target: %.1f)"), CurrentFrameRate, TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Active Dinosaurs: %d (Max: %d)"), ActiveDinosaurCount, LODSettings.MaxVisibleDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("Total Performance Impact: %.2f"), TotalPerformanceImpact);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), GetCurrentPerformanceScore());
}

void APerf_DinosaurPerformanceOptimizer::SetLODSettings(const FPerf_DinosaurLODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Updated Dinosaur LOD Settings"));
}

void APerf_DinosaurPerformanceOptimizer::EnableAggressiveOptimization(bool bEnable)
{
    bEnableAggressiveOptimization = bEnable;
    
    if (bEnable)
    {
        // Reduce LOD distances for aggressive optimization
        LODSettings.HighDetailDistance *= 0.7f;
        LODSettings.MediumDetailDistance *= 0.7f;
        LODSettings.LowDetailDistance *= 0.7f;
        LODSettings.MaxVisibleDinosaurs = FMath::Max(10, LODSettings.MaxVisibleDinosaurs - 10);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Aggressive optimization %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void APerf_DinosaurPerformanceOptimizer::TestDinosaurOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TESTING DINOSAUR OPTIMIZATION ==="));
    OptimizeDinosaurPerformance();
    AnalyzeDinosaurPerformanceImpact();
    LogPerformanceMetrics();
}

void APerf_DinosaurPerformanceOptimizer::UpdateOptimizationData()
{
    // Update internal optimization state
    // This is called every tick for real-time adjustments
}

void APerf_DinosaurPerformanceOptimizer::ApplyLODOptimizations()
{
    // Apply LOD-specific optimizations
    // This could include mesh LOD, texture resolution, etc.
}

void APerf_DinosaurPerformanceOptimizer::ManageAIComplexity()
{
    // Manage AI behavior complexity based on performance
    float PerformanceRatio = CurrentFrameRate / TargetFrameRate;
    
    if (PerformanceRatio < 0.8f)
    {
        // Reduce AI complexity
        LODSettings.AIUpdateFrequencyMultiplier = 0.5f;
    }
    else if (PerformanceRatio > 1.2f)
    {
        // Increase AI complexity
        LODSettings.AIUpdateFrequencyMultiplier = 1.5f;
    }
    else
    {
        // Normal AI complexity
        LODSettings.AIUpdateFrequencyMultiplier = 1.0f;
    }
}

void APerf_DinosaurPerformanceOptimizer::HandlePerformanceCriticalSituation()
{
    UE_LOG(LogTemp, Warning, TEXT("PERFORMANCE CRITICAL: FPS %.1f below target %.1f - applying emergency optimizations"), 
           CurrentFrameRate, TargetFrameRate);

    // Emergency optimizations
    EnableAggressiveOptimization(true);
    
    // Reduce max visible dinosaurs
    LODSettings.MaxVisibleDinosaurs = FMath::Max(5, LODSettings.MaxVisibleDinosaurs / 2);
    
    // Force immediate culling
    CullDistantDinosaurs();
    
    // Reduce AI update frequency
    LODSettings.AIUpdateFrequencyMultiplier = 0.25f;
}