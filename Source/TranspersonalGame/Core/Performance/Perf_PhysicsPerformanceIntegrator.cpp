#include "Perf_PhysicsPerformanceIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

UPerf_PhysicsPerformanceIntegrator::UPerf_PhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize performance tracking
    PhysicsFrameTimeAccumulator = 0.0f;
    PhysicsFrameCount = 0;
    LastOptimizationTime = 0.0f;
    LastPhysicsUpdateTime = 0.0f;
    
    // Initialize optimization state
    bPhysicsOptimizationActive = false;
    CurrentPhysicsLODLevel = 0;
    PhysicsQualityScale = 1.0f;
    
    // Set default optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
}

void UPerf_PhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Starting physics performance monitoring"));
    
    // Initialize physics tracking
    UpdatePhysicsActorTracking();
    
    // Register with performance systems
    RegisterWithPerformanceManager();
    
    // Integrate with core physics system
    IntegrateWithPhysicsSystemManager();
}

void UPerf_PhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePhysicsPerformanceMetrics();
    
    // Check if optimization is needed
    if (!IsPhysicsPerformanceWithinBudget())
    {
        OptimizePhysicsPerformance();
    }
    
    // Update physics actor tracking periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPhysicsUpdateTime > 1.0f) // Update every second
    {
        UpdatePhysicsActorTracking();
        LastPhysicsUpdateTime = CurrentTime;
    }
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsPerformanceIntegrator::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsPerformanceIntegrator::UpdatePhysicsPerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate physics frame time
    CalculatePhysicsFrameTime();
    
    // Count active physics bodies
    CurrentMetrics.ActivePhysicsBodies = TrackedPhysicsActors.Num();
    
    // Estimate collision checks (simplified calculation)
    CurrentMetrics.CollisionChecks = CurrentMetrics.ActivePhysicsBodies * CurrentMetrics.ActivePhysicsBodies / 10;
    
    // Calculate ragdoll performance cost
    CurrentMetrics.RagdollPerformanceCost = 0.0f;
    for (AActor* Actor : RagdollActors)
    {
        if (IsValid(Actor))
        {
            CurrentMetrics.RagdollPerformanceCost += CalculateActorPhysicsCost(Actor);
        }
    }
    
    // Estimate destruction performance cost
    CurrentMetrics.DestructionPerformanceCost = CurrentMetrics.ActivePhysicsBodies * 0.1f; // 0.1ms per body
}

bool UPerf_PhysicsPerformanceIntegrator::IsPhysicsPerformanceWithinBudget() const
{
    // Check if physics frame time is within budget
    if (CurrentMetrics.PhysicsFrameTime > OptimizationSettings.MaxPhysicsFrameTime)
    {
        return false;
    }
    
    // Check if active physics bodies are within limits
    if (CurrentMetrics.ActivePhysicsBodies > OptimizationSettings.MaxActivePhysicsBodies)
    {
        return false;
    }
    
    return true;
}

void UPerf_PhysicsPerformanceIntegrator::OptimizePhysicsPerformance()
{
    if (bPhysicsOptimizationActive)
    {
        return; // Already optimizing
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Starting physics optimization"));
    bPhysicsOptimizationActive = true;
    
    // Apply various optimization strategies
    ApplyPhysicsLOD();
    OptimizeCollisionShapes();
    ManageRagdollPerformance();
    
    // Scale physics quality if needed
    if (CurrentMetrics.PhysicsFrameTime > OptimizationSettings.MaxPhysicsFrameTime * 1.5f)
    {
        ScalePhysicsQuality(0.8f); // Reduce quality by 20%
    }
    
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    bPhysicsOptimizationActive = false;
}

void UPerf_PhysicsPerformanceIntegrator::ApplyPhysicsLOD()
{
    if (!OptimizationSettings.bEnablePhysicsLOD)
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        // Apply LOD based on distance
        if (Distance > OptimizationSettings.PhysicsLODDistance)
        {
            // Disable physics for distant objects
            UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
                Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
            
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->SetSimulatePhysics(false);
                UE_LOG(LogTemp, Log, TEXT("Physics LOD: Disabled physics for distant actor %s"), *Actor->GetName());
            }
        }
        else if (Distance < OptimizationSettings.PhysicsLODDistance * 0.8f)
        {
            // Re-enable physics for nearby objects
            UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
                Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
            
            if (PrimComp && !PrimComp->IsSimulatingPhysics() && ShouldOptimizePhysicsActor(Actor))
            {
                PrimComp->SetSimulatePhysics(true);
                UE_LOG(LogTemp, Log, TEXT("Physics LOD: Re-enabled physics for nearby actor %s"), *Actor->GetName());
            }
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::OptimizeCollisionShapes()
{
    if (!OptimizationSettings.bEnableCollisionOptimization)
    {
        return;
    }
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
            Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
        
        if (PrimComp)
        {
            // Simplify collision for performance
            if (PrimComp->GetCollisionObjectType() == ECC_WorldDynamic)
            {
                // Use simplified collision for dynamic objects
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
                PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
            }
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::ManageRagdollPerformance()
{
    // Limit number of active ragdolls
    int32 MaxRagdolls = 5; // Maximum 5 ragdolls at once
    
    if (RagdollActors.Num() > MaxRagdolls)
    {
        // Disable oldest ragdolls
        for (int32 i = 0; i < RagdollActors.Num() - MaxRagdolls; i++)
        {
            if (IsValid(RagdollActors[i]))
            {
                USkeletalMeshComponent* SkelMesh = RagdollActors[i]->FindComponentByClass<USkeletalMeshComponent>();
                if (SkelMesh)
                {
                    SkelMesh->SetSimulatePhysics(false);
                    UE_LOG(LogTemp, Log, TEXT("Ragdoll Performance: Disabled ragdoll for %s"), *RagdollActors[i]->GetName());
                }
            }
        }
    }
}

void UPerf_PhysicsPerformanceIntegrator::IntegrateWithPhysicsSystemManager()
{
    // Try to find and integrate with Core Physics System Manager
    UClass* PhysicsManagerClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Core_PhysicsSystemManager"));
    if (PhysicsManagerClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Found Core Physics System Manager"));
        
        // Find existing physics manager in world
        TArray<AActor*> PhysicsManagers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), PhysicsManagerClass, PhysicsManagers);
        
        if (PhysicsManagers.Num() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Integrated with existing Physics System Manager"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: No Physics System Manager found in world"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Core Physics System Manager class not found"));
    }
}

void UPerf_PhysicsPerformanceIntegrator::RegisterWithPerformanceManager()
{
    // Register this component with the main performance management system
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Registered with Performance Manager"));
}

void UPerf_PhysicsPerformanceIntegrator::ScalePhysicsQuality(float QualityScale)
{
    PhysicsQualityScale = FMath::Clamp(QualityScale, 0.1f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Scaled physics quality to %f"), PhysicsQualityScale);
    
    // Apply quality scaling to physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Adjust physics solver iterations based on quality scale
        // Note: This is a simplified example - real implementation would be more sophisticated
    }
}

void UPerf_PhysicsPerformanceIntegrator::SetPhysicsLODLevel(int32 LODLevel)
{
    CurrentPhysicsLODLevel = FMath::Clamp(LODLevel, 0, 3);
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator: Set physics LOD level to %d"), CurrentPhysicsLODLevel);
    
    // Apply LOD level settings
    switch (CurrentPhysicsLODLevel)
    {
        case 0: // High quality
            OptimizationSettings.MaxActivePhysicsBodies = 150;
            OptimizationSettings.PhysicsLODDistance = 3000.0f;
            break;
        case 1: // Medium quality
            OptimizationSettings.MaxActivePhysicsBodies = 100;
            OptimizationSettings.PhysicsLODDistance = 2000.0f;
            break;
        case 2: // Low quality
            OptimizationSettings.MaxActivePhysicsBodies = 50;
            OptimizationSettings.PhysicsLODDistance = 1000.0f;
            break;
        case 3: // Minimal quality
            OptimizationSettings.MaxActivePhysicsBodies = 25;
            OptimizationSettings.PhysicsLODDistance = 500.0f;
            break;
    }
}

void UPerf_PhysicsPerformanceIntegrator::UpdatePhysicsActorTracking()
{
    TrackedPhysicsActors.Empty();
    RagdollActors.Empty();
    
    if (!GetWorld())
    {
        return;
    }
    
    // Find all actors with physics components
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!IsValid(Actor))
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
            Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
        
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            TrackedPhysicsActors.Add(Actor);
            
            // Check if this is a ragdoll
            USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
            if (SkelMesh && SkelMesh->IsSimulatingPhysics())
            {
                RagdollActors.Add(Actor);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Integrator: Tracking %d physics actors, %d ragdolls"), 
           TrackedPhysicsActors.Num(), RagdollActors.Num());
}

void UPerf_PhysicsPerformanceIntegrator::CalculatePhysicsFrameTime()
{
    // Simplified physics frame time calculation
    // In a real implementation, this would use actual profiling data
    float EstimatedFrameTime = CurrentMetrics.ActivePhysicsBodies * 0.05f; // 0.05ms per physics body
    EstimatedFrameTime += CurrentMetrics.CollisionChecks * 0.001f; // 0.001ms per collision check
    EstimatedFrameTime += CurrentMetrics.RagdollPerformanceCost;
    
    CurrentMetrics.PhysicsFrameTime = EstimatedFrameTime;
}

void UPerf_PhysicsPerformanceIntegrator::ApplyPhysicsOptimizations()
{
    // Apply various physics optimizations based on current performance
    if (CurrentMetrics.PhysicsFrameTime > OptimizationSettings.MaxPhysicsFrameTime)
    {
        ApplyPhysicsLOD();
        OptimizeCollisionShapes();
    }
}

bool UPerf_PhysicsPerformanceIntegrator::ShouldOptimizePhysicsActor(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    // Don't optimize player character
    if (Actor->IsA<ACharacter>())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (Actor == PlayerPawn)
        {
            return false;
        }
    }
    
    return true;
}

float UPerf_PhysicsPerformanceIntegrator::CalculateActorPhysicsCost(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return 0.0f;
    }
    
    float Cost = 0.5f; // Base cost
    
    // Add cost for skeletal mesh (ragdoll)
    USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (SkelMesh && SkelMesh->IsSimulatingPhysics())
    {
        Cost += 2.0f; // Ragdolls are expensive
    }
    
    // Add cost for complex collision
    UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
        Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
    
    if (PrimComp)
    {
        // Complex collision is more expensive
        if (PrimComp->GetCollisionObjectType() == ECC_WorldDynamic)
        {
            Cost += 1.0f;
        }
    }
    
    return Cost;
}