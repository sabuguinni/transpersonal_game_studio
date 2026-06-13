#include "Arch_ShelterSystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UArch_ShelterSystem::UArch_ShelterSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick once per second for performance

    // Initialize default values
    WeatheringRate = 0.001f;
    MossGrowthRate = 0.0005f;
    bEnableWeathering = true;
    bEnableMossGrowth = true;

    // Initialize shelter data with defaults
    ShelterData.ShelterType = EArch_ShelterType::StoneRuin;
    ShelterData.Condition = EArch_ShelterCondition::Weathered;
    ShelterData.WeatheringLevel = 0.5f;
    ShelterData.MossGrowth = 0.3f;
    ShelterData.StructuralIntegrity = 0.7f;
    ShelterData.bCanProvideProtection = true;
    ShelterData.ProtectionRadius = 300.0f;
}

void UArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all static mesh components on the owner actor
    if (AActor* Owner = GetOwner())
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp && MeshComp->GetStaticMesh())
            {
                ShelterMeshComponents.Add(MeshComp);
            }
        }
    }

    // Create dynamic materials for all mesh components
    CreateDynamicMaterials();
    
    // Apply initial material parameters based on shelter data
    UpdateMaterialParameters();

    UE_LOG(LogTemp, Warning, TEXT("Arch_ShelterSystem initialized with %d mesh components"), ShelterMeshComponents.Num());
}

void UArch_ShelterSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update shelter condition over time
    UpdateShelterCondition(DeltaTime);
}

void UArch_ShelterSystem::InitializeShelter(EArch_ShelterType InShelterType, EArch_ShelterCondition InCondition)
{
    ShelterData.ShelterType = InShelterType;
    ShelterData.Condition = InCondition;

    // Set appropriate values based on shelter type and condition
    switch (InCondition)
    {
        case EArch_ShelterCondition::Pristine:
            ShelterData.WeatheringLevel = 0.0f;
            ShelterData.MossGrowth = 0.0f;
            ShelterData.StructuralIntegrity = 1.0f;
            break;
        case EArch_ShelterCondition::Weathered:
            ShelterData.WeatheringLevel = 0.5f;
            ShelterData.MossGrowth = 0.3f;
            ShelterData.StructuralIntegrity = 0.7f;
            break;
        case EArch_ShelterCondition::Damaged:
            ShelterData.WeatheringLevel = 0.7f;
            ShelterData.MossGrowth = 0.6f;
            ShelterData.StructuralIntegrity = 0.4f;
            break;
        case EArch_ShelterCondition::Ruined:
            ShelterData.WeatheringLevel = 0.9f;
            ShelterData.MossGrowth = 0.8f;
            ShelterData.StructuralIntegrity = 0.2f;
            break;
        case EArch_ShelterCondition::Collapsed:
            ShelterData.WeatheringLevel = 1.0f;
            ShelterData.MossGrowth = 1.0f;
            ShelterData.StructuralIntegrity = 0.0f;
            ShelterData.bCanProvideProtection = false;
            break;
    }

    UpdateMaterialParameters();
}

void UArch_ShelterSystem::UpdateShelterCondition(float DeltaTime)
{
    if (!bEnableWeathering && !bEnableMossGrowth)
    {
        return;
    }

    bool bMaterialsNeedUpdate = false;

    // Apply weathering over time
    if (bEnableWeathering && ShelterData.WeatheringLevel < 1.0f)
    {
        float WeatheringIncrease = WeatheringRate * DeltaTime;
        ShelterData.WeatheringLevel = FMath::Clamp(ShelterData.WeatheringLevel + WeatheringIncrease, 0.0f, 1.0f);
        bMaterialsNeedUpdate = true;
    }

    // Apply moss growth over time
    if (bEnableMossGrowth && ShelterData.MossGrowth < 1.0f)
    {
        float MossIncrease = MossGrowthRate * DeltaTime;
        ShelterData.MossGrowth = FMath::Clamp(ShelterData.MossGrowth + MossIncrease, 0.0f, 1.0f);
        bMaterialsNeedUpdate = true;
    }

    // Update structural integrity based on weathering
    float IntegrityLoss = ShelterData.WeatheringLevel * 0.3f; // Max 30% integrity loss from weathering
    ShelterData.StructuralIntegrity = FMath::Clamp(1.0f - IntegrityLoss, 0.0f, 1.0f);

    // Update condition enum based on weathering level
    if (ShelterData.WeatheringLevel >= 0.9f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Ruined;
    }
    else if (ShelterData.WeatheringLevel >= 0.6f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Damaged;
    }
    else if (ShelterData.WeatheringLevel >= 0.2f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Weathered;
    }
    else
    {
        ShelterData.Condition = EArch_ShelterCondition::Pristine;
    }

    // Update protection capability
    ShelterData.bCanProvideProtection = ShelterData.StructuralIntegrity > 0.1f;

    if (bMaterialsNeedUpdate)
    {
        UpdateMaterialParameters();
    }
}

void UArch_ShelterSystem::ApplyWeathering(float WeatheringAmount)
{
    ShelterData.WeatheringLevel = FMath::Clamp(ShelterData.WeatheringLevel + WeatheringAmount, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void UArch_ShelterSystem::ApplyMossGrowth(float MossAmount)
{
    ShelterData.MossGrowth = FMath::Clamp(ShelterData.MossGrowth + MossAmount, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void UArch_ShelterSystem::UpdateMaterialParameters()
{
    if (DynamicMaterials.Num() == 0)
    {
        CreateDynamicMaterials();
    }

    FLinearColor WeatheringColor = GetWeatheringColor();
    float MossOpacity = GetMossOpacity();

    for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
    {
        if (DynMat)
        {
            // Set weathering parameters
            DynMat->SetScalarParameterValue(TEXT("WeatheringLevel"), ShelterData.WeatheringLevel);
            DynMat->SetVectorParameterValue(TEXT("WeatheringColor"), WeatheringColor);
            
            // Set moss parameters
            DynMat->SetScalarParameterValue(TEXT("MossGrowth"), ShelterData.MossGrowth);
            DynMat->SetScalarParameterValue(TEXT("MossOpacity"), MossOpacity);
            
            // Set structural integrity (affects opacity/visibility)
            DynMat->SetScalarParameterValue(TEXT("StructuralIntegrity"), ShelterData.StructuralIntegrity);
        }
    }
}

bool UArch_ShelterSystem::CanProvideProtection() const
{
    return ShelterData.bCanProvideProtection && ShelterData.StructuralIntegrity > 0.1f;
}

float UArch_ShelterSystem::GetProtectionValue() const
{
    if (!CanProvideProtection())
    {
        return 0.0f;
    }

    // Protection value is based on structural integrity and shelter type
    float BaseProtection = 1.0f;
    
    switch (ShelterData.ShelterType)
    {
        case EArch_ShelterType::StoneRuin:
            BaseProtection = 0.8f;
            break;
        case EArch_ShelterType::WoodLeanTo:
            BaseProtection = 0.4f;
            break;
        case EArch_ShelterType::CaveEntrance:
            BaseProtection = 0.9f;
            break;
        case EArch_ShelterType::RockOverhang:
            BaseProtection = 0.6f;
            break;
        case EArch_ShelterType::BuriedShelter:
            BaseProtection = 0.7f;
            break;
        default:
            BaseProtection = 0.5f;
            break;
    }

    return BaseProtection * ShelterData.StructuralIntegrity;
}

void UArch_ShelterSystem::SetShelterType(EArch_ShelterType NewType)
{
    ShelterData.ShelterType = NewType;
    UpdateMaterialParameters();
}

void UArch_ShelterSystem::SetCondition(EArch_ShelterCondition NewCondition)
{
    InitializeShelter(ShelterData.ShelterType, NewCondition);
}

void UArch_ShelterSystem::RepairShelter(float RepairAmount)
{
    ShelterData.WeatheringLevel = FMath::Clamp(ShelterData.WeatheringLevel - RepairAmount, 0.0f, 1.0f);
    ShelterData.MossGrowth = FMath::Clamp(ShelterData.MossGrowth - (RepairAmount * 0.5f), 0.0f, 1.0f);
    ShelterData.StructuralIntegrity = FMath::Clamp(ShelterData.StructuralIntegrity + RepairAmount, 0.0f, 1.0f);
    ShelterData.bCanProvideProtection = ShelterData.StructuralIntegrity > 0.1f;
    
    UpdateMaterialParameters();
}

void UArch_ShelterSystem::DamageShelter(float DamageAmount)
{
    ShelterData.StructuralIntegrity = FMath::Clamp(ShelterData.StructuralIntegrity - DamageAmount, 0.0f, 1.0f);
    ShelterData.WeatheringLevel = FMath::Clamp(ShelterData.WeatheringLevel + (DamageAmount * 0.3f), 0.0f, 1.0f);
    
    if (ShelterData.StructuralIntegrity <= 0.1f)
    {
        ShelterData.bCanProvideProtection = false;
    }
    
    UpdateMaterialParameters();
}

TArray<FVector> UArch_ShelterSystem::GetShelterBounds() const
{
    TArray<FVector> BoundPoints;
    
    if (AActor* Owner = GetOwner())
    {
        FVector Origin, BoxExtent;
        Owner->GetActorBounds(false, Origin, BoxExtent);
        
        // Create 8 corner points of the bounding box
        BoundPoints.Add(Origin + FVector(BoxExtent.X, BoxExtent.Y, BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(BoxExtent.X, BoxExtent.Y, -BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(BoxExtent.X, -BoxExtent.Y, BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(-BoxExtent.X, BoxExtent.Y, BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(-BoxExtent.X, BoxExtent.Y, -BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(-BoxExtent.X, -BoxExtent.Y, BoxExtent.Z));
        BoundPoints.Add(Origin + FVector(-BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z));
    }
    
    return BoundPoints;
}

bool UArch_ShelterSystem::IsPlayerInShelter(const FVector& PlayerLocation) const
{
    if (!CanProvideProtection())
    {
        return false;
    }

    if (AActor* Owner = GetOwner())
    {
        float Distance = FVector::Dist(Owner->GetActorLocation(), PlayerLocation);
        return Distance <= ShelterData.ProtectionRadius;
    }

    return false;
}

void UArch_ShelterSystem::CreateDynamicMaterials()
{
    DynamicMaterials.Empty();

    for (UStaticMeshComponent* MeshComp : ShelterMeshComponents)
    {
        if (MeshComp)
        {
            int32 MaterialCount = MeshComp->GetNumMaterials();
            for (int32 i = 0; i < MaterialCount; i++)
            {
                UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(i);
                if (BaseMaterial)
                {
                    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                    if (DynMat)
                    {
                        MeshComp->SetMaterial(i, DynMat);
                        DynamicMaterials.Add(DynMat);
                    }
                }
            }
        }
    }
}

FLinearColor UArch_ShelterSystem::GetWeatheringColor() const
{
    // Interpolate between clean stone color and weathered brown/gray
    FLinearColor CleanColor = FLinearColor(0.8f, 0.8f, 0.7f, 1.0f); // Light gray-beige
    FLinearColor WeatheredColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Dark brown-gray
    
    return FLinearColor::LerpUsingHSV(CleanColor, WeatheredColor, ShelterData.WeatheringLevel);
}

float UArch_ShelterSystem::GetMossOpacity() const
{
    // Moss opacity increases with moss growth, but caps at 0.8 for realism
    return FMath::Clamp(ShelterData.MossGrowth * 0.8f, 0.0f, 0.8f);
}