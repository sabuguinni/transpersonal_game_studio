#include "VegetationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UVegetationSystem::UVegetationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms

    // Default settings
    MaxDrawDistance = 5000.0f;
    MaxInstancesPerType = 10000;
    LODDistanceMultiplier = 1.0f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WindStrength = 0.5f;
    WindVariation = 0.2f;

    // Initialize default biome settings
    FBiomeVegetationSettings ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.OverallDensity = 1.5f;
    ForestBiome.BiodiversityFactor = 1.2f;
    BiomeSettings.Add(ForestBiome);

    FBiomeVegetationSettings GrasslandBiome;
    GrasslandBiome.BiomeType = EBiomeType::Grassland;
    GrasslandBiome.OverallDensity = 2.0f;
    GrasslandBiome.BiodiversityFactor = 0.8f;
    BiomeSettings.Add(GrasslandBiome);
}

void UVegetationSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize instanced mesh components for each vegetation type
    for (int32 i = 0; i < (int32)EVegetationType::Vines + 1; ++i)
    {
        EVegetationType VegType = (EVegetationType)i;
        
        UInstancedStaticMeshComponent* InstanceComponent = NewObject<UInstancedStaticMeshComponent>(GetOwner());
        InstanceComponent->SetupAttachment(GetOwner()->GetRootComponent());
        InstanceComponent->RegisterComponent();
        
        // Set performance properties
        InstanceComponent->SetCastShadow(true);
        InstanceComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        InstanceComponent->SetCanEverAffectNavigation(false);
        
        VegetationInstances.Add(VegType, InstanceComponent);
    }

    UE_LOG(LogTemp, Log, TEXT("VegetationSystem initialized with %d vegetation types"), VegetationInstances.Num());
}

void UVegetationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update wind animation
    float CurrentTime = GetWorld()->GetTimeSeconds();
    FVector AnimatedWind = WindDirection * WindStrength;
    AnimatedWind += FVector(
        FMath::Sin(CurrentTime * 2.0f) * WindVariation,
        FMath::Cos(CurrentTime * 1.5f) * WindVariation,
        0.0f
    );

    // Apply wind to vegetation instances
    for (auto& VegPair : VegetationInstances)
    {
        if (VegPair.Value)
        {
            // Update material parameters for wind animation
            UMaterialInterface* Material = VegPair.Value->GetMaterial(0);
            if (Material)
            {
                UMaterialInstanceDynamic* DynamicMaterial = VegPair.Value->CreateDynamicMaterialInstance(0, Material);
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetVectorParameterValue(TEXT("WindDirection"), AnimatedWind);
                    DynamicMaterial->SetScalarParameterValue(TEXT("WindStrength"), WindStrength);
                }
            }
        }
    }
}

void UVegetationSystem::GenerateVegetationForBiome(EBiomeType BiomeType, const FVector& Center, float Radius)
{
    // Find biome settings
    FBiomeVegetationSettings* BiomeSettings = nullptr;
    for (FBiomeVegetationSettings& Settings : this->BiomeSettings)
    {
        if (Settings.BiomeType == BiomeType)
        {
            BiomeSettings = &Settings;
            break;
        }
    }

    if (!BiomeSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("No biome settings found for biome type %d"), (int32)BiomeType);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Generating vegetation for biome %d at location %s with radius %f"), 
           (int32)BiomeType, *Center.ToString(), Radius);

    // Calculate number of vegetation instances based on density
    int32 NumInstances = FMath::RoundToInt(Radius * Radius * BiomeSettings->OverallDensity * 0.001f);
    NumInstances = FMath::Clamp(NumInstances, 10, MaxInstancesPerType);

    // Generate vegetation for each type in the biome
    for (const FVegetationData& VegData : BiomeSettings->VegetationTypes)
    {
        UInstancedStaticMeshComponent* InstanceComponent = VegetationInstances.FindRef(VegData.VegetationType);
        if (!InstanceComponent)
        {
            continue;
        }

        // Load mesh if available
        if (VegData.Mesh.IsValid())
        {
            UStaticMesh* Mesh = VegData.Mesh.LoadSynchronous();
            if (Mesh)
            {
                InstanceComponent->SetStaticMesh(Mesh);
            }
        }

        // Generate random positions within radius
        int32 TypeInstances = FMath::RoundToInt(NumInstances * VegData.Density);
        for (int32 i = 0; i < TypeInstances; ++i)
        {
            // Random position within circle
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            float Distance = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * Radius;
            
            FVector Position = Center + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                0.0f
            );

            // Check if we can place vegetation here
            if (CanPlaceVegetationAt(Position, VegData.VegetationType))
            {
                FVector Scale = GetRandomScale(VegData);
                FRotator Rotation = GetRandomRotation(VegData.VegetationType);
                
                FTransform InstanceTransform(Rotation, Position, Scale);
                InstanceComponent->AddInstance(InstanceTransform);
                
                GeneratedPositions.Add(Position);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d vegetation positions for biome %d"), 
           GeneratedPositions.Num(), (int32)BiomeType);
}

void UVegetationSystem::ClearVegetationInArea(const FVector& Center, float Radius)
{
    for (auto& VegPair : VegetationInstances)
    {
        if (VegPair.Value)
        {
            VegPair.Value->ClearInstances();
        }
    }
    
    GeneratedPositions.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared vegetation in area around %s"), *Center.ToString());
}

void UVegetationSystem::UpdateVegetationLOD(const FVector& ViewerLocation)
{
    for (auto& VegPair : VegetationInstances)
    {
        if (VegPair.Value)
        {
            // Calculate distance-based LOD
            float Distance = FVector::Dist(ViewerLocation, GetOwner()->GetActorLocation());
            float LODLevel = Distance / (MaxDrawDistance * LODDistanceMultiplier);
            
            // Set culling distance
            VegPair.Value->SetCullDistances(0, MaxDrawDistance);
            
            // Adjust instance density based on distance
            if (LODLevel > 0.8f)
            {
                // Far distance - reduce instances
                VegPair.Value->SetRenderInMainPass(false);
            }
            else
            {
                VegPair.Value->SetRenderInMainPass(true);
            }
        }
    }
}

void UVegetationSystem::SetWindParameters(const FVector& NewWindDirection, float NewWindStrength)
{
    WindDirection = NewWindDirection.GetSafeNormal();
    WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Updated wind parameters: Direction %s, Strength %f"), 
           *WindDirection.ToString(), WindStrength);
}

bool UVegetationSystem::CanPlaceVegetationAt(const FVector& Location, EVegetationType Type)
{
    // Perform line trace to check ground
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0.0f, 0.0f, 1000.0f);
    FVector EndLocation = Location - FVector(0.0f, 0.0f, 1000.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (!bHit)
    {
        return false;
    }
    
    // Check slope - vegetation doesn't grow on steep slopes
    float SlopeAngle = FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector));
    float MaxSlope = (Type == EVegetationType::Trees) ? 30.0f : 45.0f; // Trees need flatter ground
    
    if (FMath::RadiansToDegrees(SlopeAngle) > MaxSlope)
    {
        return false;
    }
    
    // Check minimum distance from other vegetation
    float MinDistance = (Type == EVegetationType::Trees) ? 300.0f : 50.0f;
    for (const FVector& ExistingPos : GeneratedPositions)
    {
        if (FVector::Dist(Location, ExistingPos) < MinDistance)
        {
            return false;
        }
    }
    
    return true;
}

FVector UVegetationSystem::GetRandomScale(const FVegetationData& VegData)
{
    return FVector(
        FMath::RandRange(VegData.MinScale.X, VegData.MaxScale.X),
        FMath::RandRange(VegData.MinScale.Y, VegData.MaxScale.Y),
        FMath::RandRange(VegData.MinScale.Z, VegData.MaxScale.Z)
    );
}

FRotator UVegetationSystem::GetRandomRotation(EVegetationType Type)
{
    FRotator Rotation = FRotator::ZeroRotator;
    
    // Most vegetation only rotates around Z axis
    Rotation.Yaw = FMath::RandRange(0.0f, 360.0f);
    
    // Trees and bushes can have slight tilt
    if (Type == EVegetationType::Trees || Type == EVegetationType::Bushes)
    {
        Rotation.Pitch = FMath::RandRange(-5.0f, 5.0f);
        Rotation.Roll = FMath::RandRange(-5.0f, 5.0f);
    }
    
    return Rotation;
}

float UVegetationSystem::CalculateDensityAtLocation(const FVector& Location, EBiomeType BiomeType)
{
    // Base density from biome settings
    float BaseDensity = 1.0f;
    for (const FBiomeVegetationSettings& Settings : BiomeSettings)
    {
        if (Settings.BiomeType == BiomeType)
        {
            BaseDensity = Settings.OverallDensity;
            break;
        }
    }
    
    // Add noise-based variation
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(Location.X * 0.001f, Location.Y * 0.001f));
    float DensityMultiplier = 0.5f + (NoiseValue + 1.0f) * 0.5f; // Convert from [-1,1] to [0.5,1.5]
    
    return BaseDensity * DensityMultiplier;
}

void UVegetationSystem::OptimizeInstancedMeshes()
{
    for (auto& VegPair : VegetationInstances)
    {
        if (VegPair.Value)
        {
            // Enable GPU culling for better performance
            VegPair.Value->SetCullDistances(0, MaxDrawDistance);
            VegPair.Value->SetCanEverAffectNavigation(false);
            
            // Optimize collision
            VegPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            
            UE_LOG(LogTemp, Log, TEXT("Optimized %s vegetation instances: %d instances"), 
                   *UEnum::GetValueAsString(VegPair.Key), VegPair.Value->GetInstanceCount());
        }
    }
}

void UVegetationSystem::UpdateCullingDistances()
{
    for (auto& VegPair : VegetationInstances)
    {
        if (VegPair.Value)
        {
            float CullDistance = MaxDrawDistance;
            
            // Different vegetation types have different cull distances
            switch (VegPair.Key)
            {
                case EVegetationType::Trees:
                    CullDistance = MaxDrawDistance;
                    break;
                case EVegetationType::Bushes:
                    CullDistance = MaxDrawDistance * 0.7f;
                    break;
                case EVegetationType::Grass:
                    CullDistance = MaxDrawDistance * 0.5f;
                    break;
                default:
                    CullDistance = MaxDrawDistance * 0.6f;
                    break;
            }
            
            VegPair.Value->SetCullDistances(0, CullDistance);
        }
    }
}