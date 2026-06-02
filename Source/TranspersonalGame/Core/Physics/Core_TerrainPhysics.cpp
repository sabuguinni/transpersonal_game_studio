#include "Core_TerrainPhysics.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysics::UCore_TerrainPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize terrain physics settings
    TerrainSettings.SurfaceFriction = 0.7f;
    TerrainSettings.SurfaceRestitution = 0.3f;
    TerrainSettings.MaxSlopeAngle = 45.0f;
    TerrainSettings.StabilityThreshold = 0.8f;
    TerrainSettings.bEnableErosion = true;
    TerrainSettings.ErosionRate = 0.1f;
    
    // Initialize surface types
    InitializeSurfaceTypes();
    
    bIsInitialized = false;
    LastUpdateTime = 0.0f;
    UpdateFrequency = 0.1f; // Update every 100ms
}

void UCore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainSystem();
    CacheTerrainData();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: System initialized"));
}

void UCore_TerrainPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateTerrainPhysics();
        ProcessTerrainInteractions();
        
        if (TerrainSettings.bEnableErosion)
        {
            ProcessTerrainErosion(DeltaTime);
        }
        
        LastUpdateTime = 0.0f;
    }
}

void UCore_TerrainPhysics::InitializeTerrainSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysics: No valid world found"));
        return;
    }
    
    // Find landscape actors
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape && IsValid(Landscape))
        {
            LandscapeActors.Add(Landscape);
            UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Found landscape actor: %s"), *Landscape->GetName());
        }
    }
    
    // Cache terrain mesh components
    CacheTerrainMeshes();
    
    bIsInitialized = true;
}

void UCore_TerrainPhysics::CacheTerrainData()
{
    TerrainCache.Empty();
    
    for (ALandscape* Landscape : LandscapeActors)
    {
        if (!IsValid(Landscape))
            continue;
            
        TArray<ULandscapeComponent*> LandscapeComponents;
        Landscape->GetLandscapeComponents(LandscapeComponents);
        
        for (ULandscapeComponent* Component : LandscapeComponents)
        {
            if (!IsValid(Component))
                continue;
                
            FCore_TerrainCacheData CacheData;
            CacheData.Component = Component;
            CacheData.Bounds = Component->Bounds;
            CacheData.SurfaceType = DetermineSurfaceType(Component);
            CacheData.LastModified = GetWorld()->GetTimeSeconds();
            
            TerrainCache.Add(CacheData);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Cached %d terrain components"), TerrainCache.Num());
}

void UCore_TerrainPhysics::CacheTerrainMeshes()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Find static mesh actors that represent terrain
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!IsValid(Actor))
            continue;
            
        // Check if actor has terrain-related tags or names
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("terrain")) || 
            ActorName.Contains(TEXT("ground")) || 
            ActorName.Contains(TEXT("rock")) ||
            ActorName.Contains(TEXT("cliff")))
        {
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && IsValid(MeshComp))
            {
                TerrainMeshes.Add(MeshComp);
            }
        }
    }
}

FCore_TerrainSurfaceInfo UCore_TerrainPhysics::GetSurfaceInfoAtLocation(const FVector& WorldLocation)
{
    FCore_TerrainSurfaceInfo SurfaceInfo;
    
    // Perform line trace to get surface information
    FHitResult HitResult;
    FVector TraceStart = WorldLocation + FVector(0, 0, 1000);
    FVector TraceEnd = WorldLocation - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        SurfaceInfo.Location = HitResult.Location;
        SurfaceInfo.Normal = HitResult.Normal;
        SurfaceInfo.SurfaceType = DetermineSurfaceTypeFromHit(HitResult);
        SurfaceInfo.SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
        SurfaceInfo.bIsStable = SurfaceInfo.SlopeAngle <= TerrainSettings.MaxSlopeAngle;
        SurfaceInfo.Friction = GetSurfaceFriction(SurfaceInfo.SurfaceType);
        SurfaceInfo.Restitution = GetSurfaceRestitution(SurfaceInfo.SurfaceType);
    }
    else
    {
        // Default values if no hit
        SurfaceInfo.Location = WorldLocation;
        SurfaceInfo.Normal = FVector::UpVector;
        SurfaceInfo.SurfaceType = ECore_TerrainSurfaceType::Grass;
        SurfaceInfo.SlopeAngle = 0.0f;
        SurfaceInfo.bIsStable = true;
        SurfaceInfo.Friction = TerrainSettings.SurfaceFriction;
        SurfaceInfo.Restitution = TerrainSettings.SurfaceRestitution;
    }
    
    return SurfaceInfo;
}

bool UCore_TerrainPhysics::IsLocationStable(const FVector& WorldLocation)
{
    FCore_TerrainSurfaceInfo SurfaceInfo = GetSurfaceInfoAtLocation(WorldLocation);
    return SurfaceInfo.bIsStable && SurfaceInfo.SlopeAngle <= TerrainSettings.MaxSlopeAngle;
}

float UCore_TerrainPhysics::GetSlopeAngleAtLocation(const FVector& WorldLocation)
{
    FCore_TerrainSurfaceInfo SurfaceInfo = GetSurfaceInfoAtLocation(WorldLocation);
    return SurfaceInfo.SlopeAngle;
}

void UCore_TerrainPhysics::ApplyTerrainForces(UPrimitiveComponent* Component, const FVector& Location)
{
    if (!IsValid(Component))
        return;
        
    FCore_TerrainSurfaceInfo SurfaceInfo = GetSurfaceInfoAtLocation(Location);
    
    // Apply slope forces
    if (SurfaceInfo.SlopeAngle > 0.0f)
    {
        FVector SlopeDirection = FVector::CrossProduct(SurfaceInfo.Normal, FVector::UpVector).GetSafeNormal();
        FVector SlopeForce = SlopeDirection * SurfaceInfo.SlopeAngle * 10.0f; // Scale factor
        
        Component->AddForceAtLocation(SlopeForce, Location);
    }
    
    // Apply surface-specific forces
    ApplySurfaceTypeForces(Component, SurfaceInfo);
}

void UCore_TerrainPhysics::UpdateTerrainPhysics()
{
    // Update cached terrain data if needed
    for (FCore_TerrainCacheData& CacheData : TerrainCache)
    {
        if (!IsValid(CacheData.Component))
            continue;
            
        // Check if component has been modified
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - CacheData.LastModified > 1.0f) // Check every second
        {
            CacheData.SurfaceType = DetermineSurfaceType(CacheData.Component);
            CacheData.LastModified = CurrentTime;
        }
    }
}

void UCore_TerrainPhysics::ProcessTerrainInteractions()
{
    // Process interactions between objects and terrain
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Find all physics objects that might interact with terrain
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (!IsValid(Pawn))
            continue;
            
        FVector PawnLocation = Pawn->GetActorLocation();
        FCore_TerrainSurfaceInfo SurfaceInfo = GetSurfaceInfoAtLocation(PawnLocation);
        
        // Apply terrain effects to pawn
        ApplyTerrainEffectsToPawn(Pawn, SurfaceInfo);
    }
}

void UCore_TerrainPhysics::ProcessTerrainErosion(float DeltaTime)
{
    // Simple erosion simulation
    for (FCore_TerrainCacheData& CacheData : TerrainCache)
    {
        if (!IsValid(CacheData.Component))
            continue;
            
        // Apply erosion effects based on surface type and environmental factors
        ProcessComponentErosion(CacheData, DeltaTime);
    }
}

void UCore_TerrainPhysics::InitializeSurfaceTypes()
{
    SurfaceTypeData.Empty();
    
    // Grass
    FCore_SurfaceTypeData GrassData;
    GrassData.Friction = 0.7f;
    GrassData.Restitution = 0.3f;
    GrassData.Hardness = 0.4f;
    GrassData.Wetness = 0.2f;
    SurfaceTypeData.Add(ECore_TerrainSurfaceType::Grass, GrassData);
    
    // Rock
    FCore_SurfaceTypeData RockData;
    RockData.Friction = 0.9f;
    RockData.Restitution = 0.1f;
    RockData.Hardness = 1.0f;
    RockData.Wetness = 0.0f;
    SurfaceTypeData.Add(ECore_TerrainSurfaceType::Rock, RockData);
    
    // Mud
    FCore_SurfaceTypeData MudData;
    MudData.Friction = 0.3f;
    MudData.Restitution = 0.1f;
    MudData.Hardness = 0.2f;
    MudData.Wetness = 0.8f;
    SurfaceTypeData.Add(ECore_TerrainSurfaceType::Mud, MudData);
    
    // Sand
    FCore_SurfaceTypeData SandData;
    SandData.Friction = 0.4f;
    SandData.Restitution = 0.2f;
    SandData.Hardness = 0.3f;
    SandData.Wetness = 0.1f;
    SurfaceTypeData.Add(ECore_TerrainSurfaceType::Sand, SandData);
    
    // Water
    FCore_SurfaceTypeData WaterData;
    WaterData.Friction = 0.1f;
    WaterData.Restitution = 0.0f;
    WaterData.Hardness = 0.0f;
    WaterData.Wetness = 1.0f;
    SurfaceTypeData.Add(ECore_TerrainSurfaceType::Water, WaterData);
}

ECore_TerrainSurfaceType UCore_TerrainPhysics::DetermineSurfaceType(ULandscapeComponent* Component)
{
    // Simple surface type determination based on component properties
    // In a real implementation, this would analyze landscape materials and heightmaps
    return ECore_TerrainSurfaceType::Grass; // Default
}

ECore_TerrainSurfaceType UCore_TerrainPhysics::DetermineSurfaceTypeFromHit(const FHitResult& HitResult)
{
    // Analyze hit result to determine surface type
    if (HitResult.GetActor())
    {
        FString ActorName = HitResult.GetActor()->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")))
            return ECore_TerrainSurfaceType::Rock;
        else if (ActorName.Contains(TEXT("water")))
            return ECore_TerrainSurfaceType::Water;
        else if (ActorName.Contains(TEXT("mud")))
            return ECore_TerrainSurfaceType::Mud;
        else if (ActorName.Contains(TEXT("sand")))
            return ECore_TerrainSurfaceType::Sand;
    }
    
    return ECore_TerrainSurfaceType::Grass; // Default
}

float UCore_TerrainPhysics::GetSurfaceFriction(ECore_TerrainSurfaceType SurfaceType)
{
    if (SurfaceTypeData.Contains(SurfaceType))
    {
        return SurfaceTypeData[SurfaceType].Friction;
    }
    return TerrainSettings.SurfaceFriction;
}

float UCore_TerrainPhysics::GetSurfaceRestitution(ECore_TerrainSurfaceType SurfaceType)
{
    if (SurfaceTypeData.Contains(SurfaceType))
    {
        return SurfaceTypeData[SurfaceType].Restitution;
    }
    return TerrainSettings.SurfaceRestitution;
}

void UCore_TerrainPhysics::ApplySurfaceTypeForces(UPrimitiveComponent* Component, const FCore_TerrainSurfaceInfo& SurfaceInfo)
{
    if (!IsValid(Component))
        return;
        
    // Apply forces based on surface type
    switch (SurfaceInfo.SurfaceType)
    {
        case ECore_TerrainSurfaceType::Mud:
            // Apply drag force in mud
            {
                FVector Velocity = Component->GetPhysicsLinearVelocity();
                FVector DragForce = -Velocity * 50.0f; // Drag coefficient
                Component->AddForce(DragForce);
            }
            break;
            
        case ECore_TerrainSurfaceType::Water:
            // Apply buoyancy and water resistance
            {
                FVector BuoyancyForce = FVector::UpVector * 980.0f; // Buoyancy
                Component->AddForce(BuoyancyForce);
                
                FVector Velocity = Component->GetPhysicsLinearVelocity();
                FVector WaterResistance = -Velocity * 100.0f;
                Component->AddForce(WaterResistance);
            }
            break;
            
        case ECore_TerrainSurfaceType::Sand:
            // Slight sinking effect in sand
            {
                FVector SinkForce = -FVector::UpVector * 100.0f;
                Component->AddForce(SinkForce);
            }
            break;
            
        default:
            break;
    }
}

void UCore_TerrainPhysics::ApplyTerrainEffectsToPawn(APawn* Pawn, const FCore_TerrainSurfaceInfo& SurfaceInfo)
{
    if (!IsValid(Pawn))
        return;
        
    // Broadcast terrain interaction event
    OnTerrainInteraction.Broadcast(Pawn, SurfaceInfo.SurfaceType, SurfaceInfo.Location);
    
    // Apply movement speed modifications based on surface type
    if (SurfaceTypeData.Contains(SurfaceInfo.SurfaceType))
    {
        const FCore_SurfaceTypeData& TypeData = SurfaceTypeData[SurfaceInfo.SurfaceType];
        
        // Modify movement based on surface properties
        // This would typically interface with the character movement component
        float SpeedMultiplier = 1.0f - (TypeData.Wetness * 0.3f); // Wet surfaces slow movement
        
        // Apply the speed multiplier to the pawn's movement
        // Implementation would depend on the specific movement system
    }
}

void UCore_TerrainPhysics::ProcessComponentErosion(FCore_TerrainCacheData& CacheData, float DeltaTime)
{
    // Simple erosion simulation
    if (!IsValid(CacheData.Component))
        return;
        
    // Apply erosion based on surface type and environmental factors
    float ErosionAmount = TerrainSettings.ErosionRate * DeltaTime;
    
    // Erosion would modify the landscape heightmap in a real implementation
    // For now, we just update the cache timestamp
    CacheData.LastModified = GetWorld()->GetTimeSeconds();
}