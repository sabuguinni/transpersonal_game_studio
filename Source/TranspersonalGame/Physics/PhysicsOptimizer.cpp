// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformApplicationMisc.h"

UPhysicsOptimizer::UPhysicsOptimizer()
{
    // Initialize default LOD settings
    LODSettings.NearDistance = 1000.0f;
    LODSettings.MidDistance = 5000.0f;
    LODSettings.FarDistance = 10000.0f;
    LODSettings.MaxNearObjects = 100;
    LODSettings.MaxMidObjects = 50;
    LODSettings.MaxFarObjects = 20;

    UpdateFrequency = 0.1f;
    MaxPhysicsLoad = 0.8f;
    bEnableAdaptiveLOD = true;
    bEnablePhysicsCulling = true;

    CurrentPhysicsLoad = 0.0f;
    LastUpdateTime = 0.0f;
    FrameCounter = 0;
    AverageFrameTime = 16.67f; // Start with 60 FPS assumption

    // Reserve space for performance tracking
    RecentFrameTimes.Reserve(60); // Track last 60 frames
}

void UPhysicsOptimizer::InitializeOptimizer(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsOptimizer: Cannot initialize with null World"));
        return;
    }

    WorldContext = World;
    bOptimizationActive = true;
    LastUpdateTime = World->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimizer: Initialized for world %s"), *World->GetName());

    // Set initial physics settings for optimization
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Optimize physics substeps based on target framerate
        PhysicsSettings->MaxSubstepDeltaTime = 0.016667f; // 60 FPS
        PhysicsSettings->MaxSubsteps = 6;
        PhysicsSettings->SyncSceneSmoothingFactor = 0.0f; // Disable for performance
    }
}

void UPhysicsOptimizer::UpdateOptimization(float DeltaTime)
{
    if (!bOptimizationActive || !WorldContext.IsValid())
    {
        return;
    }

    float CurrentTime = WorldContext->GetTimeSeconds();
    
    // Update frame time tracking
    UpdateFrameTimeTracking(DeltaTime);

    // Process pending registrations/unregistrations
    ProcessPendingRegistrations();
    ProcessPendingUnregistrations();

    // Only run full optimization at specified frequency
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        // Calculate current physics load
        CalculatePhysicsLoad();

        // Update LOD based on performance
        if (bEnableAdaptiveLOD)
        {
            ApplyAdaptiveLOD();
        }

        // Update physics LOD for all objects
        UpdatePhysicsLOD();

        // Cull distant objects if enabled
        if (bEnablePhysicsCulling)
        {
            CullDistantObjects();
        }

        // Adjust physics substeps based on load
        UpdatePhysicsSubsteps();

        LastUpdateTime = CurrentTime;
    }

    FrameCounter++;
}

void UPhysicsOptimizer::RegisterPhysicsObject(UPrimitiveComponent* Component)
{
    if (!Component || !bOptimizationActive)
    {
        return;
    }

    // Add to pending registrations for thread-safe processing
    PendingRegistrations.Enqueue(Component);
}

void UPhysicsOptimizer::UnregisterPhysicsObject(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }

    // Add to pending unregistrations for thread-safe processing
    PendingUnregistrations.Enqueue(Component);
}

void UPhysicsOptimizer::UpdatePhysicsLOD()
{
    if (!WorldContext.IsValid())
    {
        return;
    }

    FVector PlayerLocation = GetPlayerLocation();
    
    // Count objects in each LOD level
    int32 NearCount = 0, MidCount = 0, FarCount = 0;

    // First pass: calculate distances and count current LOD levels
    for (auto& ObjectPair : TrackedObjects)
    {
        if (!ObjectPair.Key.IsValid())
        {
            continue;
        }

        FPhysicsObjectInfo& ObjectInfo = ObjectPair.Value;
        UPrimitiveComponent* Component = ObjectPair.Key.Get();
        
        // Calculate distance to player
        float Distance = FVector::Dist(Component->GetComponentLocation(), PlayerLocation);
        ObjectInfo.DistanceToPlayer = Distance;

        // Determine appropriate LOD level
        int32 NewLODLevel = 3; // Default to disabled
        
        if (Distance <= LODSettings.NearDistance && NearCount < LODSettings.MaxNearObjects)
        {
            NewLODLevel = 0;
            NearCount++;
        }
        else if (Distance <= LODSettings.MidDistance && MidCount < LODSettings.MaxMidObjects)
        {
            NewLODLevel = 1;
            MidCount++;
        }
        else if (Distance <= LODSettings.FarDistance && FarCount < LODSettings.MaxFarObjects)
        {
            NewLODLevel = 2;
            FarCount++;
        }

        // Apply LOD if changed
        if (ObjectInfo.LODLevel != NewLODLevel)
        {
            SetPhysicsLODLevel(Component, NewLODLevel);
            ObjectInfo.LODLevel = NewLODLevel;
        }

        ObjectInfo.LastUpdateTime = WorldContext->GetTimeSeconds();
    }
}

void UPhysicsOptimizer::SetPhysicsLODLevel(UPrimitiveComponent* Component, int32 LODLevel)
{
    if (!Component)
    {
        return;
    }

    switch (LODLevel)
    {
        case 0: // Near - Full physics
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetSimulatePhysics(true);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            OptimizeCollisionSettings(Component, LODLevel);
            break;

        case 1: // Mid - Reduced physics
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetSimulatePhysics(true);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
            OptimizeCollisionSettings(Component, LODLevel);
            break;

        case 2: // Far - Query only
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Component->SetSimulatePhysics(false);
            Component->SetCollisionResponseToAllChannels(ECR_Ignore);
            Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            break;

        case 3: // Disabled
        default:
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetSimulatePhysics(false);
            Component->SetCollisionResponseToAllChannels(ECR_Ignore);
            break;
    }
}

void UPhysicsOptimizer::ProcessPendingRegistrations()
{
    TWeakObjectPtr<UPrimitiveComponent> Component;
    while (PendingRegistrations.Dequeue(Component))
    {
        if (Component.IsValid())
        {
            FPhysicsObjectInfo ObjectInfo;
            ObjectInfo.Component = Component;
            ObjectInfo.bIsActive = true;
            ObjectInfo.LastUpdateTime = WorldContext.IsValid() ? WorldContext->GetTimeSeconds() : 0.0f;
            ObjectInfo.LODLevel = 0; // Start with full physics

            TrackedObjects.Add(Component, ObjectInfo);
            ActivePhysicsObjects.Add(Component);

            UE_LOG(LogTemp, Verbose, TEXT("PhysicsOptimizer: Registered object %s"), 
                   *Component->GetName());
        }
    }
}

void UPhysicsOptimizer::ProcessPendingUnregistrations()
{
    TWeakObjectPtr<UPrimitiveComponent> Component;
    while (PendingUnregistrations.Dequeue(Component))
    {
        TrackedObjects.Remove(Component);
        ActivePhysicsObjects.Remove(Component);

        UE_LOG(LogTemp, Verbose, TEXT("PhysicsOptimizer: Unregistered object"));
    }
}

void UPhysicsOptimizer::CalculatePhysicsLoad()
{
    // Calculate physics load based on frame time and active objects
    float TargetFrameTime = 16.67f; // 60 FPS target
    float LoadFromFrameTime = FMath::Clamp(AverageFrameTime / TargetFrameTime, 0.0f, 2.0f);
    
    // Factor in number of active physics objects
    int32 ActiveObjects = 0;
    for (const auto& ObjectPair : TrackedObjects)
    {
        if (ObjectPair.Value.LODLevel <= 1) // Near and Mid LOD
        {
            ActiveObjects++;
        }
    }
    
    float LoadFromObjects = FMath::Clamp(ActiveObjects / 100.0f, 0.0f, 2.0f);
    
    // Combine both factors
    CurrentPhysicsLoad = (LoadFromFrameTime * 0.7f + LoadFromObjects * 0.3f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("PhysicsOptimizer: Load=%.2f, FrameTime=%.2f, ActiveObjects=%d"), 
           CurrentPhysicsLoad, AverageFrameTime, ActiveObjects);
}

void UPhysicsOptimizer::UpdateFrameTimeTracking(float DeltaTime)
{
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    RecentFrameTimes.Add(FrameTimeMs);
    if (RecentFrameTimes.Num() > 60)
    {
        RecentFrameTimes.RemoveAt(0);
    }
    
    // Calculate rolling average
    float Sum = 0.0f;
    for (float Time : RecentFrameTimes)
    {
        Sum += Time;
    }
    AverageFrameTime = Sum / RecentFrameTimes.Num();
}

void UPhysicsOptimizer::ApplyAdaptiveLOD()
{
    if (CurrentPhysicsLoad > MaxPhysicsLoad)
    {
        // Reduce LOD distances to improve performance
        LODSettings.NearDistance *= 0.9f;
        LODSettings.MidDistance *= 0.9f;
        LODSettings.FarDistance *= 0.9f;
        
        // Reduce max object counts
        LODSettings.MaxNearObjects = FMath::Max(20, int32(LODSettings.MaxNearObjects * 0.9f));
        LODSettings.MaxMidObjects = FMath::Max(10, int32(LODSettings.MaxMidObjects * 0.9f));
        LODSettings.MaxFarObjects = FMath::Max(5, int32(LODSettings.MaxFarObjects * 0.9f));
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsOptimizer: Reducing LOD due to high load (%.2f)"), CurrentPhysicsLoad);
    }
    else if (CurrentPhysicsLoad < MaxPhysicsLoad * 0.5f)
    {
        // Gradually increase LOD distances when performance is good
        LODSettings.NearDistance = FMath::Min(1500.0f, LODSettings.NearDistance * 1.05f);
        LODSettings.MidDistance = FMath::Min(7500.0f, LODSettings.MidDistance * 1.05f);
        LODSettings.FarDistance = FMath::Min(15000.0f, LODSettings.FarDistance * 1.05f);
        
        // Gradually increase max object counts
        LODSettings.MaxNearObjects = FMath::Min(150, int32(LODSettings.MaxNearObjects * 1.05f));
        LODSettings.MaxMidObjects = FMath::Min(75, int32(LODSettings.MaxMidObjects * 1.05f));
        LODSettings.MaxFarObjects = FMath::Min(30, int32(LODSettings.MaxFarObjects * 1.05f));
    }
}

void UPhysicsOptimizer::CullDistantObjects()
{
    FVector PlayerLocation = GetPlayerLocation();
    float CullDistance = LODSettings.FarDistance * 2.0f; // Cull beyond far distance
    
    for (auto& ObjectPair : TrackedObjects)
    {
        if (!ObjectPair.Key.IsValid())
        {
            continue;
        }
        
        FPhysicsObjectInfo& ObjectInfo = ObjectPair.Value;
        
        if (ObjectInfo.DistanceToPlayer > CullDistance)
        {
            SetPhysicsLODLevel(ObjectPair.Key.Get(), 3); // Disable physics
            ObjectInfo.LODLevel = 3;
            ObjectInfo.bIsActive = false;
        }
        else if (!ObjectInfo.bIsActive && ObjectInfo.DistanceToPlayer <= CullDistance * 0.8f)
        {
            // Re-enable objects that come back into range
            ObjectInfo.bIsActive = true;
        }
    }
}

FVector UPhysicsOptimizer::GetPlayerLocation() const
{
    if (!WorldContext.IsValid())
    {
        return FVector::ZeroVector;
    }
    
    APlayerController* PlayerController = WorldContext->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        return PlayerController->GetPawn()->GetActorLocation();
    }
    
    return FVector::ZeroVector;
}

void UPhysicsOptimizer::OptimizeCollisionSettings(UPrimitiveComponent* Component, int32 LODLevel)
{
    if (!Component)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case 0: // Near - Full detail
            // Keep default collision complexity
            break;
            
        case 1: // Mid - Reduced detail
            // Use simpler collision shapes
            Component->SetCollisionObjectType(ECC_WorldDynamic);
            break;
            
        case 2: // Far - Minimal detail
            // Use very simple collision
            Component->SetCollisionObjectType(ECC_WorldStatic);
            break;
    }
}

void UPhysicsOptimizer::UpdatePhysicsSubsteps()
{
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        return;
    }
    
    // Adjust physics substeps based on current load
    if (CurrentPhysicsLoad > MaxPhysicsLoad)
    {
        // Reduce substeps to improve performance
        PhysicsSettings->MaxSubsteps = FMath::Max(2, PhysicsSettings->MaxSubsteps - 1);
        PhysicsSettings->MaxSubstepDeltaTime = FMath::Min(0.033333f, PhysicsSettings->MaxSubstepDeltaTime * 1.1f);
    }
    else if (CurrentPhysicsLoad < MaxPhysicsLoad * 0.6f)
    {
        // Increase quality when performance allows
        PhysicsSettings->MaxSubsteps = FMath::Min(8, PhysicsSettings->MaxSubsteps + 1);
        PhysicsSettings->MaxSubstepDeltaTime = FMath::Max(0.008333f, PhysicsSettings->MaxSubstepDeltaTime * 0.95f);
    }
}