#include "Perf_PhysicsPerformanceIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Components/PrimitiveComponent.h"

UPerf_PhysicsPerformanceIntegrator::UPerf_PhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance settings
    CurrentOptimizationLevel = EPerf_PhysicsOptimizationLevel::High;
    TargetFrameRate = 60.0f;
    PerformanceThreshold = 16.67f; // 60 FPS target
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
    
    // Initialize timers
    LastOptimizationTime = 0.0f;
    OptimizationInterval = 1.0f; // Optimize every second
    LastMetricsUpdateTime = 0.0f;
    
    // Initialize performance tracking
    AverageFrameTime = 16.67f;
    FrameTimeHistory.Reserve(60); // Store 6 seconds of history at 10Hz
}

void UPerf_PhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Initializing Core Systems integration"));
    
    // Apply initial optimization level
    ApplyOptimizationLevel();
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
    
    // Initialize Core Systems integration
    IntegrateWithCorePhysicsManager();
}

void UPerf_PhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update performance metrics
    if (CurrentTime - LastMetricsUpdateTime >= 0.1f) // Update metrics 10 times per second
    {
        UpdatePerformanceMetrics();
        LastMetricsUpdateTime = CurrentTime;
    }
    
    // Perform optimization if needed
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        if (ShouldOptimize())
        {
            OptimizePhysicsPerformance();
        }
        LastOptimizationTime = CurrentTime;
    }
    
    // Emergency optimization if performance is critical
    if (CurrentMetrics.CurrentFrameTime > PerformanceThreshold * 2.0f)
    {
        EmergencyPhysicsOptimization();
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Starting optimization cycle"));
    
    // Update current metrics
    UpdatePerformanceMetrics();
    
    // Apply LOD optimizations
    if (OptimizationSettings.bEnablePhysicsLOD)
    {
        ApplyPhysicsLOD();
    }
    
    // Optimize ragdoll systems
    if (OptimizationSettings.bEnableRagdollCulling)
    {
        OptimizeRagdollPerformance();
    }
    
    // Optimize terrain physics
    if (OptimizationSettings.bEnableTerrainOptimization)
    {
        OptimizeTerrainPhysics();
    }
    
    // Manage overall physics budget
    ManagePhysicsBudget();
    
    // Broadcast performance change event
    OnPhysicsPerformanceChanged.Broadcast(CurrentMetrics);
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Optimization complete - Frame time: %.2fms"), CurrentMetrics.CurrentFrameTime);
}

void UPerf_PhysicsPerformanceIntegrator::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    ApplyOptimizationLevel();
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Optimization level set to %d"), (int32)Level);
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeRagdollPerformance()
{
    // Find all ragdoll actors in the world
    TrackedRagdollActors.Empty();
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character && Character->GetMesh())
        {
            USkeletalMeshComponent* MeshComp = Character->GetMesh();
            if (MeshComp->IsSimulatingPhysics())
            {
                TrackedRagdollActors.Add(Character);
            }
        }
    }
    
    CurrentMetrics.ActiveRagdolls = TrackedRagdollActors.Num();
    
    // Apply ragdoll culling if we exceed the limit
    if (CurrentMetrics.ActiveRagdolls > OptimizationSettings.MaxActiveRagdolls)
    {
        CullDistantRagdolls();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Optimized %d ragdolls"), CurrentMetrics.ActiveRagdolls);
}

void UPerf_PhysicsPerformanceIntegrator::CullDistantRagdolls()
{
    if (!GetWorld()) return;
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Sort ragdolls by distance and disable the furthest ones
    TrackedRagdollActors.Sort([PlayerLocation](const AActor& A, const AActor& B) {
        float DistA = FVector::Dist(A.GetActorLocation(), PlayerLocation);
        float DistB = FVector::Dist(B.GetActorLocation(), PlayerLocation);
        return DistA < DistB;
    });
    
    // Disable ragdolls beyond the limit
    for (int32 i = OptimizationSettings.MaxActiveRagdolls; i < TrackedRagdollActors.Num(); ++i)
    {
        if (ACharacter* Character = Cast<ACharacter>(TrackedRagdollActors[i]))
        {
            if (Character->GetMesh())
            {
                Character->GetMesh()->SetSimulatePhysics(false);
                Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Culled %d distant ragdolls"), 
           FMath::Max(0, TrackedRagdollActors.Num() - OptimizationSettings.MaxActiveRagdolls));
}

int32 UPerf_PhysicsPerformanceIntegrator::GetActiveRagdollCount() const
{
    return CurrentMetrics.ActiveRagdolls;
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeTerrainPhysics()
{
    // Find terrain actors for optimization
    TrackedTerrainActors.Empty();
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find landscape actors
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        TrackedTerrainActors.Add(*ActorItr);
    }
    
    // Find terrain-related static mesh actors
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        if (MeshActor && MeshActor->GetActorNameOrLabel().Contains(TEXT("Terrain")))
        {
            TrackedTerrainActors.Add(MeshActor);
        }
    }
    
    // Optimize terrain physics settings
    for (AActor* TerrainActor : TrackedTerrainActors)
    {
        if (UPrimitiveComponent* PrimComp = TerrainActor->FindComponentByClass<UPrimitiveComponent>())
        {
            // Reduce collision complexity for distant terrain
            APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(TerrainActor->GetActorLocation(), PlayerPawn->GetActorLocation());
                if (Distance > 10000.0f) // 100 meters
                {
                    PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
                }
                else
                {
                    PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Optimized %d terrain actors"), TrackedTerrainActors.Num());
}

void UPerf_PhysicsPerformanceIntegrator::UpdateTerrainAnalysisFrequency(float NewFrequency)
{
    OptimizationSettings.TerrainAnalysisFrequency = FMath::Clamp(NewFrequency, 0.01f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Terrain analysis frequency set to %.3f"), NewFrequency);
}

void UPerf_PhysicsPerformanceIntegrator::ApplyPhysicsLOD()
{
    if (!GetWorld()) return;
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Apply LOD to all physics actors
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (!PrimComp || !PrimComp->IsSimulatingPhysics()) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Determine LOD level based on distance
        int32 LODLevel = 0;
        if (Distance > 5000.0f) LODLevel = 3;
        else if (Distance > 2000.0f) LODLevel = 2;
        else if (Distance > 1000.0f) LODLevel = 1;
        
        SetPhysicsLODLevel(Actor, LODLevel);
    }
}

void UPerf_PhysicsPerformanceIntegrator::SetPhysicsLODLevel(AActor* Actor, int32 LODLevel)
{
    if (!Actor) return;
    
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (!PrimComp) return;
    
    switch (LODLevel)
    {
        case 0: // High quality - full physics
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case 1: // Medium quality - reduced physics
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            // Could reduce physics sub-steps here
            break;
            
        case 2: // Low quality - query only
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case 3: // Minimal quality - no collision
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

bool UPerf_PhysicsPerformanceIntegrator::IsPerformanceTargetMet() const
{
    return CurrentMetrics.CurrentFrameTime <= PerformanceThreshold;
}

float UPerf_PhysicsPerformanceIntegrator::GetPhysicsFrameTime() const
{
    return CurrentMetrics.PhysicsStepTime;
}

void UPerf_PhysicsPerformanceIntegrator::EmergencyPhysicsOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: EMERGENCY OPTIMIZATION TRIGGERED"));
    
    // Immediately disable all non-essential physics
    if (GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;
            
            // Skip player and essential actors
            if (Actor->IsA<APawn>()) continue;
            
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
        }
    }
    
    // Force lowest optimization level
    SetOptimizationLevel(EPerf_PhysicsOptimizationLevel::Low);
}

void UPerf_PhysicsPerformanceIntegrator::TestPhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Running performance test"));
    
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS PERFORMANCE TEST RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2fms"), CurrentMetrics.CurrentFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Physics Step Time: %.2fms"), CurrentMetrics.PhysicsStepTime);
    UE_LOG(LogTemp, Log, TEXT("Active Ragdolls: %d"), CurrentMetrics.ActiveRagdolls);
    UE_LOG(LogTemp, Log, TEXT("Active Physics Bodies: %d"), CurrentMetrics.ActivePhysicsBodies);
    UE_LOG(LogTemp, Log, TEXT("Terrain Analysis Time: %.2fms"), CurrentMetrics.TerrainAnalysisTime);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Performance Target Met: %s"), IsPerformanceTargetMet() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("========================================="));
}

void UPerf_PhysicsPerformanceIntegrator::UpdatePerformanceMetrics()
{
    // Get current frame time
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.CurrentFrameTime = FPlatformTime::ToMilliseconds(GEngine->GetGameViewport()->GetClient()->GetWorld()->GetDeltaSeconds()) * 1000.0f;
    }
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentMetrics.CurrentFrameTime);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            Sum += FrameTime;
        }
        AverageFrameTime = Sum / FrameTimeHistory.Num();
    }
    
    // Update physics metrics
    CurrentMetrics.PhysicsStepTime = CurrentMetrics.CurrentFrameTime * 0.3f; // Estimate
    CurrentMetrics.TerrainAnalysisTime = 0.5f; // Placeholder
    CurrentMetrics.MemoryUsageMB = 128.0f; // Placeholder
    
    // Count active physics bodies
    CurrentMetrics.ActivePhysicsBodies = 0;
    if (GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    CurrentMetrics.ActivePhysicsBodies++;
                }
            }
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::ApplyOptimizationLevel()
{
    switch (CurrentOptimizationLevel)
    {
        case EPerf_PhysicsOptimizationLevel::Low:
            OptimizationSettings.MaxActiveRagdolls = 5;
            OptimizationSettings.RagdollCullingDistance = 2000.0f;
            OptimizationSettings.TerrainAnalysisFrequency = 0.5f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            OptimizationSettings.MaxActiveRagdolls = 8;
            OptimizationSettings.RagdollCullingDistance = 3000.0f;
            OptimizationSettings.TerrainAnalysisFrequency = 0.2f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            OptimizationSettings.MaxActiveRagdolls = 12;
            OptimizationSettings.RagdollCullingDistance = 4000.0f;
            OptimizationSettings.TerrainAnalysisFrequency = 0.1f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Ultra:
            OptimizationSettings.MaxActiveRagdolls = 15;
            OptimizationSettings.RagdollCullingDistance = 5000.0f;
            OptimizationSettings.TerrainAnalysisFrequency = 0.05f;
            break;
    }
}

void UPerf_PhysicsPerformanceIntegrator::ManagePhysicsBudget()
{
    // Ensure we stay within performance budget
    if (CurrentMetrics.PhysicsStepTime > OptimizationSettings.MaxPhysicsStepTime)
    {
        // Reduce physics quality temporarily
        if (CurrentOptimizationLevel != EPerf_PhysicsOptimizationLevel::Low)
        {
            EPerf_PhysicsOptimizationLevel NewLevel = static_cast<EPerf_PhysicsOptimizationLevel>(
                static_cast<int32>(CurrentOptimizationLevel) - 1
            );
            SetOptimizationLevel(NewLevel);
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsSettings()
{
    // Apply global physics settings optimization
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Adjust physics settings based on optimization level
        switch (CurrentOptimizationLevel)
        {
            case EPerf_PhysicsOptimizationLevel::Low:
                // Reduce physics accuracy for performance
                break;
                
            case EPerf_PhysicsOptimizationLevel::Ultra:
                // Maximum physics quality
                break;
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::IntegrateWithCorePhysicsManager()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Integrating with Core Systems Physics Manager"));
    
    // This would integrate with Core_PhysicsManager when available
    // For now, we set up basic integration points
    
    OptimizePhysicsPerformance();
}

bool UPerf_PhysicsPerformanceIntegrator::ShouldOptimize() const
{
    return !IsPerformanceTargetMet() || CurrentMetrics.ActivePhysicsBodies > 100;
}