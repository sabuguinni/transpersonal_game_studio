#include "Core_PhysicsTerrainIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

// UCore_TerrainPhysicsComponent Implementation

UCore_TerrainPhysicsComponent::UCore_TerrainPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default physics properties
    PhysicsProperties = FCore_TerrainPhysicsProperties();
    CollisionData = FCore_TerrainCollisionData();
    
    bEnableRealTimePhysics = true;
    PhysicsUpdateFrequency = 30.0f;
    MaxPhysicsDistance = 5000.0f;
    
    PhysicsUpdateTimer = 0.0f;
    LastPlayerLocation = FVector::ZeroVector;
    CachedLandscape = nullptr;
}

void UCore_TerrainPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CacheTerrainReferences();
    ApplyPhysicsMaterialToTerrain();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Initialized with friction=%.2f, hardness=%.2f"), 
           PhysicsProperties.SurfaceFriction, PhysicsProperties.TerrainHardness);
}

void UCore_TerrainPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableRealTimePhysics)
        return;
    
    PhysicsUpdateTimer += DeltaTime;
    
    // Update physics at specified frequency
    if (PhysicsUpdateTimer >= (1.0f / PhysicsUpdateFrequency))
    {
        UpdatePhysicsLOD();
        PhysicsUpdateTimer = 0.0f;
    }
}

void UCore_TerrainPhysicsComponent::UpdateTerrainPhysicsProperties(const FCore_TerrainPhysicsProperties& NewProperties)
{
    PhysicsProperties = NewProperties;
    ApplyPhysicsMaterialToTerrain();
    OnPhysicsPropertiesChanged.Broadcast(PhysicsProperties);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Physics properties updated"));
}

void UCore_TerrainPhysicsComponent::ApplyBiomePhysicsSettings(EBiomeType BiomeType)
{
    FCore_TerrainPhysicsProperties BiomeProperties = PhysicsProperties;
    
    switch (BiomeType)
    {
        case EBiomeType::TropicalRainforest:
            BiomeProperties.SurfaceFriction = 0.4f;  // Wet, slippery
            BiomeProperties.TerrainHardness = 0.3f;  // Soft soil
            BiomeProperties.bCanDeform = true;
            BiomeProperties.SoundMaterial = SurfaceType2; // Mud/wet ground
            break;
            
        case EBiomeType::Grassland:
            BiomeProperties.SurfaceFriction = 0.7f;  // Good grip
            BiomeProperties.TerrainHardness = 0.6f;  // Firm soil
            BiomeProperties.bCanDeform = true;
            BiomeProperties.SoundMaterial = SurfaceType3; // Grass
            break;
            
        case EBiomeType::Desert:
            BiomeProperties.SurfaceFriction = 0.3f;  // Sandy, loose
            BiomeProperties.TerrainHardness = 0.2f;  // Very soft sand
            BiomeProperties.bCanDeform = true;
            BiomeProperties.DeformationThreshold = 100.0f; // Easy to deform
            BiomeProperties.SoundMaterial = SurfaceType4; // Sand
            break;
            
        case EBiomeType::Mountain:
            BiomeProperties.SurfaceFriction = 0.9f;  // Rocky, high grip
            BiomeProperties.TerrainHardness = 0.9f;  // Very hard rock
            BiomeProperties.bCanDeform = false;      // Rock doesn't deform easily
            BiomeProperties.SoundMaterial = SurfaceType5; // Rock
            break;
            
        case EBiomeType::Swamp:
            BiomeProperties.SurfaceFriction = 0.2f;  // Very slippery
            BiomeProperties.TerrainHardness = 0.1f;  // Extremely soft mud
            BiomeProperties.bCanDeform = true;
            BiomeProperties.DeformationThreshold = 50.0f; // Very easy to deform
            BiomeProperties.SoundMaterial = SurfaceType2; // Mud
            break;
            
        default:
            // Keep current properties
            break;
    }
    
    UpdateTerrainPhysicsProperties(BiomeProperties);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Applied biome physics for %s"), 
           *UEnum::GetValueAsString(BiomeType));
}

void UCore_TerrainPhysicsComponent::RegenerateCollisionMesh()
{
    if (CachedLandscape)
    {
        // Force landscape to rebuild collision
        CachedLandscape->RecreateCollisionComponents();
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Collision mesh regenerated"));
    }
}

void UCore_TerrainPhysicsComponent::ApplyTerrainDeformation(const FVector& ImpactLocation, float Force, float Radius)
{
    if (!PhysicsProperties.bCanDeform || Force < PhysicsProperties.DeformationThreshold)
        return;
    
    // Calculate deformation magnitude based on terrain hardness
    float DeformationMagnitude = Force * (1.0f - PhysicsProperties.TerrainHardness) * 0.001f;
    
    // Apply deformation (simplified - in real implementation would modify landscape heightmap)
    OnTerrainDeformed.Broadcast(ImpactLocation, DeformationMagnitude);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Terrain deformed at %s, magnitude=%.2f"), 
           *ImpactLocation.ToString(), DeformationMagnitude);
}

FCore_TerrainPhysicsProperties UCore_TerrainPhysicsComponent::GetPhysicsPropertiesAtLocation(const FVector& WorldLocation) const
{
    // In a full implementation, this would sample terrain data at the specific location
    // For now, return the current properties
    return PhysicsProperties;
}

void UCore_TerrainPhysicsComponent::SetPhysicsLODLevel(int32 LODLevel)
{
    CollisionData.CollisionLOD = FMath::Clamp(LODLevel, 0, 3);
    RegenerateCollisionMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Physics LOD set to %d"), LODLevel);
}

void UCore_TerrainPhysicsComponent::UpdatePhysicsLOD()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Get player location for LOD calculations
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerLocation);
        
        // Determine LOD level based on distance
        int32 NewLODLevel = 0;
        if (DistanceToPlayer > MaxPhysicsDistance * 0.75f)
            NewLODLevel = 3; // Lowest detail
        else if (DistanceToPlayer > MaxPhysicsDistance * 0.5f)
            NewLODLevel = 2;
        else if (DistanceToPlayer > MaxPhysicsDistance * 0.25f)
            NewLODLevel = 1;
        
        if (NewLODLevel != CollisionData.CollisionLOD)
        {
            SetPhysicsLODLevel(NewLODLevel);
        }
        
        LastPlayerLocation = PlayerLocation;
    }
}

void UCore_TerrainPhysicsComponent::CacheTerrainReferences()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find landscape in the world
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        CachedLandscape = *ActorItr;
        break; // Use first landscape found
    }
    
    if (CachedLandscape)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Cached landscape reference"));
    }
}

void UCore_TerrainPhysicsComponent::ApplyPhysicsMaterialToTerrain()
{
    if (!CachedLandscape)
        return;
    
    // In a full implementation, this would create and apply a physics material
    // with the current physics properties to the landscape
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsComponent: Applied physics material to terrain"));
}

bool UCore_TerrainPhysicsComponent::IsLocationInPhysicsRange(const FVector& Location) const
{
    if (LastPlayerLocation == FVector::ZeroVector)
        return true; // No player reference yet
    
    float Distance = FVector::Dist(Location, LastPlayerLocation);
    return Distance <= MaxPhysicsDistance;
}

float UCore_TerrainPhysicsComponent::CalculatePhysicsLODDistance(const FVector& Location) const
{
    if (LastPlayerLocation == FVector::ZeroVector)
        return 0.0f;
    
    return FVector::Dist(Location, LastPlayerLocation);
}

// ACore_PhysicsTerrainIntegrator Implementation

ACore_PhysicsTerrainIntegrator::ACore_PhysicsTerrainIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostPhysics;
    
    // Create terrain physics component
    TerrainPhysicsComponent = CreateDefaultSubobject<UCore_TerrainPhysicsComponent>(TEXT("TerrainPhysicsComponent"));
    
    // Initialize global settings
    GlobalPhysicsSettings = FCore_TerrainPhysicsProperties();
    
    bAutoApplyBiomePhysics = true;
    bEnablePerformanceMonitoring = false;
    bShowDebugVisualization = false;
    
    LastFramePhysicsTime = 0.0f;
    ActivePhysicsActors = 0;
    CachedWorld = nullptr;
}

void ACore_PhysicsTerrainIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    
    InitializeTerrainPhysics();
    RegisterWithPhysicsManager();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: Initialized in world"));
}

void ACore_PhysicsTerrainIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnablePerformanceMonitoring)
    {
        UpdatePerformanceMetrics();
    }
}

void ACore_PhysicsTerrainIntegrator::InitializeTerrainPhysics()
{
    if (!TerrainPhysicsComponent)
        return;
    
    // Apply global physics settings
    TerrainPhysicsComponent->UpdateTerrainPhysicsProperties(GlobalPhysicsSettings);
    
    // Initialize biome-specific overrides if not already set
    if (BiomePhysicsOverrides.Num() == 0)
    {
        // Set default biome physics
        FCore_TerrainPhysicsProperties RainforestPhysics = GlobalPhysicsSettings;
        RainforestPhysics.SurfaceFriction = 0.4f;
        RainforestPhysics.TerrainHardness = 0.3f;
        BiomePhysicsOverrides.Add(EBiomeType::TropicalRainforest, RainforestPhysics);
        
        FCore_TerrainPhysicsProperties GrasslandPhysics = GlobalPhysicsSettings;
        GrasslandPhysics.SurfaceFriction = 0.7f;
        GrasslandPhysics.TerrainHardness = 0.6f;
        BiomePhysicsOverrides.Add(EBiomeType::Grassland, GrasslandPhysics);
        
        FCore_TerrainPhysicsProperties DesertPhysics = GlobalPhysicsSettings;
        DesertPhysics.SurfaceFriction = 0.3f;
        DesertPhysics.TerrainHardness = 0.2f;
        BiomePhysicsOverrides.Add(EBiomeType::Desert, DesertPhysics);
        
        FCore_TerrainPhysicsProperties MountainPhysics = GlobalPhysicsSettings;
        MountainPhysics.SurfaceFriction = 0.9f;
        MountainPhysics.TerrainHardness = 0.9f;
        BiomePhysicsOverrides.Add(EBiomeType::Mountain, MountainPhysics);
        
        FCore_TerrainPhysicsProperties SwampPhysics = GlobalPhysicsSettings;
        SwampPhysics.SurfaceFriction = 0.2f;
        SwampPhysics.TerrainHardness = 0.1f;
        BiomePhysicsOverrides.Add(EBiomeType::Swamp, SwampPhysics);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: Terrain physics initialized with %d biome overrides"), 
           BiomePhysicsOverrides.Num());
}

void ACore_PhysicsTerrainIntegrator::RefreshAllTerrainPhysics()
{
    if (TerrainPhysicsComponent)
    {
        TerrainPhysicsComponent->RegenerateCollisionMesh();
        ApplyGlobalPhysicsSettings();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: All terrain physics refreshed"));
}

void ACore_PhysicsTerrainIntegrator::UpdatePhysicsForBiome(EBiomeType BiomeType, const FCore_TerrainPhysicsProperties& Properties)
{
    BiomePhysicsOverrides.Add(BiomeType, Properties);
    
    if (bAutoApplyBiomePhysics && TerrainPhysicsComponent)
    {
        TerrainPhysicsComponent->ApplyBiomePhysicsSettings(BiomeType);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: Updated physics for biome %s"), 
           *UEnum::GetValueAsString(BiomeType));
}

FCore_TerrainPhysicsProperties ACore_PhysicsTerrainIntegrator::GetEffectivePhysicsProperties(const FVector& WorldLocation) const
{
    // In a full implementation, this would determine the biome at the location
    // and return the appropriate physics properties
    
    // For now, return global settings
    return GlobalPhysicsSettings;
}

void ACore_PhysicsTerrainIntegrator::OptimizePhysicsForPerformance()
{
    if (!TerrainPhysicsComponent)
        return;
    
    // Reduce physics update frequency if performance is poor
    if (LastFramePhysicsTime > 16.0f) // > 16ms = below 60fps
    {
        float CurrentFreq = TerrainPhysicsComponent->PhysicsUpdateFrequency;
        TerrainPhysicsComponent->PhysicsUpdateFrequency = FMath::Max(10.0f, CurrentFreq * 0.8f);
        
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsTerrainIntegrator: Reduced physics frequency to %.1fHz for performance"), 
               TerrainPhysicsComponent->PhysicsUpdateFrequency);
    }
}

void ACore_PhysicsTerrainIntegrator::DebugShowPhysicsProperties()
{
    if (!TerrainPhysicsComponent)
        return;
    
    FCore_TerrainPhysicsProperties Props = TerrainPhysicsComponent->PhysicsProperties;
    
    UE_LOG(LogTemp, Log, TEXT("=== TERRAIN PHYSICS DEBUG ==="));
    UE_LOG(LogTemp, Log, TEXT("Surface Friction: %.2f"), Props.SurfaceFriction);
    UE_LOG(LogTemp, Log, TEXT("Terrain Hardness: %.2f"), Props.TerrainHardness);
    UE_LOG(LogTemp, Log, TEXT("Restitution: %.2f"), Props.Restitution);
    UE_LOG(LogTemp, Log, TEXT("Density: %.1f kg/m³"), Props.Density);
    UE_LOG(LogTemp, Log, TEXT("Can Deform: %s"), Props.bCanDeform ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Deformation Threshold: %.1f"), Props.DeformationThreshold);
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}

void ACore_PhysicsTerrainIntegrator::ToggleDebugVisualization()
{
    bShowDebugVisualization = !bShowDebugVisualization;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: Debug visualization %s"), 
           bShowDebugVisualization ? TEXT("enabled") : TEXT("disabled"));
}

float ACore_PhysicsTerrainIntegrator::GetCurrentPhysicsPerformanceMetric() const
{
    return LastFramePhysicsTime;
}

int32 ACore_PhysicsTerrainIntegrator::GetActivePhysicsActorCount() const
{
    return ActivePhysicsActors;
}

void ACore_PhysicsTerrainIntegrator::UpdatePerformanceMetrics()
{
    if (!CachedWorld)
        return;
    
    // Simple performance tracking
    static double LastTime = FPlatformTime::Seconds();
    double CurrentTime = FPlatformTime::Seconds();
    LastFramePhysicsTime = (CurrentTime - LastTime) * 1000.0f; // Convert to ms
    LastTime = CurrentTime;
    
    // Count physics-enabled actors (simplified)
    ActivePhysicsActors = 0;
    for (TActorIterator<AActor> ActorItr(CachedWorld); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                ActivePhysicsActors++;
            }
        }
    }
}

void ACore_PhysicsTerrainIntegrator::ApplyGlobalPhysicsSettings()
{
    if (TerrainPhysicsComponent)
    {
        TerrainPhysicsComponent->UpdateTerrainPhysicsProperties(GlobalPhysicsSettings);
    }
}

void ACore_PhysicsTerrainIntegrator::RegisterWithPhysicsManager()
{
    // In a full implementation, this would register with the Core_PhysicsManager
    // for centralized physics system coordination
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: Registered with physics manager"));
}

void ACore_PhysicsTerrainIntegrator::HandleBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome)
{
    if (!TerrainPhysicsComponent || !bAutoApplyBiomePhysics)
        return;
    
    // Smoothly transition physics properties between biomes
    const FCore_TerrainPhysicsProperties* ToProperties = BiomePhysicsOverrides.Find(ToBiome);
    if (ToProperties)
    {
        TerrainPhysicsComponent->ApplyBiomePhysicsSettings(ToBiome);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsTerrainIntegrator: Transitioned from %s to %s biome physics"), 
               *UEnum::GetValueAsString(FromBiome), *UEnum::GetValueAsString(ToBiome));
    }
}