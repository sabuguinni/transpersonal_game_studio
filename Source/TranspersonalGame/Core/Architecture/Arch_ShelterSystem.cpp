#include "Arch_ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UArch_ShelterSystem::UArch_ShelterSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds for performance

    // Initialize default shelter properties
    ShelterData = FArch_ShelterProperties();
    WeatheringRate = 0.001f;
    RainDamageMultiplier = 1.5f;
    WindDamageMultiplier = 1.2f;
    CurrentOccupants = 0;
    AccumulatedDamage = 0.0f;
}

void UArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize visual components if they exist
    if (AActor* Owner = GetOwner())
    {
        MainStructureMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
        InteriorVolume = Owner->FindComponentByClass<UBoxComponent>();
        
        if (!InteriorVolume)
        {
            // Create interior volume if it doesn't exist
            InteriorVolume = NewObject<UBoxComponent>(Owner);
            InteriorVolume->SetupAttachment(Owner->GetRootComponent());
            InteriorVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
            InteriorVolume->RegisterComponent();
        }
    }

    UpdateVisualState();
}

void UArch_ShelterSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Apply gradual weathering
    if (ShelterData.Condition != EArch_ShelterCondition::Collapsed)
    {
        AccumulatedDamage += WeatheringRate * DeltaTime;
        
        if (AccumulatedDamage > 10.0f)
        {
            AccumulatedDamage = 0.0f;
            UpdateShelterCondition();
        }
    }
}

void UArch_ShelterSystem::InitializeShelter(EArch_ShelterType Type, EArch_ShelterCondition InitialCondition)
{
    ShelterData.ShelterType = Type;
    ShelterData.Condition = InitialCondition;

    // Set type-specific properties
    switch (Type)
    {
        case EArch_ShelterType::CircularStone:
            ShelterData.WeatherProtection = 0.85f;
            ShelterData.TemperatureInsulation = 0.7f;
            ShelterData.MaxOccupants = 6;
            ShelterData.StructuralIntegrity = 0.9f;
            ShelterData.bHasFirePit = true;
            break;

        case EArch_ShelterType::RectangularStone:
            ShelterData.WeatherProtection = 0.8f;
            ShelterData.TemperatureInsulation = 0.75f;
            ShelterData.MaxOccupants = 8;
            ShelterData.StructuralIntegrity = 0.85f;
            ShelterData.bHasFirePit = true;
            break;

        case EArch_ShelterType::NaturalCave:
            ShelterData.WeatherProtection = 0.95f;
            ShelterData.TemperatureInsulation = 0.9f;
            ShelterData.MaxOccupants = 12;
            ShelterData.StructuralIntegrity = 1.0f;
            ShelterData.bHasFirePit = false;
            break;

        case EArch_ShelterType::RockOverhang:
            ShelterData.WeatherProtection = 0.6f;
            ShelterData.TemperatureInsulation = 0.4f;
            ShelterData.MaxOccupants = 4;
            ShelterData.StructuralIntegrity = 0.95f;
            ShelterData.bHasFirePit = false;
            break;

        case EArch_ShelterType::TreeHollow:
            ShelterData.WeatherProtection = 0.5f;
            ShelterData.TemperatureInsulation = 0.6f;
            ShelterData.MaxOccupants = 2;
            ShelterData.StructuralIntegrity = 0.6f;
            ShelterData.bHasFirePit = false;
            break;

        case EArch_ShelterType::CliffAlcove:
            ShelterData.WeatherProtection = 0.75f;
            ShelterData.TemperatureInsulation = 0.5f;
            ShelterData.MaxOccupants = 3;
            ShelterData.StructuralIntegrity = 0.9f;
            ShelterData.bHasFirePit = false;
            break;
    }

    // Apply condition modifiers
    float ConditionMultiplier = 1.0f;
    switch (InitialCondition)
    {
        case EArch_ShelterCondition::Pristine:
            ConditionMultiplier = 1.0f;
            break;
        case EArch_ShelterCondition::Good:
            ConditionMultiplier = 0.9f;
            break;
        case EArch_ShelterCondition::Weathered:
            ConditionMultiplier = 0.75f;
            break;
        case EArch_ShelterCondition::Damaged:
            ConditionMultiplier = 0.5f;
            break;
        case EArch_ShelterCondition::Ruined:
            ConditionMultiplier = 0.25f;
            break;
        case EArch_ShelterCondition::Collapsed:
            ConditionMultiplier = 0.1f;
            break;
    }

    ShelterData.WeatherProtection *= ConditionMultiplier;
    ShelterData.TemperatureInsulation *= ConditionMultiplier;
    ShelterData.StructuralIntegrity *= ConditionMultiplier;

    UpdateVisualState();
}

bool UArch_ShelterSystem::CanProvideWeatherProtection() const
{
    return ShelterData.WeatherProtection > 0.3f && ShelterData.Condition != EArch_ShelterCondition::Collapsed;
}

float UArch_ShelterSystem::GetTemperatureModifier() const
{
    if (ShelterData.Condition == EArch_ShelterCondition::Collapsed)
    {
        return 0.0f;
    }
    return ShelterData.TemperatureInsulation;
}

int32 UArch_ShelterSystem::GetAvailableSpace() const
{
    if (ShelterData.Condition == EArch_ShelterCondition::Collapsed)
    {
        return 0;
    }
    return FMath::Max(0, ShelterData.MaxOccupants - CurrentOccupants);
}

void UArch_ShelterSystem::ApplyWeatherDamage(float DeltaTime, bool bIsRaining, float WindStrength)
{
    if (ShelterData.Condition == EArch_ShelterCondition::Collapsed)
    {
        return;
    }

    float DamageAmount = WeatheringRate * DeltaTime;

    if (bIsRaining)
    {
        DamageAmount *= RainDamageMultiplier;
    }

    if (WindStrength > 0.5f)
    {
        DamageAmount *= WindDamageMultiplier * WindStrength;
    }

    // Natural shelters are more resistant to weather
    if (ShelterData.ShelterType == EArch_ShelterType::NaturalCave || 
        ShelterData.ShelterType == EArch_ShelterType::RockOverhang)
    {
        DamageAmount *= 0.1f;
    }

    AccumulatedDamage += DamageAmount;
    
    if (AccumulatedDamage > 5.0f)
    {
        UpdateShelterCondition();
        AccumulatedDamage = 0.0f;
    }
}

void UArch_ShelterSystem::RepairShelter(float RepairAmount)
{
    if (ShelterData.Condition == EArch_ShelterCondition::Collapsed)
    {
        return; // Cannot repair collapsed structures
    }

    AccumulatedDamage = FMath::Max(0.0f, AccumulatedDamage - RepairAmount);
    
    // Potentially improve condition
    if (AccumulatedDamage <= 0.0f)
    {
        int32 CurrentConditionValue = static_cast<int32>(ShelterData.Condition);
        if (CurrentConditionValue > 0)
        {
            ShelterData.Condition = static_cast<EArch_ShelterCondition>(CurrentConditionValue - 1);
            UpdateVisualState();
        }
    }
}

FVector UArch_ShelterSystem::GetOptimalFirePitLocation() const
{
    if (!ShelterData.bHasFirePit || !InteriorVolume)
    {
        return FVector::ZeroVector;
    }

    FVector InteriorCenter = CalculateInteriorCenter();
    
    // Place fire pit slightly off-center for realism
    FVector Offset = FVector(50.0f, 0.0f, -100.0f);
    return InteriorCenter + Offset;
}

TArray<FVector> UArch_ShelterSystem::GetStorageLocations() const
{
    TArray<FVector> StorageLocations;
    
    if (!ShelterData.bHasStorageArea || !InteriorVolume)
    {
        return StorageLocations;
    }

    FVector InteriorCenter = CalculateInteriorCenter();
    FVector BoxExtent = InteriorVolume->GetScaledBoxExtent();

    // Create storage locations along the walls
    StorageLocations.Add(InteriorCenter + FVector(BoxExtent.X * 0.8f, 0.0f, -BoxExtent.Z * 0.5f));
    StorageLocations.Add(InteriorCenter + FVector(-BoxExtent.X * 0.8f, 0.0f, -BoxExtent.Z * 0.5f));
    StorageLocations.Add(InteriorCenter + FVector(0.0f, BoxExtent.Y * 0.8f, -BoxExtent.Z * 0.5f));
    StorageLocations.Add(InteriorCenter + FVector(0.0f, -BoxExtent.Y * 0.8f, -BoxExtent.Z * 0.5f));

    return StorageLocations;
}

void UArch_ShelterSystem::UpdateShelterCondition()
{
    int32 CurrentConditionValue = static_cast<int32>(ShelterData.Condition);
    
    if (AccumulatedDamage > 20.0f && CurrentConditionValue < 5)
    {
        ShelterData.Condition = static_cast<EArch_ShelterCondition>(CurrentConditionValue + 1);
        UpdateVisualState();
    }
}

void UArch_ShelterSystem::UpdateVisualState()
{
    // This would typically update material parameters, mesh visibility, etc.
    // For now, we'll log the state change
    if (GEngine)
    {
        FString ConditionString;
        switch (ShelterData.Condition)
        {
            case EArch_ShelterCondition::Pristine: ConditionString = TEXT("Pristine"); break;
            case EArch_ShelterCondition::Good: ConditionString = TEXT("Good"); break;
            case EArch_ShelterCondition::Weathered: ConditionString = TEXT("Weathered"); break;
            case EArch_ShelterCondition::Damaged: ConditionString = TEXT("Damaged"); break;
            case EArch_ShelterCondition::Ruined: ConditionString = TEXT("Ruined"); break;
            case EArch_ShelterCondition::Collapsed: ConditionString = TEXT("Collapsed"); break;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Shelter condition updated to: %s"), *ConditionString));
    }
}

FVector UArch_ShelterSystem::CalculateInteriorCenter() const
{
    if (InteriorVolume)
    {
        return InteriorVolume->GetComponentLocation();
    }
    
    if (AActor* Owner = GetOwner())
    {
        return Owner->GetActorLocation();
    }
    
    return FVector::ZeroVector;
}