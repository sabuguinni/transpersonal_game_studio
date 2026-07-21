#include "Arch_CretaceousStructureSystem.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UArch_CretaceousStructureSystem::UArch_CretaceousStructureSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for weathering
    
    StructureConfig = FArch_StructureConfig();
    WeatheringRate = 0.1f;
    bEnableWeathering = true;
    WeatheringTimer = 0.0f;
}

void UArch_CretaceousStructureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CreateStructureComponents();
    
    // Initialize interior spawn points based on structure type
    InteriorSpawnPoints.Empty();
    FVector Center = GetOwner()->GetActorLocation();
    
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::StoneDwelling:
            InteriorSpawnPoints.Add(Center + FVector(0, 0, 50));
            InteriorSpawnPoints.Add(Center + FVector(100, 100, 50));
            InteriorSpawnPoints.Add(Center + FVector(-100, 100, 50));
            break;
        case EArch_StructureType::RockShelter:
            InteriorSpawnPoints.Add(Center + FVector(0, -150, 50));
            break;
        case EArch_StructureType::CaveEntrance:
            InteriorSpawnPoints.Add(Center + FVector(0, -200, 50));
            InteriorSpawnPoints.Add(Center + FVector(0, -400, 50));
            break;
        default:
            InteriorSpawnPoints.Add(Center + FVector(0, 0, 50));
            break;
    }
}

void UArch_CretaceousStructureSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableWeathering && StructureConfig.StructuralIntegrity > 0.0f)
    {
        WeatheringTimer += DeltaTime;
        
        // Apply weathering every 60 seconds
        if (WeatheringTimer >= 60.0f)
        {
            ApplyWeathering(WeatheringRate);
            WeatheringTimer = 0.0f;
        }
    }
}

void UArch_CretaceousStructureSystem::InitializeStructure(const FArch_StructureConfig& Config)
{
    StructureConfig = Config;
    CreateStructureComponents();
    UpdateMaterialsForWeathering();
}

void UArch_CretaceousStructureSystem::ApplyWeathering(float WeatheringAmount)
{
    StructureConfig.StructuralIntegrity = FMath::Clamp(
        StructureConfig.StructuralIntegrity - WeatheringAmount, 
        0.0f, 
        100.0f
    );
    
    // Update weathering level based on structural integrity
    if (StructureConfig.StructuralIntegrity > 80.0f)
        StructureConfig.WeatheringLevel = EArch_WeatheringLevel::Fresh;
    else if (StructureConfig.StructuralIntegrity > 60.0f)
        StructureConfig.WeatheringLevel = EArch_WeatheringLevel::Aged;
    else if (StructureConfig.StructuralIntegrity > 40.0f)
        StructureConfig.WeatheringLevel = EArch_WeatheringLevel::Weathered;
    else if (StructureConfig.StructuralIntegrity > 20.0f)
        StructureConfig.WeatheringLevel = EArch_WeatheringLevel::Ancient;
    else
        StructureConfig.WeatheringLevel = EArch_WeatheringLevel::Ruins;
    
    UpdateMaterialsForWeathering();
    
    UE_LOG(LogTemp, Log, TEXT("Structure weathering applied: Integrity %.1f%%, Level %d"), 
           StructureConfig.StructuralIntegrity, (int32)StructureConfig.WeatheringLevel);
}

void UArch_CretaceousStructureSystem::SetStructureType(EArch_StructureType NewType)
{
    StructureConfig.StructureType = NewType;
    CreateStructureComponents();
}

void UArch_CretaceousStructureSystem::RepairStructure(float RepairAmount)
{
    StructureConfig.StructuralIntegrity = FMath::Clamp(
        StructureConfig.StructuralIntegrity + RepairAmount, 
        0.0f, 
        100.0f
    );
    
    UpdateMaterialsForWeathering();
}

bool UArch_CretaceousStructureSystem::CanEnterInterior() const
{
    return StructureConfig.bHasInterior && StructureConfig.StructuralIntegrity > 10.0f;
}

TArray<FVector> UArch_CretaceousStructureSystem::GetInteriorSpawnPoints() const
{
    return InteriorSpawnPoints;
}

void UArch_CretaceousStructureSystem::CreateStructureComponents()
{
    // Clear existing components
    for (UStaticMeshComponent* Component : StructureComponents)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }
    StructureComponents.Empty();
    
    // Create new structure based on type
    UStaticMesh* StructureMesh = GetMeshForStructureType(StructureConfig.StructureType);
    if (StructureMesh)
    {
        UStaticMeshComponent* MainComponent = NewObject<UStaticMeshComponent>(GetOwner());
        MainComponent->SetStaticMesh(StructureMesh);
        MainComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                                       FAttachmentTransformRules::KeepWorldTransform);
        MainComponent->RegisterComponent();
        StructureComponents.Add(MainComponent);
        
        // Add additional components based on structure complexity
        switch (StructureConfig.StructureType)
        {
            case EArch_StructureType::StoneDwelling:
                // Add foundation stones
                for (int32 i = 0; i < 8; i++)
                {
                    UStaticMeshComponent* Stone = NewObject<UStaticMeshComponent>(GetOwner());
                    float Angle = (i * 45.0f) * PI / 180.0f;
                    FVector StonePos = FVector(FMath::Cos(Angle) * 200.0f, FMath::Sin(Angle) * 200.0f, -50.0f);
                    Stone->SetRelativeLocation(StonePos);
                    Stone->AttachToComponent(MainComponent, FAttachmentTransformRules::KeepRelativeTransform);
                    Stone->RegisterComponent();
                    StructureComponents.Add(Stone);
                }
                break;
            case EArch_StructureType::WoodPlatform:
                // Add support beams
                for (int32 i = 0; i < 4; i++)
                {
                    UStaticMeshComponent* Beam = NewObject<UStaticMeshComponent>(GetOwner());
                    FVector BeamPos = FVector((i % 2) * 300.0f - 150.0f, (i / 2) * 300.0f - 150.0f, -100.0f);
                    Beam->SetRelativeLocation(BeamPos);
                    Beam->AttachToComponent(MainComponent, FAttachmentTransformRules::KeepRelativeTransform);
                    Beam->RegisterComponent();
                    StructureComponents.Add(Beam);
                }
                break;
        }
    }
    
    UpdateMaterialsForWeathering();
}

void UArch_CretaceousStructureSystem::UpdateMaterialsForWeathering()
{
    UMaterialInterface* WeatheringMaterial = GetMaterialForWeathering(StructureConfig.WeatheringLevel);
    
    for (UStaticMeshComponent* Component : StructureComponents)
    {
        if (IsValid(Component) && WeatheringMaterial)
        {
            Component->SetMaterial(0, WeatheringMaterial);
        }
    }
}

UStaticMesh* UArch_CretaceousStructureSystem::GetMeshForStructureType(EArch_StructureType Type)
{
    // Return basic cube mesh as placeholder - in production these would be proper assets
    return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
}

UMaterialInterface* UArch_CretaceousStructureSystem::GetMaterialForWeathering(EArch_WeatheringLevel Level)
{
    // Return basic material as placeholder - in production these would be weathering materials
    switch (Level)
    {
        case EArch_WeatheringLevel::Fresh:
            return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        case EArch_WeatheringLevel::Aged:
        case EArch_WeatheringLevel::Weathered:
        case EArch_WeatheringLevel::Ancient:
        case EArch_WeatheringLevel::Ruins:
        default:
            return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    }
}