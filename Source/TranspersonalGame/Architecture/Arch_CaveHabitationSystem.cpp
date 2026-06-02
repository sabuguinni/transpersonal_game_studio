#include "Arch_CaveHabitationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AArch_CaveHabitationSystem::AArch_CaveHabitationSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create cave mesh components
    CaveWallsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveWallsMesh"));
    CaveWallsMesh->SetupAttachment(RootComponent);

    CaveFloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveFloorMesh"));
    CaveFloorMesh->SetupAttachment(RootComponent);

    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(RootComponent);

    // Set default cave configuration
    CaveConfig.CaveType = EArch_CaveHabitationType::SimpleShell;
    CaveConfig.CaveDepth = 800.0f;
    CaveConfig.CaveWidth = 600.0f;
    CaveConfig.CaveHeight = 400.0f;
    CaveConfig.bHasFirePit = true;
    CaveConfig.bHasSleepingArea = true;
    CaveConfig.bHasToolStorage = true;
    CaveConfig.HumidityLevel = 0.7f;
    CaveConfig.TemperatureModifier = -5.0f;

    // Load default meshes and materials
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CaveWallMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CaveWallMeshAsset.Succeeded())
    {
        CaveWallsMesh->SetStaticMesh(CaveWallMeshAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CaveFloorMeshAsset(TEXT("/Engine/BasicShapes/Plane"));
    if (CaveFloorMeshAsset.Succeeded())
    {
        CaveFloorMesh->SetStaticMesh(CaveFloorMeshAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> FirePitMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
    if (FirePitMeshAsset.Succeeded())
    {
        FirePitMesh->SetStaticMesh(FirePitMeshAsset.Object);
    }

    // Load default materials
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultCaveWallMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (DefaultCaveWallMaterial.Succeeded())
    {
        CaveWallMaterial = DefaultCaveWallMaterial.Object;
        CaveFloorMaterial = DefaultCaveWallMaterial.Object;
        FirePitMaterial = DefaultCaveWallMaterial.Object;
    }
}

void AArch_CaveHabitationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    GenerateCaveStructure();
    PlaceCaveArtifacts();
    ApplyEnvironmentalEffects();
}

void AArch_CaveHabitationSystem::GenerateCaveStructure()
{
    CreateCaveGeometry();
    SetupCaveMaterials();
    
    if (CaveConfig.bHasFirePit)
    {
        CreateFirePit();
    }
    
    if (CaveConfig.bHasSleepingArea)
    {
        CreateSleepingArea();
    }
    
    if (CaveConfig.bHasToolStorage)
    {
        CreateToolStorage();
    }
}

void AArch_CaveHabitationSystem::CreateCaveGeometry()
{
    // Scale cave walls based on configuration
    FVector CaveWallScale = FVector(CaveConfig.CaveDepth / 100.0f, CaveConfig.CaveWidth / 100.0f, CaveConfig.CaveHeight / 100.0f);
    CaveWallsMesh->SetWorldScale3D(CaveWallScale);
    
    // Position cave floor
    FVector FloorScale = FVector(CaveConfig.CaveDepth / 100.0f, CaveConfig.CaveWidth / 100.0f, 1.0f);
    CaveFloorMesh->SetWorldScale3D(FloorScale);
    CaveFloorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -CaveConfig.CaveHeight * 0.5f));
}

void AArch_CaveHabitationSystem::SetupCaveMaterials()
{
    if (CaveWallMaterial)
    {
        CaveWallsMesh->SetMaterial(0, CaveWallMaterial);
    }
    
    if (CaveFloorMaterial)
    {
        CaveFloorMesh->SetMaterial(0, CaveFloorMaterial);
    }
}

void AArch_CaveHabitationSystem::CreateFirePit()
{
    if (!FirePitMesh)
        return;

    // Position fire pit in center of cave
    FVector FirePitLocation = FVector(CaveConfig.CaveDepth * 0.3f, 0.0f, -CaveConfig.CaveHeight * 0.4f);
    FirePitMesh->SetRelativeLocation(FirePitLocation);
    
    // Scale fire pit appropriately
    FVector FirePitScale = FVector(1.5f, 1.5f, 0.5f);
    FirePitMesh->SetWorldScale3D(FirePitScale);
    
    if (FirePitMaterial)
    {
        FirePitMesh->SetMaterial(0, FirePitMaterial);
    }
    
    FirePitMesh->SetVisibility(true);
}

void AArch_CaveHabitationSystem::CreateSleepingArea()
{
    // Sleeping area is represented by a specific region in the cave
    // This could be expanded to include actual sleeping mesh components
    UE_LOG(LogTemp, Log, TEXT("Cave sleeping area created at depth: %f"), CaveConfig.CaveDepth * 0.7f);
}

void AArch_CaveHabitationSystem::CreateToolStorage()
{
    // Tool storage area is represented by a specific region in the cave
    // This could be expanded to include actual storage mesh components
    UE_LOG(LogTemp, Log, TEXT("Cave tool storage created at width: %f"), CaveConfig.CaveWidth * 0.8f);
}

void AArch_CaveHabitationSystem::ApplyEnvironmentalEffects()
{
    // Apply humidity and temperature effects
    // This could be expanded to affect player stats when inside the cave
    UE_LOG(LogTemp, Log, TEXT("Cave environmental effects applied - Humidity: %f, Temp Modifier: %f"), 
           CaveConfig.HumidityLevel, CaveConfig.TemperatureModifier);
}

void AArch_CaveHabitationSystem::PlaceCaveArtifacts()
{
    for (int32 i = 0; i < CaveArtifacts.Num(); i++)
    {
        const FArch_CaveArtifact& Artifact = CaveArtifacts[i];
        
        if (Artifact.ArtifactMesh.IsValid())
        {
            // Create a new static mesh component for each artifact
            FString ComponentName = FString::Printf(TEXT("Artifact_%d_%s"), i, *Artifact.ArtifactName);
            UStaticMeshComponent* ArtifactComponent = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
            
            if (ArtifactComponent)
            {
                ArtifactComponent->SetupAttachment(RootComponent);
                ArtifactComponent->SetRelativeLocation(Artifact.RelativeLocation);
                ArtifactComponent->SetRelativeRotation(Artifact.RelativeRotation);
                
                if (UStaticMesh* LoadedMesh = Artifact.ArtifactMesh.LoadSynchronous())
                {
                    ArtifactComponent->SetStaticMesh(LoadedMesh);
                }
            }
        }
    }
}

void AArch_CaveHabitationSystem::SetCaveConfiguration(const FArch_CaveHabitationConfig& NewConfig)
{
    CaveConfig = NewConfig;
    GenerateCaveStructure();
}

void AArch_CaveHabitationSystem::AddArtifact(const FArch_CaveArtifact& NewArtifact)
{
    CaveArtifacts.Add(NewArtifact);
    PlaceCaveArtifacts();
}

void AArch_CaveHabitationSystem::RemoveArtifact(int32 ArtifactIndex)
{
    if (CaveArtifacts.IsValidIndex(ArtifactIndex))
    {
        CaveArtifacts.RemoveAt(ArtifactIndex);
        // Regenerate artifacts to update the cave
        PlaceCaveArtifacts();
    }
}

float AArch_CaveHabitationSystem::GetCaveTemperature() const
{
    // Base temperature would come from world/weather system
    float BaseTemperature = 20.0f; // Default 20°C
    return BaseTemperature + CaveConfig.TemperatureModifier;
}