#include "TerrainDeformationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTerrainDeformation);

UTerrainDeformationSystem::UTerrainDeformationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Performance settings for large worlds
    MaxDeformationRadius = 500.0f;
    DeformationStrength = 1.0f;
    MaxActiveDeformations = 100;
    DeformationLODDistance = 2000.0f;
    
    // Mud and sand physics parameters
    MudViscosity = 0.8f;
    SandDensity = 1.6f;
    WaterAbsorption = 0.3f;
    
    bUseGPUDeformation = true;
    bEnableLOD = true;
}

void UTerrainDeformationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDeformationSystem();
    SetupMaterialParameters();
    CacheNearbyLandscapes();
    
    UE_LOG(LogTerrainDeformation, Log, TEXT("TerrainDeformationSystem initialized with %d cached landscapes"), 
           CachedLandscapes.Num());
}

void UTerrainDeformationSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bSystemEnabled)
        return;
    
    UpdateActiveDeformations(DeltaTime);
    ProcessPendingDeformations();
    UpdateMudPhysics(DeltaTime);
    
    if (bEnableLOD)
    {
        UpdateDeformationLOD();
    }
    
    // Performance monitoring
    LastUpdateTime = DeltaTime;
    FrameCounter++;
    
    if (FrameCounter % 60 == 0) // Log every second at 60fps
    {
        UE_LOG(LogTerrainDeformation, VeryVerbose, 
               TEXT("Terrain Deformation: %d active, %.2fms update time"), 
               ActiveDeformations.Num(), LastUpdateTime * 1000.0f);
    }
}

void UTerrainDeformationSystem::InitializeDeformationSystem()
{
    // Initialize heightfield data structures
    DeformationGrid.SetNum(GridResolution * GridResolution);
    VelocityField.SetNum(GridResolution * GridResolution);
    MudDepthField.SetNum(GridResolution * GridResolution);
    
    // Clear all grids
    for (int32 i = 0; i < DeformationGrid.Num(); i++)
    {
        DeformationGrid[i] = 0.0f;
        VelocityField[i] = FVector::ZeroVector;
        MudDepthField[i] = 0.0f;
    }
    
    // Setup GPU compute shader if enabled
    if (bUseGPUDeformation)
    {
        SetupGPUDeformation();
    }
    
    bSystemEnabled = true;
}

void UTerrainDeformationSystem::SetupMaterialParameters()
{
    if (DeformationMaterialCollection)
    {
        MaterialCollectionInstance = GetWorld()->GetParameterCollectionInstance(DeformationMaterialCollection);
        
        if (MaterialCollectionInstance)
        {
            MaterialCollectionInstance->SetScalarParameterValue(FName("MudViscosity"), MudViscosity);
            MaterialCollectionInstance->SetScalarParameterValue(FName("SandDensity"), SandDensity);
            MaterialCollectionInstance->SetScalarParameterValue(FName("WaterAbsorption"), WaterAbsorption);
            
            UE_LOG(LogTerrainDeformation, Log, TEXT("Material parameters initialized"));
        }
    }
}

void UTerrainDeformationSystem::CacheNearbyLandscapes()
{
    CachedLandscapes.Empty();
    
    for (TActorIterator<ALandscape> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        ALandscape* Landscape = *ActorIterator;
        if (Landscape && IsValid(Landscape))
        {
            CachedLandscapes.Add(Landscape);
        }
    }
}

void UTerrainDeformationSystem::CreateDeformation(const FVector& Location, float Radius, 
                                                float Strength, EDeformationType Type)
{
    if (!bSystemEnabled || ActiveDeformations.Num() >= MaxActiveDeformations)
        return;
    
    // Check if location is within deformation range
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(Location, OwnerLocation);
    
    if (Distance > MaxDeformationRadius)
        return;
    
    // Create new deformation
    FTerrainDeformation NewDeformation;
    NewDeformation.Location = Location;
    NewDeformation.Radius = FMath::Clamp(Radius, 10.0f, 200.0f);
    NewDeformation.Strength = FMath::Clamp(Strength, 0.1f, 5.0f);
    NewDeformation.Type = Type;
    NewDeformation.CreationTime = GetWorld()->GetTimeSeconds();
    NewDeformation.bIsActive = true;
    
    ActiveDeformations.Add(NewDeformation);
    
    // Apply immediate deformation
    ApplyDeformationToTerrain(NewDeformation);
    
    // Create mud physics if applicable
    if (Type == EDeformationType::Mud || Type == EDeformationType::Footprint)
    {
        CreateMudPhysics(Location, Radius, Strength);
    }
    
    UE_LOG(LogTerrainDeformation, Log, 
           TEXT("Created %s deformation at %s (R:%.1f, S:%.1f)"), 
           *UEnum::GetValueAsString(Type), *Location.ToString(), Radius, Strength);
}

void UTerrainDeformationSystem::ApplyDeformationToTerrain(const FTerrainDeformation& Deformation)
{
    for (ALandscape* Landscape : CachedLandscapes)
    {
        if (!Landscape)
            continue;
        
        // Check if deformation affects this landscape
        FBox LandscapeBounds = Landscape->GetComponentsBoundingBox();
        FVector DeformCenter = Deformation.Location;
        float DeformRadius = Deformation.Radius;
        
        if (LandscapeBounds.IsInsideOrOn(DeformCenter) || 
            LandscapeBounds.GetDistanceToPoint(DeformCenter) < DeformRadius)
        {
            ApplyDeformationToLandscape(Landscape, Deformation);
        }
    }
}

void UTerrainDeformationSystem::ApplyDeformationToLandscape(ALandscape* Landscape, 
                                                          const FTerrainDeformation& Deformation)
{
    if (!Landscape)
        return;
    
    // Get landscape transform
    FTransform LandscapeTransform = Landscape->GetActorTransform();
    FVector LocalDeformLocation = LandscapeTransform.InverseTransformPosition(Deformation.Location);
    
    // Apply deformation based on type
    switch (Deformation.Type)
    {
        case EDeformationType::Crater:
            ApplyCraterDeformation(Landscape, LocalDeformLocation, Deformation);
            break;
        case EDeformationType::Footprint:
            ApplyFootprintDeformation(Landscape, LocalDeformLocation, Deformation);
            break;
        case EDeformationType::Mud:
            ApplyMudDeformation(Landscape, LocalDeformLocation, Deformation);
            break;
        case EDeformationType::Erosion:
            ApplyErosionDeformation(Landscape, LocalDeformLocation, Deformation);
            break;
    }
}

void UTerrainDeformationSystem::ApplyCraterDeformation(ALandscape* Landscape, 
                                                     const FVector& LocalLocation, 
                                                     const FTerrainDeformation& Deformation)
{
    // Crater deformation creates a depression with raised edges
    float Radius = Deformation.Radius;
    float Strength = Deformation.Strength;
    
    // Apply heightfield modification
    ModifyLandscapeHeightfield(Landscape, LocalLocation, Radius, -Strength * 0.8f);
    
    // Create raised rim
    ModifyLandscapeHeightfield(Landscape, LocalLocation, Radius * 1.2f, Strength * 0.3f);
}

void UTerrainDeformationSystem::ApplyFootprintDeformation(ALandscape* Landscape, 
                                                        const FVector& LocalLocation, 
                                                        const FTerrainDeformation& Deformation)
{
    // Footprint creates a shallow depression
    float Radius = Deformation.Radius;
    float Strength = Deformation.Strength * 0.3f; // Shallow depression
    
    ModifyLandscapeHeightfield(Landscape, LocalLocation, Radius, -Strength);
}

void UTerrainDeformationSystem::ApplyMudDeformation(ALandscape* Landscape, 
                                                  const FVector& LocalLocation, 
                                                  const FTerrainDeformation& Deformation)
{
    // Mud deformation creates soft, viscous terrain
    float Radius = Deformation.Radius;
    float Strength = Deformation.Strength * 0.5f;
    
    ModifyLandscapeHeightfield(Landscape, LocalLocation, Radius, -Strength);
    
    // Update mud depth field
    UpdateMudDepthField(LocalLocation, Radius, Strength);
}

void UTerrainDeformationSystem::ApplyErosionDeformation(ALandscape* Landscape, 
                                                      const FVector& LocalLocation, 
                                                      const FTerrainDeformation& Deformation)
{
    // Erosion creates gradual wearing away of terrain
    float Radius = Deformation.Radius;
    float Strength = Deformation.Strength * 0.2f; // Gradual effect
    
    ModifyLandscapeHeightfield(Landscape, LocalLocation, Radius, -Strength);
}

void UTerrainDeformationSystem::ModifyLandscapeHeightfield(ALandscape* Landscape, 
                                                         const FVector& LocalLocation, 
                                                         float Radius, float Strength)
{
    // This is a simplified version - in production, you'd use Landscape editing API
    // For now, we'll update our internal heightfield and apply via material parameters
    
    FVector2D GridLocation = WorldToGridCoordinates(LocalLocation);
    int32 GridRadius = FMath::CeilToInt(Radius / GridCellSize);
    
    for (int32 x = -GridRadius; x <= GridRadius; x++)
    {
        for (int32 y = -GridRadius; y <= GridRadius; y++)
        {
            int32 GridX = FMath::Clamp(FMath::RoundToInt(GridLocation.X) + x, 0, GridResolution - 1);
            int32 GridY = FMath::Clamp(FMath::RoundToInt(GridLocation.Y) + y, 0, GridResolution - 1);
            int32 GridIndex = GridY * GridResolution + GridX;
            
            float Distance = FMath::Sqrt(x * x + y * y) * GridCellSize;
            if (Distance <= Radius)
            {
                float Falloff = 1.0f - (Distance / Radius);
                Falloff = FMath::Pow(Falloff, 2.0f); // Smooth falloff
                
                DeformationGrid[GridIndex] += Strength * Falloff;
            }
        }
    }
}

void UTerrainDeformationSystem::CreateMudPhysics(const FVector& Location, float Radius, float Strength)
{
    FMudPhysicsData MudData;
    MudData.Location = Location;
    MudData.Radius = Radius;
    MudData.Viscosity = MudViscosity;
    MudData.Depth = Strength * 50.0f; // Convert strength to depth in cm
    MudData.CreationTime = GetWorld()->GetTimeSeconds();
    
    ActiveMudPhysics.Add(MudData);
}

void UTerrainDeformationSystem::UpdateActiveDeformations(float DeltaTime)
{
    for (int32 i = ActiveDeformations.Num() - 1; i >= 0; i--)
    {
        FTerrainDeformation& Deformation = ActiveDeformations[i];
        
        if (!Deformation.bIsActive)
        {
            ActiveDeformations.RemoveAt(i);
            continue;
        }
        
        // Update deformation lifetime
        float Age = GetWorld()->GetTimeSeconds() - Deformation.CreationTime;
        
        // Remove old deformations
        if (Age > DeformationLifetime)
        {
            ActiveDeformations.RemoveAt(i);
            continue;
        }
        
        // Apply time-based effects (erosion, settling, etc.)
        if (Deformation.Type == EDeformationType::Mud)
        {
            // Mud gradually settles
            Deformation.Strength *= FMath::Max(0.1f, 1.0f - DeltaTime * 0.1f);
        }
    }
}

void UTerrainDeformationSystem::UpdateMudPhysics(float DeltaTime)
{
    for (int32 i = ActiveMudPhysics.Num() - 1; i >= 0; i--)
    {
        FMudPhysicsData& MudData = ActiveMudPhysics[i];
        
        // Update mud viscosity over time
        float Age = GetWorld()->GetTimeSeconds() - MudData.CreationTime;
        
        if (Age > MudLifetime)
        {
            ActiveMudPhysics.RemoveAt(i);
            continue;
        }
        
        // Mud dries out over time
        float DryingFactor = FMath::Max(0.1f, 1.0f - Age / MudLifetime);
        MudData.Viscosity = MudViscosity * DryingFactor;
        MudData.Depth *= FMath::Max(0.5f, DryingFactor);
    }
}

void UTerrainDeformationSystem::ProcessPendingDeformations()
{
    // Process any deformations that were queued for next frame
    if (PendingDeformations.Num() > 0)
    {
        for (const FTerrainDeformation& PendingDeformation : PendingDeformations)
        {
            ApplyDeformationToTerrain(PendingDeformation);
        }
        PendingDeformations.Empty();
    }
}

void UTerrainDeformationSystem::UpdateDeformationLOD()
{
    if (!GetOwner())
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (FTerrainDeformation& Deformation : ActiveDeformations)
    {
        float Distance = FVector::Dist(Deformation.Location, OwnerLocation);
        
        if (Distance > DeformationLODDistance)
        {
            Deformation.bIsActive = false;
        }
        else
        {
            Deformation.bIsActive = true;
        }
    }
}

void UTerrainDeformationSystem::SetupGPUDeformation()
{
    // Setup compute shader for GPU-based deformation
    // This would involve creating compute shader resources
    UE_LOG(LogTerrainDeformation, Log, TEXT("GPU deformation setup - placeholder implementation"));
}

FVector2D UTerrainDeformationSystem::WorldToGridCoordinates(const FVector& WorldLocation) const
{
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector RelativeLocation = WorldLocation - OwnerLocation;
    
    float GridX = (RelativeLocation.X / GridCellSize) + (GridResolution * 0.5f);
    float GridY = (RelativeLocation.Y / GridCellSize) + (GridResolution * 0.5f);
    
    return FVector2D(GridX, GridY);
}

void UTerrainDeformationSystem::UpdateMudDepthField(const FVector& Location, float Radius, float Depth)
{
    FVector2D GridLocation = WorldToGridCoordinates(Location);
    int32 GridRadius = FMath::CeilToInt(Radius / GridCellSize);
    
    for (int32 x = -GridRadius; x <= GridRadius; x++)
    {
        for (int32 y = -GridRadius; y <= GridRadius; y++)
        {
            int32 GridX = FMath::Clamp(FMath::RoundToInt(GridLocation.X) + x, 0, GridResolution - 1);
            int32 GridY = FMath::Clamp(FMath::RoundToInt(GridLocation.Y) + y, 0, GridResolution - 1);
            int32 GridIndex = GridY * GridResolution + GridX;
            
            float Distance = FMath::Sqrt(x * x + y * y) * GridCellSize;
            if (Distance <= Radius)
            {
                float Falloff = 1.0f - (Distance / Radius);
                MudDepthField[GridIndex] = FMath::Max(MudDepthField[GridIndex], Depth * Falloff);
            }
        }
    }
}

bool UTerrainDeformationSystem::IsLocationInMud(const FVector& Location, float& OutMudDepth, float& OutViscosity) const
{
    for (const FMudPhysicsData& MudData : ActiveMudPhysics)
    {
        float Distance = FVector::Dist(Location, MudData.Location);
        if (Distance <= MudData.Radius)
        {
            float Falloff = 1.0f - (Distance / MudData.Radius);
            OutMudDepth = MudData.Depth * Falloff;
            OutViscosity = MudData.Viscosity * Falloff;
            return true;
        }
    }
    
    OutMudDepth = 0.0f;
    OutViscosity = 0.0f;
    return false;
}

void UTerrainDeformationSystem::GetPerformanceMetrics(int32& OutActiveDeformations, 
                                                    int32& OutActiveMudPhysics, 
                                                    float& OutLastUpdateTime) const
{
    OutActiveDeformations = ActiveDeformations.Num();
    OutActiveMudPhysics = ActiveMudPhysics.Num();
    OutLastUpdateTime = LastUpdateTime;
}

void UTerrainDeformationSystem::SetDeformationEnabled(bool bEnabled)
{
    bSystemEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Clear all active deformations when disabled
        ActiveDeformations.Empty();
        ActiveMudPhysics.Empty();
    }
    
    UE_LOG(LogTerrainDeformation, Log, TEXT("Terrain deformation system %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}