#include "Arch_CretaceousFoundationSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UArch_CretaceousFoundationSystem::UArch_CretaceousFoundationSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;

	FoundationMesh = nullptr;
	WeatherProtectionZone = nullptr;
	FoundationBounds = nullptr;
	CurrentStability = 1.0f;
	WeatheringAccumulation = 0.0f;
	bIsInitialized = false;
	LastWeatheringUpdate = 0.0f;
}

void UArch_CretaceousFoundationSystem::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bIsInitialized)
	{
		InitializeFoundation(FoundationConfig);
	}
}

void UArch_CretaceousFoundationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsInitialized)
	{
		ApplyWeatheringEffect(DeltaTime);
		CalculateStability();
	}
}

void UArch_CretaceousFoundationSystem::InitializeFoundation(const FArch_FoundationProperties& Properties)
{
	FoundationConfig = Properties;
	
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Foundation System: No owner actor found"));
		return;
	}

	// Create foundation mesh component if it doesn't exist
	if (!FoundationMesh)
	{
		FoundationMesh = NewObject<UStaticMeshComponent>(Owner);
		FoundationMesh->SetupAttachment(Owner->GetRootComponent());
		FoundationMesh->RegisterComponent();
	}

	// Create weather protection zone
	if (!WeatherProtectionZone)
	{
		WeatherProtectionZone = NewObject<UBoxComponent>(Owner);
		WeatherProtectionZone->SetupAttachment(FoundationMesh);
		WeatherProtectionZone->RegisterComponent();
	}

	// Create foundation bounds
	if (!FoundationBounds)
	{
		FoundationBounds = NewObject<USphereComponent>(Owner);
		FoundationBounds->SetupAttachment(FoundationMesh);
		FoundationBounds->RegisterComponent();
	}

	UpdateFoundationMesh();
	GenerateAnchorPoints();
	UpdateWeatherProtectionZone();
	CalculateStability();
	
	bIsInitialized = true;
	
	UE_LOG(LogTemp, Log, TEXT("Foundation System: Initialized %s foundation with diameter %.1f"), 
		*UEnum::GetValueAsString(FoundationConfig.FoundationType), FoundationConfig.Diameter);
}

void UArch_CretaceousFoundationSystem::UpdateFoundationMesh()
{
	if (!FoundationMesh)
		return;

	// Set mesh based on foundation type
	UStaticMesh* MeshToUse = GetFoundationMeshForType(FoundationConfig.FoundationType);
	if (MeshToUse)
	{
		FoundationMesh->SetStaticMesh(MeshToUse);
	}

	// Scale based on diameter
	float ScaleFactor = FoundationConfig.Diameter / 4.0f; // Base diameter is 4 meters
	FoundationMesh->SetWorldScale3D(FVector(ScaleFactor, ScaleFactor, 1.0f + FoundationConfig.ElevationHeight));

	// Apply elevation
	FVector CurrentLocation = FoundationMesh->GetRelativeLocation();
	CurrentLocation.Z = FoundationConfig.ElevationHeight * 50.0f; // Convert to cm
	FoundationMesh->SetRelativeLocation(CurrentLocation);

	ApplyMaterialProperties();
}

UStaticMesh* UArch_CretaceousFoundationSystem::GetFoundationMeshForType(EArch_FoundationType Type)
{
	// Return basic cube mesh for now - in production this would load specific foundation meshes
	return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
}

void UArch_CretaceousFoundationSystem::ApplyMaterialProperties()
{
	if (!FoundationMesh)
		return;

	// Apply weathering and moss growth effects to material parameters
	// This would typically involve dynamic material instances
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		FoundationMesh->SetMaterial(0, BaseMaterial);
	}
}

void UArch_CretaceousFoundationSystem::GenerateAnchorPoints()
{
	AnchorPoints.Empty();
	
	float Radius = FoundationConfig.Diameter * 0.5f;
	int32 NumPoints = 8; // 8 anchor points around the foundation
	
	for (int32 i = 0; i < NumPoints; i++)
	{
		float Angle = (2.0f * PI * i) / NumPoints;
		FVector AnchorPoint = FVector(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			0.0f
		);
		AnchorPoints.Add(AnchorPoint);
	}
	
	// Add center point
	AnchorPoints.Add(FVector::ZeroVector);
}

void UArch_CretaceousFoundationSystem::UpdateWeatherProtectionZone()
{
	if (!WeatherProtectionZone)
		return;

	float ProtectionRadius = FoundationConfig.Diameter * 0.6f;
	float ProtectionHeight = FoundationConfig.bHasWeatherProtection ? 300.0f : 50.0f;
	
	WeatherProtectionZone->SetBoxExtent(FVector(ProtectionRadius * 100.0f, ProtectionRadius * 100.0f, ProtectionHeight));
	WeatherProtectionZone->SetRelativeLocation(FVector(0.0f, 0.0f, ProtectionHeight * 0.5f));
}

void UArch_CretaceousFoundationSystem::CalculateStability()
{
	float BaseStability = 1.0f;
	
	// Reduce stability based on weathering
	BaseStability -= (FoundationConfig.WeatheringLevel * 0.3f);
	
	// Reduce stability if partially buried (less stable foundation)
	if (FoundationConfig.bIsPartiallyBuried)
	{
		BaseStability -= 0.1f;
	}
	
	// Increase stability for elevated platforms
	if (FoundationConfig.ElevationHeight > 0.5f)
	{
		BaseStability += 0.2f;
	}
	
	// Foundation type affects stability
	switch (FoundationConfig.FoundationType)
	{
		case EArch_FoundationType::CircularStone:
			BaseStability += 0.1f; // Circular is more stable
			break;
		case EArch_FoundationType::RectangularStone:
			// No modifier
			break;
		case EArch_FoundationType::NaturalRock:
			BaseStability += 0.3f; // Natural rock is very stable
			break;
		case EArch_FoundationType::ElevatedPlatform:
			BaseStability += 0.2f;
			break;
		case EArch_FoundationType::CaveEntrance:
			BaseStability += 0.4f; // Cave is very stable
			break;
		case EArch_FoundationType::CliffLedge:
			BaseStability -= 0.2f; // Cliff ledge is less stable
			break;
	}
	
	CurrentStability = FMath::Clamp(BaseStability, 0.1f, 1.0f);
}

void UArch_CretaceousFoundationSystem::SetWeatheringLevel(float NewWeatheringLevel)
{
	FoundationConfig.WeatheringLevel = FMath::Clamp(NewWeatheringLevel, 0.0f, 1.0f);
	ApplyMaterialProperties();
	CalculateStability();
}

void UArch_CretaceousFoundationSystem::SetMossGrowth(float NewMossGrowth)
{
	FoundationConfig.MossGrowth = FMath::Clamp(NewMossGrowth, 0.0f, 1.0f);
	ApplyMaterialProperties();
}

bool UArch_CretaceousFoundationSystem::IsLocationSuitableForFoundation(const FVector& Location, float RequiredRadius)
{
	UWorld* World = GetWorld();
	if (!World)
		return false;

	// Check for overlapping foundations
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	bool bHasOverlap = World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldStatic,
		FCollisionShape::MakeSphere(RequiredRadius * 100.0f), // Convert to cm
		QueryParams
	);
	
	return !bHasOverlap;
}

FVector UArch_CretaceousFoundationSystem::GetOptimalFoundationLocation(const FVector& SearchCenter, float SearchRadius)
{
	UWorld* World = GetWorld();
	if (!World)
		return SearchCenter;

	// Simple grid search for optimal location
	float GridSize = 100.0f; // 1 meter grid
	FVector BestLocation = SearchCenter;
	float BestScore = -1.0f;
	
	for (float X = -SearchRadius; X <= SearchRadius; X += GridSize)
	{
		for (float Y = -SearchRadius; Y <= SearchRadius; Y += GridSize)
		{
			FVector TestLocation = SearchCenter + FVector(X, Y, 0.0f);
			
			// Trace to ground
			FHitResult HitResult;
			FVector TraceStart = TestLocation + FVector(0.0f, 0.0f, 1000.0f);
			FVector TraceEnd = TestLocation - FVector(0.0f, 0.0f, 1000.0f);
			
			if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldStatic))
			{
				TestLocation = HitResult.Location;
				
				// Score based on flatness and suitability
				float Score = 1.0f;
				
				// Prefer flatter surfaces
				float SurfaceAngle = FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector));
				Score -= (SurfaceAngle / PI) * 0.5f;
				
				// Check if location is suitable
				if (IsLocationSuitableForFoundation(TestLocation, FoundationConfig.Diameter * 0.5f))
				{
					Score += 0.3f;
				}
				
				if (Score > BestScore)
				{
					BestScore = Score;
					BestLocation = TestLocation;
				}
			}
		}
	}
	
	return BestLocation;
}

bool UArch_CretaceousFoundationSystem::CanSupportStructure(float StructureWeight, float StructureRadius)
{
	float MaxWeight = CurrentStability * FoundationConfig.Diameter * 1000.0f; // kg capacity
	float MaxRadius = FoundationConfig.Diameter * 0.8f; // 80% of foundation diameter
	
	return (StructureWeight <= MaxWeight) && (StructureRadius <= MaxRadius);
}

TArray<FVector> UArch_CretaceousFoundationSystem::GetFoundationAnchorPoints()
{
	TArray<FVector> WorldAnchorPoints;
	
	if (FoundationMesh)
	{
		FTransform FoundationTransform = FoundationMesh->GetComponentTransform();
		
		for (const FVector& LocalPoint : AnchorPoints)
		{
			FVector WorldPoint = FoundationTransform.TransformPosition(LocalPoint);
			WorldAnchorPoints.Add(WorldPoint);
		}
	}
	
	return WorldAnchorPoints;
}

void UArch_CretaceousFoundationSystem::ApplyWeatheringEffect(float DeltaTime)
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	
	if (CurrentTime - LastWeatheringUpdate > 10.0f) // Update every 10 seconds
	{
		WeatheringAccumulation += DeltaTime * 0.001f; // Very slow weathering
		
		if (WeatheringAccumulation > 0.01f)
		{
			float NewWeathering = FoundationConfig.WeatheringLevel + WeatheringAccumulation;
			SetWeatheringLevel(NewWeathering);
			WeatheringAccumulation = 0.0f;
		}
		
		LastWeatheringUpdate = CurrentTime;
	}
}

bool UArch_CretaceousFoundationSystem::IsPlayerInWeatherProtectionZone() const
{
	if (!WeatherProtectionZone)
		return false;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
		return false;

	return WeatherProtectionZone->IsOverlappingActor(PlayerPawn);
}

float UArch_CretaceousFoundationSystem::GetFoundationStability() const
{
	return CurrentStability;
}

// Foundation Actor Implementation
AArch_FoundationActor::AArch_FoundationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Create foundation mesh component
	FoundationMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoundationMesh"));
	FoundationMeshComponent->SetupAttachment(RootComponent);

	// Create foundation system component
	FoundationSystem = CreateDefaultSubobject<UArch_CretaceousFoundationSystem>(TEXT("FoundationSystem"));
}

void AArch_FoundationActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (FoundationSystem)
	{
		FoundationSystem->FoundationMesh = FoundationMeshComponent;
	}
}

void AArch_FoundationActor::SetupFoundation(EArch_FoundationType Type, float Diameter, float WeatheringLevel)
{
	if (FoundationSystem)
	{
		FArch_FoundationProperties Properties;
		Properties.FoundationType = Type;
		Properties.Diameter = Diameter;
		Properties.WeatheringLevel = WeatheringLevel;
		
		FoundationSystem->InitializeFoundation(Properties);
		OnFoundationInitialized();
	}
}

bool AArch_FoundationActor::ValidateFoundationPlacement()
{
	if (!FoundationSystem)
		return false;

	FVector CurrentLocation = GetActorLocation();
	float RequiredRadius = FoundationSystem->FoundationConfig.Diameter * 0.5f;
	
	return FoundationSystem->IsLocationSuitableForFoundation(CurrentLocation, RequiredRadius);
}