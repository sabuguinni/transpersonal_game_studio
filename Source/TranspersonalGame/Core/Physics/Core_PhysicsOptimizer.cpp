#include "Core_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsOptimizer::UCore_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance history
    PerformanceHistory.Reserve(60); // Store 60 samples (6 seconds at 10Hz)
}

void UCore_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache physics settings reference
    PhysicsSettings = GetMutableDefault<UPhysicsSettings>();
    
    // Initialize physics object tracking
    RefreshPhysicsObjectList();
    
    // Set initial optimization time
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Initialized with %d physics objects"), TrackedPhysicsObjects.Num());
}

void UCore_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics every tick
    UpdatePerformanceMetrics();
    
    // Run optimization at specified intervals
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        if (bEnablePhysicsLOD)
        {
            UpdatePhysicsLOD();
        }
        
        if (bEnableAdaptiveTimestep)
        {
            AdjustPhysicsTimestep();
        }
        
        if (bEnablePhysicsCulling)
        {
            CullDistantPhysicsObjects();
        }
        
        OptimizePhysicsSettings();
        LastOptimizationTime = CurrentTime;
    }
}

void UCore_PhysicsOptimizer::UpdatePhysicsLOD()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Clean up invalid object references
    TrackedPhysicsObjects.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
    
    // Sort objects by distance and importance
    TArray<TPair<float, TWeakObjectPtr<AActor>>> ObjectDistances;
    
    for (const TWeakObjectPtr<AActor>& WeakActor : TrackedPhysicsObjects)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            ObjectDistances.Add(TPair<float, TWeakObjectPtr<AActor>>(Distance, WeakActor));
        }
    }
    
    // Sort by distance (closest first)
    ObjectDistances.Sort([](const TPair<float, TWeakObjectPtr<AActor>>& A, const TPair<float, TWeakObjectPtr<AActor>>& B)
    {
        return A.Key < B.Key;
    });
    
    // Apply LOD levels based on distance and budget
    int32 HighDetailCount = 0;
    int32 MediumDetailCount = 0;
    
    for (const auto& ObjectDistance : ObjectDistances)
    {
        AActor* Actor = ObjectDistance.Value.Get();
        if (!Actor)
        {
            continue;
        }
        
        float Distance = ObjectDistance.Key;
        int32 LODLevel = 0; // Default to high detail
        
        // Determine LOD level based on distance and budget
        if (Distance <= HighDetailDistance && HighDetailCount < MaxHighDetailObjects)
        {
            LODLevel = 0; // High detail
            HighDetailCount++;
        }
        else if (Distance <= MediumDetailDistance && MediumDetailCount < MaxMediumDetailObjects)
        {
            LODLevel = 1; // Medium detail
            MediumDetailCount++;
        }
        else if (Distance <= LowDetailDistance)
        {
            LODLevel = 2; // Low detail
        }
        else
        {
            LODLevel = 3; // Disabled/culled
        }
        
        ApplyPhysicsLOD(Actor, LODLevel);
    }
    
    ActivePhysicsObjects = HighDetailCount + MediumDetailCount;
}

void UCore_PhysicsOptimizer::ApplyPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
        {
            continue;
        }
        
        switch (LODLevel)
        {
            case 0: // High detail
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                if (PrimComp->IsSimulatingPhysics())
                {
                    // Full physics simulation
                    PrimComp->SetSimulatePhysics(true);
                }
                break;
                
            case 1: // Medium detail
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                if (PrimComp->IsSimulatingPhysics())
                {
                    // Reduced physics update rate
                    PrimComp->SetSimulatePhysics(true);
                }
                break;
                
            case 2: // Low detail
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                // Disable physics simulation but keep collision
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                }
                break;
                
            case 3: // Disabled/culled
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                }
                break;
        }
    }
}

void UCore_PhysicsOptimizer::OptimizePhysicsSettings()
{
    if (!PhysicsSettings)
    {
        return;
    }
    
    // Calculate performance budget
    PhysicsPerformanceBudget = CalculatePerformanceBudget();
    
    // Adjust physics settings based on performance
    if (PhysicsPerformanceBudget < 0.5f) // Performance is poor
    {
        // Reduce physics quality
        SetPhysicsQualityLevel(0); // Low quality
    }
    else if (PhysicsPerformanceBudget < 0.8f) // Performance is moderate
    {
        // Medium physics quality
        SetPhysicsQualityLevel(1); // Medium quality
    }
    else // Performance is good
    {
        // High physics quality
        SetPhysicsQualityLevel(2); // High quality
    }
}

void UCore_PhysicsOptimizer::UpdatePerformanceMetrics()
{
    // Get current frame time (simplified - in real implementation would use more sophisticated timing)
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentPhysicsFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Add to performance history
    PerformanceHistory.Add(CurrentPhysicsFrameTime);
    if (PerformanceHistory.Num() > 60)
    {
        PerformanceHistory.RemoveAt(0); // Keep only last 60 samples
    }
}

float UCore_PhysicsOptimizer::CalculatePerformanceBudget()
{
    if (PerformanceHistory.Num() == 0)
    {
        return 1.0f;
    }
    
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    for (float FrameTime : PerformanceHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= PerformanceHistory.Num();
    
    // Calculate budget based on target vs actual performance
    float Budget = TargetPhysicsFrameTime / FMath::Max(AverageFrameTime, 0.1f);
    return FMath::Clamp(Budget, 0.0f, 1.0f);
}

void UCore_PhysicsOptimizer::AdjustPhysicsTimestep()
{
    if (!PhysicsSettings)
    {
        return;
    }
    
    // Adjust timestep based on performance budget
    float TargetTimestep = FMath::Lerp(MaxPhysicsTimestep, MinPhysicsTimestep, PhysicsPerformanceBudget);
    
    // Apply the new timestep (this would require engine modifications in a real implementation)
    // For now, we'll just log the recommended timestep
    UE_LOG(LogTemp, VeryVerbose, TEXT("Recommended Physics Timestep: %f"), TargetTimestep);
}

void UCore_PhysicsOptimizer::CullDistantPhysicsObjects()
{
    if (!GetWorld())
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Disable physics for very distant objects
    for (const TWeakObjectPtr<AActor>& WeakActor : TrackedPhysicsObjects)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance > LowDetailDistance * 2.0f) // Double the low detail distance for culling
            {
                SetObjectPhysicsEnabled(Actor, false);
            }
        }
    }
}

void UCore_PhysicsOptimizer::RefreshPhysicsObjectList()
{
    TrackedPhysicsObjects.Empty();
    
    if (!GetWorld())
    {
        return;
    }
    
    // Find all actors with physics components
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }
        
        // Check if actor has physics components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        bool bHasPhysics = false;
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && (PrimComp->IsSimulatingPhysics() || PrimComp->IsCollisionEnabled()))
            {
                bHasPhysics = true;
                break;
            }
        }
        
        if (bHasPhysics)
        {
            TrackedPhysicsObjects.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsOptimizer: Tracking %d physics objects"), TrackedPhysicsObjects.Num());
}

// === PUBLIC BLUEPRINT FUNCTIONS ===

void UCore_PhysicsOptimizer::GetPhysicsPerformanceMetrics(float& FrameTime, int32& ObjectCount, float& Budget)
{
    FrameTime = CurrentPhysicsFrameTime;
    ObjectCount = ActivePhysicsObjects;
    Budget = PhysicsPerformanceBudget;
}

void UCore_PhysicsOptimizer::SetPhysicsQualityLevel(int32 QualityLevel)
{
    switch (QualityLevel)
    {
        case 0: // Low quality
            MaxHighDetailObjects = 25;
            MaxMediumDetailObjects = 50;
            TargetPhysicsFrameTime = 16.67f; // 60Hz
            break;
            
        case 1: // Medium quality
            MaxHighDetailObjects = 50;
            MaxMediumDetailObjects = 100;
            TargetPhysicsFrameTime = 11.11f; // 90Hz
            break;
            
        case 2: // High quality
            MaxHighDetailObjects = 100;
            MaxMediumDetailObjects = 200;
            TargetPhysicsFrameTime = 8.33f; // 120Hz
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Quality Level set to: %d"), QualityLevel);
}

void UCore_PhysicsOptimizer::SetObjectPhysicsEnabled(AActor* Actor, bool bEnabled)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            if (bEnabled)
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                if (PrimComp->GetBodyInstance()->bSimulatePhysics)
                {
                    PrimComp->SetSimulatePhysics(true);
                }
            }
            else
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimComp->SetSimulatePhysics(false);
            }
        }
    }
}

void UCore_PhysicsOptimizer::GetRecommendedPhysicsSettings(float& Gravity, float& Timestep, int32& MaxObjects)
{
    if (PhysicsSettings)
    {
        Gravity = PhysicsSettings->DefaultGravityZ;
    }
    else
    {
        Gravity = -980.0f; // Default gravity
    }
    
    // Recommend timestep based on current performance
    Timestep = FMath::Lerp(MaxPhysicsTimestep, MinPhysicsTimestep, PhysicsPerformanceBudget);
    
    // Recommend max objects based on performance
    MaxObjects = FMath::RoundToInt(FMath::Lerp(50.0f, 200.0f, PhysicsPerformanceBudget));
}