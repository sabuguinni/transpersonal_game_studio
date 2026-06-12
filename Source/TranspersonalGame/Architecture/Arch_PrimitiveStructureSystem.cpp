#include "Arch_PrimitiveStructureSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

UArch_PrimitiveStructureSystem::UArch_PrimitiveStructureSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds for weather damage
    
    StructureMesh = nullptr;
    WeatherDamageRate = 0.1f;
    bAffectedByWeather = true;
}

void UArch_PrimitiveStructureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default structure if not set
    if (StructureData.StructureType == EArch_StructureType::None)
    {
        InitializeStructure(EArch_StructureType::StoneShelter, EArch_MaterialType::Stone);
    }
    
    UpdateStructureMaterial();
    CalculateWeatherResistance();
}

void UArch_PrimitiveStructureSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Apply weather damage over time if enabled
    if (bAffectedByWeather && StructureData.StructuralIntegrity > 0.0f)
    {
        float WeatherDamage = WeatherDamageRate * DeltaTime;
        ApplyWeatherDamage(WeatherDamage);
    }
}

void UArch_PrimitiveStructureSystem::InitializeStructure(EArch_StructureType Type, EArch_MaterialType Material)
{
    StructureData.StructureType = Type;
    StructureData.PrimaryMaterial = Material;
    
    // Set default values based on structure type
    switch (Type)
    {
        case EArch_StructureType::StoneShelter:
            StructureData.StructuralIntegrity = 100.0f;
            StructureData.WeatherResistance = 80.0f;
            StructureData.bIsHabitable = true;
            StructureData.MaxOccupants = 4;
            break;
            
        case EArch_StructureType::WoodLean:
            StructureData.StructuralIntegrity = 60.0f;
            StructureData.WeatherResistance = 30.0f;
            StructureData.bIsHabitable = true;
            StructureData.MaxOccupants = 2;
            break;
            
        case EArch_StructureType::CaveEntrance:
            StructureData.StructuralIntegrity = 150.0f;
            StructureData.WeatherResistance = 95.0f;
            StructureData.bIsHabitable = true;
            StructureData.MaxOccupants = 8;
            break;
            
        case EArch_StructureType::RockFormation:
            StructureData.StructuralIntegrity = 200.0f;
            StructureData.WeatherResistance = 90.0f;
            StructureData.bIsHabitable = false;
            StructureData.MaxOccupants = 0;
            break;
            
        case EArch_StructureType::BoneStructure:
            StructureData.StructuralIntegrity = 40.0f;
            StructureData.WeatherResistance = 20.0f;
            StructureData.bIsHabitable = true;
            StructureData.MaxOccupants = 1;
            break;
            
        default:
            StructureData.StructuralIntegrity = 50.0f;
            StructureData.WeatherResistance = 25.0f;
            StructureData.bIsHabitable = false;
            StructureData.MaxOccupants = 0;
            break;
    }
    
    UpdateStructureMaterial();
    CalculateWeatherResistance();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture: Initialized %s structure with %s material"), 
           *UEnum::GetValueAsString(Type), *UEnum::GetValueAsString(Material));
}

void UArch_PrimitiveStructureSystem::ApplyWeatherDamage(float DamageAmount)
{
    if (!bAffectedByWeather || DamageAmount <= 0.0f)
    {
        return;
    }
    
    // Reduce damage based on weather resistance
    float ActualDamage = DamageAmount * (1.0f - (StructureData.WeatherResistance / 100.0f));
    StructureData.StructuralIntegrity = FMath::Max(0.0f, StructureData.StructuralIntegrity - ActualDamage);
    
    // Update material to show wear
    UpdateStructureMaterial();
    
    if (StructureData.StructuralIntegrity <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture: Structure has collapsed due to weather damage!"));
        StructureData.bIsHabitable = false;
        StructureData.MaxOccupants = 0;
    }
}

void UArch_PrimitiveStructureSystem::RepairStructure(float RepairAmount)
{
    if (RepairAmount <= 0.0f)
    {
        return;
    }
    
    float MaxIntegrity = 100.0f;
    switch (StructureData.StructureType)
    {
        case EArch_StructureType::CaveEntrance:
            MaxIntegrity = 150.0f;
            break;
        case EArch_StructureType::RockFormation:
            MaxIntegrity = 200.0f;
            break;
        default:
            MaxIntegrity = 100.0f;
            break;
    }
    
    StructureData.StructuralIntegrity = FMath::Min(MaxIntegrity, StructureData.StructuralIntegrity + RepairAmount);
    
    // Restore habitability if structure is stable again
    if (IsStructureStable())
    {
        InitializeStructure(StructureData.StructureType, StructureData.PrimaryMaterial);
    }
    
    UpdateStructureMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture: Structure repaired by %.1f points, integrity now %.1f"), 
           RepairAmount, StructureData.StructuralIntegrity);
}

bool UArch_PrimitiveStructureSystem::CanSupportOccupants(int32 NumOccupants) const
{
    return IsStructureStable() && 
           StructureData.bIsHabitable && 
           NumOccupants <= StructureData.MaxOccupants;
}

FVector UArch_PrimitiveStructureSystem::GetShelterPoint() const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return FVector::ZeroVector;
    }
    
    FVector OwnerLocation = Owner->GetActorLocation();
    FVector Forward = Owner->GetActorForwardVector();
    
    // Return a point slightly inside the structure for shelter
    return OwnerLocation + (Forward * -50.0f) + FVector(0.0f, 0.0f, 100.0f);
}

bool UArch_PrimitiveStructureSystem::IsStructureStable() const
{
    return StructureData.StructuralIntegrity > 25.0f;
}

void UArch_PrimitiveStructureSystem::UpdateStructureMaterial()
{
    if (!StructureMaterials.IsEmpty() && GetOwner())
    {
        UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && StructureMaterials.IsValidIndex(0))
        {
            UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(StructureMaterials[0], this);
            if (DynMaterial)
            {
                // Set wear parameter based on structural integrity
                float WearAmount = 1.0f - (StructureData.StructuralIntegrity / 100.0f);
                DynMaterial->SetScalarParameterValue(TEXT("WearAmount"), WearAmount);
                
                // Set material type color
                FLinearColor MaterialColor = FLinearColor::White;
                switch (StructureData.PrimaryMaterial)
                {
                    case EArch_MaterialType::Stone:
                        MaterialColor = FLinearColor(0.6f, 0.6f, 0.5f, 1.0f);
                        break;
                    case EArch_MaterialType::Wood:
                        MaterialColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
                        break;
                    case EArch_MaterialType::Bone:
                        MaterialColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
                        break;
                    case EArch_MaterialType::Hide:
                        MaterialColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
                        break;
                    case EArch_MaterialType::Mud:
                        MaterialColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
                        break;
                }
                
                DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), MaterialColor);
                MeshComp->SetMaterial(0, DynMaterial);
            }
        }
    }
}

void UArch_PrimitiveStructureSystem::CalculateWeatherResistance()
{
    float BaseResistance = 50.0f;
    
    // Material-based resistance modifiers
    switch (StructureData.PrimaryMaterial)
    {
        case EArch_MaterialType::Stone:
            BaseResistance = 80.0f;
            break;
        case EArch_MaterialType::Wood:
            BaseResistance = 30.0f;
            break;
        case EArch_MaterialType::Bone:
            BaseResistance = 20.0f;
            break;
        case EArch_MaterialType::Hide:
            BaseResistance = 40.0f;
            break;
        case EArch_MaterialType::Mud:
            BaseResistance = 25.0f;
            break;
    }
    
    // Structure type modifiers
    switch (StructureData.StructureType)
    {
        case EArch_StructureType::CaveEntrance:
            BaseResistance += 15.0f;
            break;
        case EArch_StructureType::RockFormation:
            BaseResistance += 10.0f;
            break;
        case EArch_StructureType::StoneShelter:
            BaseResistance += 5.0f;
            break;
        default:
            break;
    }
    
    StructureData.WeatherResistance = FMath::Clamp(BaseResistance, 0.0f, 100.0f);
}