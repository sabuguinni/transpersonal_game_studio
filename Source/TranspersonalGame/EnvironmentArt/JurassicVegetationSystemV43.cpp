#include "JurassicVegetationSystemV43.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "InstancedFoliageActor.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

UJurassicVegetationSystemV43::UJurassicVegetationSystemV43()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance monitoring
    
    // Default settings optimized for Jurassic environments
    GenerationRadius = 1000.0f;
    ChunkSize = 200.0f;
    TargetFPS = 60;
    bEnableLODSystem = true;
    bEnableGPUCulling = true;
    bUsePCGForGeneration = true;
    bUseWorldPartition = true;
    bAutoGenerateOnBeginPlay = true;
}

void UJurassicVegetationSystemV43::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicVegetationSystemV43: Initializing Jurassic vegetation system"));
    
    if (bAutoGenerateOnBeginPlay)
    {
        InitializeVegetationSystem();
    }
}

void UJurassicVegetationSystemV43::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CurrentFrameTime = DeltaTime;
    
    // Performance monitoring
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= 1.0f) // Check every second
    {
        MonitorPerformance();
        LastPerformanceCheck = 0.0f;
    }
    
    // Update LOD system if enabled
    if (bEnableLODSystem)
    {
        UpdateLODSystem();
    }
}

void UJurassicVegetationSystemV43::InitializeVegetationSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("JurassicVegetationSystemV43: Initializing vegetation system"));
    
    // Load vegetation data asset
    if (VegetationDataAsset.IsValid() || VegetationDataAsset.LoadSynchronous())
    {
        UJurassicVegetationDataAsset* DataAsset = VegetationDataAsset.Get();
        if (DataAsset)
        {
            // Cache biome configurations for performance
            for (const FBiomeVegetationConfig& BiomeConfig : DataAsset->BiomeConfigurations)
            {
                CachedBiomeConfigs.Add(BiomeConfig.BiomeType, BiomeConfig);
                UE_LOG(LogTemp, Log, TEXT("Cached biome configuration: %s"), 
                       *UEnum::GetValueAsString(BiomeConfig.BiomeType));
            }
            
            // Cache vegetation specifications
            for (const FJurassicVegetationSpec& VegSpec : DataAsset->VegetationDatabase)
            {
                CachedVegetationSpecs.Add(VegSpec.VegetationType, VegSpec);
                UE_LOG(LogTemp, Log, TEXT("Cached vegetation spec: %s"), 
                       *UEnum::GetValueAsString(VegSpec.VegetationType));
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Loaded %d biome configurations and %d vegetation types"), 
                   CachedBiomeConfigs.Num(), CachedVegetationSpecs.Num());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicVegetationSystemV43: Failed to load vegetation data asset"));
        return;
    }
    
    // Setup PCG integration
    if (bUsePCGForGeneration)
    {
        SetupPCGIntegration();
    }
    
    // Configure foliage types
    ConfigureFoliageTypes();
    
    UE_LOG(LogTemp, Warning, TEXT("JurassicVegetationSystemV43: Initialization complete"));
}

void UJurassicVegetationSystemV43::SetupPCGIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicVegetationSystemV43: Setting up PCG integration"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for PCG setup"));
        return;
    }
    
    // Get or create PCG subsystem
    UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(World);
    if (!PCGSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("PCG Subsystem not available"));
        return;
    }
    
    // Setup PCG component if not already set
    if (!PCGComponent.IsValid())
    {
        // Try to find existing PCG component on owner
        if (AActor* Owner = GetOwner())
        {
            UPCGComponent* ExistingPCG = Owner->FindComponentByClass<UPCGComponent>();
            if (ExistingPCG)
            {
                PCGComponent = ExistingPCG;
                UE_LOG(LogTemp, Log, TEXT("Found existing PCG component"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No PCG component found - vegetation generation will use fallback method"));
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PCG integration setup complete"));
}

void UJurassicVegetationSystemV43::ConfigureFoliageTypes()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicVegetationSystemV43: Configuring foliage types"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get the instanced foliage actor
    AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World);
    if (!IFA)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Instanced Foliage Actor found - creating one"));
        IFA = World->SpawnActor<AInstancedFoliageActor>();
    }
    
    if (!IFA)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create Instanced Foliage Actor"));
        return;
    }
    
    // Configure foliage types for each vegetation specification
    for (const auto& VegSpecPair : CachedVegetationSpecs)
    {
        const FJurassicVegetationSpec& VegSpec = VegSpecPair.Value;
        
        if (VegSpec.VegetationMesh.IsValid() || VegSpec.VegetationMesh.LoadSynchronous())
        {
            UStaticMesh* Mesh = VegSpec.VegetationMesh.Get();
            if (Mesh)
            {
                // Create foliage type
                UFoliageType_InstancedStaticMesh* FoliageType = NewObject<UFoliageType_InstancedStaticMesh>();
                FoliageType->SetStaticMesh(Mesh);
                
                // Configure foliage settings based on vegetation spec
                FoliageType->Density = VegSpec.DensityMultiplier * 100.0f; // Convert to foliage density scale
                FoliageType->Radius = ChunkSize * 0.1f; // 10% of chunk size for spacing
                FoliageType->ScaleX = FFloatInterval(VegSpec.MinScale, VegSpec.MaxScale);
                FoliageType->ScaleY = FFloatInterval(VegSpec.MinScale, VegSpec.MaxScale);
                FoliageType->ScaleZ = FFloatInterval(VegSpec.MinScale, VegSpec.MaxScale);
                
                // Placement settings
                FoliageType->GroundSlopeAngle = FFloatInterval(0.0f, VegSpec.MaxSlope);
                FoliageType->Height = FFloatInterval(VegSpec.MinAltitude, VegSpec.MaxAltitude);
                
                // Performance settings
                FoliageType->CullDistance = FInt32Interval(static_cast<int32>(VegSpec.CullDistance * 0.8f), 
                                                          static_cast<int32>(VegSpec.CullDistance));
                FoliageType->bEnableStaticLighting = false; // Use dynamic lighting for better performance
                FoliageType->CastShadow = true;
                FoliageType->bAffectDistanceFieldLighting = VegSpec.bEnableNanite;
                
                // Nanite settings
                if (VegSpec.bEnableNanite && Mesh->GetRenderData() && Mesh->GetRenderData()->IsInitialized())
                {
                    // Nanite meshes handle LOD automatically
                    FoliageType->bEnableDensityScaling = true;
                }
                
                UE_LOG(LogTemp, Log, TEXT("Configured foliage type for: %s"), 
                       *UEnum::GetValueAsString(VegSpec.VegetationType));
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Foliage type configuration complete"));
}

void UJurassicVegetationSystemV43::GenerateVegetationInArea(FVector Center, float Radius, EJurassicBiomeType BiomeType)
{
    UE_LOG(LogTemp, Log, TEXT("Generating vegetation in area: Center(%s), Radius(%.2f), Biome(%s)"), 
           *Center.ToString(), Radius, *UEnum::GetValueAsString(BiomeType));
    
    if (!CachedBiomeConfigs.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Error, TEXT("No configuration found for biome type: %s"), 
               *UEnum::GetValueAsString(BiomeType));
        return;
    }
    
    const FBiomeVegetationConfig& BiomeConfig = CachedBiomeConfigs[BiomeType];
    
    // Calculate number of chunks to generate
    int32 ChunksPerSide = FMath::CeilToInt((Radius * 2.0f) / ChunkSize);
    FVector StartCorner = Center - FVector(Radius, Radius, 0.0f);
    
    for (int32 X = 0; X < ChunksPerSide; X++)
    {
        for (int32 Y = 0; Y < ChunksPerSide; Y++)
        {
            FVector ChunkCenter = StartCorner + FVector(X * ChunkSize + ChunkSize * 0.5f, 
                                                       Y * ChunkSize + ChunkSize * 0.5f, 0.0f);
            
            // Check if chunk is within generation radius
            float DistanceToCenter = FVector::Dist2D(ChunkCenter, Center);
            if (DistanceToCenter <= Radius)
            {
                GenerateVegetationChunk(ChunkCenter, BiomeType);
            }
        }
    }
    
    // Update active biomes list
    ActiveBiomes.AddUnique(BiomeType);
    
    UE_LOG(LogTemp, Log, TEXT("Vegetation generation complete for biome: %s"), 
           *UEnum::GetValueAsString(BiomeType));
}

void UJurassicVegetationSystemV43::GenerateVegetationChunk(FVector ChunkCenter, EJurassicBiomeType BiomeType)
{
    if (!CachedBiomeConfigs.Contains(BiomeType))
    {
        return;
    }
    
    const FBiomeVegetationConfig& BiomeConfig = CachedBiomeConfigs[BiomeType];
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get landscape height at chunk center
    FVector GroundLocation = ChunkCenter;
    FHitResult HitResult;
    FVector TraceStart = ChunkCenter + FVector(0, 0, 1000.0f);
    FVector TraceEnd = ChunkCenter - FVector(0, 0, 1000.0f);
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        GroundLocation = HitResult.Location;
    }
    
    // Generate vegetation based on biome configuration
    for (const FJurassicVegetationSpec& VegSpec : BiomeConfig.VegetationTypes)
    {
        int32 InstanceCount = FMath::RoundToInt(VegSpec.MaxInstancesPerChunk * 
                                              BiomeConfig.OverallDensity * 
                                              VegSpec.DensityMultiplier);
        
        for (int32 i = 0; i < InstanceCount; i++)
        {
            // Generate random position within chunk
            FVector RandomOffset = FVector(
                FMath::RandRange(-ChunkSize * 0.5f, ChunkSize * 0.5f),
                FMath::RandRange(-ChunkSize * 0.5f, ChunkSize * 0.5f),
                0.0f
            );
            
            FVector ProposedLocation = GroundLocation + RandomOffset;
            
            // Validate placement location
            if (ValidatePlacementLocation(ProposedLocation, VegSpec))
            {
                // Calculate transform
                FTransform VegetationTransform = CalculateVegetationTransform(ProposedLocation, VegSpec);
                
                // Place vegetation (this would integrate with actual foliage placement system)
                CurrentVegetationCount++;
                
                // Debug visualization in development
                if (GEngine && GEngine->GetNetMode(World) != NM_DedicatedServer)
                {
                    DrawDebugSphere(World, VegetationTransform.GetLocation(), 
                                   VegSpec.MinScale * 50.0f, 8, FColor::Green, false, 10.0f);
                }
            }
        }
    }
}

bool UJurassicVegetationSystemV43::ValidatePlacementLocation(FVector Location, const FJurassicVegetationSpec& VegSpec) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check altitude constraints
    if (Location.Z < VegSpec.MinAltitude || Location.Z > VegSpec.MaxAltitude)
    {
        return false;
    }
    
    // Check slope constraints using landscape normal
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 100.0f);
    FVector TraceEnd = Location - FVector(0, 0, 100.0f);
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
        if (SlopeAngle > VegSpec.MaxSlope)
        {
            return false;
        }
    }
    
    // Additional validation could include:
    // - Water proximity checks
    // - Existing vegetation density
    // - Player proximity
    // - Performance constraints
    
    return true;
}

FTransform UJurassicVegetationSystemV43::CalculateVegetationTransform(FVector Location, const FJurassicVegetationSpec& VegSpec) const
{
    // Random scale within spec range
    float RandomScale = FMath::RandRange(VegSpec.MinScale, VegSpec.MaxScale);
    
    // Random rotation (vegetation typically only rotates around Z-axis)
    FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    
    // Slight random height variation for natural look
    FVector FinalLocation = Location + FVector(0, 0, FMath::RandRange(-10.0f, 10.0f));
    
    return FTransform(RandomRotation, FinalLocation, FVector(RandomScale));
}

void UJurassicVegetationSystemV43::UpdateLODSystem()
{
    // LOD system updates would be handled here
    // This includes distance-based culling, quality adjustments, etc.
    
    if (bEnableGPUCulling)
    {
        // GPU-based culling optimizations
        ProcessPerformanceOptimization();
    }
}

void UJurassicVegetationSystemV43::MonitorPerformance()
{
    float CurrentFPS = 1.0f / CurrentFrameTime;
    
    if (CurrentFPS < TargetFPS * 0.8f) // 80% of target FPS
    {
        if (!bPerformanceOptimizationActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance below target (%.1f FPS), activating optimizations"), CurrentFPS);
            bPerformanceOptimizationActive = true;
            AdjustQualitySettings();
        }
    }
    else if (CurrentFPS > TargetFPS * 0.95f && bPerformanceOptimizationActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance recovered (%.1f FPS), maintaining optimizations"), CurrentFPS);
        bPerformanceOptimizationActive = false;
    }
}

void UJurassicVegetationSystemV43::ProcessPerformanceOptimization()
{
    // Implement GPU-based culling and performance optimizations
    // This would integrate with UE5's GPU Scene and Nanite systems
}

void UJurassicVegetationSystemV43::AdjustQualitySettings()
{
    if (bPerformanceOptimizationActive)
    {
        // Reduce vegetation density
        // Increase culling distances
        // Disable some visual effects
        UE_LOG(LogTemp, Log, TEXT("Adjusting quality settings for performance"));
    }
}

void UJurassicVegetationSystemV43::ClearVegetationInArea(FVector Center, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("Clearing vegetation in area: Center(%s), Radius(%.2f)"), 
           *Center.ToString(), Radius);
    
    // Implementation would clear foliage instances in the specified area
    // This integrates with UE5's foliage system
}

void UJurassicVegetationSystemV43::RegenerateAllVegetation()
{
    UE_LOG(LogTemp, Warning, TEXT("Regenerating all vegetation"));
    
    CurrentVegetationCount = 0;
    ActiveBiomes.Empty();
    
    // Regenerate vegetation for all active areas
    // This would typically be called when biome configurations change
}

void UJurassicVegetationSystemV43::SetBiomeType(EJurassicBiomeType NewBiomeType)
{
    UE_LOG(LogTemp, Log, TEXT("Setting biome type to: %s"), *UEnum::GetValueAsString(NewBiomeType));
    
    // Update current biome and regenerate vegetation accordingly
    ActiveBiomes.Empty();
    ActiveBiomes.Add(NewBiomeType);
}

void UJurassicVegetationSystemV43::UpdatePerformanceSettings(int32 NewTargetFPS, bool bNewEnableGPUCulling)
{
    TargetFPS = NewTargetFPS;
    bEnableGPUCulling = bNewEnableGPUCulling;
    
    UE_LOG(LogTemp, Log, TEXT("Updated performance settings: Target FPS(%d), GPU Culling(%s)"), 
           TargetFPS, bEnableGPUCulling ? TEXT("Enabled") : TEXT("Disabled"));
}

FVector UJurassicVegetationSystemV43::GetOptimalSpawnLocation(FVector DesiredLocation, EJurassicVegetationType VegetationType) const
{
    if (!CachedVegetationSpecs.Contains(VegetationType))
    {
        return DesiredLocation;
    }
    
    const FJurassicVegetationSpec& VegSpec = CachedVegetationSpecs[VegetationType];
    
    // Find optimal location based on vegetation preferences
    // This could include slope analysis, water proximity, sunlight, etc.
    
    return DesiredLocation;
}

bool UJurassicVegetationSystemV43::CanPlaceVegetationAt(FVector Location, EJurassicVegetationType VegetationType) const
{
    if (!CachedVegetationSpecs.Contains(VegetationType))
    {
        return false;
    }
    
    const FJurassicVegetationSpec& VegSpec = CachedVegetationSpecs[VegetationType];
    return ValidatePlacementLocation(Location, VegSpec);
}

void UJurassicVegetationSystemV43::AddStorytellingElement(FVector Location, const FString& StoryElement)
{
    UE_LOG(LogTemp, Log, TEXT("Adding storytelling element at %s: %s"), *Location.ToString(), *StoryElement);
    
    // Implementation would place narrative environmental elements
    // Such as fossilized remains, ancient damage, etc.
}

void UJurassicVegetationSystemV43::CreateFossilizedVegetation(FVector Location, EJurassicVegetationType OriginalType)
{
    UE_LOG(LogTemp, Log, TEXT("Creating fossilized vegetation at %s, original type: %s"), 
           *Location.ToString(), *UEnum::GetValueAsString(OriginalType));
    
    // Create fossilized version of vegetation for environmental storytelling
}

void UJurassicVegetationSystemV43::AddEnvironmentalDamage(FVector Location, float Radius, const FString& DamageType)
{
    UE_LOG(LogTemp, Log, TEXT("Adding environmental damage at %s, radius %.2f, type: %s"), 
           *Location.ToString(), Radius, *DamageType);
    
    // Add environmental damage effects like fallen trees, burnt areas, etc.
}

// Data Asset Implementation
FJurassicVegetationSpec UJurassicVegetationDataAsset::GetVegetationSpec(EJurassicVegetationType VegetationType) const
{
    for (const FJurassicVegetationSpec& Spec : VegetationDatabase)
    {
        if (Spec.VegetationType == VegetationType)
        {
            return Spec;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Vegetation spec not found for type: %s"), 
           *UEnum::GetValueAsString(VegetationType));
    return FJurassicVegetationSpec();
}

FBiomeVegetationConfig UJurassicVegetationDataAsset::GetBiomeConfig(EJurassicBiomeType BiomeType) const
{
    for (const FBiomeVegetationConfig& Config : BiomeConfigurations)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome configuration not found for type: %s"), 
           *UEnum::GetValueAsString(BiomeType));
    return FBiomeVegetationConfig();
}

TArray<FJurassicVegetationSpec> UJurassicVegetationDataAsset::GetVegetationForBiome(EJurassicBiomeType BiomeType) const
{
    FBiomeVegetationConfig BiomeConfig = GetBiomeConfig(BiomeType);
    return BiomeConfig.VegetationTypes;
}