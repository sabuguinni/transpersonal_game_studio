#include "Core_PhysicsWorldIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Physics/PhysicsFiltering.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/CollisionProfile.h"

UCore_PhysicsWorldIntegrator::UCore_PhysicsWorldIntegrator()
{
    // Initialize default settings
    WorldSettings = FCore_PhysicsWorldSettings();
    PerformanceMetrics = FCore_PhysicsPerformanceMetrics();
    
    bIsInitialized = false;
    bIsPhysicsEnabled = true;
    bDebugDrawEnabled = false;
    LastUpdateTime = 0.0f;
    FrameCounter = 0;
    LastFrameTime = 0.0;
    PhysicsUpdateCounter = 0;
    AccumulatedFrameTime = 0.0f;
}

void UCore_PhysicsWorldIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Initializing physics world integration"));
    
    InitializePhysicsWorld();
}

void UCore_PhysicsWorldIntegrator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Shutting down physics world integration"));
    
    ShutdownPhysicsWorld();
    
    Super::Deinitialize();
}

bool UCore_PhysicsWorldIntegrator::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCore_PhysicsWorldIntegrator::InitializePhysicsWorld()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldIntegrator: Already initialized"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldIntegrator: No valid world found"));
        return;
    }

    // Initialize physics settings
    InitializePhysicsSettings();

    // Clear existing registrations
    RegisteredTerrain.Empty();
    RegisteredPhysicsActors.Empty();

    // Find and register existing terrain
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ALandscape* Landscape = Cast<ALandscape>(Actor))
        {
            RegisterTerrainActor(Landscape);
        }
    }

    // Initialize performance metrics
    PerformanceMetrics = FCore_PhysicsPerformanceMetrics();
    LastFrameTime = FPlatformTime::Seconds();

    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Physics world initialized with %d terrain actors"), RegisteredTerrain.Num());
}

void UCore_PhysicsWorldIntegrator::ShutdownPhysicsWorld()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Clear all registrations
    RegisteredTerrain.Empty();
    RegisteredPhysicsActors.Empty();

    // Reset state
    bIsInitialized = false;
    bIsPhysicsEnabled = false;
    bDebugDrawEnabled = false;

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Physics world shutdown complete"));
}

void UCore_PhysicsWorldIntegrator::UpdatePhysicsWorld(float DeltaTime)
{
    if (!bIsInitialized || !bIsPhysicsEnabled)
    {
        return;
    }

    // Update frame timing
    LastUpdateTime = DeltaTime;
    FrameCounter++;
    AccumulatedFrameTime += DeltaTime;

    // Process physics updates
    ProcessPhysicsUpdates(DeltaTime);

    // Update performance metrics every 60 frames
    if (FrameCounter % 60 == 0)
    {
        UpdatePerformanceMetrics();
    }

    // Clean up invalid actors every 300 frames
    if (FrameCounter % 300 == 0)
    {
        CleanupInvalidActors();
    }

    // Debug drawing
    if (bDebugDrawEnabled)
    {
        DrawPhysicsDebugInfo();
    }
}

void UCore_PhysicsWorldIntegrator::SetPhysicsWorldSettings(const FCore_PhysicsWorldSettings& NewSettings)
{
    WorldSettings = NewSettings;
    
    if (bIsInitialized)
    {
        InitializePhysicsSettings();
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Physics settings updated"));
}

FCore_PhysicsWorldSettings UCore_PhysicsWorldIntegrator::GetPhysicsWorldSettings() const
{
    return WorldSettings;
}

void UCore_PhysicsWorldIntegrator::RegisterTerrainActor(ALandscape* TerrainActor)
{
    if (!TerrainActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldIntegrator: Attempted to register null terrain actor"));
        return;
    }

    TWeakObjectPtr<ALandscape> WeakTerrain(TerrainActor);
    if (!RegisteredTerrain.Contains(WeakTerrain))
    {
        RegisteredTerrain.Add(WeakTerrain);
        EnableTerrainPhysics(TerrainActor, true);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Registered terrain actor: %s"), *TerrainActor->GetName());
    }
}

void UCore_PhysicsWorldIntegrator::UnregisterTerrainActor(ALandscape* TerrainActor)
{
    if (!TerrainActor)
    {
        return;
    }

    TWeakObjectPtr<ALandscape> WeakTerrain(TerrainActor);
    RegisteredTerrain.Remove(WeakTerrain);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Unregistered terrain actor: %s"), *TerrainActor->GetName());
}

void UCore_PhysicsWorldIntegrator::EnableTerrainPhysics(ALandscape* TerrainActor, bool bEnable)
{
    if (!TerrainActor)
    {
        return;
    }

    // Configure terrain collision based on physics mode
    ULandscapeHeightfieldCollisionComponent* CollisionComp = TerrainActor->GetLandscapeInfo()->GetLandscapeProxy()->CollisionComponent;
    if (CollisionComp)
    {
        CollisionComp->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        
        // Set collision profile based on terrain mode
        switch (WorldSettings.TerrainMode)
        {
        case ECore_TerrainPhysicsMode::Static:
            CollisionComp->SetCollisionProfileName(TEXT("BlockAll"));
            break;
        case ECore_TerrainPhysicsMode::Dynamic:
            CollisionComp->SetCollisionProfileName(TEXT("WorldDynamic"));
            break;
        case ECore_TerrainPhysicsMode::Deformable:
            CollisionComp->SetCollisionProfileName(TEXT("WorldDynamic"));
            break;
        case ECore_TerrainPhysicsMode::Destructible:
            CollisionComp->SetCollisionProfileName(TEXT("Destructible"));
            break;
        }
    }
}

void UCore_PhysicsWorldIntegrator::RegisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldIntegrator: Attempted to register null physics actor"));
        return;
    }

    TWeakObjectPtr<AActor> WeakActor(Actor);
    if (!RegisteredPhysicsActors.Contains(WeakActor))
    {
        RegisteredPhysicsActors.Add(WeakActor);
        SetActorPhysicsEnabled(Actor, true);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Registered physics actor: %s"), *Actor->GetName());
    }
}

void UCore_PhysicsWorldIntegrator::UnregisterPhysicsActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    TWeakObjectPtr<AActor> WeakActor(Actor);
    RegisteredPhysicsActors.Remove(WeakActor);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Unregistered physics actor: %s"), *Actor->GetName());
}

void UCore_PhysicsWorldIntegrator::SetActorPhysicsEnabled(AActor* Actor, bool bEnabled)
{
    if (!Actor)
    {
        return;
    }

    // Enable physics on all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
            
            if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Component))
            {
                StaticMeshComp->SetSimulatePhysics(bEnabled && WorldSettings.bEnableRealTimePhysics);
            }
        }
    }
}

bool UCore_PhysicsWorldIntegrator::PerformPhysicsTrace(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, ECC_WorldStatic, QueryParams);
    
    if (bHit)
    {
        PerformanceMetrics.CollisionQueries++;
    }

    return bHit;
}

TArray<FHitResult> UCore_PhysicsWorldIntegrator::PerformPhysicsSweep(const FVector& Start, const FVector& End, float Radius)
{
    TArray<FHitResult> HitResults;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return HitResults;
    }

    FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    World->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_WorldStatic, SphereShape, QueryParams);
    
    PerformanceMetrics.CollisionQueries += HitResults.Num();

    return HitResults;
}

void UCore_PhysicsWorldIntegrator::SetCollisionProfile(UPrimitiveComponent* Component, const FName& ProfileName)
{
    if (Component)
    {
        Component->SetCollisionProfileName(ProfileName);
    }
}

void UCore_PhysicsWorldIntegrator::ApplyPhysicalMaterial(UPrimitiveComponent* Component, UPhysicalMaterial* Material)
{
    if (Component && Material)
    {
        Component->SetPhysMaterialOverride(Material);
    }
}

UPhysicalMaterial* UCore_PhysicsWorldIntegrator::GetSurfacePhysicalMaterial(const FVector& Location)
{
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 100);
    FVector TraceEnd = Location - FVector(0, 0, 100);

    if (PerformPhysicsTrace(TraceStart, TraceEnd, HitResult))
    {
        return HitResult.PhysMaterial.Get();
    }

    return nullptr;
}

FCore_PhysicsPerformanceMetrics UCore_PhysicsWorldIntegrator::GetPerformanceMetrics() const
{
    return PerformanceMetrics;
}

void UCore_PhysicsWorldIntegrator::OptimizePhysicsPerformance()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Optimize physics LOD
    OptimizePhysicsLOD();

    // Clean up invalid actors
    CleanupInvalidActors();

    // Adjust physics settings based on performance
    if (PerformanceMetrics.PhysicsFrameTime > 16.67f) // Over 60 FPS threshold
    {
        SetPerformanceMode(true);
    }

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Performance optimization complete"));
}

void UCore_PhysicsWorldIntegrator::SetPerformanceMode(bool bHighPerformance)
{
    if (bHighPerformance)
    {
        // Reduce physics quality for better performance
        WorldSettings.PhysicsTickRate = 30.0f;
        WorldSettings.MaxPhysicsActors = 2500;
        WorldSettings.bEnableFluidDynamics = false;
    }
    else
    {
        // Restore full quality
        WorldSettings.PhysicsTickRate = 60.0f;
        WorldSettings.MaxPhysicsActors = 5000;
        WorldSettings.bEnableFluidDynamics = true;
    }

    InitializePhysicsSettings();
}

void UCore_PhysicsWorldIntegrator::EnablePhysicsDebugDraw(bool bEnable)
{
    bDebugDrawEnabled = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Debug draw %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsWorldIntegrator::DrawPhysicsDebugInfo()
{
    UWorld* World = GetWorld();
    if (!World || !bDebugDrawEnabled)
    {
        return;
    }

    // Draw physics actor bounds
    for (const TWeakObjectPtr<AActor>& WeakActor : RegisteredPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            FVector ActorLocation = Actor->GetActorLocation();
            DrawDebugSphere(World, ActorLocation, 50.0f, 12, FColor::Green, false, 0.1f);
        }
    }

    // Draw terrain bounds
    for (const TWeakObjectPtr<ALandscape>& WeakTerrain : RegisteredTerrain)
    {
        if (ALandscape* Terrain = WeakTerrain.Get())
        {
            FVector TerrainLocation = Terrain->GetActorLocation();
            DrawDebugBox(World, TerrainLocation, FVector(1000, 1000, 100), FColor::Blue, false, 0.1f);
        }
    }
}

void UCore_PhysicsWorldIntegrator::ValidatePhysicsIntegration()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldIntegrator: System not initialized"));
        return;
    }

    ValidateSystemIntegrity();

    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldIntegrator: Validation complete"));
    UE_LOG(LogTemp, Log, TEXT("- Registered terrain actors: %d"), RegisteredTerrain.Num());
    UE_LOG(LogTemp, Log, TEXT("- Registered physics actors: %d"), RegisteredPhysicsActors.Num());
    UE_LOG(LogTemp, Log, TEXT("- Physics enabled: %s"), bIsPhysicsEnabled ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("- Performance optimal: %s"), PerformanceMetrics.bIsPerformanceOptimal ? TEXT("Yes") : TEXT("No"));
}

void UCore_PhysicsWorldIntegrator::InitializePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Apply gravity settings
    if (AWorldSettings* WorldSettings_UE = World->GetWorldSettings())
    {
        WorldSettings_UE->GlobalGravityZ = -980.0f * WorldSettings.GravityScale;
    }

    // Configure physics scene
    if (FPhysScene* PhysScene = World->GetPhysicsScene())
    {
        // Configure physics simulation parameters
        // Note: Direct physics scene configuration may require engine modifications
        // This is a placeholder for physics configuration
    }
}

void UCore_PhysicsWorldIntegrator::UpdatePerformanceMetrics()
{
    double CurrentTime = FPlatformTime::Seconds();
    float FrameTime = (float)(CurrentTime - LastFrameTime) * 1000.0f; // Convert to milliseconds
    LastFrameTime = CurrentTime;

    PerformanceMetrics.PhysicsFrameTime = FrameTime;
    PerformanceMetrics.ActivePhysicsActors = RegisteredPhysicsActors.Num();
    PerformanceMetrics.CollisionQueryTime = FrameTime * 0.3f; // Estimate collision time as 30% of frame time
    
    // Estimate memory usage (simplified)
    PerformanceMetrics.MemoryUsageMB = (RegisteredPhysicsActors.Num() * 0.5f) + (RegisteredTerrain.Num() * 10.0f);
    
    // Determine if performance is optimal
    PerformanceMetrics.bIsPerformanceOptimal = (FrameTime < 16.67f) && (PerformanceMetrics.ActivePhysicsActors < WorldSettings.MaxPhysicsActors);

    PhysicsUpdateCounter++;
}

void UCore_PhysicsWorldIntegrator::CleanupInvalidActors()
{
    // Remove invalid terrain references
    RegisteredTerrain.RemoveAll([](const TWeakObjectPtr<ALandscape>& WeakTerrain)
    {
        return !WeakTerrain.IsValid();
    });

    // Remove invalid physics actor references
    RegisteredPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
}

void UCore_PhysicsWorldIntegrator::ProcessPhysicsUpdates(float DeltaTime)
{
    if (!bIsPhysicsEnabled)
    {
        return;
    }

    // Handle terrain physics
    HandleTerrainPhysics(DeltaTime);

    // Handle actor physics
    HandleActorPhysics(DeltaTime);

    // Optimize physics LOD if needed
    if (FrameCounter % 120 == 0) // Every 2 seconds at 60 FPS
    {
        OptimizePhysicsLOD();
    }
}

void UCore_PhysicsWorldIntegrator::HandleTerrainPhysics(float DeltaTime)
{
    // Process terrain-specific physics updates
    for (const TWeakObjectPtr<ALandscape>& WeakTerrain : RegisteredTerrain)
    {
        if (ALandscape* Terrain = WeakTerrain.Get())
        {
            // Handle terrain deformation if enabled
            if (WorldSettings.bEnableTerrainDeformation && WorldSettings.TerrainMode == ECore_TerrainPhysicsMode::Deformable)
            {
                // Terrain deformation logic would go here
                // This is a placeholder for future terrain deformation implementation
            }
        }
    }
}

void UCore_PhysicsWorldIntegrator::HandleActorPhysics(float DeltaTime)
{
    // Process physics actor updates
    for (const TWeakObjectPtr<AActor>& WeakActor : RegisteredPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            // Update actor physics properties if needed
            // This could include dynamic physics material changes, force applications, etc.
        }
    }
}

void UCore_PhysicsWorldIntegrator::OptimizePhysicsLOD()
{
    // Implement physics LOD optimization
    // Reduce physics complexity for distant objects
    // This is a placeholder for future LOD optimization implementation
}

void UCore_PhysicsWorldIntegrator::ValidateSystemIntegrity()
{
    // Validate that all registered actors are still valid
    CleanupInvalidActors();

    // Check for performance issues
    if (!PerformanceMetrics.bIsPerformanceOptimal)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldIntegrator: Performance not optimal - Frame time: %.2fms"), PerformanceMetrics.PhysicsFrameTime);
    }

    // Validate physics settings
    if (WorldSettings.MaxPhysicsActors < RegisteredPhysicsActors.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldIntegrator: Physics actor count exceeds maximum limit"));
    }
}