#include "Arch_ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AArch_ShelterSystem::AArch_ShelterSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create shelter mesh component
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    ShelterMesh->SetupAttachment(RootComponent);
    ShelterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShelterMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create interaction volume
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(RootComponent);
    InteractionVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Initialize default shelter data
    ShelterData = FArch_ShelterData();
    WeatheringRate = 1.0f;
    RainDamageMultiplier = 2.0f;
    WindDamageMultiplier = 1.5f;
    LastWeatheringUpdate = 0.0f;
    WeatheringUpdateInterval = 5.0f;
}

void AArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateShelterMesh();
    UpdateVisualCondition();
}

void AArch_ShelterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastWeatheringUpdate += DeltaTime;
    if (LastWeatheringUpdate >= WeatheringUpdateInterval)
    {
        // Apply natural weathering over time
        ApplyWeathering(0.1f, LastWeatheringUpdate);
        LastWeatheringUpdate = 0.0f;
    }

    // Check for structural failure
    CheckStructuralFailure();
}

void AArch_ShelterSystem::InitializeShelter(EArch_ShelterType Type, const FVector& Location)
{
    ShelterData.ShelterType = Type;
    SetActorLocation(Location);

    // Set type-specific properties
    switch (Type)
    {
        case EArch_ShelterType::BasicLeanTo:
            ShelterData.StructuralIntegrity = 60.0f;
            ShelterData.ProtectionValue = 30.0f;
            ShelterData.MaxOccupants = 1;
            WeatheringRate = 2.0f;
            break;

        case EArch_ShelterType::StoneFoundation:
            ShelterData.StructuralIntegrity = 90.0f;
            ShelterData.ProtectionValue = 70.0f;
            ShelterData.MaxOccupants = 3;
            WeatheringRate = 0.5f;
            break;

        case EArch_ShelterType::CaveEntrance:
            ShelterData.StructuralIntegrity = 100.0f;
            ShelterData.ProtectionValue = 90.0f;
            ShelterData.MaxOccupants = 5;
            WeatheringRate = 0.1f;
            break;

        case EArch_ShelterType::TreeHollow:
            ShelterData.StructuralIntegrity = 70.0f;
            ShelterData.ProtectionValue = 50.0f;
            ShelterData.MaxOccupants = 2;
            WeatheringRate = 1.5f;
            break;

        case EArch_ShelterType::RockOverhang:
            ShelterData.StructuralIntegrity = 95.0f;
            ShelterData.ProtectionValue = 80.0f;
            ShelterData.MaxOccupants = 4;
            WeatheringRate = 0.3f;
            break;

        case EArch_ShelterType::BuriedShelter:
            ShelterData.StructuralIntegrity = 80.0f;
            ShelterData.ProtectionValue = 85.0f;
            ShelterData.MaxOccupants = 2;
            WeatheringRate = 0.8f;
            break;

        default:
            break;
    }

    UpdateShelterMesh();
    UpdateVisualCondition();

    UE_LOG(LogTemp, Log, TEXT("Shelter initialized: Type=%d, Integrity=%.1f, Protection=%.1f"), 
           (int32)Type, ShelterData.StructuralIntegrity, ShelterData.ProtectionValue);
}

void AArch_ShelterSystem::ApplyWeathering(float WeatherIntensity, float DeltaTime)
{
    if (ShelterData.Condition == EArch_ShelterCondition::Collapsed)
    {
        return;
    }

    float WeatheringDamage = WeatherIntensity * WeatheringRate * DeltaTime;
    
    // Apply weather-specific multipliers
    // Note: In a full implementation, this would check actual weather conditions
    WeatheringDamage *= RainDamageMultiplier * 0.5f; // Assume moderate rain
    
    ShelterData.WeatheringLevel = FMath::Clamp(ShelterData.WeatheringLevel + WeatheringDamage, 0.0f, 100.0f);
    ShelterData.StructuralIntegrity = FMath::Clamp(ShelterData.StructuralIntegrity - WeatheringDamage * 0.5f, 0.0f, 100.0f);

    // Update condition based on weathering level
    if (ShelterData.WeatheringLevel < 20.0f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Good;
    }
    else if (ShelterData.WeatheringLevel < 40.0f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Weathered;
    }
    else if (ShelterData.WeatheringLevel < 70.0f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Damaged;
    }
    else if (ShelterData.WeatheringLevel < 90.0f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Ruined;
    }

    // Update protection value based on condition
    float ConditionMultiplier = 1.0f - (ShelterData.WeatheringLevel / 100.0f);
    ShelterData.ProtectionValue = FMath::Max(ShelterData.ProtectionValue * ConditionMultiplier, 5.0f);

    UpdateVisualCondition();
}

void AArch_ShelterSystem::RepairShelter(float RepairAmount)
{
    if (ShelterData.Condition == EArch_ShelterCondition::Collapsed)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot repair collapsed shelter"));
        return;
    }

    ShelterData.WeatheringLevel = FMath::Clamp(ShelterData.WeatheringLevel - RepairAmount, 0.0f, 100.0f);
    ShelterData.StructuralIntegrity = FMath::Clamp(ShelterData.StructuralIntegrity + RepairAmount * 0.8f, 0.0f, 100.0f);

    // Recalculate condition
    if (ShelterData.WeatheringLevel < 10.0f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Good;
    }
    else if (ShelterData.WeatheringLevel < 30.0f)
    {
        ShelterData.Condition = EArch_ShelterCondition::Weathered;
    }

    UpdateVisualCondition();
    UE_LOG(LogTemp, Log, TEXT("Shelter repaired: Weathering=%.1f, Integrity=%.1f"), 
           ShelterData.WeatheringLevel, ShelterData.StructuralIntegrity);
}

bool AArch_ShelterSystem::CanProvideProtection() const
{
    return ShelterData.Condition != EArch_ShelterCondition::Collapsed && 
           ShelterData.StructuralIntegrity > 10.0f;
}

float AArch_ShelterSystem::GetProtectionValue() const
{
    if (!CanProvideProtection())
    {
        return 0.0f;
    }
    return ShelterData.ProtectionValue;
}

void AArch_ShelterSystem::UpdateVisualCondition()
{
    if (!ShelterMesh)
    {
        return;
    }

    // Update materials based on weathering level
    if (WeatheringMaterials.Num() > 0)
    {
        int32 MaterialIndex = FMath::Clamp(
            FMath::FloorToInt(ShelterData.WeatheringLevel / 25.0f), 
            0, 
            WeatheringMaterials.Num() - 1
        );

        if (WeatheringMaterials.IsValidIndex(MaterialIndex))
        {
            ShelterMesh->SetMaterial(0, WeatheringMaterials[MaterialIndex]);
        }
    }

    // Create dynamic material for weathering effects
    UMaterialInstanceDynamic* DynamicMaterial = ShelterMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), ShelterData.WeatheringLevel / 100.0f);
        DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), ShelterData.WeatheringLevel / 100.0f * 0.7f);
        DynamicMaterial->SetScalarParameterValue(TEXT("Damage"), (100.0f - ShelterData.StructuralIntegrity) / 100.0f);
    }
}

void AArch_ShelterSystem::AddOccupant(AActor* Occupant)
{
    if (!Occupant || CurrentOccupants.Contains(Occupant))
    {
        return;
    }

    if (CurrentOccupants.Num() >= ShelterData.MaxOccupants)
    {
        UE_LOG(LogTemp, Warning, TEXT("Shelter is at maximum capacity"));
        return;
    }

    CurrentOccupants.Add(Occupant);
    UE_LOG(LogTemp, Log, TEXT("Occupant added to shelter: %s (%d/%d)"), 
           *Occupant->GetName(), CurrentOccupants.Num(), ShelterData.MaxOccupants);
}

void AArch_ShelterSystem::RemoveOccupant(AActor* Occupant)
{
    if (CurrentOccupants.Remove(Occupant) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Occupant removed from shelter: %s (%d/%d)"), 
               *Occupant->GetName(), CurrentOccupants.Num(), ShelterData.MaxOccupants);
    }
}

bool AArch_ShelterSystem::HasSpace() const
{
    return CurrentOccupants.Num() < ShelterData.MaxOccupants && CanProvideProtection();
}

void AArch_ShelterSystem::UpdateShelterMesh()
{
    if (!ShelterMesh || ShelterMeshVariants.Num() == 0)
    {
        return;
    }

    int32 MeshIndex = (int32)ShelterData.ShelterType;
    if (ShelterMeshVariants.IsValidIndex(MeshIndex) && ShelterMeshVariants[MeshIndex])
    {
        ShelterMesh->SetStaticMesh(ShelterMeshVariants[MeshIndex]);
    }
}

void AArch_ShelterSystem::UpdateMaterials()
{
    UpdateVisualCondition();
}

void AArch_ShelterSystem::CheckStructuralFailure()
{
    if (ShelterData.StructuralIntegrity <= 0.0f && ShelterData.Condition != EArch_ShelterCondition::Collapsed)
    {
        ShelterData.Condition = EArch_ShelterCondition::Collapsed;
        ShelterData.ProtectionValue = 0.0f;
        
        // Remove all occupants
        for (AActor* Occupant : CurrentOccupants)
        {
            if (Occupant)
            {
                UE_LOG(LogTemp, Warning, TEXT("Shelter collapsed! Occupant %s ejected"), *Occupant->GetName());
            }
        }
        CurrentOccupants.Empty();

        UpdateVisualCondition();
        UE_LOG(LogTemp, Warning, TEXT("Shelter has collapsed due to structural failure"));
    }
}