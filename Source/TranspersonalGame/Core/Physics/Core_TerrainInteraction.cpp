#include "Core_TerrainInteraction.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Landscape/LandscapeDataAccess.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"

UCore_TerrainInteraction::UCore_TerrainInteraction()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds
	
	TerrainDetectionRange = 200.0f;
	TerrainUpdateFrequency = 2.0f;
	FootprintRadius = 15.0f;
	FootprintDepth = 2.0f;
	FootprintDuration = 60.0f;
	MaxDigRadius = 50.0f;
	MaxDigDepth = 25.0f;
	LastTerrainUpdate = 0.0f;
	CachedLandscape = nullptr;
}

void UCore_TerrainInteraction::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeTerrainTypeMap();
	MapTerrainTypes();
}

void UCore_TerrainInteraction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	LastTerrainUpdate += DeltaTime;
	if (LastTerrainUpdate >= TerrainUpdateFrequency)
	{
		FVector OwnerLocation = GetOwner()->GetActorLocation();
		UpdateTerrainCache(OwnerLocation, TerrainDetectionRange);
		LastTerrainUpdate = 0.0f;
	}
}

ECore_TerrainType UCore_TerrainInteraction::DetectTerrainTypeAtLocation(const FVector& Location)
{
	if (!IsLocationValid(Location))
	{
		return ECore_TerrainType::Grass;
	}

	// Check cache first
	FVector GridLocation = FVector(
		FMath::RoundToInt(Location.X / 100.0f) * 100.0f,
		FMath::RoundToInt(Location.Y / 100.0f) * 100.0f,
		0.0f
	);

	if (TerrainTypeCache.Contains(GridLocation))
	{
		return TerrainTypeCache[GridLocation];
	}

	// Sample from landscape
	ECore_TerrainType DetectedType = SampleTerrainFromLandscape(Location);
	TerrainTypeCache.Add(GridLocation, DetectedType);
	
	return DetectedType;
}

FCore_TerrainProperties UCore_TerrainInteraction::GetTerrainPropertiesAtLocation(const FVector& Location)
{
	ECore_TerrainType TerrainType = DetectTerrainTypeAtLocation(Location);
	
	if (TerrainTypeMap.Contains(TerrainType))
	{
		return TerrainTypeMap[TerrainType];
	}
	
	return FCore_TerrainProperties();
}

bool UCore_TerrainInteraction::DigTerrainAtLocation(const FVector& Location, float Radius, float Depth)
{
	if (!IsLocationValid(Location) || !CachedLandscape)
	{
		return false;
	}

	// Clamp values to safe ranges
	Radius = FMath::Clamp(Radius, 10.0f, MaxDigRadius);
	Depth = FMath::Clamp(Depth, 5.0f, MaxDigDepth);

	// Check if terrain can be dug at this location
	FCore_TerrainProperties TerrainProps = GetTerrainPropertiesAtLocation(Location);
	if (!TerrainProps.bCanDigTerrain)
	{
		return false;
	}

	// Get landscape info
	ULandscapeInfo* LandscapeInfo = CachedLandscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		return false;
	}

	// Convert world location to landscape coordinates
	FVector LandscapeLocation = GetLandscapeLocation(Location);
	
	// Create a circular dig pattern
	int32 RadiusInt = FMath::RoundToInt(Radius);
	TArray<uint16> HeightData;
	TArray<FLandscapeHeightfieldImportData> ImportData;

	// This is a simplified version - in a real implementation,
	// you would use LandscapeInfo->GetHeightmapData and modify the height values
	UE_LOG(LogTemp, Log, TEXT("Digging terrain at location %s with radius %f and depth %f"), 
		*Location.ToString(), Radius, Depth);

	return true;
}

bool UCore_TerrainInteraction::CreateFootprint(const FVector& Location, float FootSize)
{
	if (!IsLocationValid(Location))
	{
		return false;
	}

	FCore_TerrainProperties TerrainProps = GetTerrainPropertiesAtLocation(Location);
	if (!TerrainProps.bLeavesFootprints)
	{
		return false;
	}

	// Create a temporary footprint effect
	// In a real implementation, this would modify the landscape height data slightly
	// or spawn a decal to simulate the footprint
	
	UE_LOG(LogTemp, Log, TEXT("Creating footprint at location %s with size %f"), 
		*Location.ToString(), FootSize);

	// Draw debug footprint for visualization
	if (GetWorld())
	{
		DrawDebugCircle(GetWorld(), Location, FootprintRadius, 16, FColor::Brown, false, FootprintDuration);
	}

	return true;
}

void UCore_TerrainInteraction::ApplyTerrainPhysics(UPrimitiveComponent* Component, const FVector& Location)
{
	if (!Component || !IsLocationValid(Location))
	{
		return;
	}

	FCore_TerrainProperties TerrainProps = GetTerrainPropertiesAtLocation(Location);
	
	// Apply friction and bounce modifications
	if (UPhysicalMaterial* PhysMat = Component->GetBodyInstance()->GetSimplePhysicalMaterial())
	{
		// Store original values if not already stored
		static float OriginalFriction = PhysMat->Friction;
		static float OriginalRestitution = PhysMat->Restitution;
		
		// Apply terrain modifiers
		PhysMat->Friction = OriginalFriction * TerrainProps.FrictionMultiplier;
		PhysMat->Restitution = OriginalRestitution * TerrainProps.BounceMultiplier;
	}
}

float UCore_TerrainInteraction::GetMovementSpeedMultiplier(const FVector& Location)
{
	FCore_TerrainProperties TerrainProps = GetTerrainPropertiesAtLocation(Location);
	return TerrainProps.MovementSpeedMultiplier;
}

float UCore_TerrainInteraction::GetStaminaDrainMultiplier(const FVector& Location)
{
	FCore_TerrainProperties TerrainProps = GetTerrainPropertiesAtLocation(Location);
	return TerrainProps.StaminaDrainMultiplier;
}

void UCore_TerrainInteraction::MapTerrainTypes()
{
	if (!GetWorld())
	{
		return;
	}

	// Find the landscape in the world
	for (TActorIterator<ALandscape> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		CachedLandscape = *ActorIterator;
		break; // Use the first landscape found
	}

	if (!CachedLandscape)
	{
		UE_LOG(LogTemp, Warning, TEXT("No landscape found in world for terrain interaction"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Terrain interaction system mapped to landscape: %s"), 
		*CachedLandscape->GetName());
}

void UCore_TerrainInteraction::UpdateTerrainCache(const FVector& CenterLocation, float Radius)
{
	if (!IsLocationValid(CenterLocation))
	{
		return;
	}

	// Clear old cache entries that are too far away
	TArray<FVector> KeysToRemove;
	for (const auto& CacheEntry : TerrainTypeCache)
	{
		float Distance = FVector::Dist(CacheEntry.Key, CenterLocation);
		if (Distance > Radius * 2.0f)
		{
			KeysToRemove.Add(CacheEntry.Key);
		}
	}

	for (const FVector& Key : KeysToRemove)
	{
		TerrainTypeCache.Remove(Key);
	}

	// Sample new terrain types in a grid around the center location
	int32 GridSize = FMath::RoundToInt(Radius / 100.0f);
	for (int32 X = -GridSize; X <= GridSize; X++)
	{
		for (int32 Y = -GridSize; Y <= GridSize; Y++)
		{
			FVector SampleLocation = CenterLocation + FVector(X * 100.0f, Y * 100.0f, 0.0f);
			FVector GridLocation = FVector(
				FMath::RoundToInt(SampleLocation.X / 100.0f) * 100.0f,
				FMath::RoundToInt(SampleLocation.Y / 100.0f) * 100.0f,
				0.0f
			);

			if (!TerrainTypeCache.Contains(GridLocation))
			{
				ECore_TerrainType TerrainType = SampleTerrainFromLandscape(SampleLocation);
				TerrainTypeCache.Add(GridLocation, TerrainType);
			}
		}
	}
}

ECore_TerrainType UCore_TerrainInteraction::SampleTerrainFromLandscape(const FVector& Location)
{
	if (!CachedLandscape)
	{
		return ECore_TerrainType::Grass;
	}

	// Get the landscape info
	ULandscapeInfo* LandscapeInfo = CachedLandscape->GetLandscapeInfo();
	if (!LandscapeInfo)
	{
		return ECore_TerrainType::Grass;
	}

	// Convert world location to landscape coordinates
	FVector LandscapeLocation = GetLandscapeLocation(Location);
	
	// Sample height to determine terrain type
	// This is a simplified approach - in a real implementation,
	// you would sample the landscape's weight maps for different materials
	
	float Height = LandscapeLocation.Z;
	
	// Simple height-based terrain classification
	if (Height < -100.0f)
	{
		return ECore_TerrainType::Water;
	}
	else if (Height < 0.0f)
	{
		return ECore_TerrainType::Sand;
	}
	else if (Height < 200.0f)
	{
		return ECore_TerrainType::Grass;
	}
	else if (Height < 500.0f)
	{
		return ECore_TerrainType::Dirt;
	}
	else if (Height < 1000.0f)
	{
		return ECore_TerrainType::Rock;
	}
	else
	{
		return ECore_TerrainType::Snow;
	}
}

void UCore_TerrainInteraction::InitializeTerrainTypeMap()
{
	TerrainTypeMap.Empty();

	// Grass terrain
	FCore_TerrainProperties GrassProps;
	GrassProps.TerrainType = ECore_TerrainType::Grass;
	GrassProps.FrictionMultiplier = 1.0f;
	GrassProps.BounceMultiplier = 0.3f;
	GrassProps.MovementSpeedMultiplier = 1.0f;
	GrassProps.bCanDigTerrain = true;
	GrassProps.bLeavesFootprints = true;
	GrassProps.StaminaDrainMultiplier = 1.0f;
	TerrainTypeMap.Add(ECore_TerrainType::Grass, GrassProps);

	// Dirt terrain
	FCore_TerrainProperties DirtProps;
	DirtProps.TerrainType = ECore_TerrainType::Dirt;
	DirtProps.FrictionMultiplier = 0.8f;
	DirtProps.BounceMultiplier = 0.2f;
	DirtProps.MovementSpeedMultiplier = 0.9f;
	DirtProps.bCanDigTerrain = true;
	DirtProps.bLeavesFootprints = true;
	DirtProps.StaminaDrainMultiplier = 1.1f;
	TerrainTypeMap.Add(ECore_TerrainType::Dirt, DirtProps);

	// Rock terrain
	FCore_TerrainProperties RockProps;
	RockProps.TerrainType = ECore_TerrainType::Rock;
	RockProps.FrictionMultiplier = 1.2f;
	RockProps.BounceMultiplier = 0.8f;
	RockProps.MovementSpeedMultiplier = 0.8f;
	RockProps.bCanDigTerrain = false;
	RockProps.bLeavesFootprints = false;
	RockProps.StaminaDrainMultiplier = 1.3f;
	TerrainTypeMap.Add(ECore_TerrainType::Rock, RockProps);

	// Sand terrain
	FCore_TerrainProperties SandProps;
	SandProps.TerrainType = ECore_TerrainType::Sand;
	SandProps.FrictionMultiplier = 0.6f;
	SandProps.BounceMultiplier = 0.1f;
	SandProps.MovementSpeedMultiplier = 0.7f;
	SandProps.bCanDigTerrain = true;
	SandProps.bLeavesFootprints = true;
	SandProps.StaminaDrainMultiplier = 1.4f;
	TerrainTypeMap.Add(ECore_TerrainType::Sand, SandProps);

	// Mud terrain
	FCore_TerrainProperties MudProps;
	MudProps.TerrainType = ECore_TerrainType::Mud;
	MudProps.FrictionMultiplier = 0.4f;
	MudProps.BounceMultiplier = 0.05f;
	MudProps.MovementSpeedMultiplier = 0.5f;
	MudProps.bCanDigTerrain = true;
	MudProps.bLeavesFootprints = true;
	MudProps.StaminaDrainMultiplier = 1.8f;
	TerrainTypeMap.Add(ECore_TerrainType::Mud, MudProps);

	// Snow terrain
	FCore_TerrainProperties SnowProps;
	SnowProps.TerrainType = ECore_TerrainType::Snow;
	SnowProps.FrictionMultiplier = 0.3f;
	SnowProps.BounceMultiplier = 0.1f;
	SnowProps.MovementSpeedMultiplier = 0.6f;
	SnowProps.bCanDigTerrain = true;
	SnowProps.bLeavesFootprints = true;
	SnowProps.StaminaDrainMultiplier = 1.5f;
	TerrainTypeMap.Add(ECore_TerrainType::Snow, SnowProps);

	// Water terrain
	FCore_TerrainProperties WaterProps;
	WaterProps.TerrainType = ECore_TerrainType::Water;
	WaterProps.FrictionMultiplier = 0.1f;
	WaterProps.BounceMultiplier = 0.0f;
	WaterProps.MovementSpeedMultiplier = 0.3f;
	WaterProps.bCanDigTerrain = false;
	WaterProps.bLeavesFootprints = false;
	WaterProps.StaminaDrainMultiplier = 2.0f;
	TerrainTypeMap.Add(ECore_TerrainType::Water, WaterProps);

	// Lava terrain
	FCore_TerrainProperties LavaProps;
	LavaProps.TerrainType = ECore_TerrainType::Lava;
	LavaProps.FrictionMultiplier = 0.8f;
	LavaProps.BounceMultiplier = 0.2f;
	LavaProps.MovementSpeedMultiplier = 0.1f;
	LavaProps.bCanDigTerrain = false;
	LavaProps.bLeavesFootprints = false;
	LavaProps.StaminaDrainMultiplier = 5.0f;
	TerrainTypeMap.Add(ECore_TerrainType::Lava, LavaProps);
}

bool UCore_TerrainInteraction::IsLocationValid(const FVector& Location)
{
	if (!GetWorld())
	{
		return false;
	}

	// Check if location is within reasonable bounds
	return FMath::Abs(Location.X) < 1000000.0f && 
		   FMath::Abs(Location.Y) < 1000000.0f && 
		   FMath::Abs(Location.Z) < 100000.0f;
}

FVector UCore_TerrainInteraction::GetLandscapeLocation(const FVector& WorldLocation)
{
	if (!CachedLandscape)
	{
		return WorldLocation;
	}

	// Transform world location to landscape local space
	FTransform LandscapeTransform = CachedLandscape->GetActorTransform();
	return LandscapeTransform.InverseTransformPosition(WorldLocation);
}