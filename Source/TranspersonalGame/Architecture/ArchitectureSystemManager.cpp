#include "ArchitectureSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInstance.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"

UArchitectureSystemManager::UArchitectureSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second for performance
    
    // Initialize default values
    ArchitectureDataTable = nullptr;
}

void UArchitectureSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureSystemManager: Initializing architecture system..."));
    
    InitializeDefaultMaterials();
    LoadArchitectureDatabase();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureSystemManager: Architecture system initialized with %d registered structures"), RegisteredStructures.Num());
}

void UArchitectureSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic updates for structural integrity and atmospheric effects
    // This could be optimized to only update when needed
}

bool UArchitectureSystemManager::RegisterStructure(const FString& StructureID, const FArchitectureData& ArchitectureData)
{
    if (StructureID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureSystemManager: Cannot register structure with empty ID"));
        return false;
    }
    
    if (RegisteredStructures.Contains(StructureID))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureSystemManager: Structure %s already registered, updating data"), *StructureID);
    }
    
    RegisteredStructures.Add(StructureID, ArchitectureData);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureSystemManager: Registered structure %s of type %s"), 
           *StructureID, 
           *UEnum::GetValueAsString(ArchitectureData.ArchitectureType));
    
    return true;
}

bool UArchitectureSystemManager::UnregisterStructure(const FString& StructureID)
{
    if (!RegisteredStructures.Contains(StructureID))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureSystemManager: Structure %s not found for unregistration"), *StructureID);
        return false;
    }
    
    RegisteredStructures.Remove(StructureID);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureSystemManager: Unregistered structure %s"), *StructureID);
    
    return true;
}

FArchitectureData UArchitectureSystemManager::GetStructureData(const FString& StructureID)
{
    if (RegisteredStructures.Contains(StructureID))
    {
        return RegisteredStructures[StructureID];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureSystemManager: Structure %s not found, returning default data"), *StructureID);
    return FArchitectureData(); // Return default structure
}

TArray<FString> UArchitectureSystemManager::GetStructuresByType(EArchitectureType ArchitectureType)
{
    TArray<FString> StructuresOfType;
    
    for (const auto& StructurePair : RegisteredStructures)
    {
        if (StructurePair.Value.ArchitectureType == ArchitectureType)
        {
            StructuresOfType.Add(StructurePair.Key);
        }
    }
    
    return StructuresOfType;
}

UMaterialInstance* UArchitectureSystemManager::GetMaterialForType(EArchitectureMaterial MaterialType)
{
    if (MaterialInstances.Contains(MaterialType))
    {
        return MaterialInstances[MaterialType];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureSystemManager: Material instance not found for type %s"), 
           *UEnum::GetValueAsString(MaterialType));
    return nullptr;
}

void UArchitectureSystemManager::SetMaterialForType(EArchitectureMaterial MaterialType, UMaterialInstance* Material)
{
    if (Material)
    {
        MaterialInstances.Add(MaterialType, Material);
        UE_LOG(LogTemp, Log, TEXT("ArchitectureSystemManager: Set material for type %s"), 
               *UEnum::GetValueAsString(MaterialType));
    }
}

float UArchitectureSystemManager::CalculateStructuralIntegrity(const FArchitectureData& ArchitectureData, float WeatherDamage)
{
    float BaseIntegrity = ArchitectureData.StructuralIntegrity;
    
    // Material durability modifiers
    float MaterialModifier = 1.0f;
    switch (ArchitectureData.PrimaryMaterial)
    {
        case EArchitectureMaterial::Stone:
            MaterialModifier = 1.2f; // Stone is more durable
            break;
        case EArchitectureMaterial::Wood:
            MaterialModifier = 1.0f; // Wood is baseline
            break;
        case EArchitectureMaterial::Hide:
            MaterialModifier = 0.7f; // Hide is less durable
            break;
        case EArchitectureMaterial::Thatch:
            MaterialModifier = 0.6f; // Thatch is fragile
            break;
        case EArchitectureMaterial::Bone:
            MaterialModifier = 0.9f; // Bone is moderately durable
            break;
        case EArchitectureMaterial::Clay:
            MaterialModifier = 0.8f; // Clay is somewhat fragile
            break;
    }
    
    // Apply weather damage
    float FinalIntegrity = (BaseIntegrity * MaterialModifier) - WeatherDamage;
    
    return FMath::Clamp(FinalIntegrity, 0.0f, 1.0f);
}

bool UArchitectureSystemManager::IsStructureHabitable(const FString& StructureID)
{
    if (!RegisteredStructures.Contains(StructureID))
    {
        return false;
    }
    
    FArchitectureData StructureData = RegisteredStructures[StructureID];
    float CurrentIntegrity = CalculateStructuralIntegrity(StructureData);
    
    // Structure is habitable if integrity is above 30%
    return CurrentIntegrity > 0.3f;
}

TArray<FString> UArchitectureSystemManager::GetCulturalSignificance(const FString& StructureID)
{
    if (RegisteredStructures.Contains(StructureID))
    {
        return RegisteredStructures[StructureID].CulturalSignificance;
    }
    
    return TArray<FString>();
}

bool UArchitectureSystemManager::HasSacredSymbols(const FString& StructureID)
{
    if (RegisteredStructures.Contains(StructureID))
    {
        return RegisteredStructures[StructureID].bHasSacredSymbols;
    }
    
    return false;
}

void UArchitectureSystemManager::SetupInteriorLighting(AActor* StructureActor, EArchitectureType ArchitectureType)
{
    if (!StructureActor)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureSystemManager: Cannot setup lighting for null actor"));
        return;
    }
    
    UWorld* World = StructureActor->GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get lighting parameters based on architecture type
    FLinearColor LightColor = GetLightColorForArchitectureType(ArchitectureType);
    float LightIntensity = GetLightIntensityForArchitectureType(ArchitectureType);
    
    // Spawn appropriate lighting
    FVector LightLocation = StructureActor->GetActorLocation() + FVector(0, 0, 150);
    
    if (ArchitectureType == EArchitectureType::Cave)
    {
        // Cave lighting - warm fire light
        APointLight* FireLight = World->SpawnActor<APointLight>(LightLocation, FRotator::ZeroRotator);
        if (FireLight)
        {
            UPointLightComponent* LightComp = FireLight->GetPointLightComponent();
            if (LightComp)
            {
                LightComp->SetLightColor(LightColor);
                LightComp->SetIntensity(LightIntensity);
                LightComp->SetAttenuationRadius(800.0f);
                LightComp->SetSourceRadius(50.0f);
            }
            FireLight->SetActorLabel(FString::Printf(TEXT("CaveFireLight_%s"), *StructureActor->GetName()));
        }
    }
    else
    {
        // General interior lighting
        APointLight* InteriorLight = World->SpawnActor<APointLight>(LightLocation, FRotator::ZeroRotator);
        if (InteriorLight)
        {
            UPointLightComponent* LightComp = InteriorLight->GetPointLightComponent();
            if (LightComp)
            {
                LightComp->SetLightColor(LightColor);
                LightComp->SetIntensity(LightIntensity);
                LightComp->SetAttenuationRadius(600.0f);
            }
            InteriorLight->SetActorLabel(FString::Printf(TEXT("InteriorLight_%s"), *StructureActor->GetName()));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureSystemManager: Setup interior lighting for %s"), *StructureActor->GetName());
}

void UArchitectureSystemManager::UpdateAtmosphericEffects(AActor* StructureActor, float TimeOfDay)
{
    // Update lighting intensity based on time of day
    // This would integrate with the lighting system
    // For now, just log the update
    UE_LOG(LogTemp, VeryVerbose, TEXT("ArchitectureSystemManager: Updating atmospheric effects for %s at time %.2f"), 
           StructureActor ? *StructureActor->GetName() : TEXT("Unknown"), TimeOfDay);
}

void UArchitectureSystemManager::InitializeDefaultMaterials()
{
    // Initialize with null materials - these would be set up in Blueprint or by other systems
    MaterialInstances.Add(EArchitectureMaterial::Stone, nullptr);
    MaterialInstances.Add(EArchitectureMaterial::Wood, nullptr);
    MaterialInstances.Add(EArchitectureMaterial::Hide, nullptr);
    MaterialInstances.Add(EArchitectureMaterial::Thatch, nullptr);
    MaterialInstances.Add(EArchitectureMaterial::Bone, nullptr);
    MaterialInstances.Add(EArchitectureMaterial::Clay, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureSystemManager: Initialized default material slots"));
}

void UArchitectureSystemManager::LoadArchitectureDatabase()
{
    // Create some default prehistoric structures
    FArchitectureData CaveData;
    CaveData.ArchitectureType = EArchitectureType::Cave;
    CaveData.ArchitectureName = TEXT("Prehistoric Cave Dwelling");
    CaveData.PrimaryMaterial = EArchitectureMaterial::Stone;
    CaveData.SecondaryMaterial = EArchitectureMaterial::Hide;
    CaveData.StructuralIntegrity = 0.9f;
    CaveData.MaxOccupants = 8;
    CaveData.bHasFirePit = true;
    CaveData.bHasSacredSymbols = true;
    CaveData.CulturalSignificance.Add(TEXT("Sacred dwelling place"));
    CaveData.CulturalSignificance.Add(TEXT("Protection from elements"));
    CaveData.CulturalSignificance.Add(TEXT("Cave paintings record tribal history"));
    RegisterStructure(TEXT("DefaultCave"), CaveData);
    
    FArchitectureData HutData;
    HutData.ArchitectureType = EArchitectureType::Hut;
    HutData.ArchitectureName = TEXT("Tribal Hut");
    HutData.PrimaryMaterial = EArchitectureMaterial::Wood;
    HutData.SecondaryMaterial = EArchitectureMaterial::Thatch;
    HutData.StructuralIntegrity = 0.7f;
    HutData.MaxOccupants = 4;
    HutData.bHasFirePit = true;
    HutData.bHasSacredSymbols = false;
    HutData.CulturalSignificance.Add(TEXT("Family dwelling"));
    HutData.CulturalSignificance.Add(TEXT("Seasonal shelter"));
    RegisterStructure(TEXT("DefaultHut"), HutData);
    
    FArchitectureData LonghouseData;
    LonghouseData.ArchitectureType = EArchitectureType::Longhouse;
    LonghouseData.ArchitectureName = TEXT("Tribal Longhouse");
    LonghouseData.PrimaryMaterial = EArchitectureMaterial::Wood;
    LonghouseData.SecondaryMaterial = EArchitectureMaterial::Thatch;
    LonghouseData.StructuralIntegrity = 0.8f;
    LonghouseData.MaxOccupants = 20;
    LonghouseData.bHasFirePit = true;
    LonghouseData.bHasSacredSymbols = false;
    LonghouseData.CulturalSignificance.Add(TEXT("Communal living space"));
    LonghouseData.CulturalSignificance.Add(TEXT("Social gathering place"));
    LonghouseData.CulturalSignificance.Add(TEXT("Multi-family dwelling"));
    RegisterStructure(TEXT("DefaultLonghouse"), LonghouseData);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureSystemManager: Loaded default architecture database"));
}

FLinearColor UArchitectureSystemManager::GetLightColorForArchitectureType(EArchitectureType ArchitectureType)
{
    switch (ArchitectureType)
    {
        case EArchitectureType::Cave:
            return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Warm orange fire light
        case EArchitectureType::Hut:
            return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm yellow
        case EArchitectureType::Longhouse:
            return FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Warm amber
        case EArchitectureType::StoneCirlce:
            return FLinearColor(0.8f, 0.9f, 1.0f, 1.0f); // Cool moonlight
        case EArchitectureType::Ruins:
            return FLinearColor(0.7f, 0.8f, 0.9f, 1.0f); // Cool dim light
        case EArchitectureType::Shelter:
            return FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Natural daylight
        default:
            return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White light
    }
}

float UArchitectureSystemManager::GetLightIntensityForArchitectureType(EArchitectureType ArchitectureType)
{
    switch (ArchitectureType)
    {
        case EArchitectureType::Cave:
            return 2000.0f; // Bright fire light
        case EArchitectureType::Hut:
            return 1500.0f; // Moderate interior light
        case EArchitectureType::Longhouse:
            return 1800.0f; // Brighter for larger space
        case EArchitectureType::StoneCirlce:
            return 800.0f; // Dim mystical light
        case EArchitectureType::Ruins:
            return 500.0f; // Very dim
        case EArchitectureType::Shelter:
            return 1200.0f; // Natural light level
        default:
            return 1000.0f; // Default intensity
    }
}