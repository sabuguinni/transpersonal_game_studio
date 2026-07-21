#include "Arch_StructuralManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

UArch_StructuralManager::UArch_StructuralManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    WeatheringRate = 0.1f;
    bEnableWeathering = true;
    
    InitializeDefaultStructures();
}

void UArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture Structural Manager initialized"));
    
    // Auto-populate biomes with structures
    PopulateBiomeWithStructures(EBiomeType::Montanha, 15);
    PopulateBiomeWithStructures(EBiomeType::Savana, 10);
    PopulateBiomeWithStructures(EBiomeType::Floresta, 8);
}

void UArch_StructuralManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableWeathering)
    {
        ApplyWeathering(DeltaTime);
    }
}

void UArch_StructuralManager::SpawnStructuralElement(const FArch_StructuralElement& Element)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No world context for spawning structural element"));
        return;
    }

    // Spawn a static mesh actor as structural element
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*Element.ElementName);
    
    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Element.Location,
        Element.Rotation,
        SpawnParams
    );

    if (StructureActor)
    {
        StructureActor->SetActorScale3D(Element.Scale);
        StructureActor->SetActorLabel(Element.ElementName);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned structural element: %s at %s"), 
               *Element.ElementName, *Element.Location.ToString());
        
        // Add to our tracking array
        StructuralElements.Add(Element);
    }
}

void UArch_StructuralManager::CreateInteriorSpace(const FArch_InteriorSpace& Space)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create interior elements
    if (Space.bHasFirePit)
    {
        FArch_StructuralElement FirePit;
        FirePit.ElementName = FString::Printf(TEXT("%s_FirePit"), *Space.SpaceName);
        FirePit.Location = Space.CenterLocation;
        FirePit.Scale = FVector(0.5f, 0.5f, 0.2f);
        SpawnStructuralElement(FirePit);
    }

    // Add bedding areas around the space
    for (int32 i = 0; i < 3; i++)
    {
        float Angle = (i * 120.0f) * PI / 180.0f;
        FVector BeddingLocation = Space.CenterLocation + FVector(
            FMath::Cos(Angle) * Space.SpaceRadius * 0.7f,
            FMath::Sin(Angle) * Space.SpaceRadius * 0.7f,
            0.0f
        );

        FArch_StructuralElement Bedding;
        Bedding.ElementName = FString::Printf(TEXT("%s_Bedding_%d"), *Space.SpaceName, i + 1);
        Bedding.Location = BeddingLocation;
        Bedding.Scale = FVector(1.5f, 1.0f, 0.1f);
        SpawnStructuralElement(Bedding);
    }

    InteriorSpaces.Add(Space);
    UE_LOG(LogTemp, Warning, TEXT("Created interior space: %s"), *Space.SpaceName);
}

void UArch_StructuralManager::ApplyWeathering(float DeltaTime)
{
    for (FArch_StructuralElement& Element : StructuralElements)
    {
        if (!Element.bIsWeathered && Element.StructuralIntegrity > 0.0f)
        {
            Element.StructuralIntegrity -= WeatheringRate * DeltaTime;
            
            if (Element.StructuralIntegrity <= 50.0f)
            {
                Element.bIsWeathered = true;
            }
        }
    }
}

TArray<FArch_StructuralElement> UArch_StructuralManager::GetStructuresInBiome(EBiomeType BiomeType)
{
    TArray<FArch_StructuralElement> BiomeStructures;
    
    for (const FArch_StructuralElement& Element : StructuralElements)
    {
        if (Element.BiomeType == BiomeType)
        {
            BiomeStructures.Add(Element);
        }
    }
    
    return BiomeStructures;
}

void UArch_StructuralManager::PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount)
{
    FVector BiomeBase = GetBiomeBaseLocation(BiomeType);
    
    for (int32 i = 0; i < StructureCount; i++)
    {
        FArch_StructuralElement NewStructure;
        NewStructure.ElementName = FString::Printf(TEXT("Structure_%s_%03d"), 
                                                   *UEnum::GetValueAsString(BiomeType), i + 1);
        
        // Random position within biome area (5km radius)
        float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        float RandomDistance = FMath::RandRange(1000.0f, 5000.0f);
        
        NewStructure.Location = BiomeBase + FVector(
            FMath::Cos(RandomAngle) * RandomDistance,
            FMath::Sin(RandomAngle) * RandomDistance,
            FMath::RandRange(50.0f, 200.0f)
        );
        
        NewStructure.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        NewStructure.Scale = FVector(
            FMath::RandRange(0.8f, 2.0f),
            FMath::RandRange(0.8f, 2.0f),
            FMath::RandRange(1.0f, 3.0f)
        );
        NewStructure.BiomeType = BiomeType;
        NewStructure.StructuralIntegrity = FMath::RandRange(60.0f, 100.0f);
        NewStructure.bIsWeathered = FMath::RandBool();
        
        SpawnStructuralElement(NewStructure);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Populated %s biome with %d structures"), 
           *UEnum::GetValueAsString(BiomeType), StructureCount);
}

void UArch_StructuralManager::GenerateArchitecturalElements()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating architectural elements..."));
    
    // Clear existing structures
    StructuralElements.Empty();
    InteriorSpaces.Empty();
    
    // Generate structures for each biome
    PopulateBiomeWithStructures(EBiomeType::Montanha, 20);
    PopulateBiomeWithStructures(EBiomeType::Savana, 15);
    PopulateBiomeWithStructures(EBiomeType::Floresta, 12);
    PopulateBiomeWithStructures(EBiomeType::Pantano, 8);
    PopulateBiomeWithStructures(EBiomeType::Deserto, 10);
    
    // Create some interior spaces
    for (int32 i = 0; i < 5; i++)
    {
        FArch_InteriorSpace CaveSpace;
        CaveSpace.SpaceName = FString::Printf(TEXT("Cave_Dwelling_%03d"), i + 1);
        CaveSpace.CenterLocation = GetBiomeBaseLocation(EBiomeType::Montanha) + 
                                   FVector(FMath::RandRange(-2000.0f, 2000.0f), 
                                          FMath::RandRange(-2000.0f, 2000.0f), 
                                          100.0f);
        CaveSpace.SpaceRadius = FMath::RandRange(300.0f, 800.0f);
        CaveSpace.bHasFirePit = FMath::RandBool();
        CaveSpace.bHasShelter = true;
        
        CreateInteriorSpace(CaveSpace);
    }
}

void UArch_StructuralManager::InitializeDefaultStructures()
{
    // Initialize with some default structural elements
    StructuralElements.Reserve(100);
    InteriorSpaces.Reserve(20);
}

FVector UArch_StructuralManager::GetBiomeBaseLocation(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Pantano:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Floresta:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Deserto:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Montanha:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}