#include "ArchitecturalManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "DrawDebugHelpers.h"

AArchitecturalManager::AArchitecturalManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create PCG component for procedural building placement
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    RootComponent = PCGComponent;

    // Set default values
    MinDistanceBetweenBuildings = 1000.0f; // 10 meters minimum
    MaxBuildingsPerSquareKm = 5.0f; // Sparse prehistoric settlements

    // Initialize building specifications
    BuildingSpecs.Empty();
}

void AArchitecturalManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Initializing prehistoric building system"));
    
    InitializeBuildingAssets();
    SetupPCGGeneration();
    
    // Generate initial buildings if in valid world
    if (GetWorld())
    {
        FVector WorldCenter = GetActorLocation();
        GenerateBuildingsInArea(WorldCenter, 5000.0f); // 50m radius initial generation
    }
}

void AArchitecturalManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update building deterioration over time
    static float AccumulatedTime = 0.0f;
    AccumulatedTime += DeltaTime;
    
    if (AccumulatedTime >= 60.0f) // Update every minute
    {
        AccumulatedTime = 0.0f;
        
        // Find all buildings and apply gradual deterioration
        TArray<AActor*> FoundBuildings;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("PrehistoricBuilding"), FoundBuildings);
        
        for (AActor* Building : FoundBuildings)
        {
            if (Building && Building->Tags.Contains(FName("Deteriorating")))
            {
                // Apply very slow deterioration over game time
                ApplyDeteriorationToBuilding(Building, 0.001f);
            }
        }
    }
}

void AArchitecturalManager::GenerateBuildingsInArea(const FVector& Center, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Generating buildings in area - Center: %s, Radius: %.2f"), 
           *Center.ToString(), Radius);

    // Find suitable locations for buildings
    TArray<FVector> SuitableLocations = FindSuitableBuildingLocations(Center, Radius, 
                                                                     FMath::RoundToInt(MaxBuildingsPerSquareKm * (Radius * Radius) / 1000000.0f));

    for (const FVector& Location : SuitableLocations)
    {
        // Generate random building specification
        FArchitecturalSpec Spec = GenerateRandomBuildingSpec();
        
        // Create random rotation for natural placement
        FRotator Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        // Create the building
        AActor* NewBuilding = CreateBuilding(Spec, Location, Rotation);
        
        if (NewBuilding)
        {
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Created building at %s"), *Location.ToString());
        }
    }
}

AActor* AArchitecturalManager::CreateBuilding(const FArchitecturalSpec& Spec, const FVector& Location, const FRotator& Rotation)
{
    if (!GetWorld())
    {
        return nullptr;
    }

    // Spawn building actor
    AStaticMeshActor* BuildingActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (!BuildingActor)
    {
        return nullptr;
    }

    // Get appropriate mesh for the architectural style
    UStaticMesh* BuildingMesh = StyleMeshes.FindRef(Spec.Style);
    if (BuildingMesh)
    {
        BuildingActor->GetStaticMeshComponent()->SetStaticMesh(BuildingMesh);
    }

    // Apply purpose-specific material
    UMaterialInterface* PurposeMaterial = PurposeMaterials.FindRef(Spec.Purpose);
    if (PurposeMaterial)
    {
        BuildingActor->GetStaticMeshComponent()->SetMaterial(0, PurposeMaterial);
    }

    // Scale building to specification
    FVector Scale = Spec.Dimensions / 100.0f; // Convert to UE5 scale
    BuildingActor->SetActorScale3D(Scale);

    // Add building tags for identification
    BuildingActor->Tags.Add(FName("PrehistoricBuilding"));
    BuildingActor->Tags.Add(FName("Deteriorating"));
    
    // Add style and purpose tags
    FString StyleTag = FString::Printf(TEXT("Style_%s"), *UEnum::GetValueAsString(Spec.Style));
    FString PurposeTag = FString::Printf(TEXT("Purpose_%s"), *UEnum::GetValueAsString(Spec.Purpose));
    BuildingActor->Tags.Add(FName(*StyleTag));
    BuildingActor->Tags.Add(FName(*PurposeTag));

    // Apply deterioration
    ApplyDeteriorationToBuilding(BuildingActor, Spec.DeteriorationLevel);

    // Populate interior if specified
    if (Spec.bHasInterior)
    {
        PopulateInterior(BuildingActor, Spec);
    }

    // Create environmental storytelling
    CreateEnvironmentalStorytelling(BuildingActor, Spec);

    return BuildingActor;
}

void AArchitecturalManager::ApplyDeteriorationToBuilding(AActor* Building, float DeteriorationLevel)
{
    if (!Building)
    {
        return;
    }

    AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Building);
    if (!MeshActor)
    {
        return;
    }

    UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
    if (!MeshComponent)
    {
        return;
    }

    // Apply weathering effects based on deterioration level
    ApplyWeatheringEffects(Building, 100.0f, DeteriorationLevel);

    // Adjust material properties to show aging
    UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        // Darken and desaturate based on deterioration
        float ColorMultiplier = 1.0f - (DeteriorationLevel * 0.5f);
        DynamicMaterial->SetScalarParameterValue(FName("ColorMultiplier"), ColorMultiplier);
        DynamicMaterial->SetScalarParameterValue(FName("Roughness"), 0.8f + (DeteriorationLevel * 0.2f));
        DynamicMaterial->SetScalarParameterValue(FName("Weathering"), DeteriorationLevel);
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Applied deterioration level %.2f to building"), DeteriorationLevel);
}

void AArchitecturalManager::PopulateInterior(AActor* Building, const FArchitecturalSpec& Spec)
{
    if (!Building || !Spec.bHasInterior)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Populating interior for %s building"), 
           *UEnum::GetValueAsString(Spec.Purpose));

    // Add interior details based on story elements
    AddInteriorDetails(Building, Spec.InteriorStoryElements);

    // Create purpose-specific interior layout
    switch (Spec.Purpose)
    {
        case EBuildingPurpose::Shelter:
            // Add sleeping areas, fire pit, storage
            break;
        case EBuildingPurpose::Crafting:
            // Add work surfaces, tool storage, material piles
            break;
        case EBuildingPurpose::Ritual:
            // Add altar, sacred symbols, offering areas
            break;
        case EBuildingPurpose::Storage:
            // Add containers, preserved goods, organization systems
            break;
        default:
            break;
    }
}

TArray<FVector> AArchitecturalManager::FindSuitableBuildingLocations(const FVector& Center, float Radius, int32 MaxBuildings)
{
    TArray<FVector> SuitableLocations;
    
    int32 AttemptCount = 0;
    const int32 MaxAttempts = MaxBuildings * 10; // Try 10x more locations than needed

    while (SuitableLocations.Num() < MaxBuildings && AttemptCount < MaxAttempts)
    {
        AttemptCount++;

        // Generate random location within radius
        FVector2D RandomPoint = FMath::RandPointInCircle(Radius);
        FVector TestLocation = Center + FVector(RandomPoint.X, RandomPoint.Y, 0.0f);

        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = TestLocation + FVector(0.0f, 0.0f, 1000.0f);
        FVector TraceEnd = TestLocation - FVector(0.0f, 0.0f, 1000.0f);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            FVector GroundLocation = HitResult.Location;

            // Check if location is suitable
            if (IsLocationSuitableForBuilding(GroundLocation, EArchitecturalStyle::PrimitiveHut))
            {
                // Check minimum distance from other buildings
                bool bTooClose = false;
                for (const FVector& ExistingLocation : SuitableLocations)
                {
                    if (FVector::Dist(GroundLocation, ExistingLocation) < MinDistanceBetweenBuildings)
                    {
                        bTooClose = true;
                        break;
                    }
                }

                if (!bTooClose)
                {
                    SuitableLocations.Add(GroundLocation);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Found %d suitable building locations out of %d attempts"), 
           SuitableLocations.Num(), AttemptCount);

    return SuitableLocations;
}

bool AArchitecturalManager::IsLocationSuitableForBuilding(const FVector& Location, EArchitecturalStyle Style)
{
    if (!GetWorld())
    {
        return false;
    }

    // Check slope - buildings need relatively flat ground
    FVector UpVector = FVector::UpVector;
    FHitResult HitResult;
    
    // Trace down to get surface normal
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Location + FVector(0, 0, 100), Location - FVector(0, 0, 100), ECC_WorldStatic))
    {
        float SlopeDot = FVector::DotProduct(HitResult.Normal, UpVector);
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(SlopeDot));

        // Different building styles have different slope tolerances
        float MaxSlope = 15.0f; // Default max slope in degrees
        
        switch (Style)
        {
            case EArchitecturalStyle::PrehistoricCave:
                MaxSlope = 45.0f; // Caves can be on steep terrain
                break;
            case EArchitecturalStyle::StoneCircle:
                MaxSlope = 5.0f; // Stone circles need flat ground
                break;
            default:
                MaxSlope = 15.0f;
                break;
        }

        if (SlopeAngle > MaxSlope)
        {
            return false;
        }
    }

    // Check for water - buildings shouldn't be underwater
    // This would need integration with water system when available

    // Check for existing buildings nearby
    TArray<AActor*> NearbyBuildings;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("PrehistoricBuilding"), NearbyBuildings);
    
    for (AActor* Building : NearbyBuildings)
    {
        if (Building && FVector::Dist(Building->GetActorLocation(), Location) < MinDistanceBetweenBuildings)
        {
            return false;
        }
    }

    return true;
}

void AArchitecturalManager::CreateEnvironmentalStorytelling(AActor* Building, const FArchitecturalSpec& Spec)
{
    if (!Building || !GetWorld())
    {
        return;
    }

    FVector BuildingLocation = Building->GetActorLocation();
    
    // Create story elements around the building based on its purpose and age
    switch (Spec.Purpose)
    {
        case EBuildingPurpose::Crafting:
            // Scatter tool fragments, material scraps around the building
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Adding crafting story elements around building"));
            break;
            
        case EBuildingPurpose::Ritual:
            // Add stone arrangements, burnt areas, offering remnants
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Adding ritual story elements around building"));
            break;
            
        case EBuildingPurpose::Shelter:
            // Add refuse piles, worn paths, storage areas
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Adding shelter story elements around building"));
            break;
            
        default:
            break;
    }

    // Add age-appropriate weathering story
    if (Spec.bIsAbandoned)
    {
        // Add overgrowth, collapsed sections, scattered belongings
        UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Adding abandonment story elements"));
    }
}

void AArchitecturalManager::InitializeBuildingAssets()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Initializing building asset mappings"));
    
    // Initialize style mesh mappings
    // These would be loaded from actual asset paths in a real implementation
    StyleMeshes.Empty();
    PurposeMaterials.Empty();
    
    // Note: In a real implementation, these would load actual UE5 assets
    // StyleMeshes.Add(EArchitecturalStyle::PrimitiveHut, LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Architecture/Meshes/SM_PrimitiveHut")));
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Building assets initialized"));
}

void AArchitecturalManager::SetupPCGGeneration()
{
    if (!PCGComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalManager: Setting up PCG generation for buildings"));
    
    // Configure PCG component for building placement
    // This would be connected to actual PCG graphs in a real implementation
}

FArchitecturalSpec AArchitecturalManager::GenerateRandomBuildingSpec()
{
    FArchitecturalSpec Spec;
    
    // Random style selection
    int32 StyleIndex = FMath::RandRange(0, (int32)EArchitecturalStyle::BurialMound);
    Spec.Style = (EArchitecturalStyle)StyleIndex;
    
    // Random purpose selection
    int32 PurposeIndex = FMath::RandRange(0, (int32)EBuildingPurpose::Gathering);
    Spec.Purpose = (EBuildingPurpose)PurposeIndex;
    
    // Random dimensions within reasonable prehistoric ranges
    Spec.Dimensions = FVector(
        FMath::RandRange(300.0f, 800.0f),  // Width
        FMath::RandRange(300.0f, 800.0f),  // Depth
        FMath::RandRange(200.0f, 500.0f)   // Height
    );
    
    // Random deterioration level
    Spec.DeteriorationLevel = FMath::RandRange(0.1f, 0.8f);
    
    // Random age
    Spec.AgeInYears = FMath::RandRange(10, 500);
    
    // Most buildings have interiors
    Spec.bHasInterior = FMath::RandBool();
    
    // Some buildings are abandoned
    Spec.bIsAbandoned = FMath::RandRange(0.0f, 1.0f) < 0.3f; // 30% chance
    
    // Generate story elements
    Spec.InteriorStoryElements.Empty();
    Spec.InteriorStoryElements.Add(TEXT("Worn sleeping area with animal hide remnants"));
    Spec.InteriorStoryElements.Add(TEXT("Cold fire pit with charred bones"));
    Spec.InteriorStoryElements.Add(TEXT("Stone tools scattered near entrance"));
    
    return Spec;
}

void AArchitecturalManager::ApplyWeatheringEffects(AActor* Building, float Age, float Deterioration)
{
    if (!Building)
    {
        return;
    }

    // Apply visual weathering effects based on age and deterioration
    // This would modify materials, add debris, create wear patterns
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Applied weathering - Age: %.1f years, Deterioration: %.2f"), 
           Age, Deterioration);
}

void AArchitecturalManager::AddInteriorDetails(AActor* Building, const TArray<FString>& StoryElements)
{
    if (!Building)
    {
        return;
    }

    // Add interior props and details based on story elements
    for (const FString& Element : StoryElements)
    {
        UE_LOG(LogTemp, Log, TEXT("ArchitecturalManager: Adding interior element: %s"), *Element);
        // In a real implementation, this would spawn appropriate props and details
    }
}