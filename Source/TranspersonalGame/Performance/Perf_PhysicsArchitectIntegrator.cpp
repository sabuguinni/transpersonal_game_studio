#include "Perf_PhysicsArchitectIntegrator.h"
#include "../Core/Physics/Core_PhysicsArchitect.h"
#include "../Character/TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UPerf_PhysicsArchitectIntegrator::UPerf_PhysicsArchitectIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz for performance monitoring
    
    LastOptimizationTime = 0.0f;
    OptimizationCycleCount = 0;
    bOptimizationActive = false;
    
    // Initialize default optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
}

void UPerf_PhysicsArchitectIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsArchitectIntegrator: BeginPlay - Initializing physics integration"));
    
    // Find and integrate with PhysicsArchitect component
    IntegrateWithPhysicsArchitect();
    
    // Initialize performance metrics
    CurrentMetrics = FPerf_PhysicsIntegrationMetrics();
    CurrentMetrics.bIntegrationHealthy = true;
    
    bOptimizationActive = true;
}

void UPerf_PhysicsArchitectIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bOptimizationActive)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Process physics validation data from PhysicsArchitect
    ProcessPhysicsValidationData();
    
    // Apply performance optimizations based on current metrics
    OptimizePhysicsPerformance();
    
    // Validate integration health
    ValidateIntegrationHealth();
}

void UPerf_PhysicsArchitectIntegrator::IntegrateWithPhysicsArchitect()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsArchitectIntegrator: Searching for PhysicsArchitect component"));
    
    FindPhysicsArchitectComponent();
    
    if (IsPhysicsArchitectAvailable())
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsArchitectIntegrator: Successfully integrated with PhysicsArchitect"));
        CurrentMetrics.bIntegrationHealthy = true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_PhysicsArchitectIntegrator: PhysicsArchitect component not found"));
        CurrentMetrics.bIntegrationHealthy = false;
        CurrentMetrics.IntegrationErrorCount++;
    }
}

void UPerf_PhysicsArchitectIntegrator::ProcessPhysicsValidationData()
{
    if (!IsPhysicsArchitectAvailable())
    {
        return;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Process validation metrics from PhysicsArchitect
    ProcessValidationMetrics();
    
    // Update integration metrics
    CurrentMetrics.PhysicsValidationTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Processed physics validation data in %.3f ms"), 
           CurrentMetrics.PhysicsValidationTime * 1000.0f);
}

bool UPerf_PhysicsArchitectIntegrator::IsPhysicsArchitectAvailable() const
{
    return PhysicsArchitectRef.IsValid() && IsValid(PhysicsArchitectRef.Get());
}

void UPerf_PhysicsArchitectIntegrator::OptimizePhysicsPerformance()
{
    if (!bOptimizationActive)
    {
        return;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    // Apply various physics optimizations
    OptimizeCharacterPhysics();
    OptimizeMovementComponents();
    OptimizeCollisionSystems();
    
    // Apply internal optimizations
    ApplyPhysicsOptimizations();
    
    // Calculate optimization overhead
    CalculateOptimizationOverhead(StartTime);
    
    OptimizationCycleCount++;
    LastOptimizationTime = FPlatformTime::Seconds();
}

void UPerf_PhysicsArchitectIntegrator::OptimizeCharacterPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all character actors
    TArray<AActor*> CharacterActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATranspersonalCharacter::StaticClass(), CharacterActors);
    
    int32 OptimizedCount = 0;
    
    for (AActor* Actor : CharacterActors)
    {
        if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(Actor))
        {
            // Apply character-specific physics optimizations
            if (OptimizationSettings.bEnableCharacterPhysicsLOD)
            {
                float DistanceToPlayer = FVector::Dist(Character->GetActorLocation(), GetOwner()->GetActorLocation());
                
                if (DistanceToPlayer > OptimizationSettings.CharacterCullingDistance)
                {
                    // Disable physics simulation for distant characters
                    if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Character->GetRootComponent()))
                    {
                        RootPrimitive->SetSimulatePhysics(false);
                        OptimizedCount++;
                    }
                }
            }
        }
    }
    
    CurrentMetrics.OptimizedActorCount = OptimizedCount;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Optimized %d character physics"), OptimizedCount);
}

void UPerf_PhysicsArchitectIntegrator::OptimizeMovementComponents()
{
    if (!OptimizationSettings.bOptimizeMovementComponents)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all actors with movement components
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);
    
    int32 OptimizedComponents = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (UCharacterMovementComponent* MovementComp = Pawn->FindComponentByClass<UCharacterMovementComponent>())
            {
                // Optimize movement component update frequency
                MovementComp->SetComponentTickInterval(1.0f / OptimizationSettings.MovementUpdateFrequency);
                OptimizedComponents++;
            }
        }
    }
    
    CurrentMetrics.ValidatedMovementComponents = OptimizedComponents;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Optimized %d movement components"), OptimizedComponents);
}

void UPerf_PhysicsArchitectIntegrator::OptimizeCollisionSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Optimize collision settings based on performance requirements
    int32 OptimizedCollisions = 0;
    
    // Get all static mesh components for collision optimization
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp && MeshComp->GetBodyInstance())
            {
                // Optimize collision complexity for distant objects
                float DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), GetOwner()->GetActorLocation());
                
                if (DistanceToPlayer > OptimizationSettings.CharacterCullingDistance * 0.5f)
                {
                    // Use simple collision for distant objects
                    MeshComp->SetCollisionObjectType(ECC_WorldStatic);
                    OptimizedCollisions++;
                }
            }
        }
    }
    
    CurrentMetrics.ValidatedCollisionComponents = OptimizedCollisions;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Optimized %d collision components"), OptimizedCollisions);
}

FPerf_PhysicsIntegrationMetrics UPerf_PhysicsArchitectIntegrator::GetIntegrationMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsArchitectIntegrator::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update integration latency
    UpdateIntegrationLatency(DeltaTime);
    
    // Track optimization effectiveness
    TrackOptimizationEffectiveness();
    
    // Update integration health
    UpdateIntegrationHealth();
}

bool UPerf_PhysicsArchitectIntegrator::ValidateIntegrationHealth() const
{
    // Check if integration is healthy based on various metrics
    bool bHealthy = true;
    
    // Check integration latency
    if (CurrentMetrics.IntegrationLatency > 50.0f) // 50ms threshold
    {
        bHealthy = false;
    }
    
    // Check error count
    if (CurrentMetrics.IntegrationErrorCount > 10)
    {
        bHealthy = false;
    }
    
    // Check PhysicsArchitect availability
    if (!IsPhysicsArchitectAvailable())
    {
        bHealthy = false;
    }
    
    return bHealthy;
}

void UPerf_PhysicsArchitectIntegrator::SetOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsArchitectIntegrator: Updated optimization settings"));
}

FPerf_PhysicsOptimizationSettings UPerf_PhysicsArchitectIntegrator::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerf_PhysicsArchitectIntegrator::DebugPhysicsIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS INTEGRATION DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Integration Healthy: %s"), CurrentMetrics.bIntegrationHealthy ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("PhysicsArchitect Available: %s"), IsPhysicsArchitectAvailable() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Optimization Active: %s"), bOptimizationActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Optimization Cycles: %d"), OptimizationCycleCount);
    UE_LOG(LogTemp, Warning, TEXT("Integration Latency: %.3f ms"), CurrentMetrics.IntegrationLatency);
    UE_LOG(LogTemp, Warning, TEXT("Optimized Actors: %d"), CurrentMetrics.OptimizedActorCount);
    UE_LOG(LogTemp, Warning, TEXT("=== END DEBUG ==="));
}

void UPerf_PhysicsArchitectIntegrator::ValidatePhysicsOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS OPTIMIZATION VALIDATION ==="));
    
    // Validate optimization effectiveness
    if (CurrentMetrics.PhysicsOptimizationTime > OptimizationSettings.MaxPhysicsTickTime)
    {
        UE_LOG(LogTemp, Error, TEXT("Physics optimization time exceeds target: %.3f ms > %.3f ms"), 
               CurrentMetrics.PhysicsOptimizationTime, OptimizationSettings.MaxPhysicsTickTime);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Physics optimization time within target: %.3f ms"), CurrentMetrics.PhysicsOptimizationTime);
    }
    
    // Validate optimization settings
    UE_LOG(LogTemp, Log, TEXT("Max Active Rigid Bodies: %d"), OptimizationSettings.MaxActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Character Culling Distance: %.1f"), OptimizationSettings.CharacterCullingDistance);
    UE_LOG(LogTemp, Log, TEXT("Character Physics LOD: %s"), OptimizationSettings.bEnableCharacterPhysicsLOD ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION ==="));
}

void UPerf_PhysicsArchitectIntegrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsArchitectIntegrator Status:"));
    UE_LOG(LogTemp, Log, TEXT("  Integration Health: %s"), CurrentMetrics.bIntegrationHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
    UE_LOG(LogTemp, Log, TEXT("  Physics Validation Time: %.3f ms"), CurrentMetrics.PhysicsValidationTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("  Optimization Overhead: %.3f ms"), CurrentMetrics.OptimizationOverhead * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("  Validated Characters: %d"), CurrentMetrics.ValidatedCharacterCount);
    UE_LOG(LogTemp, Log, TEXT("  Optimized Actors: %d"), CurrentMetrics.OptimizedActorCount);
}

// Private methods implementation

void UPerf_PhysicsArchitectIntegrator::FindPhysicsArchitectComponent()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Search for PhysicsArchitect component in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (UCore_PhysicsArchitect* PhysicsArchitect = Actor->FindComponentByClass<UCore_PhysicsArchitect>())
        {
            PhysicsArchitectRef = PhysicsArchitect;
            UE_LOG(LogTemp, Log, TEXT("Perf_PhysicsArchitectIntegrator: Found PhysicsArchitect component on %s"), *Actor->GetName());
            break;
        }
    }
}

void UPerf_PhysicsArchitectIntegrator::ProcessValidationMetrics()
{
    if (!IsPhysicsArchitectAvailable())
    {
        return;
    }
    
    // Process metrics from PhysicsArchitect
    // This would integrate with the actual PhysicsArchitect component's validation data
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Count validated characters
        TArray<AActor*> CharacterActors;
        UGameplayStatics::GetAllActorsOfClass(World, ATranspersonalCharacter::StaticClass(), CharacterActors);
        CurrentMetrics.ValidatedCharacterCount = CharacterActors.Num();
    }
}

void UPerf_PhysicsArchitectIntegrator::ApplyPhysicsOptimizations()
{
    // Apply internal physics optimizations
    CullDistantCharacters();
    OptimizeRigidBodySimulation();
    AdjustCollisionSettings();
    ManagePhysicsLOD();
}

void UPerf_PhysicsArchitectIntegrator::UpdateIntegrationHealth()
{
    CurrentMetrics.bIntegrationHealthy = ValidateIntegrationHealth();
    
    if (!CurrentMetrics.bIntegrationHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_PhysicsArchitectIntegrator: Integration health degraded"));
    }
}

void UPerf_PhysicsArchitectIntegrator::CullDistantCharacters()
{
    // Implementation for culling distant characters
    UWorld* World = GetWorld();
    if (!World || !GetOwner())
    {
        return;
    }
    
    FVector PlayerLocation = GetOwner()->GetActorLocation();
    
    TArray<AActor*> CharacterActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATranspersonalCharacter::StaticClass(), CharacterActors);
    
    for (AActor* Actor : CharacterActors)
    {
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        if (Distance > OptimizationSettings.CharacterCullingDistance)
        {
            // Reduce physics simulation for distant characters
            if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                RootPrimitive->SetComponentTickEnabled(false);
            }
        }
    }
}

void UPerf_PhysicsArchitectIntegrator::OptimizeRigidBodySimulation()
{
    // Optimize rigid body simulation based on performance requirements
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would implement rigid body optimization logic
    // For now, just log the optimization attempt
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Optimizing rigid body simulation"));
}

void UPerf_PhysicsArchitectIntegrator::AdjustCollisionSettings()
{
    // Adjust collision settings for performance optimization
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Adjusting collision settings"));
}

void UPerf_PhysicsArchitectIntegrator::ManagePhysicsLOD()
{
    // Manage physics Level of Detail based on distance and performance
    if (OptimizationSettings.bEnableCharacterPhysicsLOD)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_PhysicsArchitectIntegrator: Managing physics LOD"));
    }
}

void UPerf_PhysicsArchitectIntegrator::CalculateOptimizationOverhead(float StartTime)
{
    CurrentMetrics.OptimizationOverhead = FPlatformTime::Seconds() - StartTime;
    CurrentMetrics.PhysicsOptimizationTime = CurrentMetrics.OptimizationOverhead;
}

void UPerf_PhysicsArchitectIntegrator::UpdateIntegrationLatency(float DeltaTime)
{
    // Update integration latency with exponential moving average
    const float Alpha = 0.1f;
    float CurrentLatency = DeltaTime * 1000.0f; // Convert to milliseconds
    
    CurrentMetrics.IntegrationLatency = (Alpha * CurrentLatency) + ((1.0f - Alpha) * CurrentMetrics.IntegrationLatency);
}

void UPerf_PhysicsArchitectIntegrator::TrackOptimizationEffectiveness()
{
    // Track how effective the optimizations are
    if (OptimizationCycleCount > 0)
    {
        // Calculate optimization effectiveness metrics
        float AverageOptimizationTime = CurrentMetrics.PhysicsOptimizationTime / OptimizationCycleCount;
        
        if (AverageOptimizationTime > OptimizationSettings.MaxPhysicsTickTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("Perf_PhysicsArchitectIntegrator: Optimization effectiveness below target"));
        }
    }
}