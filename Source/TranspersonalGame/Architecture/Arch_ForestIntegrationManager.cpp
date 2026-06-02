#include "Arch_ForestIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Landscape/LandscapeProxy.h"

UArch_ForestIntegrationManager::UArch_ForestIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f; // Update every 2 seconds for performance

    // Initialize default configuration
    ForestIntegrationRadius = 2000.0f;
    MaxVegetationInstancesPerStructure = 50;
    bForestIntegrationActive = false;
    ActiveIntegrationCount = 0;
    LastIntegrationTime = 0.0f;

    // Set default shelter configuration
    DefaultShelterConfig.TargetBiome = EBiomeType::Forest;
    DefaultShelterConfig.MinTreeDensity = 0.3f;
    DefaultShelterConfig.MaxSlopeAngle = 25.0f;
    DefaultShelterConfig.bRequireWaterAccess = true;
    DefaultShelterConfig.WindProtectionLevel = 0.7f;
    DefaultShelterConfig.PreferredMaterials = {TEXT("Wood"), TEXT("Stone"), TEXT("Leaves")};

    // Set default camouflage settings
    DefaultCamouflageSettings.VegetationCoveragePercent = 60.0f;
    DefaultCamouflageSettings.bUseNaturalColors = true;
    DefaultCamouflageSettings.WeatheringLevel = 0.8f;
    DefaultCamouflageSettings.VisibilityFromDistance = 50.0f;
    DefaultCamouflageSettings.NaturalColorPalette = {
        FLinearColor(0.4f, 0.3f, 0.2f, 1.0f), // Brown
        FLinearColor(0.2f, 0.4f, 0.1f, 1.0f), // Green
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)  // Grey
    };
}

void UArch_ForestIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Forest Integration Manager initialized"));
    InitializeForestIntegration();
}

void UArch_ForestIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bForestIntegrationActive)
    {
        UpdateStructureVisibility();
        OptimizePerformance();
    }
}

void UArch_ForestIntegrationManager::InitializeForestIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get world for forest integration"));
        return;
    }

    bForestIntegrationActive = true;
    IntegratedStructures.Empty();
    StructureVisibilityMap.Empty();

    UE_LOG(LogTemp, Log, TEXT("Forest integration system activated"));
}

bool UArch_ForestIntegrationManager::IntegrateStructureWithForest(const FArch_ForestStructureData& StructureData)
{
    if (!bForestIntegrationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Forest integration not active"));
        return false;
    }

    // Validate location
    if (!ValidateForestLocation(StructureData.Location, DefaultShelterConfig))
    {
        UE_LOG(LogTemp, Warning, TEXT("Location not suitable for forest integration: %s"), *StructureData.Location.ToString());
        return false;
    }

    // Add to integrated structures
    FArch_ForestStructureData NewStructure = StructureData;
    NewStructure.ForestIntegrationLevel = FMath::Clamp(StructureData.ForestIntegrationLevel, 0.0f, 1.0f);
    IntegratedStructures.Add(NewStructure);

    // Apply natural camouflage if enabled
    if (StructureData.bNaturalCamouflage)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // Find the structure actor at this location
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (FVector::Dist(Actor->GetActorLocation(), StructureData.Location) < 100.0f)
                {
                    ApplyNaturalCamouflage(Actor, DefaultCamouflageSettings);
                    AttachVegetationToStructure(Actor, StructureData.AttachedVegetation);
                    break;
                }
            }
        }
    }

    ActiveIntegrationCount++;
    UE_LOG(LogTemp, Log, TEXT("Successfully integrated structure: %s"), *StructureData.StructureName);
    return true;
}

void UArch_ForestIntegrationManager::ApplyNaturalCamouflage(AActor* StructureActor, const FArch_CamouflageSettings& Settings)
{
    if (!StructureActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid actor for camouflage application"));
        return;
    }

    // Get all static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    StructureActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            // Apply weathering effects
            ApplyMaterialWeathering(MeshComp, Settings.WeatheringLevel);

            // Apply natural color palette
            if (Settings.bUseNaturalColors && Settings.NaturalColorPalette.Num() > 0)
            {
                UMaterialInterface* CurrentMaterial = MeshComp->GetMaterial(0);
                if (CurrentMaterial)
                {
                    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, MeshComp);
                    if (DynamicMaterial)
                    {
                        // Apply random natural color
                        int32 ColorIndex = FMath::RandRange(0, Settings.NaturalColorPalette.Num() - 1);
                        FLinearColor NaturalColor = Settings.NaturalColorPalette[ColorIndex];
                        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), NaturalColor);
                        MeshComp->SetMaterial(0, DynamicMaterial);
                    }
                }
            }
        }
    }

    // Update visibility map
    StructureVisibilityMap.Add(StructureActor, Settings.VisibilityFromDistance);

    UE_LOG(LogTemp, Log, TEXT("Applied natural camouflage to structure: %s"), *StructureActor->GetName());
}

TArray<FVector> UArch_ForestIntegrationManager::FindOptimalShelterLocations(const FArch_NaturalShelterConfig& Config, int32 MaxLocations)
{
    TArray<FVector> OptimalLocations;
    UWorld* World = GetWorld();
    if (!World)
    {
        return OptimalLocations;
    }

    // Sample locations in a grid pattern
    const float SampleSpacing = 500.0f;
    const int32 GridSize = 20;
    const FVector CenterLocation = GetOwner()->GetActorLocation();

    for (int32 X = -GridSize; X <= GridSize; X++)
    {
        for (int32 Y = -GridSize; Y <= GridSize; Y++)
        {
            FVector SampleLocation = CenterLocation + FVector(X * SampleSpacing, Y * SampleSpacing, 0.0f);
            
            if (ValidateForestLocation(SampleLocation, Config))
            {
                OptimalLocations.Add(SampleLocation);
                
                if (OptimalLocations.Num() >= MaxLocations)
                {
                    return OptimalLocations;
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Found %d optimal shelter locations"), OptimalLocations.Num());
    return OptimalLocations;
}

void UArch_ForestIntegrationManager::AttachVegetationToStructure(AActor* StructureActor, const TArray<FString>& VegetationTypes)
{
    if (!StructureActor || VegetationTypes.Num() == 0)
    {
        return;
    }

    // Get nearby vegetation
    TArray<AActor*> NearbyVegetation = GetNearbyVegetation(StructureActor->GetActorLocation(), 200.0f);
    
    int32 AttachedCount = 0;
    for (AActor* VegActor : NearbyVegetation)
    {
        if (AttachedCount >= MaxVegetationInstancesPerStructure)
        {
            break;
        }

        // Check if this vegetation type is in our desired list
        FString VegName = VegActor->GetName().ToLower();
        for (const FString& DesiredType : VegetationTypes)
        {
            if (VegName.Contains(DesiredType.ToLower()))
            {
                // Attach to structure (conceptually - in a real implementation, you'd use attachment components)
                UE_LOG(LogTemp, Log, TEXT("Attached vegetation %s to structure %s"), *VegActor->GetName(), *StructureActor->GetName());
                AttachedCount++;
                break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Attached %d vegetation instances to structure"), AttachedCount);
}

void UArch_ForestIntegrationManager::CreateVineOvergrowth(AActor* StructureActor, float CoveragePercent)
{
    if (!StructureActor)
    {
        return;
    }

    // In a full implementation, this would create instanced vine meshes
    // For now, we'll log the action and modify material properties
    TArray<UStaticMeshComponent*> MeshComponents;
    StructureActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            UMaterialInterface* CurrentMaterial = MeshComp->GetMaterial(0);
            if (CurrentMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, MeshComp);
                if (DynamicMaterial)
                {
                    // Simulate vine coverage by adjusting material properties
                    float VineIntensity = CoveragePercent / 100.0f;
                    DynamicMaterial->SetScalarParameterValue(TEXT("VineCoverage"), VineIntensity);
                    DynamicMaterial->SetVectorParameterValue(TEXT("VineColor"), FLinearColor(0.1f, 0.4f, 0.1f, 1.0f));
                    MeshComp->SetMaterial(0, DynamicMaterial);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied vine overgrowth (%.1f%%) to structure: %s"), CoveragePercent, *StructureActor->GetName());
}

void UArch_ForestIntegrationManager::PlaceMossAndLichen(AActor* StructureActor, float DensityLevel)
{
    if (!StructureActor)
    {
        return;
    }

    // Apply moss and lichen effects through material modifications
    TArray<UStaticMeshComponent*> MeshComponents;
    StructureActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        ApplyMaterialWeathering(MeshComp, DensityLevel);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied moss and lichen (density %.2f) to structure: %s"), DensityLevel, *StructureActor->GetName());
}

void UArch_ForestIntegrationManager::AdaptToForestLighting(AActor* StructureActor)
{
    if (!StructureActor)
    {
        return;
    }

    // Adjust material properties for forest lighting conditions
    TArray<UStaticMeshComponent*> MeshComponents;
    StructureActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            UMaterialInterface* CurrentMaterial = MeshComp->GetMaterial(0);
            if (CurrentMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, MeshComp);
                if (DynamicMaterial)
                {
                    // Adjust for dappled forest lighting
                    DynamicMaterial->SetScalarParameterValue(TEXT("Brightness"), 0.7f);
                    DynamicMaterial->SetScalarParameterValue(TEXT("Contrast"), 1.2f);
                    DynamicMaterial->SetVectorParameterValue(TEXT("ShadowTint"), FLinearColor(0.3f, 0.4f, 0.2f, 1.0f));
                    MeshComp->SetMaterial(0, DynamicMaterial);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Adapted structure lighting for forest environment: %s"), *StructureActor->GetName());
}

void UArch_ForestIntegrationManager::ApplyWeatheringEffects(AActor* StructureActor, float WeatheringLevel)
{
    if (!StructureActor)
    {
        return;
    }

    TArray<UStaticMeshComponent*> MeshComponents;
    StructureActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        ApplyMaterialWeathering(MeshComp, WeatheringLevel);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied weathering effects (level %.2f) to structure: %s"), WeatheringLevel, *StructureActor->GetName());
}

float UArch_ForestIntegrationManager::CalculateForestDensityAtLocation(const FVector& Location, float SampleRadius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    TArray<AActor*> NearbyVegetation = GetNearbyVegetation(Location, SampleRadius);
    float SampleArea = PI * SampleRadius * SampleRadius;
    float Density = static_cast<float>(NearbyVegetation.Num()) / (SampleArea / 10000.0f); // Per 100m²

    return FMath::Clamp(Density / 50.0f, 0.0f, 1.0f); // Normalize to 0-1 range
}

float UArch_ForestIntegrationManager::CalculateStructureVisibility(AActor* StructureActor, const FVector& ObserverLocation)
{
    if (!StructureActor)
    {
        return 1.0f; // Fully visible if no structure
    }

    float Distance = FVector::Dist(StructureActor->GetActorLocation(), ObserverLocation);
    float ForestDensity = CalculateForestDensityAtLocation(StructureActor->GetActorLocation(), 500.0f);
    
    // Calculate visibility based on distance, forest density, and camouflage
    float BaseVisibility = 1.0f - (ForestDensity * 0.5f);
    float DistanceReduction = FMath::Clamp(Distance / 1000.0f, 0.0f, 0.8f);
    
    float FinalVisibility = BaseVisibility * (1.0f - DistanceReduction);
    
    // Apply camouflage modifier if present
    if (StructureVisibilityMap.Contains(StructureActor))
    {
        float CamouflageReduction = 1.0f - (StructureVisibilityMap[StructureActor] / 100.0f);
        FinalVisibility *= CamouflageReduction;
    }

    return FMath::Clamp(FinalVisibility, 0.0f, 1.0f);
}

void UArch_ForestIntegrationManager::OptimizeForStealth(AActor* StructureActor)
{
    if (!StructureActor)
    {
        return;
    }

    // Apply maximum camouflage settings
    FArch_CamouflageSettings StealthSettings = DefaultCamouflageSettings;
    StealthSettings.VegetationCoveragePercent = 90.0f;
    StealthSettings.WeatheringLevel = 1.0f;
    StealthSettings.VisibilityFromDistance = 20.0f;

    ApplyNaturalCamouflage(StructureActor, StealthSettings);
    CreateVineOvergrowth(StructureActor, 80.0f);
    PlaceMossAndLichen(StructureActor, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Optimized structure for stealth: %s"), *StructureActor->GetName());
}

bool UArch_ForestIntegrationManager::IsStructureHiddenFromPredators(AActor* StructureActor)
{
    if (!StructureActor)
    {
        return false;
    }

    // Check visibility from multiple predator viewpoints
    TArray<FVector> PredatorViewpoints = {
        StructureActor->GetActorLocation() + FVector(500, 0, 100),   // East
        StructureActor->GetActorLocation() + FVector(-500, 0, 100),  // West
        StructureActor->GetActorLocation() + FVector(0, 500, 100),   // North
        StructureActor->GetActorLocation() + FVector(0, -500, 100),  // South
        StructureActor->GetActorLocation() + FVector(0, 0, 300)      // Above
    };

    float TotalVisibility = 0.0f;
    for (const FVector& Viewpoint : PredatorViewpoints)
    {
        TotalVisibility += CalculateStructureVisibility(StructureActor, Viewpoint);
    }

    float AverageVisibility = TotalVisibility / PredatorViewpoints.Num();
    bool bHidden = AverageVisibility < 0.3f; // Hidden if less than 30% visible on average

    UE_LOG(LogTemp, Log, TEXT("Structure %s predator visibility: %.2f (Hidden: %s)"), 
           *StructureActor->GetName(), AverageVisibility, bHidden ? TEXT("Yes") : TEXT("No"));

    return bHidden;
}

bool UArch_ForestIntegrationManager::ValidateForestLocation(const FVector& Location, const FArch_NaturalShelterConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Check forest density
    float ForestDensity = CalculateForestDensityAtLocation(Location, 300.0f);
    if (ForestDensity < Config.MinTreeDensity)
    {
        return false;
    }

    // Check slope angle (simplified - would need landscape height sampling in full implementation)
    // For now, assume valid slope

    // Check water access if required
    if (Config.bRequireWaterAccess)
    {
        // In full implementation, would check for nearby water bodies
        // For now, assume water access is available
    }

    return true;
}

void UArch_ForestIntegrationManager::UpdateStructureVisibility()
{
    // Update visibility calculations for all integrated structures
    for (auto& VisibilityPair : StructureVisibilityMap)
    {
        if (VisibilityPair.Key)
        {
            // Recalculate visibility based on current conditions
            float NewVisibility = CalculateStructureVisibility(VisibilityPair.Key, GetOwner()->GetActorLocation());
            VisibilityPair.Value = NewVisibility;
        }
    }
}

void UArch_ForestIntegrationManager::OptimizePerformance()
{
    // Remove invalid entries from visibility map
    TArray<AActor*> InvalidActors;
    for (auto& VisibilityPair : StructureVisibilityMap)
    {
        if (!IsValid(VisibilityPair.Key))
        {
            InvalidActors.Add(VisibilityPair.Key);
        }
    }

    for (AActor* InvalidActor : InvalidActors)
    {
        StructureVisibilityMap.Remove(InvalidActor);
    }

    // Update performance counters
    ActiveIntegrationCount = IntegratedStructures.Num();
    LastIntegrationTime = GetWorld()->GetTimeSeconds();
}

TArray<AActor*> UArch_ForestIntegrationManager::GetNearbyVegetation(const FVector& Location, float Radius)
{
    TArray<AActor*> NearbyVegetation;
    UWorld* World = GetWorld();
    if (!World)
    {
        return NearbyVegetation;
    }

    // Get all actors and filter for vegetation
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Location) <= Radius)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("bush")) || 
                ActorName.Contains(TEXT("fern")) || ActorName.Contains(TEXT("plant")))
            {
                NearbyVegetation.Add(Actor);
            }
        }
    }

    return NearbyVegetation;
}

void UArch_ForestIntegrationManager::ApplyMaterialWeathering(UStaticMeshComponent* MeshComponent, float WeatheringLevel)
{
    if (!MeshComponent)
    {
        return;
    }

    UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(0);
    if (CurrentMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, MeshComponent);
        if (DynamicMaterial)
        {
            // Apply weathering effects
            DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), WeatheringLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.8f + (WeatheringLevel * 0.2f));
            DynamicMaterial->SetVectorParameterValue(TEXT("WeatherTint"), FLinearColor(0.6f, 0.5f, 0.4f, 1.0f));
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}