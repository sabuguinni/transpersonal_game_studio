#include "Eng_ArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEng_ArchitectureManager::UEng_ArchitectureManager()
{
    bIsInitialized = false;
}

void UEng_ArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultSettings();
    Buildings.Empty();
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Initialized with max buildings: %d"), Settings.MaxBuildings);
}

void UEng_ArchitectureManager::Deinitialize()
{
    ClearAllBuildings();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

void UEng_ArchitectureManager::InitializeDefaultSettings()
{
    Settings.MaxBuildings = 50;
    Settings.BuildingSnapDistance = 100.0f;
    Settings.bEnableAutoRepair = true;
    Settings.WeatherDamageRate = 1.0f;
    Settings.bRequireMaterials = true;
}

bool UEng_ArchitectureManager::CreateBuilding(EEng_BuildingType BuildingType, FVector Location, FRotator Rotation)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: Not initialized"));
        return false;
    }

    if (Buildings.Num() >= Settings.MaxBuildings)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Maximum buildings reached (%d)"), Settings.MaxBuildings);
        return false;
    }

    if (!IsValidBuildingLocation(Location, BuildingType))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Invalid building location"));
        return false;
    }

    if (Settings.bRequireMaterials && !HasRequiredMaterials(BuildingType))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Missing required materials"));
        return false;
    }

    FEng_BuildingData NewBuilding;
    NewBuilding.BuildingType = BuildingType;
    NewBuilding.Location = Location;
    NewBuilding.Rotation = Rotation;
    NewBuilding.Health = 100.0f;
    NewBuilding.bIsConstructed = false;
    NewBuilding.ConstructionProgress = 0.0f;
    NewBuilding.RequiredMaterials = GetRequiredMaterials(BuildingType);

    Buildings.Add(NewBuilding);
    
    // Spawn the actual building actor
    AActor* BuildingActor = SpawnBuildingActor(NewBuilding);
    if (BuildingActor)
    {
        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Created building %s at %s"), 
               *UEnum::GetValueAsString(BuildingType), *Location.ToString());
        return true;
    }

    // Remove from array if spawn failed
    Buildings.RemoveAt(Buildings.Num() - 1);
    return false;
}

bool UEng_ArchitectureManager::RemoveBuilding(int32 BuildingIndex)
{
    if (!Buildings.IsValidIndex(BuildingIndex))
    {
        return false;
    }

    Buildings.RemoveAt(BuildingIndex);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Removed building at index %d"), BuildingIndex);
    return true;
}

void UEng_ArchitectureManager::UpdateBuildingConstruction(int32 BuildingIndex, float ProgressDelta)
{
    if (!Buildings.IsValidIndex(BuildingIndex))
    {
        return;
    }

    FEng_BuildingData& Building = Buildings[BuildingIndex];
    Building.ConstructionProgress = FMath::Clamp(Building.ConstructionProgress + ProgressDelta, 0.0f, 100.0f);
    
    if (Building.ConstructionProgress >= 100.0f && !Building.bIsConstructed)
    {
        Building.bIsConstructed = true;
        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Building construction completed"));
    }
}

TArray<FEng_BuildingData> UEng_ArchitectureManager::GetAllBuildings() const
{
    return Buildings;
}

FEng_BuildingData UEng_ArchitectureManager::GetBuildingData(int32 BuildingIndex) const
{
    if (Buildings.IsValidIndex(BuildingIndex))
    {
        return Buildings[BuildingIndex];
    }
    return FEng_BuildingData();
}

bool UEng_ArchitectureManager::IsValidBuildingLocation(FVector Location, EEng_BuildingType BuildingType) const
{
    // Check minimum distance from other buildings
    float NearestDistance = GetNearestBuildingDistance(Location);
    if (NearestDistance < Settings.BuildingSnapDistance)
    {
        return false;
    }

    // Additional validation based on building type
    switch (BuildingType)
    {
        case EEng_BuildingType::Bridge:
            // Bridges need special validation for water/gaps
            break;
        case EEng_BuildingType::Watchtower:
            // Watchtowers need elevation validation
            break;
        default:
            break;
    }

    return true;
}

float UEng_ArchitectureManager::GetNearestBuildingDistance(FVector Location) const
{
    float NearestDistance = FLT_MAX;
    
    for (const FEng_BuildingData& Building : Buildings)
    {
        float Distance = FVector::Dist(Location, Building.Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
        }
    }
    
    return NearestDistance;
}

void UEng_ArchitectureManager::SetArchitectureSettings(const FEng_ArchitectureSettings& NewSettings)
{
    Settings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Settings updated"));
}

FEng_ArchitectureSettings UEng_ArchitectureManager::GetArchitectureSettings() const
{
    return Settings;
}

void UEng_ArchitectureManager::ApplyWeatherDamage(float DeltaTime)
{
    if (!Settings.bEnableAutoRepair)
    {
        return;
    }

    for (FEng_BuildingData& Building : Buildings)
    {
        if (Building.bIsConstructed && Building.Health > 0.0f)
        {
            float Damage = Settings.WeatherDamageRate * DeltaTime;
            Building.Health = FMath::Max(0.0f, Building.Health - Damage);
            
            if (Building.Health <= 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Building destroyed by weather"));
            }
        }
    }
}

TArray<FString> UEng_ArchitectureManager::GetRequiredMaterials(EEng_BuildingType BuildingType) const
{
    TArray<FString> Materials;
    
    switch (BuildingType)
    {
        case EEng_BuildingType::Shelter:
            Materials.Add("Wood");
            Materials.Add("Stone");
            Materials.Add("Leaves");
            break;
        case EEng_BuildingType::Storage:
            Materials.Add("Wood");
            Materials.Add("Vine");
            break;
        case EEng_BuildingType::Crafting:
            Materials.Add("Stone");
            Materials.Add("Wood");
            Materials.Add("Bone");
            break;
        case EEng_BuildingType::Firepit:
            Materials.Add("Stone");
            Materials.Add("Flint");
            break;
        case EEng_BuildingType::Watchtower:
            Materials.Add("Wood");
            Materials.Add("Stone");
            Materials.Add("Vine");
            break;
        case EEng_BuildingType::Fence:
            Materials.Add("Wood");
            Materials.Add("Vine");
            break;
        case EEng_BuildingType::Bridge:
            Materials.Add("Wood");
            Materials.Add("Vine");
            Materials.Add("Stone");
            break;
        default:
            break;
    }
    
    return Materials;
}

bool UEng_ArchitectureManager::HasRequiredMaterials(EEng_BuildingType BuildingType) const
{
    // This would integrate with inventory system
    // For now, return true to allow building
    return true;
}

FString UEng_ArchitectureManager::GetBuildingMeshPath(EEng_BuildingType BuildingType) const
{
    switch (BuildingType)
    {
        case EEng_BuildingType::Shelter:
            return TEXT("/Engine/BasicShapes/Cube");
        case EEng_BuildingType::Storage:
            return TEXT("/Engine/BasicShapes/Cube");
        case EEng_BuildingType::Crafting:
            return TEXT("/Engine/BasicShapes/Cylinder");
        case EEng_BuildingType::Firepit:
            return TEXT("/Engine/BasicShapes/Cylinder");
        case EEng_BuildingType::Watchtower:
            return TEXT("/Engine/BasicShapes/Cube");
        case EEng_BuildingType::Fence:
            return TEXT("/Engine/BasicShapes/Cube");
        case EEng_BuildingType::Bridge:
            return TEXT("/Engine/BasicShapes/Cube");
        default:
            return TEXT("/Engine/BasicShapes/Cube");
    }
}

AActor* UEng_ArchitectureManager::SpawnBuildingActor(const FEng_BuildingData& BuildingData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Spawn a static mesh actor
    AStaticMeshActor* BuildingActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        BuildingData.Location,
        BuildingData.Rotation
    );

    if (BuildingActor)
    {
        // Load appropriate mesh
        FString MeshPath = GetBuildingMeshPath(BuildingData.BuildingType);
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
        
        if (Mesh && BuildingActor->GetStaticMeshComponent())
        {
            BuildingActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
            
            // Set building type as actor label
            FString BuildingName = UEnum::GetValueAsString(BuildingData.BuildingType);
            BuildingActor->SetActorLabel(BuildingName);
        }
    }

    return BuildingActor;
}

void UEng_ArchitectureManager::DebugSpawnTestBuildings()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: Not initialized for debug spawn"));
        return;
    }

    // Spawn test buildings in a grid pattern
    TArray<EEng_BuildingType> TestTypes = {
        EEng_BuildingType::Shelter,
        EEng_BuildingType::Storage,
        EEng_BuildingType::Crafting,
        EEng_BuildingType::Firepit,
        EEng_BuildingType::Watchtower
    };

    FVector BaseLocation(0.0f, 0.0f, 100.0f);
    
    for (int32 i = 0; i < TestTypes.Num(); i++)
    {
        FVector SpawnLocation = BaseLocation + FVector(i * 300.0f, 0.0f, 0.0f);
        CreateBuilding(TestTypes[i], SpawnLocation, FRotator::ZeroRotator);
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Spawned %d test buildings"), TestTypes.Num());
}

void UEng_ArchitectureManager::ClearAllBuildings()
{
    Buildings.Empty();
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Cleared all buildings"));
}