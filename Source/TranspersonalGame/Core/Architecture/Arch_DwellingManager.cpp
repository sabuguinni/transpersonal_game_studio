#include "Arch_DwellingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AArch_DwellingManager::AArch_DwellingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);

    // Create fire pit mesh
    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(RootComponent);

    // Initialize default configuration
    DwellingConfig.DwellingType = EArch_DwellingType::SimpleHut;
    DwellingConfig.CurrentState = EArch_DwellingState::Pristine;
    DwellingConfig.StructuralIntegrity = 100.0f;
    DwellingConfig.WeatheringRate = 0.1f;
    DwellingConfig.bHasFirePit = true;
    DwellingConfig.MaxOccupants = 4;

    // Load default meshes
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MainStructureMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderMeshAsset.Succeeded())
    {
        FirePitMesh->SetStaticMesh(CylinderMeshAsset.Object);
        FirePitMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.3f));
        FirePitMesh->SetRelativeLocation(FVector(0, 0, -50));
    }
}

void AArch_DwellingManager::BeginPlay()
{
    Super::BeginPlay();
    
    CreateInteriorLayout();
    UpdateMaterialBasedOnState();
}

void AArch_DwellingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Apply weathering over time
    ApplyWeathering(DeltaTime);
}

void AArch_DwellingManager::InitializeDwelling(EArch_DwellingType Type, FVector Location)
{
    DwellingConfig.DwellingType = Type;
    SetActorLocation(Location);

    // Adjust scale and properties based on dwelling type
    switch (Type)
    {
        case EArch_DwellingType::SimpleHut:
            MainStructureMesh->SetRelativeScale3D(FVector(3, 3, 2));
            DwellingConfig.MaxOccupants = 2;
            break;
        case EArch_DwellingType::StoneArchway:
            MainStructureMesh->SetRelativeScale3D(FVector(4, 1, 3));
            DwellingConfig.MaxOccupants = 0; // Just an entrance
            DwellingConfig.bHasFirePit = false;
            break;
        case EArch_DwellingType::CaveEntrance:
            MainStructureMesh->SetRelativeScale3D(FVector(2, 2, 2.5));
            DwellingConfig.MaxOccupants = 6;
            break;
        case EArch_DwellingType::RockShelter:
            MainStructureMesh->SetRelativeScale3D(FVector(5, 3, 2));
            DwellingConfig.MaxOccupants = 8;
            break;
        case EArch_DwellingType::TribalLodge:
            MainStructureMesh->SetRelativeScale3D(FVector(6, 6, 3));
            DwellingConfig.MaxOccupants = 12;
            break;
    }

    CreateInteriorLayout();
}

void AArch_DwellingManager::UpdateStructuralIntegrity(float DeltaIntegrity)
{
    DwellingConfig.StructuralIntegrity = FMath::Clamp(
        DwellingConfig.StructuralIntegrity + DeltaIntegrity, 
        0.0f, 
        100.0f
    );

    // Update state based on integrity
    if (DwellingConfig.StructuralIntegrity <= 0.0f)
    {
        SetDwellingState(EArch_DwellingState::Collapsed);
    }
    else if (DwellingConfig.StructuralIntegrity <= 25.0f)
    {
        SetDwellingState(EArch_DwellingState::Ruined);
    }
    else if (DwellingConfig.StructuralIntegrity <= 50.0f)
    {
        SetDwellingState(EArch_DwellingState::Damaged);
    }
    else if (DwellingConfig.StructuralIntegrity <= 75.0f)
    {
        SetDwellingState(EArch_DwellingState::Weathered);
    }
    else
    {
        SetDwellingState(EArch_DwellingState::Pristine);
    }
}

void AArch_DwellingManager::ApplyWeathering(float DeltaTime)
{
    if (DwellingConfig.CurrentState != EArch_DwellingState::Collapsed)
    {
        float WeatheringDamage = DwellingConfig.WeatheringRate * DeltaTime;
        UpdateStructuralIntegrity(-WeatheringDamage);
    }
}

void AArch_DwellingManager::SetDwellingState(EArch_DwellingState NewState)
{
    if (DwellingConfig.CurrentState != NewState)
    {
        DwellingConfig.CurrentState = NewState;
        UpdateMaterialBasedOnState();

        if (NewState == EArch_DwellingState::Collapsed)
        {
            HandleStructuralCollapse();
        }
    }
}

bool AArch_DwellingManager::CanAccommodateOccupants(int32 OccupantCount) const
{
    return OccupantCount <= DwellingConfig.MaxOccupants && 
           DwellingConfig.CurrentState != EArch_DwellingState::Collapsed;
}

void AArch_DwellingManager::CreateInteriorLayout()
{
    if (DwellingConfig.bHasFirePit)
    {
        SpawnFirePit();
    }

    if (DwellingConfig.MaxOccupants > 0)
    {
        AddSleepingAreas(DwellingConfig.MaxOccupants);
    }
}

void AArch_DwellingManager::SpawnFirePit()
{
    FirePitMesh->SetVisibility(true);
    FirePitMesh->SetRelativeLocation(FVector(0, 0, -50));
}

void AArch_DwellingManager::AddSleepingAreas(int32 Count)
{
    // Create sleeping areas around the perimeter
    for (int32 i = 0; i < Count; ++i)
    {
        float Angle = (360.0f / Count) * i;
        FVector SleepingPosition = GetRandomInteriorPosition();
        
        // For now, we'll just log the sleeping area positions
        // In a full implementation, these would spawn actual sleeping platform meshes
        UE_LOG(LogTemp, Log, TEXT("Sleeping area %d at position: %s"), i, *SleepingPosition.ToString());
    }
}

void AArch_DwellingManager::UpdateMaterialBasedOnState()
{
    if (StateMaterials.Num() > static_cast<int32>(DwellingConfig.CurrentState))
    {
        UMaterialInterface* StateMaterial = StateMaterials[static_cast<int32>(DwellingConfig.CurrentState)];
        if (StateMaterial && MainStructureMesh)
        {
            MainStructureMesh->SetMaterial(0, StateMaterial);
        }
    }
}

void AArch_DwellingManager::HandleStructuralCollapse()
{
    // Reduce scale to simulate collapse
    FVector CurrentScale = MainStructureMesh->GetRelativeScale3D();
    MainStructureMesh->SetRelativeScale3D(FVector(CurrentScale.X, CurrentScale.Y, CurrentScale.Z * 0.3f));
    
    // Disable fire pit
    if (FirePitMesh)
    {
        FirePitMesh->SetVisibility(false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Dwelling has collapsed due to structural failure!"));
}

FVector AArch_DwellingManager::GetRandomInteriorPosition() const
{
    FVector Scale = MainStructureMesh->GetRelativeScale3D();
    float MaxRadius = FMath::Min(Scale.X, Scale.Y) * 50.0f; // Convert to reasonable radius
    
    FVector RandomPosition = UKismetMathLibrary::RandomUnitVector() * UKismetMathLibrary::RandomFloatInRange(50.0f, MaxRadius);
    RandomPosition.Z = 0; // Keep on ground level
    
    return RandomPosition;
}