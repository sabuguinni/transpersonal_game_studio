#include "EnvArt_ProceduralFoliage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UEnvArt_ProceduralFoliage::UEnvArt_ProceduralFoliage()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    GenerationRadius = 15000.0f;
    MaxTotalInstances = 5000;
    bAutoGenerate = true;
    LODDistance1 = 5000.0f;
    LODDistance2 = 10000.0f;
    CullDistance = 15000.0f;
}

void UEnvArt_ProceduralFoliage::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultFoliageTypes();
    
    if (bAutoGenerate)
    {
        // Generate foliage for all biomes with proper distribution
        TArray<TPair<EEnvArt_BiomeType, FVector>> BiomeCenters = {
            {EEnvArt_BiomeType::Savanna, FVector(0, 0, 100)},
            {EEnvArt_BiomeType::Swamp, FVector(-50000, -45000, 100)},
            {EEnvArt_BiomeType::Forest, FVector(-45000, 40000, 100)},
            {EEnvArt_BiomeType::Desert, FVector(55000, 0, 100)},
            {EEnvArt_BiomeType::Mountain, FVector(40000, 50000, 100)}
        };
        
        for (const auto& BiomePair : BiomeCenters)
        {
            GenerateFoliageForBiome(BiomePair.Key, BiomePair.Value);
        }
    }
}

void UEnvArt_ProceduralFoliage::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update LOD levels based on player distance
    UpdateLODLevels();
}

void UEnvArt_ProceduralFoliage::GenerateFoliageForBiome(EEnvArt_BiomeType BiomeType, FVector BiomeCenter)
{
    if (!BiomeFoliageTypes.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("No foliage types defined for biome: %d"), (int32)BiomeType);
        return;
    }
    
    const TArray<FEnvArt_FoliageType>& FoliageTypes = BiomeFoliageTypes[BiomeType];
    int32 CurrentBiomeInstances = BiomeInstanceCounts.FindRef(BiomeType);
    
    // Respect per-biome limits (max 1000 props per biome from memory)
    int32 MaxInstancesForBiome = 1000;
    
    for (const FEnvArt_FoliageType& FoliageType : FoliageTypes)
    {
        if (CurrentBiomeInstances >= MaxInstancesForBiome)
        {
            UE_LOG(LogTemp, Warning, TEXT("Biome %d reached max instances limit: %d"), (int32)BiomeType, MaxInstancesForBiome);
            break;
        }
        
        int32 InstancesToGenerate = FMath::Min(FoliageType.MaxInstancesPerBiome, MaxInstancesForBiome - CurrentBiomeInstances);
        
        for (int32 i = 0; i < InstancesToGenerate; i++)
        {
            // Generate random location within biome radius
            FVector RandomOffset = FVector(
                FMath::RandRange(-GenerationRadius * 0.5f, GenerationRadius * 0.5f),
                FMath::RandRange(-GenerationRadius * 0.5f, GenerationRadius * 0.5f),
                0
            );
            
            FVector SpawnLocation = BiomeCenter + RandomOffset;
            
            // Perform ground trace to get proper Z coordinate
            FHitResult HitResult;
            FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
            FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                SpawnLocation = HitResult.Location;
                
                if (IsValidPlacementLocation(SpawnLocation, FoliageType.MaxSlope))
                {
                    FRotator RandomRotation = GetRandomRotation();
                    FVector RandomScale = GetRandomScaleFromRange(FoliageType.ScaleRange);
                    
                    CreateFoliageInstance(FoliageType, SpawnLocation, RandomRotation, RandomScale, BiomeType);
                    CurrentBiomeInstances++;
                }
            }
        }
    }
    
    BiomeInstanceCounts.Add(BiomeType, CurrentBiomeInstances);
    
    UE_LOG(LogTemp, Log, TEXT("Generated foliage for biome %d: %d instances"), (int32)BiomeType, CurrentBiomeInstances);
}

void UEnvArt_ProceduralFoliage::CreateFoliageInstance(const FEnvArt_FoliageType& FoliageType, const FVector& Location, const FRotator& Rotation, const FVector& Scale, EEnvArt_BiomeType BiomeType)
{
    if (!FoliageType.Mesh.IsValid())
    {
        return;
    }
    
    // Find or create instanced static mesh component for this foliage type
    UInstancedStaticMeshComponent* InstancedComponent = nullptr;
    
    for (UInstancedStaticMeshComponent* Component : FoliageComponents)
    {
        if (Component && Component->GetStaticMesh() == FoliageType.Mesh.Get())
        {
            InstancedComponent = Component;
            break;
        }
    }
    
    if (!InstancedComponent)
    {
        InstancedComponent = NewObject<UInstancedStaticMeshComponent>(GetOwner());
        InstancedComponent->SetStaticMesh(FoliageType.Mesh.Get());
        InstancedComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        InstancedComponent->RegisterComponent();
        
        // Set LOD and culling distances
        InstancedComponent->SetCullDistances(LODDistance1, CullDistance);
        
        FoliageComponents.Add(InstancedComponent);
    }
    
    // Add instance
    FTransform InstanceTransform(Rotation, Location, Scale);
    InstancedComponent->AddInstance(InstanceTransform);
}

bool UEnvArt_ProceduralFoliage::IsValidPlacementLocation(const FVector& Location, float MaxSlope) const
{
    // Check slope by tracing nearby points
    TArray<FVector> TracePoints = {
        Location + FVector(100, 0, 0),
        Location + FVector(-100, 0, 0),
        Location + FVector(0, 100, 0),
        Location + FVector(0, -100, 0)
    };
    
    for (const FVector& TracePoint : TracePoints)
    {
        FHitResult HitResult;
        FVector TraceStart = TracePoint + FVector(0, 0, 500);
        FVector TraceEnd = TracePoint - FVector(0, 0, 500);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            FVector SlopeVector = (HitResult.Location - Location).GetSafeNormal();
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SlopeVector, FVector::UpVector)));
            
            if (SlopeAngle > MaxSlope)
            {
                return false;
            }
        }
    }
    
    return true;
}

FVector UEnvArt_ProceduralFoliage::GetRandomScaleFromRange(const FVector2D& ScaleRange) const
{
    float RandomScale = FMath::RandRange(ScaleRange.X, ScaleRange.Y);
    return FVector(RandomScale);
}

FRotator UEnvArt_ProceduralFoliage::GetRandomRotation() const
{
    return FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
}

EEnvArt_BiomeType UEnvArt_ProceduralFoliage::GetBiomeTypeAtLocation(const FVector& Location) const
{
    // Determine biome based on location (simplified grid system)
    if (Location.X < -20000)
    {
        return Location.Y > 0 ? EEnvArt_BiomeType::Forest : EEnvArt_BiomeType::Swamp;
    }
    else if (Location.X > 20000)
    {
        return Location.Y > 0 ? EEnvArt_BiomeType::Mountain : EEnvArt_BiomeType::Desert;
    }
    else
    {
        return EEnvArt_BiomeType::Savanna;
    }
}

void UEnvArt_ProceduralFoliage::InitializeDefaultFoliageTypes()
{
    // Initialize default foliage types for each biome
    // These will use basic UE5 meshes until custom assets are available
    
    // Savanna - Grass and sparse trees
    TArray<FEnvArt_FoliageType> SavannaFoliage;
    FEnvArt_FoliageType SavannaGrass;
    SavannaGrass.Density = 2.0f;
    SavannaGrass.ScaleRange = FVector2D(0.5f, 1.5f);
    SavannaGrass.MaxInstancesPerBiome = 300;
    SavannaFoliage.Add(SavannaGrass);
    
    // Forest - Dense trees and undergrowth
    TArray<FEnvArt_FoliageType> ForestFoliage;
    FEnvArt_FoliageType ForestTree;
    ForestTree.Density = 1.5f;
    ForestTree.ScaleRange = FVector2D(0.8f, 1.8f);
    ForestTree.MaxInstancesPerBiome = 250;
    ForestFoliage.Add(ForestTree);
    
    // Swamp - Wetland vegetation
    TArray<FEnvArt_FoliageType> SwampFoliage;
    FEnvArt_FoliageType SwampReed;
    SwampReed.Density = 1.8f;
    SwampReed.ScaleRange = FVector2D(0.6f, 1.4f);
    SwampReed.MaxInstancesPerBiome = 280;
    SwampFoliage.Add(SwampReed);
    
    // Desert - Sparse cacti and rocks
    TArray<FEnvArt_FoliageType> DesertFoliage;
    FEnvArt_FoliageType DesertCactus;
    DesertCactus.Density = 0.5f;
    DesertCactus.ScaleRange = FVector2D(0.7f, 1.3f);
    DesertCactus.MaxInstancesPerBiome = 150;
    DesertFoliage.Add(DesertCactus);
    
    // Mountain - Alpine vegetation
    TArray<FEnvArt_FoliageType> MountainFoliage;
    FEnvArt_FoliageType MountainShrub;
    MountainShrub.Density = 1.0f;
    MountainShrub.ScaleRange = FVector2D(0.4f, 1.0f);
    MountainShrub.MaxInstancesPerBiome = 200;
    MountainFoliage.Add(MountainShrub);
    
    // Assign to biome map
    BiomeFoliageTypes.Add(EEnvArt_BiomeType::Savanna, SavannaFoliage);
    BiomeFoliageTypes.Add(EEnvArt_BiomeType::Forest, ForestFoliage);
    BiomeFoliageTypes.Add(EEnvArt_BiomeType::Swamp, SwampFoliage);
    BiomeFoliageTypes.Add(EEnvArt_BiomeType::Desert, DesertFoliage);
    BiomeFoliageTypes.Add(EEnvArt_BiomeType::Mountain, MountainFoliage);
}

void UEnvArt_ProceduralFoliage::ClearAllFoliage()
{
    for (UInstancedStaticMeshComponent* Component : FoliageComponents)
    {
        if (Component)
        {
            Component->ClearInstances();
        }
    }
    
    BiomeInstanceCounts.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all procedural foliage"));
}

void UEnvArt_ProceduralFoliage::RegenerateFoliage()
{
    ClearAllFoliage();
    
    if (bAutoGenerate)
    {
        // Regenerate for all biomes
        TArray<TPair<EEnvArt_BiomeType, FVector>> BiomeCenters = {
            {EEnvArt_BiomeType::Savanna, FVector(0, 0, 100)},
            {EEnvArt_BiomeType::Swamp, FVector(-50000, -45000, 100)},
            {EEnvArt_BiomeType::Forest, FVector(-45000, 40000, 100)},
            {EEnvArt_BiomeType::Desert, FVector(55000, 0, 100)},
            {EEnvArt_BiomeType::Mountain, FVector(40000, 50000, 100)}
        };
        
        for (const auto& BiomePair : BiomeCenters)
        {
            GenerateFoliageForBiome(BiomePair.Key, BiomePair.Value);
        }
    }
}

void UEnvArt_ProceduralFoliage::UpdateLODLevels()
{
    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (UInstancedStaticMeshComponent* Component : FoliageComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        // Update LOD based on distance to player
        for (int32 i = 0; i < Component->GetInstanceCount(); i++)
        {
            FTransform InstanceTransform;
            if (Component->GetInstanceTransform(i, InstanceTransform, true))
            {
                float Distance = FVector::Dist(PlayerLocation, InstanceTransform.GetLocation());
                
                // Set LOD level based on distance
                if (Distance > CullDistance)
                {
                    // Cull instance
                    Component->UpdateInstanceTransform(i, InstanceTransform, false, false, false);
                }
                else if (Distance > LODDistance2)
                {
                    // LOD 2 - lowest detail
                    Component->UpdateInstanceTransform(i, InstanceTransform, false, false, true);
                }
                else if (Distance > LODDistance1)
                {
                    // LOD 1 - medium detail
                    Component->UpdateInstanceTransform(i, InstanceTransform, false, true, true);
                }
                else
                {
                    // LOD 0 - full detail
                    Component->UpdateInstanceTransform(i, InstanceTransform, true, true, true);
                }
            }
        }
    }
}