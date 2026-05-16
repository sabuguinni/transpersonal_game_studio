#include "Core_PhysicsWorldManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACore_PhysicsWorldManager::ACore_PhysicsWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update physics management 10 times per second
    
    // Set default physics LOD distances (in cm)
    PhysicsLODDistances = {5000.0f, 15000.0f, 30000.0f}; // 50m, 150m, 300m
    PhysicsUpdateRates = {60.0f, 30.0f, 10.0f}; // Full rate, half rate, low rate
    
    // Initialize destruction zone tracking
    ActiveDestructionZones.Reserve(MaxDestructionZones);
}

void ACore_PhysicsWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWorldPhysics();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Initialized prehistoric world physics"));
}

void ACore_PhysicsWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdatePhysicsMetrics();
    CleanupDestructionZones();
    
    // Update physics LOD for all tracked actors
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        
        for (auto& ActorLODPair : ActorPhysicsLODCache)
        {
            if (IsValid(ActorLODPair.Key))
            {
                float Distance = FVector::Dist(PlayerLocation, ActorLODPair.Key->GetActorLocation());
                UpdateActorPhysicsLOD(ActorLODPair.Key, Distance);
            }
        }
    }
}

void ACore_PhysicsWorldManager::InitializeWorldPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldManager: No world context for physics initialization"));
        return;
    }
    
    // Set prehistoric world gravity
    World->GetWorldSettings()->GlobalGravityZ = BaseGravityZ * GravityMultiplier;
    
    // Configure physics settings for large-scale simulation
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Optimize for large dinosaur interactions
        PhysicsSettings->MaxSubstepDeltaTime = 0.0166f; // 60fps max
        PhysicsSettings->MaxSubsteps = 6;
        PhysicsSettings->SyncSceneSmoothingFactor = 0.0f; // Disable smoothing for accuracy
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Physics settings optimized for prehistoric simulation"));
    }
    
    // Initialize physics material defaults if not set
    if (!TerrainPhysicsMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: No terrain physics material assigned"));
    }
    
    ActivePhysicsBodies = 0;
}

void ACore_PhysicsWorldManager::ApplyPhysicsMaterialToActor(AActor* Actor, ECore_SurfaceType SurfaceType)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    UPhysicalMaterial* MaterialToApply = nullptr;
    
    switch (SurfaceType)
    {
        case ECore_SurfaceType::Terrain:
            MaterialToApply = TerrainPhysicsMaterial;
            break;
        case ECore_SurfaceType::DinosaurSkin:
        case ECore_SurfaceType::DinosaurBone:
            MaterialToApply = DinosaurPhysicsMaterial;
            break;
        case ECore_SurfaceType::Vegetation:
        case ECore_SurfaceType::Wood:
            MaterialToApply = VegetationPhysicsMaterial;
            break;
        case ECore_SurfaceType::Rock:
        case ECore_SurfaceType::Stone:
            MaterialToApply = RockPhysicsMaterial;
            break;
        default:
            MaterialToApply = TerrainPhysicsMaterial; // Default fallback
            break;
    }
    
    if (MaterialToApply)
    {
        // Apply to all primitive components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            AssignPhysicsMaterialToComponent(Component, MaterialToApply);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Applied physics material to %s"), *Actor->GetName());
    }
}

void ACore_PhysicsWorldManager::RegisterDestructionZone(FVector Location, float Radius)
{
    if (ActiveDestructionZones.Num() >= MaxDestructionZones)
    {
        // Remove oldest destruction zone
        ActiveDestructionZones.RemoveAt(0);
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Max destruction zones reached, removing oldest"));
    }
    
    ActiveDestructionZones.Add(Location);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Registered destruction zone at %s"), *Location.ToString());
}

void ACore_PhysicsWorldManager::CleanupDestructionZones()
{
    // For now, just limit the number of zones
    // In a full implementation, we'd track zone lifetimes and clean up expired ones
    while (ActiveDestructionZones.Num() > MaxDestructionZones)
    {
        ActiveDestructionZones.RemoveAt(0);
    }
}

void ACore_PhysicsWorldManager::SetGravityMultiplier(float NewMultiplier, float Duration)
{
    GravityMultiplier = NewMultiplier;
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetWorldSettings()->GlobalGravityZ = BaseGravityZ * GravityMultiplier;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Gravity multiplier set to %f"), NewMultiplier);
    }
    
    if (Duration > 0.0f)
    {
        // Reset gravity after duration
        GetWorldTimerManager().SetTimer(GravityResetTimer, this, &ACore_PhysicsWorldManager::ResetGravityMultiplier, Duration, false);
    }
}

int32 ACore_PhysicsWorldManager::GetPhysicsLODLevel(float DistanceFromPlayer) const
{
    for (int32 i = 0; i < PhysicsLODDistances.Num(); ++i)
    {
        if (DistanceFromPlayer <= PhysicsLODDistances[i])
        {
            return i;
        }
    }
    
    return PhysicsLODDistances.Num(); // Maximum LOD level (disabled)
}

void ACore_PhysicsWorldManager::UpdateActorPhysicsLOD(AActor* Actor, float DistanceFromPlayer)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    int32 NewLODLevel = GetPhysicsLODLevel(DistanceFromPlayer);
    int32* CurrentLODLevel = ActorPhysicsLODCache.Find(Actor);
    
    if (!CurrentLODLevel || *CurrentLODLevel != NewLODLevel)
    {
        // Update LOD level
        ActorPhysicsLODCache.Add(Actor, NewLODLevel);
        
        // Apply LOD settings to actor
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (NewLODLevel >= PhysicsLODDistances.Num())
            {
                // Disable physics simulation at maximum distance
                Component->SetSimulatePhysics(false);
            }
            else
            {
                // Enable physics with appropriate update rate
                Component->SetSimulatePhysics(true);
                // Note: Update rate control would require custom physics stepping
            }
        }
    }
}

TArray<UPrimitiveComponent*> ACore_PhysicsWorldManager::GetPhysicsBodiesInRadius(FVector Center, float Radius)
{
    TArray<UPrimitiveComponent*> PhysicsBodies;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return PhysicsBodies;
    }
    
    // Use overlap sphere to find physics bodies
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    bool bHit = World->OverlapMultiByChannel(
        OverlapResults,
        Center,
        FQuat::Identity,
        ECollisionChannel::ECC_WorldDynamic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (Result.Component.IsValid() && Result.Component->IsSimulatingPhysics())
            {
                PhysicsBodies.Add(Result.Component.Get());
            }
        }
    }
    
    return PhysicsBodies;
}

bool ACore_PhysicsWorldManager::IsLocationInDestructionZone(FVector Location) const
{
    const float DestructionRadius = 2000.0f; // 20 meter radius per zone
    
    for (const FVector& ZoneCenter : ActiveDestructionZones)
    {
        if (FVector::Dist(Location, ZoneCenter) <= DestructionRadius)
        {
            return true;
        }
    }
    
    return false;
}

float ACore_PhysicsWorldManager::GetPhysicsSimulationLoad() const
{
    // Simple load calculation based on active physics bodies
    const int32 MaxBodies = 1000; // Arbitrary maximum for prehistoric world
    return FMath::Clamp(static_cast<float>(ActivePhysicsBodies) / MaxBodies, 0.0f, 1.0f);
}

void ACore_PhysicsWorldManager::AssignPhysicsMaterialToComponent(UPrimitiveComponent* Component, UPhysicalMaterial* Material)
{
    if (!IsValid(Component) || !IsValid(Material))
    {
        return;
    }
    
    Component->SetPhysMaterialOverride(Material);
    
    // Update physics body if simulating
    if (Component->IsSimulatingPhysics())
    {
        Component->RecreatePhysicsState();
    }
}

void ACore_PhysicsWorldManager::ResetGravityMultiplier()
{
    SetGravityMultiplier(1.0f, 0.0f);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Gravity multiplier reset to normal"));
}

void ACore_PhysicsWorldManager::UpdatePhysicsMetrics()
{
    // Count active physics bodies in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 NewActiveBodyCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (IsValid(Actor))
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Component : PrimitiveComponents)
            {
                if (Component && Component->IsSimulatingPhysics())
                {
                    NewActiveBodyCount++;
                }
            }
        }
    }
    
    ActivePhysicsBodies = NewActiveBodyCount;
    
    // Track physics update frequency
    PhysicsUpdatesThisSecond++;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastPhysicsUpdateTime >= 1.0f)
    {
        // Reset counter every second
        PhysicsUpdatesThisSecond = 0;
        LastPhysicsUpdateTime = CurrentTime;
    }
}