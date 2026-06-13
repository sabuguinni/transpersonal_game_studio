#include "Core_TerrainPhysicsManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Landscape/Landscape.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UCore_TerrainPhysicsManager::UCore_TerrainPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    TerrainSampleRadius = 100.0f;
    MaxWalkableSlope = 45.0f;
    MaxBuildableSlope = 30.0f;
    bEnableTerrainPhysicsDebug = false;
    PhysicsUpdateInterval = 0.1f;
    LastUpdateTime = 0.0f;
    CachedLandscape = nullptr;
}

void UCore_TerrainPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultPhysicsMaterials();
    CacheLandscapeReference();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Initialized with %d biome materials"), BiomePhysicsMaterials.Num());
}

void UCore_TerrainPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= PhysicsUpdateInterval)
    {
        LastUpdateTime = 0.0f;
        
        // Update terrain physics for nearby actors if needed
        if (bEnableTerrainPhysicsDebug && GetOwner())
        {
            FVector OwnerLocation = GetOwner()->GetActorLocation();
            FCore_TerrainPhysicsData TerrainData = GetTerrainDataAtLocation(OwnerLocation);
            DebugDrawTerrainData(OwnerLocation, TerrainData);
        }
    }
}

FCore_TerrainPhysicsData UCore_TerrainPhysicsManager::GetTerrainDataAtLocation(const FVector& Location)
{
    // Check if we have cached data for this location
    FVector GridLocation = FVector(
        FMath::RoundToInt(Location.X / 100.0f) * 100.0f,
        FMath::RoundToInt(Location.Y / 100.0f) * 100.0f,
        Location.Z
    );
    
    if (TerrainDataMap.Contains(GridLocation))
    {
        return TerrainDataMap[GridLocation];
    }
    
    // Calculate terrain data
    FCore_TerrainPhysicsData TerrainData;
    
    // Calculate slope angle
    TerrainData.SlopeAngle = CalculateSlopeAngle(Location, TerrainSampleRadius);
    
    // Determine biome type based on location (simplified)
    float Height = Location.Z;
    if (Height > 1000.0f)
    {
        TerrainData.TerrainBiome = EBiomeType::Mountain;
        TerrainData.Roughness = 0.8f;
        TerrainData.Friction = 0.9f;
        TerrainData.Hardness = 1.5f;
    }
    else if (Height < -500.0f)
    {
        TerrainData.TerrainBiome = EBiomeType::Ocean;
        TerrainData.Roughness = 0.1f;
        TerrainData.Friction = 0.2f;
        TerrainData.Hardness = 0.1f;
    }
    else if (FMath::Abs(Location.X) > 5000.0f || FMath::Abs(Location.Y) > 5000.0f)
    {
        TerrainData.TerrainBiome = EBiomeType::Desert;
        TerrainData.Roughness = 0.3f;
        TerrainData.Friction = 0.4f;
        TerrainData.Hardness = 0.7f;
    }
    else
    {
        TerrainData.TerrainBiome = EBiomeType::Grassland;
        TerrainData.Roughness = 0.5f;
        TerrainData.Friction = 0.7f;
        TerrainData.Hardness = 1.0f;
    }
    
    // Determine walkability
    TerrainData.bIsWalkable = IsLocationWalkable(Location, MaxWalkableSlope);
    TerrainData.bCanBuildOn = CanBuildAtLocation(Location, MaxBuildableSlope);
    
    // Cache the data
    TerrainDataMap.Add(GridLocation, TerrainData);
    
    return TerrainData;
}

float UCore_TerrainPhysicsManager::CalculateSlopeAngle(const FVector& Location, float SampleRadius)
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    // Sample points in a cross pattern
    TArray<FVector> SamplePoints = {
        Location + FVector(SampleRadius, 0, 0),
        Location + FVector(-SampleRadius, 0, 0),
        Location + FVector(0, SampleRadius, 0),
        Location + FVector(0, -SampleRadius, 0)
    };
    
    TArray<float> Heights;
    Heights.Add(Location.Z); // Center height
    
    // Perform line traces to get heights
    for (const FVector& SamplePoint : SamplePoints)
    {
        FVector TraceStart = SamplePoint + FVector(0, 0, 1000.0f);
        FVector TraceEnd = SamplePoint + FVector(0, 0, -1000.0f);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = true;
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            Heights.Add(HitResult.Location.Z);
        }
        else
        {
            Heights.Add(Location.Z); // Fallback to center height
        }
    }
    
    // Calculate maximum height difference
    float MinHeight = FMath::Min(Heights);
    float MaxHeight = FMath::Max(Heights);
    float HeightDiff = MaxHeight - MinHeight;
    
    // Calculate slope angle
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Atan(HeightDiff / SampleRadius));
    
    return FMath::Clamp(SlopeAngle, 0.0f, 90.0f);
}

bool UCore_TerrainPhysicsManager::IsLocationWalkable(const FVector& Location, float MaxSlopeAngle)
{
    float SlopeAngle = CalculateSlopeAngle(Location, TerrainSampleRadius);
    return SlopeAngle <= MaxSlopeAngle;
}

bool UCore_TerrainPhysicsManager::CanBuildAtLocation(const FVector& Location, float MaxSlopeAngle)
{
    float SlopeAngle = CalculateSlopeAngle(Location, TerrainSampleRadius);
    bool bSlopeOK = SlopeAngle <= MaxSlopeAngle;
    bool bNotUnderwater = !IsLocationUnderwater(Location);
    
    return bSlopeOK && bNotUnderwater;
}

void UCore_TerrainPhysicsManager::UpdatePhysicsMaterialAtLocation(const FVector& Location, const FCore_PhysicsMaterialSettings& Settings)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Find actors at this location and update their physics materials
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), OverlappingActors);
    
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || FVector::Dist(Actor->GetActorLocation(), Location) > 100.0f)
        {
            continue;
        }
        
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && MeshComp->GetBodyInstance())
        {
            // Update physics properties
            FBodyInstance* BodyInstance = MeshComp->GetBodyInstance();
            // Note: Direct physics material modification requires custom physics materials
            // This is a simplified implementation
        }
    }
}

FVector UCore_TerrainPhysicsManager::GetSurfaceNormal(const FVector& Location)
{
    if (!GetWorld())
    {
        return FVector::UpVector;
    }
    
    FVector TraceStart = Location + FVector(0, 0, 100.0f);
    FVector TraceEnd = Location + FVector(0, 0, -100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}

float UCore_TerrainPhysicsManager::GetTerrainHardness(const FVector& Location)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainDataAtLocation(Location);
    return TerrainData.Hardness;
}

void UCore_TerrainPhysicsManager::ApplyTerrainEffectsToActor(AActor* Actor, const FVector& Location)
{
    if (!Actor)
    {
        return;
    }
    
    FCore_TerrainPhysicsData TerrainData = GetTerrainDataAtLocation(Location);
    
    // Apply movement speed modifications based on terrain
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp)
    {
        // Modify physics properties based on terrain
        float FrictionMultiplier = TerrainData.Friction;
        // Apply friction effects (simplified implementation)
    }
}

void UCore_TerrainPhysicsManager::RegisterTerrainPhysicsData(const FVector& Location, const FCore_TerrainPhysicsData& Data)
{
    FVector GridLocation = FVector(
        FMath::RoundToInt(Location.X / 100.0f) * 100.0f,
        FMath::RoundToInt(Location.Y / 100.0f) * 100.0f,
        Location.Z
    );
    
    TerrainDataMap.Add(GridLocation, Data);
}

void UCore_TerrainPhysicsManager::ClearTerrainPhysicsData()
{
    TerrainDataMap.Empty();
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Cleared all terrain physics data"));
}

void UCore_TerrainPhysicsManager::CreatePhysicsMaterialForBiome(EBiomeType BiomeType, const FCore_PhysicsMaterialSettings& Settings)
{
    // Create a new physics material (simplified - in practice would load from assets)
    UPhysicalMaterial* PhysMat = NewObject<UPhysicalMaterial>();
    if (PhysMat)
    {
        PhysMat->StaticFriction = Settings.StaticFriction;
        PhysMat->DynamicFriction = Settings.DynamicFriction;
        PhysMat->Restitution = Settings.Restitution;
        PhysMat->Density = Settings.Density;
        PhysMat->SleepLinearVelocityThreshold *= Settings.SleepThresholdMultiplier;
        
        BiomePhysicsMaterials.Add(BiomeType, PhysMat);
    }
}

UPhysicalMaterial* UCore_TerrainPhysicsManager::GetPhysicsMaterialForBiome(EBiomeType BiomeType)
{
    if (BiomePhysicsMaterials.Contains(BiomeType))
    {
        return BiomePhysicsMaterials[BiomeType];
    }
    
    return nullptr;
}

TArray<FVector> UCore_TerrainPhysicsManager::GetNearbyWalkablePositions(const FVector& CenterLocation, float Radius, int32 NumSamples)
{
    TArray<FVector> WalkablePositions;
    
    for (int32 i = 0; i < NumSamples; i++)
    {
        float Angle = (2.0f * PI * i) / NumSamples;
        FVector SampleLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        if (IsLocationWalkable(SampleLocation))
        {
            WalkablePositions.Add(SampleLocation);
        }
    }
    
    return WalkablePositions;
}

bool UCore_TerrainPhysicsManager::IsLocationUnderwater(const FVector& Location)
{
    // Simplified water detection - assumes water level at Z=0
    return Location.Z < 0.0f;
}

float UCore_TerrainPhysicsManager::GetWaterDepthAtLocation(const FVector& Location)
{
    if (IsLocationUnderwater(Location))
    {
        return FMath::Abs(Location.Z);
    }
    
    return 0.0f;
}

void UCore_TerrainPhysicsManager::InitializeDefaultPhysicsMaterials()
{
    // Create default physics materials for each biome type
    FCore_PhysicsMaterialSettings GrasslandSettings;
    GrasslandSettings.StaticFriction = 0.7f;
    GrasslandSettings.DynamicFriction = 0.6f;
    GrasslandSettings.Restitution = 0.3f;
    CreatePhysicsMaterialForBiome(EBiomeType::Grassland, GrasslandSettings);
    
    FCore_PhysicsMaterialSettings DesertSettings;
    DesertSettings.StaticFriction = 0.4f;
    DesertSettings.DynamicFriction = 0.3f;
    DesertSettings.Restitution = 0.1f;
    CreatePhysicsMaterialForBiome(EBiomeType::Desert, DesertSettings);
    
    FCore_PhysicsMaterialSettings MountainSettings;
    MountainSettings.StaticFriction = 0.9f;
    MountainSettings.DynamicFriction = 0.8f;
    MountainSettings.Restitution = 0.5f;
    CreatePhysicsMaterialForBiome(EBiomeType::Mountain, MountainSettings);
    
    FCore_PhysicsMaterialSettings OceanSettings;
    OceanSettings.StaticFriction = 0.1f;
    OceanSettings.DynamicFriction = 0.05f;
    OceanSettings.Restitution = 0.0f;
    CreatePhysicsMaterialForBiome(EBiomeType::Ocean, OceanSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Initialized %d default physics materials"), BiomePhysicsMaterials.Num());
}

void UCore_TerrainPhysicsManager::CacheLandscapeReference()
{
    if (GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            CachedLandscape = Cast<ALandscape>(FoundActors[0]);
            UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsManager: Cached landscape reference"));
        }
    }
}

FVector UCore_TerrainPhysicsManager::PerformLineTrace(const FVector& Start, const FVector& End)
{
    if (!GetWorld())
    {
        return Start;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return End;
}

void UCore_TerrainPhysicsManager::DebugDrawTerrainData(const FVector& Location, const FCore_TerrainPhysicsData& Data)
{
    if (!GetWorld() || !bEnableTerrainPhysicsDebug)
    {
        return;
    }
    
    // Draw slope angle visualization
    FColor SlopeColor = Data.SlopeAngle > MaxWalkableSlope ? FColor::Red : FColor::Green;
    DrawDebugSphere(GetWorld(), Location, 50.0f, 8, SlopeColor, false, PhysicsUpdateInterval);
    
    // Draw terrain type text
    FString TerrainInfo = FString::Printf(TEXT("Slope: %.1f°\nBiome: %d\nWalkable: %s"), 
        Data.SlopeAngle, 
        (int32)Data.TerrainBiome,
        Data.bIsWalkable ? TEXT("Yes") : TEXT("No"));
    
    DrawDebugString(GetWorld(), Location + FVector(0, 0, 100), TerrainInfo, nullptr, FColor::White, PhysicsUpdateInterval);
}