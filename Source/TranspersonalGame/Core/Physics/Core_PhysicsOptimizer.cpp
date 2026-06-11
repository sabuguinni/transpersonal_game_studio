#include "Core_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"

UCore_PhysicsOptimizer::UCore_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    MaxPhysicsObjects = 500;
    PhysicsLODDistance = 2000.0f;
    bEnablePhysicsPooling = true;
    PhysicsSubsteps = 2;
    bOptimizeCollisionComplexity = true;
    CollisionLODDistance = 1500.0f;
    bSimplifyCollisionShapes = true;
    
    // Performance monitoring defaults
    PhysicsFrameTime = 0.0f;
    ActivePhysicsObjects = 0;
    PhysicsMemoryUsage = 0.0f;
    PerformanceUpdateTimer = 0.0f;
    TargetPhysicsFrameTime = 16.67f; // 60fps target
    CurrentQualityLevel = 2; // Medium-High quality by default
    LastPerformanceMeasurement = 0.0f;
}

void UCore_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Beginning physics optimization"));
    
    // Initial physics scan and optimization
    ScanPhysicsObjects();
    OptimizePhysicsSettings();
    UpdatePhysicsWorldSettings();
}

void UCore_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    PerformanceUpdateTimer += DeltaTime;
    
    // Update performance metrics every 0.5 seconds
    if (PerformanceUpdateTimer >= 0.5f)
    {
        UpdatePerformanceMetrics();
        UpdatePhysicsLOD();
        ManageDistantPhysics();
        
        // Adaptive quality adjustment
        if (PhysicsFrameTime > TargetPhysicsFrameTime * 1.2f)
        {
            AdaptPhysicsQuality();
        }
        
        PerformanceUpdateTimer = 0.0f;
    }
}

void UCore_PhysicsOptimizer::OptimizePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsOptimizer: No world found for optimization"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Optimizing physics settings"));
    
    // Get physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Optimize solver settings based on quality level
        switch (CurrentQualityLevel)
        {
        case 0: // Low quality
            PhysicsSettings->DefaultGravityZ = -980.0f;
            PhysicsSettings->bSubstepping = false;
            break;
        case 1: // Medium quality
            PhysicsSettings->DefaultGravityZ = -980.0f;
            PhysicsSettings->bSubstepping = true;
            PhysicsSettings->MaxSubstepDeltaTime = 0.01f;
            break;
        case 2: // High quality
            PhysicsSettings->DefaultGravityZ = -980.0f;
            PhysicsSettings->bSubstepping = true;
            PhysicsSettings->MaxSubstepDeltaTime = 0.008f;
            break;
        case 3: // Ultra quality
            PhysicsSettings->DefaultGravityZ = -980.0f;
            PhysicsSettings->bSubstepping = true;
            PhysicsSettings->MaxSubstepDeltaTime = 0.005f;
            break;
        }
    }
    
    // Optimize collision complexity if enabled
    if (bOptimizeCollisionComplexity)
    {
        OptimizeCollisionComplexity();
    }
}

void UCore_PhysicsOptimizer::UpdatePhysicsLOD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for tracked physics actors
    for (int32 i = TrackedPhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (TrackedPhysicsActors[i].IsValid())
        {
            AActor* Actor = TrackedPhysicsActors[i].Get();
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            ApplyPhysicsLOD(Actor, Distance);
        }
        else
        {
            // Remove invalid actors
            TrackedPhysicsActors.RemoveAt(i);
        }
    }
}

void UCore_PhysicsOptimizer::ManageDistantPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Disable physics for very distant objects
    for (TWeakObjectPtr<AActor>& ActorPtr : TrackedPhysicsActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (RootPrimitive)
            {
                if (Distance > PhysicsLODDistance * 2.0f)
                {
                    // Disable physics for very distant objects
                    RootPrimitive->SetSimulatePhysics(false);
                }
                else if (Distance < PhysicsLODDistance * 1.5f && !RootPrimitive->IsSimulatingPhysics())
                {
                    // Re-enable physics for objects coming back into range
                    RootPrimitive->SetSimulatePhysics(true);
                }
            }
        }
    }
}

void UCore_PhysicsOptimizer::OptimizeCollisionComplexity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize collision complexity based on distance
    for (TWeakObjectPtr<AActor>& ActorPtr : TrackedPhysicsActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (RootPrimitive)
            {
                if (Distance > CollisionLODDistance)
                {
                    // Use simple collision for distant objects
                    RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                else
                {
                    // Use full collision for nearby objects
                    RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
            }
        }
    }
}

void UCore_PhysicsOptimizer::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update active physics object count
    ActivePhysicsObjects = TrackedPhysicsActors.Num();
    
    // Estimate physics frame time (simplified)
    float CurrentTime = World->GetTimeSeconds();
    if (LastPerformanceMeasurement > 0.0f)
    {
        float DeltaTime = CurrentTime - LastPerformanceMeasurement;
        // Rough estimation of physics contribution to frame time
        PhysicsFrameTime = DeltaTime * 1000.0f * (ActivePhysicsObjects / 100.0f);
        PhysicsFrameTime = FMath::Clamp(PhysicsFrameTime, 0.1f, 50.0f);
    }
    LastPerformanceMeasurement = CurrentTime;
    
    // Estimate physics memory usage (simplified)
    PhysicsMemoryUsage = ActivePhysicsObjects * 0.1f; // Rough estimate: 0.1MB per physics object
}

void UCore_PhysicsOptimizer::ForcePhysicsCleanup()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Forcing physics cleanup"));
    
    // Remove invalid actors from tracking
    for (int32 i = TrackedPhysicsActors.Num() - 1; i >= 0; i--)
    {
        if (!TrackedPhysicsActors[i].IsValid())
        {
            TrackedPhysicsActors.RemoveAt(i);
        }
    }
    
    // Force garbage collection
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    // Rescan physics objects
    ScanPhysicsObjects();
}

void UCore_PhysicsOptimizer::AdaptPhysicsQuality()
{
    if (PhysicsFrameTime > TargetPhysicsFrameTime * 1.5f)
    {
        // Reduce quality if performance is poor
        CurrentQualityLevel = FMath::Max(0, CurrentQualityLevel - 1);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Reducing physics quality to level %d"), CurrentQualityLevel);
    }
    else if (PhysicsFrameTime < TargetPhysicsFrameTime * 0.8f && CurrentQualityLevel < 3)
    {
        // Increase quality if performance allows
        CurrentQualityLevel = FMath::Min(3, CurrentQualityLevel + 1);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Increasing physics quality to level %d"), CurrentQualityLevel);
    }
    
    OptimizePhysicsSettings();
}

void UCore_PhysicsOptimizer::SetPhysicsQualityLevel(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    OptimizePhysicsSettings();
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Physics quality set to level %d"), CurrentQualityLevel);
}

void UCore_PhysicsOptimizer::ScanPhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TrackedPhysicsActors.Empty();
    
    // Scan all actors in the world for physics simulation
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (RootPrimitive && RootPrimitive->IsSimulatingPhysics())
            {
                TrackedPhysicsActors.Add(Actor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsOptimizer: Scanned %d physics objects"), TrackedPhysicsActors.Num());
}

void UCore_PhysicsOptimizer::ApplyPhysicsLOD(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!RootPrimitive)
    {
        return;
    }
    
    int32 PhysicsQuality = CalculatePhysicsQuality(Distance);
    
    // Apply LOD based on calculated quality
    switch (PhysicsQuality)
    {
    case 0: // Lowest quality - disable physics
        RootPrimitive->SetSimulatePhysics(false);
        break;
    case 1: // Low quality - simple collision only
        RootPrimitive->SetSimulatePhysics(true);
        RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        break;
    case 2: // Medium quality - full collision, reduced substeps
        RootPrimitive->SetSimulatePhysics(true);
        RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        break;
    case 3: // High quality - full physics
        RootPrimitive->SetSimulatePhysics(true);
        RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        break;
    }
}

int32 UCore_PhysicsOptimizer::CalculatePhysicsQuality(float Distance)
{
    if (Distance > PhysicsLODDistance * 2.0f)
    {
        return 0; // Disable physics
    }
    else if (Distance > PhysicsLODDistance * 1.5f)
    {
        return 1; // Low quality
    }
    else if (Distance > PhysicsLODDistance)
    {
        return 2; // Medium quality
    }
    else
    {
        return 3; // High quality
    }
}

void UCore_PhysicsOptimizer::UpdatePhysicsWorldSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update world physics settings based on current quality level
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Apply quality-based settings
        switch (CurrentQualityLevel)
        {
        case 0: // Low
            PhysicsSettings->bSubstepping = false;
            break;
        case 1: // Medium
            PhysicsSettings->bSubstepping = true;
            PhysicsSettings->MaxSubsteps = 2;
            break;
        case 2: // High
            PhysicsSettings->bSubstepping = true;
            PhysicsSettings->MaxSubsteps = 4;
            break;
        case 3: // Ultra
            PhysicsSettings->bSubstepping = true;
            PhysicsSettings->MaxSubsteps = 6;
            break;
        }
    }
}