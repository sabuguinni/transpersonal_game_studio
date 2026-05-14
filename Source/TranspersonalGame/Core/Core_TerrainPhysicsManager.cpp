#include "Core_TerrainPhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"

ACore_TerrainPhysicsManager::ACore_TerrainPhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for physics updates
    
    // Initialize default values
    MaxFootprints = 1000.0f;
    FootprintLifetime = 300.0f; // 5 minutes
    bEnableTerrainDeformation = true;
    bEnableFootprints = true;
    PhysicsUpdateInterval = 0.1f;
    MaxPhysicsDistance = 5000.0f;
    LastPhysicsUpdate = 0.0f;
    bIsInitialized = false;
    LandscapeRef = nullptr;
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TerrainPhysicsRoot"));
}

void ACore_TerrainPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainPhysics();
}

void ACore_TerrainPhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsInitialized)
    {
        UpdateTerrainPhysics(DeltaTime);
    }
}

void ACore_TerrainPhysicsManager::InitializeTerrainPhysics()
{
    if (bIsInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Initializing terrain physics system"));
    
    // Initialize default terrain physics data
    InitializeDefaultTerrainPhysics();
    
    // Cache terrain references
    CacheTerrainReferences();
    
    // Apply physics materials to existing terrain
    RefreshTerrainPhysicsMaterials();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Initialization complete"));
}

void ACore_TerrainPhysicsManager::InitializeDefaultTerrainPhysics()
{
    // Rock terrain
    FCore_TerrainPhysicsData RockData;
    RockData.Friction = 0.8f;
    RockData.Restitution = 0.1f;
    RockData.Density = 2.5f;
    RockData.bSupportsFootprints = false;
    RockData.DeformationResistance = 10.0f;
    RockData.TerrainType = ECore_TerrainType::Rock;
    TerrainPhysicsMap.Add(ECore_TerrainType::Rock, RockData);
    
    // Dirt terrain
    FCore_TerrainPhysicsData DirtData;
    DirtData.Friction = 0.6f;
    DirtData.Restitution = 0.05f;
    DirtData.Density = 1.3f;
    DirtData.bSupportsFootprints = true;
    DirtData.DeformationResistance = 2.0f;
    DirtData.TerrainType = ECore_TerrainType::Dirt;
    TerrainPhysicsMap.Add(ECore_TerrainType::Dirt, DirtData);
    
    // Grass terrain
    FCore_TerrainPhysicsData GrassData;
    GrassData.Friction = 0.7f;
    GrassData.Restitution = 0.02f;
    GrassData.Density = 1.1f;
    GrassData.bSupportsFootprints = true;
    GrassData.DeformationResistance = 1.5f;
    GrassData.TerrainType = ECore_TerrainType::Grass;
    TerrainPhysicsMap.Add(ECore_TerrainType::Grass, GrassData);
    
    // Sand terrain
    FCore_TerrainPhysicsData SandData;
    SandData.Friction = 0.4f;
    SandData.Restitution = 0.01f;
    SandData.Density = 1.6f;
    SandData.bSupportsFootprints = true;
    SandData.DeformationResistance = 0.5f;
    SandData.TerrainType = ECore_TerrainType::Sand;
    TerrainPhysicsMap.Add(ECore_TerrainType::Sand, SandData);
    
    // Mud terrain
    FCore_TerrainPhysicsData MudData;
    MudData.Friction = 0.3f;
    MudData.Restitution = 0.0f;
    MudData.Density = 1.8f;
    MudData.bSupportsFootprints = true;
    MudData.DeformationResistance = 0.3f;
    MudData.TerrainType = ECore_TerrainType::Mud;
    TerrainPhysicsMap.Add(ECore_TerrainType::Mud, MudData);
    
    // Snow terrain
    FCore_TerrainPhysicsData SnowData;
    SnowData.Friction = 0.2f;
    SnowData.Restitution = 0.0f;
    SnowData.Density = 0.5f;
    SnowData.bSupportsFootprints = true;
    SnowData.DeformationResistance = 0.1f;
    SnowData.TerrainType = ECore_TerrainType::Snow;
    TerrainPhysicsMap.Add(ECore_TerrainType::Snow, SnowData);
    
    // Water terrain
    FCore_TerrainPhysicsData WaterData;
    WaterData.Friction = 0.1f;
    WaterData.Restitution = 0.0f;
    WaterData.Density = 1.0f;
    WaterData.bSupportsFootprints = false;
    WaterData.DeformationResistance = 0.0f;
    WaterData.TerrainType = ECore_TerrainType::Water;
    TerrainPhysicsMap.Add(ECore_TerrainType::Water, WaterData);
}

void ACore_TerrainPhysicsManager::CacheTerrainReferences()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find landscape actor
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        LandscapeRef = *ActorItr;
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Found landscape actor: %s"), *LandscapeRef->GetName());
        break; // Use first landscape found
    }
    
    // Find terrain static mesh actors
    TerrainMeshActors.Empty();
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        FString ActorName = MeshActor->GetName().ToLower();
        
        // Check if this is a terrain-related mesh
        if (ActorName.Contains(TEXT("rock")) || 
            ActorName.Contains(TEXT("stone")) || 
            ActorName.Contains(TEXT("terrain")) ||
            ActorName.Contains(TEXT("ground")) ||
            ActorName.Contains(TEXT("cliff")))
        {
            TerrainMeshActors.Add(MeshActor);
            UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Registered terrain mesh: %s"), *MeshActor->GetName());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Cached %d terrain mesh actors"), TerrainMeshActors.Num());
}

void ACore_TerrainPhysicsManager::UpdateTerrainPhysics(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if it's time for physics update
    if (CurrentTime - LastPhysicsUpdate < PhysicsUpdateInterval)
    {
        return;
    }
    
    LastPhysicsUpdate = CurrentTime;
    
    // Update footprint system
    if (bEnableFootprints)
    {
        UpdateFootprintSystem(DeltaTime);
    }
    
    // Optimize performance
    OptimizePhysicsPerformance();
}

void ACore_TerrainPhysicsManager::UpdateFootprintSystem(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old footprints
    RemoveOldFootprints(CurrentTime);
    
    // Limit total footprints
    if (ActiveFootprints.Num() > MaxFootprints)
    {
        int32 FootprintsToRemove = ActiveFootprints.Num() - MaxFootprints;
        ActiveFootprints.RemoveAt(0, FootprintsToRemove);
    }
}

void ACore_TerrainPhysicsManager::OptimizePhysicsPerformance()
{
    // This function can be expanded to include LOD systems, culling, etc.
    // For now, it's a placeholder for future optimization features
}

ECore_TerrainType ACore_TerrainPhysicsManager::GetTerrainTypeAtLocation(const FVector& WorldLocation) const
{
    // Simple terrain type detection based on height and slope
    // In a full implementation, this would query landscape materials or use texture sampling
    
    float Height = GetTerrainHeightAtLocation(WorldLocation);
    FVector Normal = GetTerrainNormalAtLocation(WorldLocation);
    float Slope = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector));
    
    // Basic terrain classification
    if (Height < -100.0f)
    {
        return ECore_TerrainType::Water;
    }
    else if (Slope > FMath::DegreesToRadians(45.0f))
    {
        return ECore_TerrainType::Rock;
    }
    else if (Height > 1000.0f)
    {
        return ECore_TerrainType::Snow;
    }
    else if (Height < 50.0f)
    {
        return ECore_TerrainType::Sand;
    }
    else
    {
        return ECore_TerrainType::Grass;
    }
}

FCore_TerrainPhysicsData ACore_TerrainPhysicsManager::GetTerrainPhysicsData(ECore_TerrainType TerrainType) const
{
    if (const FCore_TerrainPhysicsData* FoundData = TerrainPhysicsMap.Find(TerrainType))
    {
        return *FoundData;
    }
    
    // Return default dirt data if not found
    FCore_TerrainPhysicsData DefaultData;
    return DefaultData;
}

void ACore_TerrainPhysicsManager::SetTerrainPhysicsData(ECore_TerrainType TerrainType, const FCore_TerrainPhysicsData& PhysicsData)
{
    TerrainPhysicsMap.Add(TerrainType, PhysicsData);
}

void ACore_TerrainPhysicsManager::CreateFootprint(const FVector& Location, float Depth, float Radius, ECore_TerrainType TerrainType)
{
    if (!bEnableFootprints)
    {
        return;
    }
    
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsData(TerrainType);
    if (!TerrainData.bSupportsFootprints)
    {
        return;
    }
    
    FCore_FootprintData NewFootprint;
    NewFootprint.Location = Location;
    NewFootprint.Depth = Depth;
    NewFootprint.Radius = Radius;
    NewFootprint.TimeStamp = GetWorld()->GetTimeSeconds();
    NewFootprint.TerrainType = TerrainType;
    
    ActiveFootprints.Add(NewFootprint);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_TerrainPhysicsManager: Created footprint at %s"), *Location.ToString());
}

void ACore_TerrainPhysicsManager::RemoveOldFootprints(float CurrentTime)
{
    ActiveFootprints.RemoveAll([this, CurrentTime](const FCore_FootprintData& Footprint)
    {
        return (CurrentTime - Footprint.TimeStamp) > FootprintLifetime;
    });
}

void ACore_TerrainPhysicsManager::ClearAllFootprints()
{
    ActiveFootprints.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Cleared all footprints"));
}

void ACore_TerrainPhysicsManager::DeformTerrainAtLocation(const FVector& Location, float Radius, float Intensity, bool bRaise)
{
    if (!bEnableTerrainDeformation || !LandscapeRef)
    {
        return;
    }
    
    // Terrain deformation implementation would go here
    // This is a complex feature that requires landscape heightfield modification
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Deforming terrain at %s (R:%.1f, I:%.1f)"), 
           *Location.ToString(), Radius, Intensity);
}

void ACore_TerrainPhysicsManager::CreateImpactCrater(const FVector& Location, float Radius, float Depth)
{
    DeformTerrainAtLocation(Location, Radius, -Depth, false);
    
    // Create footprint for the crater
    CreateFootprint(Location, Depth, Radius, ECore_TerrainType::Dirt);
}

void ACore_TerrainPhysicsManager::CreateExplosionDeformation(const FVector& Location, float Radius, float Force)
{
    float CraterDepth = FMath::Clamp(Force * 0.1f, 10.0f, 200.0f);
    CreateImpactCrater(Location, Radius, CraterDepth);
}

void ACore_TerrainPhysicsManager::ApplyPhysicsMaterialToTerrain(ECore_TerrainType TerrainType, UPhysicalMaterial* PhysicsMaterial)
{
    if (!PhysicsMaterial)
    {
        return;
    }
    
    PhysicsMaterials.Add(TerrainType, PhysicsMaterial);
    
    // Apply to landscape if available
    if (LandscapeRef)
    {
        // Implementation would set the physics material on landscape components
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Applied physics material to landscape for terrain type %d"), 
               (int32)TerrainType);
    }
    
    // Apply to terrain mesh actors
    for (AStaticMeshActor* MeshActor : TerrainMeshActors)
    {
        if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
        {
            MeshComp->SetPhysMaterialOverride(PhysicsMaterial);
        }
    }
}

UPhysicalMaterial* ACore_TerrainPhysicsManager::GetPhysicsMaterialForTerrain(ECore_TerrainType TerrainType) const
{
    if (const TSoftObjectPtr<UPhysicalMaterial>* FoundMaterial = PhysicsMaterials.Find(TerrainType))
    {
        return FoundMaterial->LoadSynchronous();
    }
    
    return nullptr;
}

void ACore_TerrainPhysicsManager::RefreshTerrainPhysicsMaterials()
{
    // Apply physics materials to all terrain objects
    for (const auto& TerrainPair : TerrainPhysicsMap)
    {
        ECore_TerrainType TerrainType = TerrainPair.Key;
        UPhysicalMaterial* PhysicsMaterial = GetPhysicsMaterialForTerrain(TerrainType);
        
        if (PhysicsMaterial)
        {
            ApplyPhysicsMaterialToTerrain(TerrainType, PhysicsMaterial);
        }
    }
}

void ACore_TerrainPhysicsManager::RegisterTerrainMeshActor(AStaticMeshActor* MeshActor)
{
    if (MeshActor && !TerrainMeshActors.Contains(MeshActor))
    {
        TerrainMeshActors.Add(MeshActor);
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Registered terrain mesh actor: %s"), *MeshActor->GetName());
    }
}

void ACore_TerrainPhysicsManager::UnregisterTerrainMeshActor(AStaticMeshActor* MeshActor)
{
    if (MeshActor)
    {
        TerrainMeshActors.Remove(MeshActor);
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Unregistered terrain mesh actor: %s"), *MeshActor->GetName());
    }
}

float ACore_TerrainPhysicsManager::GetTerrainHeightAtLocation(const FVector& WorldLocation) const
{
    if (LandscapeRef)
    {
        // Use landscape height sampling
        return LandscapeRef->GetActorLocation().Z;
    }
    
    // Fallback: use line trace
    UWorld* World = GetWorld();
    if (World)
    {
        FHitResult HitResult;
        FVector TraceStart = WorldLocation + FVector(0, 0, 10000);
        FVector TraceEnd = WorldLocation - FVector(0, 0, 10000);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            return HitResult.Location.Z;
        }
    }
    
    return 0.0f;
}

FVector ACore_TerrainPhysicsManager::GetTerrainNormalAtLocation(const FVector& WorldLocation) const
{
    UWorld* World = GetWorld();
    if (World)
    {
        FHitResult HitResult;
        FVector TraceStart = WorldLocation + FVector(0, 0, 1000);
        FVector TraceEnd = WorldLocation - FVector(0, 0, 1000);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            return HitResult.Normal;
        }
    }
    
    return FVector::UpVector;
}

void ACore_TerrainPhysicsManager::DrawDebugFootprints() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (const FCore_FootprintData& Footprint : ActiveFootprints)
    {
        FColor DebugColor = FColor::Brown;
        switch (Footprint.TerrainType)
        {
            case ECore_TerrainType::Sand:
                DebugColor = FColor::Yellow;
                break;
            case ECore_TerrainType::Mud:
                DebugColor = FColor::Black;
                break;
            case ECore_TerrainType::Snow:
                DebugColor = FColor::White;
                break;
            default:
                DebugColor = FColor::Brown;
                break;
        }
        
        DrawDebugSphere(World, Footprint.Location, Footprint.Radius, 12, DebugColor, false, 0.1f);
    }
}

void ACore_TerrainPhysicsManager::LogTerrainPhysicsStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== Core_TerrainPhysicsManager Stats ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Footprints: %d"), ActiveFootprints.Num());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Mesh Actors: %d"), TerrainMeshActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Physics Materials: %d"), PhysicsMaterials.Num());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Types Configured: %d"), TerrainPhysicsMap.Num());
    UE_LOG(LogTemp, Warning, TEXT("Landscape Reference: %s"), LandscapeRef ? TEXT("Valid") : TEXT("None"));
    UE_LOG(LogTemp, Warning, TEXT("Deformation Enabled: %s"), bEnableTerrainDeformation ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Footprints Enabled: %s"), bEnableFootprints ? TEXT("Yes") : TEXT("No"));
}

void ACore_TerrainPhysicsManager::EditorRefreshTerrainPhysics()
{
    if (GetWorld())
    {
        bIsInitialized = false;
        InitializeTerrainPhysics();
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Editor refresh complete"));
    }
}