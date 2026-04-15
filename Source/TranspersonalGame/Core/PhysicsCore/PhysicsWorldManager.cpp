#include "PhysicsWorldManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UPhysicsWorldManager::UPhysicsWorldManager()
{
    // Initialize default physics settings
    WorldGravity = FVector(0.0f, 0.0f, -980.0f); // Standard Earth gravity
    PhysicsTickRate = 60.0f;
    MaxPhysicsObjects = 1000;
    PhysicsLODDistance = 2000.0f;
    bPhysicsSimulationEnabled = true;
    bDebugVisualizationEnabled = false;
    
    LastPerformanceCheckTime = 0.0f;
    PhysicsObjectCount = 0;
    AveragePhysicsFrameTime = 0.0f;
}

void UPhysicsWorldManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Initializing physics world subsystem"));
    
    InitializePhysicsWorld();
}

void UPhysicsWorldManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Deinitializing physics world subsystem"));
    
    // Clear physics actors
    ActivePhysicsActors.Empty();
    PhysicsMaterials.Empty();
    
    Super::Deinitialize();
}

void UPhysicsWorldManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: World begin play - setting up physics"));
    
    // Apply physics settings to the world
    UpdatePhysicsSettings();
    
    // Start performance monitoring
    MonitorPhysicsPerformance();
}

void UPhysicsWorldManager::InitializePhysicsWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsWorldManager: No world available for physics initialization"));
        return;
    }
    
    // Set up default physics materials
    RegisterPhysicsMaterial(TEXT("Default"), nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics world initialized successfully"));
}

void UPhysicsWorldManager::SetWorldGravity(const FVector& NewGravity)
{
    WorldGravity = NewGravity;
    
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        World->GetPhysicsScene()->SetGravity(NewGravity);
        UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: World gravity set to %s"), *NewGravity.ToString());
        
        OnPhysicsWorldEvent.Broadcast(nullptr, TEXT("GravityChanged"));
    }
}

FVector UPhysicsWorldManager::GetWorldGravity() const
{
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        return World->GetPhysicsScene()->GetGravity();
    }
    return WorldGravity;
}

void UPhysicsWorldManager::SetPhysicsSimulationEnabled(bool bEnabled)
{
    bPhysicsSimulationEnabled = bEnabled;
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Enable/disable physics for all active actors
        for (AActor* Actor : ActivePhysicsActors)
        {
            if (Actor)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* Component : PrimitiveComponents)
                {
                    if (Component)
                    {
                        Component->SetSimulatePhysics(bEnabled);
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics simulation %s"), 
               bEnabled ? TEXT("enabled") : TEXT("disabled"));
        
        OnPhysicsWorldEvent.Broadcast(nullptr, bEnabled ? TEXT("SimulationEnabled") : TEXT("SimulationDisabled"));
    }
}

bool UPhysicsWorldManager::IsPhysicsSimulationEnabled() const
{
    return bPhysicsSimulationEnabled;
}

void UPhysicsWorldManager::SetPhysicsTickRate(float TickRate)
{
    PhysicsTickRate = FMath::Clamp(TickRate, 30.0f, 120.0f);
    
    UWorld* World = GetWorld();
    if (World)
    {
        // Apply tick rate to physics scene
        UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics tick rate set to %f"), PhysicsTickRate);
    }
}

float UPhysicsWorldManager::GetPhysicsTickRate() const
{
    return PhysicsTickRate;
}

void UPhysicsWorldManager::SetMaxPhysicsObjects(int32 MaxObjects)
{
    MaxPhysicsObjects = FMath::Max(MaxObjects, 100);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Max physics objects set to %d"), MaxPhysicsObjects);
}

int32 UPhysicsWorldManager::GetCurrentPhysicsObjectCount() const
{
    return PhysicsObjectCount;
}

void UPhysicsWorldManager::RegisterPhysicsMaterial(const FString& MaterialName, UPhysicalMaterial* Material)
{
    PhysicsMaterials.Add(MaterialName, Material);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Registered physics material '%s'"), *MaterialName);
}

UPhysicalMaterial* UPhysicsWorldManager::GetPhysicsMaterial(const FString& MaterialName) const
{
    if (UPhysicalMaterial* const* FoundMaterial = PhysicsMaterials.Find(MaterialName))
    {
        return *FoundMaterial;
    }
    return nullptr;
}

void UPhysicsWorldManager::SetCollisionProfile(AActor* Actor, const FString& ProfileName)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsWorldManager: Cannot set collision profile - Actor is null"));
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetCollisionProfileName(FName(*ProfileName));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Set collision profile '%s' for actor '%s'"), 
           *ProfileName, *Actor->GetName());
}

void UPhysicsWorldManager::EnableActorPhysics(AActor* Actor, bool bSimulatePhysics)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsWorldManager: Cannot enable physics - Actor is null"));
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetSimulatePhysics(bSimulatePhysics);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    RegisterPhysicsActor(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Enabled physics for actor '%s'"), *Actor->GetName());
    
    OnPhysicsWorldEvent.Broadcast(Actor, TEXT("PhysicsEnabled"));
}

void UPhysicsWorldManager::DisableActorPhysics(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsWorldManager: Cannot disable physics - Actor is null"));
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetSimulatePhysics(false);
        }
    }
    
    UnregisterPhysicsActor(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Disabled physics for actor '%s'"), *Actor->GetName());
    
    OnPhysicsWorldEvent.Broadcast(Actor, TEXT("PhysicsDisabled"));
}

void UPhysicsWorldManager::EnablePhysicsDebugVisualization(bool bEnabled)
{
    bDebugVisualizationEnabled = bEnabled;
    
    UWorld* World = GetWorld();
    if (World)
    {
        if (bEnabled)
        {
            // Enable debug drawing
            UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics debug visualization enabled"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics debug visualization disabled"));
        }
    }
}

void UPhysicsWorldManager::ShowPhysicsStats(bool bShow)
{
    UWorld* World = GetWorld();
    if (World)
    {
        if (bShow)
        {
            UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics stats display enabled"));
            UE_LOG(LogTemp, Log, TEXT("  Active Physics Objects: %d"), PhysicsObjectCount);
            UE_LOG(LogTemp, Log, TEXT("  Average Frame Time: %f ms"), AveragePhysicsFrameTime);
            UE_LOG(LogTemp, Log, TEXT("  Physics Tick Rate: %f"), PhysicsTickRate);
        }
    }
}

void UPhysicsWorldManager::DumpPhysicsWorldInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS WORLD INFO ==="));
    UE_LOG(LogTemp, Log, TEXT("World Gravity: %s"), *GetWorldGravity().ToString());
    UE_LOG(LogTemp, Log, TEXT("Physics Simulation Enabled: %s"), bPhysicsSimulationEnabled ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Physics Tick Rate: %f"), PhysicsTickRate);
    UE_LOG(LogTemp, Log, TEXT("Max Physics Objects: %d"), MaxPhysicsObjects);
    UE_LOG(LogTemp, Log, TEXT("Current Physics Objects: %d"), PhysicsObjectCount);
    UE_LOG(LogTemp, Log, TEXT("Physics LOD Distance: %f"), PhysicsLODDistance);
    UE_LOG(LogTemp, Log, TEXT("Debug Visualization: %s"), bDebugVisualizationEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Log, TEXT("Registered Materials: %d"), PhysicsMaterials.Num());
    UE_LOG(LogTemp, Log, TEXT("========================="));
}

void UPhysicsWorldManager::OptimizePhysicsForPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count and optimize physics objects
    int32 OptimizedCount = 0;
    
    for (AActor* Actor : ActivePhysicsActors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    // Apply performance optimizations
                    Component->SetGenerateOverlapEvents(false);
                    OptimizedCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Optimized %d physics objects for performance"), OptimizedCount);
    
    OnPhysicsWorldEvent.Broadcast(nullptr, TEXT("PerformanceOptimized"));
}

void UPhysicsWorldManager::SetPhysicsLODDistance(float Distance)
{
    PhysicsLODDistance = FMath::Max(Distance, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics LOD distance set to %f"), PhysicsLODDistance);
}

void UPhysicsWorldManager::CullDistantPhysicsObjects(const FVector& ViewLocation, float CullDistance)
{
    int32 CulledCount = 0;
    
    for (AActor* Actor : ActivePhysicsActors)
    {
        if (Actor)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), ViewLocation);
            
            if (Distance > CullDistance)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* Component : PrimitiveComponents)
                {
                    if (Component && Component->IsSimulatingPhysics())
                    {
                        Component->SetSimulatePhysics(false);
                        CulledCount++;
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Culled %d distant physics objects"), CulledCount);
}

void UPhysicsWorldManager::UpdatePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply world gravity
    if (World->GetPhysicsScene())
    {
        World->GetPhysicsScene()->SetGravity(WorldGravity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsWorldManager: Physics settings updated"));
}

void UPhysicsWorldManager::MonitorPhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update physics object count
    PhysicsObjectCount = ActivePhysicsActors.Num();
    
    // Calculate average frame time (simplified)
    float CurrentTime = World->GetTimeSeconds();
    if (LastPerformanceCheckTime > 0.0f)
    {
        float DeltaTime = CurrentTime - LastPerformanceCheckTime;
        AveragePhysicsFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }
    LastPerformanceCheckTime = CurrentTime;
}

void UPhysicsWorldManager::RegisterPhysicsActor(AActor* Actor)
{
    if (Actor && !ActivePhysicsActors.Contains(Actor))
    {
        ActivePhysicsActors.Add(Actor);
        PhysicsObjectCount = ActivePhysicsActors.Num();
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("PhysicsWorldManager: Registered physics actor '%s'"), *Actor->GetName());
    }
}

void UPhysicsWorldManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (Actor)
    {
        ActivePhysicsActors.Remove(Actor);
        PhysicsObjectCount = ActivePhysicsActors.Num();
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("PhysicsWorldManager: Unregistered physics actor '%s'"), *Actor->GetName());
    }
}