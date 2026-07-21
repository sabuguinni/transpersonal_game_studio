#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ConstructorHelpers.h"

UArchitecturalStructureManager::UArchitecturalStructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Tick every 5 seconds for weather damage
    
    MaxStructuralIntegrity = 100.0f;
    WeatherDamageRate = 0.1f; // 0.1% damage per tick
    
    // Initialize structure count per biome
    StructureCountPerBiome.Add(EBiomeType::Savana, 0);
    StructureCountPerBiome.Add(EBiomeType::Pantano, 0);
    StructureCountPerBiome.Add(EBiomeType::Floresta, 0);
    StructureCountPerBiome.Add(EBiomeType::Deserto, 0);
    StructureCountPerBiome.Add(EBiomeType::Montanha, 0);
}

void UArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager initialized"));
}

void UArchitecturalStructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Apply weather damage to all structures
    ApplyWeatherDamage(DeltaTime);
}

bool UArchitecturalStructureManager::RegisterStructure(const FArch_StructureData& StructureData)
{
    RegisteredStructures.Add(StructureData);
    
    // Update biome count
    if (StructureCountPerBiome.Contains(StructureData.BiomeType))
    {
        StructureCountPerBiome[StructureData.BiomeType]++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Structure registered: %s in biome %d"), 
           *StructureData.StructureName, (int32)StructureData.BiomeType);
    
    return true;
}

void UArchitecturalStructureManager::SpawnStructureAtBiome(EBiomeType BiomeType, EArch_StructureType StructureType)
{
    FVector BiomeLocation = GetBiomeCoordinates(BiomeType);
    
    // Add random offset within biome area
    FVector RandomOffset = FVector(
        FMath::RandRange(-2000.0f, 2000.0f),
        FMath::RandRange(-2000.0f, 2000.0f),
        0.0f
    );
    
    FVector SpawnLocation = BiomeLocation + RandomOffset;
    CreateBasicStructure(SpawnLocation, StructureType, BiomeType);
}

TArray<FArch_StructureData> UArchitecturalStructureManager::GetStructuresInBiome(EBiomeType BiomeType) const
{
    TArray<FArch_StructureData> BiomeStructures;
    
    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        if (Structure.BiomeType == BiomeType)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

void UArchitecturalStructureManager::ApplyWeatherDamage(float DeltaTime)
{
    for (FArch_StructureData& Structure : RegisteredStructures)
    {
        // Different biomes cause different damage rates
        float BiomeDamageMultiplier = 1.0f;
        switch (Structure.BiomeType)
        {
            case EBiomeType::Pantano:
                BiomeDamageMultiplier = 2.0f; // Swamp causes more damage
                break;
            case EBiomeType::Deserto:
                BiomeDamageMultiplier = 1.5f; // Desert sandstorms
                break;
            case EBiomeType::Montanha:
                BiomeDamageMultiplier = 1.3f; // Mountain weather
                break;
            default:
                BiomeDamageMultiplier = 1.0f;
                break;
        }
        
        float DamageAmount = WeatherDamageRate * BiomeDamageMultiplier * DeltaTime;
        Structure.StructuralIntegrity = FMath::Max(0.0f, Structure.StructuralIntegrity - DamageAmount);
        
        // Mark as uninhabitable if integrity drops below 30%
        if (Structure.StructuralIntegrity < 30.0f)
        {
            Structure.bIsHabitable = false;
        }
    }
}

int32 UArchitecturalStructureManager::GetTotalStructureCount() const
{
    return RegisteredStructures.Num();
}

void UArchitecturalStructureManager::PopulateAllBiomesWithStructures()
{
    // Spawn structures in each biome
    TArray<EBiomeType> Biomes = {
        EBiomeType::Savana,
        EBiomeType::Pantano,
        EBiomeType::Floresta,
        EBiomeType::Deserto,
        EBiomeType::Montanha
    };
    
    TArray<EArch_StructureType> StructureTypes = {
        EArch_StructureType::Shelter,
        EArch_StructureType::Platform,
        EArch_StructureType::Pillar,
        EArch_StructureType::Wall
    };
    
    for (EBiomeType Biome : Biomes)
    {
        for (int32 i = 0; i < 3; i++) // 3 structures per biome
        {
            EArch_StructureType StructureType = StructureTypes[FMath::RandRange(0, StructureTypes.Num() - 1)];
            SpawnStructureAtBiome(Biome, StructureType);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Populated all biomes with architectural structures"));
}

FVector UArchitecturalStructureManager::GetBiomeCoordinates(EBiomeType BiomeType) const
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
            return FVector(40000.0f, 50000.0f, 200.0f);
        default:
            return FVector::ZeroVector;
    }
}

void UArchitecturalStructureManager::CreateBasicStructure(const FVector& Location, EArch_StructureType StructureType, EBiomeType BiomeType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Spawn a basic static mesh actor as placeholder
    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator
    );
    
    if (StructureActor)
    {
        FString StructureName = FString::Printf(TEXT("%s_%s_%d"),
            *UEnum::GetValueAsString(StructureType),
            *UEnum::GetValueAsString(BiomeType),
            FMath::RandRange(1000, 9999)
        );
        
        StructureActor->SetActorLabel(StructureName);
        
        // Register the structure
        FArch_StructureData NewStructure;
        NewStructure.StructureName = StructureName;
        NewStructure.Location = Location;
        NewStructure.BiomeType = BiomeType;
        NewStructure.StructuralIntegrity = MaxStructuralIntegrity;
        NewStructure.bIsHabitable = (StructureType == EArch_StructureType::Shelter);
        
        RegisterStructure(NewStructure);
    }
}