#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh component
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);

    // Initialize defaults
    StructureType = EArch_StructureType::Dwelling;
    CurrentWeatheringLevel = 0.3f;
    bInteriorPopulated = false;

    // Set default structure data
    StructureData.StructureName = TEXT("Prehistoric Dwelling");
    StructureData.BiomeType = EBiomeType::Savanna;
    StructureData.WeatheringLevel = 0.3f;
    StructureData.bIsRuin = false;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial weathering and biome-specific materials
    ApplyBiomeSpecificWeathering();
    UpdateStructureAppearance();
    
    if (UE_LOG_ACTIVE(LogTemp, Log))
    {
        UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager: %s initialized at %s"), 
               *StructureData.StructureName, *GetActorLocation().ToString());
    }
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Gradual weathering over time (very slow)
    if (CurrentWeatheringLevel < 1.0f)
    {
        float WeatheringRate = 0.00001f; // Very slow weathering
        CurrentWeatheringLevel = FMath::Min(1.0f, CurrentWeatheringLevel + (WeatheringRate * DeltaTime));
        
        if (FMath::Abs(CurrentWeatheringLevel - StructureData.WeatheringLevel) > 0.1f)
        {
            StructureData.WeatheringLevel = CurrentWeatheringLevel;
            UpdateStructureAppearance();
        }
    }
}

void AArch_StructureManager::InitializeStructure(const FArch_StructureData& InStructureData, EArch_StructureType InType)
{
    StructureData = InStructureData;
    StructureType = InType;
    CurrentWeatheringLevel = StructureData.WeatheringLevel;
    
    // Set actor location and rotation
    SetActorLocation(StructureData.Location);
    SetActorRotation(StructureData.Rotation);
    
    // Update appearance based on new data
    ApplyBiomeSpecificWeathering();
    UpdateStructureAppearance();
    
    // Populate interior if not a ruin
    if (!StructureData.bIsRuin && StructureType != EArch_StructureType::Ruin)
    {
        PopulateInterior();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Structure initialized: %s (Type: %d)"), 
           *StructureData.StructureName, (int32)StructureType);
}

void AArch_StructureManager::ApplyWeathering(float WeatheringAmount)
{
    CurrentWeatheringLevel = FMath::Clamp(CurrentWeatheringLevel + WeatheringAmount, 0.0f, 1.0f);
    StructureData.WeatheringLevel = CurrentWeatheringLevel;
    
    UpdateStructureAppearance();
    
    // Convert to ruin if heavily weathered
    if (CurrentWeatheringLevel > 0.8f && !StructureData.bIsRuin)
    {
        ConvertToRuin(CurrentWeatheringLevel);
    }
}

void AArch_StructureManager::PopulateInterior()
{
    if (bInteriorPopulated)
    {
        ClearInterior();
    }
    
    // Define interior layouts based on structure type
    TArray<FVector> PropLocations;
    TArray<FRotator> PropRotations;
    TArray<int32> PropIndices;
    
    switch (StructureType)
    {
        case EArch_StructureType::Dwelling:
            // Basic dwelling: fire pit, sleeping area, storage
            PropLocations.Add(FVector(0, 0, 10));      // Fire pit center
            PropLocations.Add(FVector(-150, 100, 0));  // Sleeping area
            PropLocations.Add(FVector(150, -100, 0));  // Storage area
            PropRotations.Add(FRotator::ZeroRotator);
            PropRotations.Add(FRotator(0, 45, 0));
            PropRotations.Add(FRotator(0, -30, 0));
            PropIndices = {0, 1, 2}; // Assuming first 3 props are for dwelling
            break;
            
        case EArch_StructureType::Workshop:
            // Workshop: work bench, tool storage, materials
            PropLocations.Add(FVector(0, -50, 20));    // Work bench
            PropLocations.Add(FVector(-100, 0, 0));    // Tool storage
            PropLocations.Add(FVector(100, 50, 0));    // Materials pile
            PropRotations.Add(FRotator::ZeroRotator);
            PropRotations.Add(FRotator(0, 90, 0));
            PropRotations.Add(FRotator(0, -45, 0));
            PropIndices = {0, 1, 2};
            break;
            
        case EArch_StructureType::Storage:
            // Storage: multiple containers
            for (int32 i = 0; i < 4; i++)
            {
                float Angle = i * 90.0f;
                FVector Location = FVector(
                    FMath::Cos(FMath::DegreesToRadians(Angle)) * 80,
                    FMath::Sin(FMath::DegreesToRadians(Angle)) * 80,
                    0
                );
                PropLocations.Add(Location);
                PropRotations.Add(FRotator(0, Angle, 0));
                PropIndices.Add(0); // Storage containers
            }
            break;
            
        default:
            // Minimal props for other types
            PropLocations.Add(FVector::ZeroVector);
            PropRotations.Add(FRotator::ZeroRotator);
            PropIndices.Add(0);
            break;
    }
    
    // Spawn props
    for (int32 i = 0; i < PropLocations.Num() && i < InteriorProps.Num(); i++)
    {
        if (InteriorProps[PropIndices[i % PropIndices.Num()]])
        {
            SpawnInteriorProp(
                InteriorProps[PropIndices[i % PropIndices.Num()]], 
                PropLocations[i], 
                PropRotations[i]
            );
        }
    }
    
    bInteriorPopulated = true;
    UE_LOG(LogTemp, Log, TEXT("Interior populated with %d props"), SpawnedInteriorActors.Num());
}

void AArch_StructureManager::ClearInterior()
{
    for (AActor* Actor : SpawnedInteriorActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedInteriorActors.Empty();
    bInteriorPopulated = false;
}

void AArch_StructureManager::ConvertToRuin(float DestructionLevel)
{
    StructureData.bIsRuin = true;
    StructureType = EArch_StructureType::Ruin;
    
    // Clear interior when converting to ruin
    ClearInterior();
    
    // Apply heavy weathering
    CurrentWeatheringLevel = FMath::Max(0.7f, DestructionLevel);
    StructureData.WeatheringLevel = CurrentWeatheringLevel;
    
    UpdateStructureAppearance();
    
    UE_LOG(LogTemp, Warning, TEXT("Structure %s converted to ruin (Destruction: %.2f)"), 
           *StructureData.StructureName, DestructionLevel);
}

void AArch_StructureManager::SetStructureType(EArch_StructureType NewType)
{
    if (StructureType != NewType)
    {
        StructureType = NewType;
        
        // Re-populate interior for new type
        if (!StructureData.bIsRuin)
        {
            PopulateInterior();
        }
    }
}

void AArch_StructureManager::UpdateMaterialsForBiome(EBiomeType BiomeType)
{
    StructureData.BiomeType = BiomeType;
    ApplyBiomeSpecificWeathering();
    UpdateStructureAppearance();
}

bool AArch_StructureManager::ValidateStructurePlacement() const
{
    // Check if structure is placed on valid ground
    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 1000);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    UWorld* World = GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Check if ground is reasonably flat
        FVector HitNormal = HitResult.Normal;
        float SlopeAngle = FMath::Acos(FVector::DotProduct(HitNormal, FVector::UpVector));
        
        return FMath::RadiansToDegrees(SlopeAngle) < 30.0f; // Max 30 degree slope
    }
    
    return false;
}

void AArch_StructureManager::SpawnInteriorProp(UStaticMesh* PropMesh, const FVector& RelativeLocation, const FRotator& RelativeRotation)
{
    if (!PropMesh || !GetWorld())
    {
        return;
    }
    
    FVector WorldLocation = GetActorLocation() + GetActorTransform().TransformVector(RelativeLocation);
    FRotator WorldRotation = GetActorRotation() + RelativeRotation;
    
    AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        WorldLocation,
        WorldRotation
    );
    
    if (PropActor)
    {
        PropActor->GetStaticMeshComponent()->SetStaticMesh(PropMesh);
        PropActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        SpawnedInteriorActors.Add(PropActor);
    }
}

void AArch_StructureManager::ApplyBiomeSpecificWeathering()
{
    // Different biomes cause different weathering patterns
    switch (StructureData.BiomeType)
    {
        case EBiomeType::Swamp:
            // High humidity causes more moss and decay
            CurrentWeatheringLevel += 0.1f;
            break;
            
        case EBiomeType::Desert:
            // Sand erosion and heat cracking
            CurrentWeatheringLevel += 0.05f;
            break;
            
        case EBiomeType::Mountain:
            // Freeze-thaw cycles cause structural damage
            CurrentWeatheringLevel += 0.08f;
            break;
            
        case EBiomeType::Forest:
            // Root growth and moisture
            CurrentWeatheringLevel += 0.06f;
            break;
            
        case EBiomeType::Savanna:
        default:
            // Moderate weathering
            CurrentWeatheringLevel += 0.03f;
            break;
    }
    
    CurrentWeatheringLevel = FMath::Clamp(CurrentWeatheringLevel, 0.0f, 1.0f);
}

void AArch_StructureManager::UpdateStructureAppearance()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Apply weathered materials based on weathering level
    int32 MaterialIndex = FMath::FloorToInt(CurrentWeatheringLevel * (WeatheredMaterials.Num() - 1));
    MaterialIndex = FMath::Clamp(MaterialIndex, 0, WeatheredMaterials.Num() - 1);
    
    if (WeatheredMaterials.IsValidIndex(MaterialIndex) && WeatheredMaterials[MaterialIndex])
    {
        MainStructureMesh->SetMaterial(0, WeatheredMaterials[MaterialIndex]);
    }
    
    // Adjust scale slightly based on weathering (ruins shrink)
    if (StructureData.bIsRuin)
    {
        float RuinScale = FMath::Lerp(1.0f, 0.7f, CurrentWeatheringLevel);
        SetActorScale3D(FVector(RuinScale));
    }
}