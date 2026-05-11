#include "Arch_StructureManager.h"
#include "ArchitecturalStructure.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UArch_StructureManager::UArch_StructureManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize structure type mappings
    InitializeStructureTypes();
    
    // Set default parameters
    MaxStructuresPerType = 10;
    MinDistanceBetweenStructures = 500.0f;
    DefaultInteriorLightIntensity = 600.0f;
    DefaultInteriorLightRadius = 300.0f;
}

void UArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with world for structure management
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("Arch_StructureManager initialized for world: %s"), *World->GetName());
    }
}

void UArch_StructureManager::InitializeStructureTypes()
{
    // Cave Dwelling Configuration
    FArch_StructureConfig CaveConfig;
    CaveConfig.StructureType = EArch_StructureType::CaveDwelling;
    CaveConfig.RequiredSpace = FVector(400, 400, 300);
    CaveConfig.PreferredTerrain = EArch_TerrainType::Rocky;
    CaveConfig.MaxOccupants = 4;
    CaveConfig.InteriorLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f); // Warm fire orange
    CaveConfig.RequiresFirePit = true;
    CaveConfig.RequiresWaterAccess = false;
    StructureConfigs.Add(EArch_StructureType::CaveDwelling, CaveConfig);
    
    // Elevated Platform Configuration
    FArch_StructureConfig PlatformConfig;
    PlatformConfig.StructureType = EArch_StructureType::ElevatedPlatform;
    PlatformConfig.RequiredSpace = FVector(600, 600, 400);
    PlatformConfig.PreferredTerrain = EArch_TerrainType::Forest;
    PlatformConfig.MaxOccupants = 6;
    PlatformConfig.InteriorLightColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f); // Torch amber
    PlatformConfig.RequiresFirePit = true;
    PlatformConfig.RequiresWaterAccess = true;
    StructureConfigs.Add(EArch_StructureType::ElevatedPlatform, PlatformConfig);
    
    // Rock Shelter Configuration
    FArch_StructureConfig ShelterConfig;
    ShelterConfig.StructureType = EArch_StructureType::RockShelter;
    ShelterConfig.RequiredSpace = FVector(500, 300, 250);
    ShelterConfig.PreferredTerrain = EArch_TerrainType::Rocky;
    ShelterConfig.MaxOccupants = 3;
    ShelterConfig.InteriorLightColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f); // Soft daylight
    ShelterConfig.RequiresFirePit = false;
    ShelterConfig.RequiresWaterAccess = false;
    StructureConfigs.Add(EArch_StructureType::RockShelter, ShelterConfig);
    
    // Communal Longhouse Configuration
    FArch_StructureConfig LonghouseConfig;
    LonghouseConfig.StructureType = EArch_StructureType::CommunalLonghouse;
    LonghouseConfig.RequiredSpace = FVector(1000, 400, 350);
    LonghouseConfig.PreferredTerrain = EArch_TerrainType::Plains;
    LonghouseConfig.MaxOccupants = 12;
    LonghouseConfig.InteriorLightColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // Central hearth
    LonghouseConfig.RequiresFirePit = true;
    LonghouseConfig.RequiresWaterAccess = true;
    StructureConfigs.Add(EArch_StructureType::CommunalLonghouse, LonghouseConfig);
}

AArchitecturalStructure* UArch_StructureManager::CreateStructure(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create structure - no valid world"));
        return nullptr;
    }
    
    // Check if we've reached the maximum for this type
    int32 CurrentCount = GetStructureCountByType(StructureType);
    if (CurrentCount >= MaxStructuresPerType)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum structures of type %d reached"), (int32)StructureType);
        return nullptr;
    }
    
    // Validate location spacing
    if (!IsValidStructureLocation(Location, StructureType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid location for structure at %s"), *Location.ToString());
        return nullptr;
    }
    
    // Get structure configuration
    const FArch_StructureConfig* Config = StructureConfigs.Find(StructureType);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("No configuration found for structure type %d"), (int32)StructureType);
        return nullptr;
    }
    
    // Spawn the structure actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AArchitecturalStructure* NewStructure = GetWorld()->SpawnActor<AArchitecturalStructure>(
        AArchitecturalStructure::StaticClass(),
        Location,
        Rotation,
        SpawnParams
    );
    
    if (NewStructure)
    {
        // Configure the structure
        NewStructure->SetStructureType(StructureType);
        NewStructure->SetMaxOccupants(Config->MaxOccupants);
        
        // Add interior lighting if required
        if (Config->RequiresFirePit || StructureType == EArch_StructureType::CommunalLonghouse)
        {
            CreateInteriorLighting(NewStructure, *Config);
        }
        
        // Register the structure
        ManagedStructures.Add(NewStructure);
        
        UE_LOG(LogTemp, Log, TEXT("Created structure of type %d at location %s"), 
               (int32)StructureType, *Location.ToString());
        
        // Broadcast structure creation event
        OnStructureCreated.Broadcast(NewStructure, StructureType);
    }
    
    return NewStructure;
}

void UArch_StructureManager::CreateInteriorLighting(AArchitecturalStructure* Structure, const FArch_StructureConfig& Config)
{
    if (!Structure || !GetWorld())
    {
        return;
    }
    
    // Calculate interior light position based on structure type
    FVector LightOffset = FVector::ZeroVector;
    switch (Config.StructureType)
    {
        case EArch_StructureType::CaveDwelling:
            LightOffset = FVector(0, 0, -30); // Lower for cave fire pit
            break;
        case EArch_StructureType::ElevatedPlatform:
            LightOffset = FVector(0, 0, 20); // Higher for platform torch
            break;
        case EArch_StructureType::CommunalLonghouse:
            LightOffset = FVector(0, 0, 0); // Center height for hearth
            break;
        default:
            LightOffset = FVector(0, 0, 0);
            break;
    }
    
    FVector LightLocation = Structure->GetActorLocation() + LightOffset;
    
    // Spawn interior light
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Structure;
    
    AActor* LightActor = GetWorld()->SpawnActor<AActor>(
        AActor::StaticClass(),
        LightLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (LightActor)
    {
        // Add point light component
        UPointLightComponent* LightComponent = NewObject<UPointLightComponent>(LightActor);
        LightComponent->SetLightColor(Config.InteriorLightColor);
        LightComponent->SetIntensity(DefaultInteriorLightIntensity);
        LightComponent->SetAttenuationRadius(DefaultInteriorLightRadius);
        LightComponent->SetCastShadows(true);
        LightComponent->AttachToComponent(LightActor->GetRootComponent(), 
                                        FAttachmentTransformRules::KeepWorldTransform);
        LightActor->AddInstanceComponent(LightComponent);
        
        // Set appropriate name
        FString LightName = FString::Printf(TEXT("%s_InteriorLight"), *Structure->GetName());
        LightActor->SetActorLabel(LightName);
        
        UE_LOG(LogTemp, Log, TEXT("Created interior lighting for structure %s"), *Structure->GetName());
    }
}

bool UArch_StructureManager::IsValidStructureLocation(const FVector& Location, EArch_StructureType StructureType) const
{
    // Check minimum distance from existing structures
    for (const AArchitecturalStructure* ExistingStructure : ManagedStructures)
    {
        if (ExistingStructure && IsValid(ExistingStructure))
        {
            float Distance = FVector::Dist(Location, ExistingStructure->GetActorLocation());
            if (Distance < MinDistanceBetweenStructures)
            {
                return false;
            }
        }
    }
    
    // Additional terrain-specific validation could be added here
    // For now, accept all locations that pass distance check
    return true;
}

int32 UArch_StructureManager::GetStructureCountByType(EArch_StructureType StructureType) const
{
    int32 Count = 0;
    for (const AArchitecturalStructure* Structure : ManagedStructures)
    {
        if (Structure && IsValid(Structure) && Structure->GetStructureType() == StructureType)
        {
            Count++;
        }
    }
    return Count;
}

TArray<AArchitecturalStructure*> UArch_StructureManager::GetStructuresByType(EArch_StructureType StructureType) const
{
    TArray<AArchitecturalStructure*> FilteredStructures;
    for (AArchitecturalStructure* Structure : ManagedStructures)
    {
        if (Structure && IsValid(Structure) && Structure->GetStructureType() == StructureType)
        {
            FilteredStructures.Add(Structure);
        }
    }
    return FilteredStructures;
}

void UArch_StructureManager::CleanupInvalidStructures()
{
    ManagedStructures.RemoveAll([](const AArchitecturalStructure* Structure)
    {
        return !Structure || !IsValid(Structure);
    });
}

FArch_StructureConfig UArch_StructureManager::GetStructureConfig(EArch_StructureType StructureType) const
{
    const FArch_StructureConfig* Config = StructureConfigs.Find(StructureType);
    return Config ? *Config : FArch_StructureConfig();
}