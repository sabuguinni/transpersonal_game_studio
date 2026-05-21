#include "Arch_ShelterSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AArch_ShelterSystem::AArch_ShelterSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create mesh components
    WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
    WallMesh->SetupAttachment(RootComponent);

    RoofMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofMesh"));
    RoofMesh->SetupAttachment(RootComponent);

    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    FloorMesh->SetupAttachment(RootComponent);

    // Initialize default values
    ShelterConfig = FArch_ShelterConfig();
    WeatherDamageRate = 1.0f;
    RepairEfficiency = 5.0f;
    CurrentOccupants.Empty();

    // Set default mesh positions
    if (WallMesh)
    {
        WallMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    }

    if (RoofMesh)
    {
        RoofMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    }

    if (FloorMesh)
    {
        FloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    }
}

void AArch_ShelterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeShelter(ShelterConfig.ShelterType);
    UpdateMeshVisibility();
    ApplyStateEffects();
}

void AArch_ShelterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    ProcessDegradation(DeltaTime);
    UpdateShelterState();
}

void AArch_ShelterSystem::InitializeShelter(EArch_ShelterType NewType)
{
    ShelterConfig.ShelterType = NewType;

    // Configure shelter based on type
    switch (NewType)
    {
        case EArch_ShelterType::StoneHut:
            ShelterConfig.StructuralIntegrity = 100.0f;
            ShelterConfig.WeatherResistance = 90.0f;
            ShelterConfig.InsulationValue = 80.0f;
            ShelterConfig.MaxOccupants = 4;
            break;

        case EArch_ShelterType::WoodLean:
            ShelterConfig.StructuralIntegrity = 70.0f;
            ShelterConfig.WeatherResistance = 60.0f;
            ShelterConfig.InsulationValue = 50.0f;
            ShelterConfig.MaxOccupants = 2;
            break;

        case EArch_ShelterType::CaveEntrance:
            ShelterConfig.StructuralIntegrity = 95.0f;
            ShelterConfig.WeatherResistance = 100.0f;
            ShelterConfig.InsulationValue = 90.0f;
            ShelterConfig.MaxOccupants = 6;
            break;

        case EArch_ShelterType::BoneFrame:
            ShelterConfig.StructuralIntegrity = 60.0f;
            ShelterConfig.WeatherResistance = 40.0f;
            ShelterConfig.InsulationValue = 30.0f;
            ShelterConfig.MaxOccupants = 3;
            break;

        case EArch_ShelterType::HideWall:
            ShelterConfig.StructuralIntegrity = 50.0f;
            ShelterConfig.WeatherResistance = 30.0f;
            ShelterConfig.InsulationValue = 60.0f;
            ShelterConfig.MaxOccupants = 2;
            break;

        default:
            ShelterConfig.StructuralIntegrity = 50.0f;
            ShelterConfig.WeatherResistance = 50.0f;
            ShelterConfig.InsulationValue = 50.0f;
            ShelterConfig.MaxOccupants = 1;
            break;
    }

    UpdateMeshVisibility();
}

void AArch_ShelterSystem::ApplyWeatherDamage(float DamageAmount)
{
    float ActualDamage = DamageAmount * (1.0f - (ShelterConfig.WeatherResistance / 100.0f));
    ShelterConfig.StructuralIntegrity = FMath::Max(0.0f, ShelterConfig.StructuralIntegrity - ActualDamage);
    
    UpdateShelterState();
}

void AArch_ShelterSystem::RepairShelter(float RepairAmount)
{
    float ActualRepair = RepairAmount * (RepairEfficiency / 10.0f);
    ShelterConfig.StructuralIntegrity = FMath::Min(100.0f, ShelterConfig.StructuralIntegrity + ActualRepair);
    
    UpdateShelterState();
}

bool AArch_ShelterSystem::CanEnterShelter(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }

    if (ShelterConfig.CurrentState == EArch_ShelterState::Collapsed)
    {
        return false;
    }

    if (CurrentOccupants.Num() >= ShelterConfig.MaxOccupants)
    {
        return false;
    }

    return true;
}

void AArch_ShelterSystem::EnterShelter(AActor* Actor)
{
    if (CanEnterShelter(Actor) && !CurrentOccupants.Contains(Actor))
    {
        CurrentOccupants.Add(Actor);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Actor entered shelter: %s"), *Actor->GetName()));
        }
    }
}

void AArch_ShelterSystem::ExitShelter(AActor* Actor)
{
    if (Actor && CurrentOccupants.Contains(Actor))
    {
        CurrentOccupants.Remove(Actor);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("Actor exited shelter: %s"), *Actor->GetName()));
        }
    }
}

float AArch_ShelterSystem::GetShelterEffectiveness() const
{
    float BaseEffectiveness = ShelterConfig.StructuralIntegrity / 100.0f;
    float WeatherBonus = ShelterConfig.WeatherResistance / 200.0f;
    float InsulationBonus = ShelterConfig.InsulationValue / 200.0f;
    
    return FMath::Clamp(BaseEffectiveness + WeatherBonus + InsulationBonus, 0.0f, 1.0f);
}

void AArch_ShelterSystem::UpdateShelterState()
{
    if (ShelterConfig.StructuralIntegrity > 80.0f)
    {
        ShelterConfig.CurrentState = EArch_ShelterState::Intact;
    }
    else if (ShelterConfig.StructuralIntegrity > 60.0f)
    {
        ShelterConfig.CurrentState = EArch_ShelterState::Weathered;
    }
    else if (ShelterConfig.StructuralIntegrity > 30.0f)
    {
        ShelterConfig.CurrentState = EArch_ShelterState::Damaged;
    }
    else if (ShelterConfig.StructuralIntegrity > 10.0f)
    {
        ShelterConfig.CurrentState = EArch_ShelterState::Ruined;
    }
    else
    {
        ShelterConfig.CurrentState = EArch_ShelterState::Collapsed;
        CurrentOccupants.Empty(); // Force evacuation
    }

    ApplyStateEffects();
}

void AArch_ShelterSystem::SetFirePit(bool bEnabled)
{
    ShelterConfig.bHasFirePit = bEnabled;
    
    if (bEnabled)
    {
        ShelterConfig.InsulationValue += 20.0f;
    }
    else
    {
        ShelterConfig.InsulationValue = FMath::Max(0.0f, ShelterConfig.InsulationValue - 20.0f);
    }
}

void AArch_ShelterSystem::SetStorageArea(bool bEnabled)
{
    ShelterConfig.bHasStorageArea = bEnabled;
    
    if (bEnabled)
    {
        ShelterConfig.MaxOccupants = FMath::Max(1, ShelterConfig.MaxOccupants - 1);
    }
    else
    {
        ShelterConfig.MaxOccupants += 1;
    }
}

void AArch_ShelterSystem::UpdateMeshVisibility()
{
    if (!WallMesh || !RoofMesh || !FloorMesh)
    {
        return;
    }

    bool bShouldBeVisible = (ShelterConfig.CurrentState != EArch_ShelterState::Collapsed);
    
    WallMesh->SetVisibility(bShouldBeVisible);
    RoofMesh->SetVisibility(bShouldBeVisible && ShelterConfig.CurrentState != EArch_ShelterState::Ruined);
    FloorMesh->SetVisibility(bShouldBeVisible);
}

void AArch_ShelterSystem::ApplyStateEffects()
{
    float StateMultiplier = 1.0f;
    
    switch (ShelterConfig.CurrentState)
    {
        case EArch_ShelterState::Intact:
            StateMultiplier = 1.0f;
            break;
        case EArch_ShelterState::Weathered:
            StateMultiplier = 0.9f;
            break;
        case EArch_ShelterState::Damaged:
            StateMultiplier = 0.7f;
            break;
        case EArch_ShelterState::Ruined:
            StateMultiplier = 0.4f;
            break;
        case EArch_ShelterState::Collapsed:
            StateMultiplier = 0.0f;
            break;
    }

    // Apply state effects to shelter properties
    float OriginalWeatherResistance = ShelterConfig.WeatherResistance;
    float OriginalInsulation = ShelterConfig.InsulationValue;
    
    // These are temporary effects, not permanent modifications
    // In a real implementation, you'd store base values separately
}

float AArch_ShelterSystem::CalculateWeatherProtection() const
{
    float BaseProtection = ShelterConfig.WeatherResistance / 100.0f;
    float StateModifier = 1.0f;
    
    switch (ShelterConfig.CurrentState)
    {
        case EArch_ShelterState::Intact:
            StateModifier = 1.0f;
            break;
        case EArch_ShelterState::Weathered:
            StateModifier = 0.85f;
            break;
        case EArch_ShelterState::Damaged:
            StateModifier = 0.6f;
            break;
        case EArch_ShelterState::Ruined:
            StateModifier = 0.3f;
            break;
        case EArch_ShelterState::Collapsed:
            StateModifier = 0.0f;
            break;
    }
    
    return BaseProtection * StateModifier;
}

void AArch_ShelterSystem::ProcessDegradation(float DeltaTime)
{
    // Apply gradual weather damage over time
    float DegradationRate = WeatherDamageRate * DeltaTime * 0.1f; // Very slow degradation
    
    // Increase degradation if shelter is occupied beyond capacity
    if (CurrentOccupants.Num() > ShelterConfig.MaxOccupants)
    {
        DegradationRate *= 2.0f;
    }
    
    ApplyWeatherDamage(DegradationRate);
}