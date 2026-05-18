#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

UArchitecturalStructureManager::UArchitecturalStructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance
    
    StructureData = FArch_StructureData();
    InteriorSpaces.Empty();
    StructureMesh = nullptr;
    WeatheredMaterial = nullptr;
    StructuralIntegrity = 100.0f;
    bIsHistoricalSite = false;
}

void UArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default structure if not set
    if (StructureData.StructureType == EArch_StructureType::None)
    {
        InitializeStructure(EArch_StructureType::StoneArch, EArch_ConstructionMaterial::Limestone, FVector(400.0f, 100.0f, 600.0f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalStructureManager initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UArchitecturalStructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structural integrity over time
    UpdateStructuralIntegrity(DeltaTime);
    
    // Process natural weathering
    ProcessWeathering(DeltaTime);
}

void UArchitecturalStructureManager::InitializeStructure(EArch_StructureType InType, EArch_ConstructionMaterial InMaterial, FVector InDimensions)
{
    StructureData.StructureType = InType;
    StructureData.Material = InMaterial;
    StructureData.Dimensions = InDimensions;
    StructureData.Age = 0.0f;
    StructureData.WeatheringLevel = 0.0f;
    StructureData.bHasMossGrowth = false;
    StructureData.bIsRuined = false;
    
    StructuralIntegrity = 100.0f;
    
    // Set appropriate material based on construction material
    WeatheredMaterial = GetMaterialForType(InMaterial);
    
    UE_LOG(LogTemp, Log, TEXT("Structure initialized: Type=%d, Material=%d, Dimensions=%s"), 
           (int32)InType, (int32)InMaterial, *InDimensions.ToString());
}

void UArchitecturalStructureManager::AddInteriorSpace(const FArch_InteriorSpace& NewSpace)
{
    InteriorSpaces.Add(NewSpace);
    
    UE_LOG(LogTemp, Log, TEXT("Added interior space: %s with dimensions %s"), 
           *NewSpace.SpaceName, *NewSpace.SpaceDimensions.ToString());
}

void UArchitecturalStructureManager::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(StructureData.WeatheringLevel + WeatheringAmount, 0.0f, 100.0f);
    
    // Apply moss growth if weathering is significant
    if (StructureData.WeatheringLevel > 30.0f && !StructureData.bHasMossGrowth)
    {
        SetMossGrowth(true);
    }
    
    // Mark as ruined if heavily weathered
    if (StructureData.WeatheringLevel > 80.0f)
    {
        StructureData.bIsRuined = true;
        StructuralIntegrity = FMath::Min(StructuralIntegrity, 25.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied weathering: %f, Total weathering: %f"), 
           WeatheringAmount, StructureData.WeatheringLevel);
}

void UArchitecturalStructureManager::SetMossGrowth(bool bEnabled)
{
    StructureData.bHasMossGrowth = bEnabled;
    
    // Update visual appearance if owner has static mesh component
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            if (WeatheredMaterial)
            {
                MeshComp->SetMaterial(0, WeatheredMaterial);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Moss growth %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UArchitecturalStructureManager::RepairStructure(float RepairAmount)
{
    StructuralIntegrity = FMath::Clamp(StructuralIntegrity + RepairAmount, 0.0f, 100.0f);
    
    // Reduce weathering with repairs
    StructureData.WeatheringLevel = FMath::Max(StructureData.WeatheringLevel - (RepairAmount * 0.5f), 0.0f);
    
    // Remove ruined status if sufficiently repaired
    if (StructuralIntegrity > 50.0f)
    {
        StructureData.bIsRuined = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Structure repaired by %f, Integrity now: %f"), 
           RepairAmount, StructuralIntegrity);
}

void UArchitecturalStructureManager::UpdateStructuralIntegrity(float DeltaTime)
{
    // Gradual decay over time (very slow for stone structures)
    float DecayRate = 0.001f; // 0.1% per second
    
    // Faster decay for wooden structures
    if (StructureData.Material == EArch_ConstructionMaterial::Wood)
    {
        DecayRate = 0.005f;
    }
    
    // Slower decay for historical sites (better preservation)
    if (bIsHistoricalSite)
    {
        DecayRate *= 0.1f;
    }
    
    StructuralIntegrity = FMath::Max(StructuralIntegrity - (DecayRate * DeltaTime), 0.0f);
    
    // Age the structure
    StructureData.Age += DeltaTime;
}

void UArchitecturalStructureManager::ProcessWeathering(float DeltaTime)
{
    // Environmental weathering based on age and exposure
    float WeatheringRate = 0.0005f; // Base weathering rate
    
    // Faster weathering for certain materials
    switch (StructureData.Material)
    {
        case EArch_ConstructionMaterial::Wood:
            WeatheringRate *= 3.0f;
            break;
        case EArch_ConstructionMaterial::Clay:
            WeatheringRate *= 2.0f;
            break;
        case EArch_ConstructionMaterial::Limestone:
            WeatheringRate *= 1.5f;
            break;
        case EArch_ConstructionMaterial::Granite:
            WeatheringRate *= 0.5f;
            break;
        default:
            break;
    }
    
    ApplyWeathering(WeatheringRate * DeltaTime);
}

UMaterialInterface* UArchitecturalStructureManager::GetMaterialForType(EArch_ConstructionMaterial MaterialType)
{
    // In a real implementation, this would load appropriate materials from content
    // For now, return nullptr and let the default material system handle it
    
    switch (MaterialType)
    {
        case EArch_ConstructionMaterial::Limestone:
            // Would load limestone material
            break;
        case EArch_ConstructionMaterial::Sandstone:
            // Would load sandstone material
            break;
        case EArch_ConstructionMaterial::Basalt:
            // Would load basalt material
            break;
        case EArch_ConstructionMaterial::Granite:
            // Would load granite material
            break;
        case EArch_ConstructionMaterial::Wood:
            // Would load weathered wood material
            break;
        case EArch_ConstructionMaterial::Clay:
            // Would load clay/adobe material
            break;
        default:
            break;
    }
    
    return nullptr;
}