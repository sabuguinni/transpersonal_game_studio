#include "EnvArt_CaveVisualSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_CaveVisualSystem::AEnvArt_CaveVisualSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create ambient light component
    AmbientLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("AmbientLightComponent"));
    AmbientLightComponent->SetupAttachment(RootComponent);
    AmbientLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

    // Initialize default cave configuration
    CaveConfig = FEnvArt_CaveVisualConfig();
    
    // Initialize arrays
    StalactiteMeshes.Empty();
    StalagmiteMeshes.Empty();
    CrystalFormations.Empty();
    WaterFeatures.Empty();
}

void AEnvArt_CaveVisualSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize cave visuals on begin play
    InitializeCaveVisuals();
}

void AEnvArt_CaveVisualSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update dynamic lighting effects for crystal caves
    if (CaveConfig.CaveType == EEnvArt_CaveVisualType::CrystalCave && CaveConfig.bHasCrystalFormations)
    {
        float PulseIntensity = CaveConfig.AmbientLightIntensity + (FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 100.0f);
        AmbientLightComponent->SetIntensity(PulseIntensity);
    }
}

void AEnvArt_CaveVisualSystem::InitializeCaveVisuals()
{
    // Clean up existing visuals
    CleanupCaveVisuals();
    
    // Setup ambient lighting
    SetupAmbientLighting();
    
    // Create cave features based on configuration
    CreateStalactites();
    CreateStalagmites();
    
    if (CaveConfig.bHasCrystalFormations)
    {
        CreateCrystalFormations();
    }
    
    if (CaveConfig.bHasWaterFeatures)
    {
        CreateWaterFeatures();
    }
    
    // Apply cave type specific features
    SetupCaveTypeSpecificFeatures();
    
    UE_LOG(LogTemp, Warning, TEXT("Cave visual system initialized with %d stalactites, %d stalagmites"), 
           StalactiteMeshes.Num(), StalagmiteMeshes.Num());
}

void AEnvArt_CaveVisualSystem::CreateStalactites()
{
    for (int32 i = 0; i < CaveConfig.StalactiteCount; ++i)
    {
        FString ComponentName = FString::Printf(TEXT("Stalactite_%d"), i);
        
        // Random position around cave entrance
        float Angle = (360.0f / CaveConfig.StalactiteCount) * i + FMath::RandRange(-30.0f, 30.0f);
        float Distance = FMath::RandRange(300.0f, 800.0f);
        
        FVector StalactiteLocation = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(400.0f, 800.0f)
        );
        
        FVector StalactiteScale = FVector(
            FMath::RandRange(0.8f, 1.5f),
            FMath::RandRange(0.8f, 1.5f),
            FMath::RandRange(1.0f, 2.5f)
        );
        
        CreateMeshComponent(ComponentName, StalactiteLocation, StalactiteScale);
    }
}

void AEnvArt_CaveVisualSystem::CreateStalagmites()
{
    for (int32 i = 0; i < CaveConfig.StalagmiteCount; ++i)
    {
        FString ComponentName = FString::Printf(TEXT("Stalagmite_%d"), i);
        
        // Random position on cave floor
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(200.0f, 600.0f);
        
        FVector StalagmiteLocation = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(-50.0f, 0.0f)
        );
        
        FVector StalagmiteScale = FVector(
            FMath::RandRange(0.6f, 1.2f),
            FMath::RandRange(0.6f, 1.2f),
            FMath::RandRange(0.8f, 2.0f)
        );
        
        CreateMeshComponent(ComponentName, StalagmiteLocation, StalagmiteScale);
    }
}

void AEnvArt_CaveVisualSystem::CreateCrystalFormations()
{
    if (!CaveConfig.bHasCrystalFormations) return;
    
    int32 CrystalCount = FMath::RandRange(3, 8);
    
    for (int32 i = 0; i < CrystalCount; ++i)
    {
        FString ComponentName = FString::Printf(TEXT("Crystal_%d"), i);
        
        // Crystals positioned near walls
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(400.0f, 900.0f);
        
        FVector CrystalLocation = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(50.0f, 300.0f)
        );
        
        FVector CrystalScale = FVector(
            FMath::RandRange(0.5f, 1.0f),
            FMath::RandRange(0.5f, 1.0f),
            FMath::RandRange(1.0f, 2.0f)
        );
        
        CreateMeshComponent(ComponentName, CrystalLocation, CrystalScale);
    }
}

void AEnvArt_CaveVisualSystem::CreateWaterFeatures()
{
    if (!CaveConfig.bHasWaterFeatures) return;
    
    int32 WaterFeatureCount = FMath::RandRange(1, 3);
    
    for (int32 i = 0; i < WaterFeatureCount; ++i)
    {
        FString ComponentName = FString::Printf(TEXT("WaterFeature_%d"), i);
        
        // Water features on cave floor
        FVector WaterLocation = FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            -100.0f
        );
        
        FVector WaterScale = FVector(
            FMath::RandRange(2.0f, 4.0f),
            FMath::RandRange(2.0f, 4.0f),
            0.2f
        );
        
        CreateMeshComponent(ComponentName, WaterLocation, WaterScale);
    }
}

void AEnvArt_CaveVisualSystem::SetupAmbientLighting()
{
    if (!AmbientLightComponent) return;
    
    AmbientLightComponent->SetLightColor(CaveConfig.AmbientLightColor);
    AmbientLightComponent->SetIntensity(CaveConfig.AmbientLightIntensity);
    AmbientLightComponent->SetAttenuationRadius(CaveConfig.LightAttenuationRadius);
    
    // Update lighting based on cave type
    UpdateLightingBasedOnCaveType();
}

void AEnvArt_CaveVisualSystem::ConfigureCaveType(EEnvArt_CaveVisualType NewCaveType)
{
    CaveConfig.CaveType = NewCaveType;
    
    // Update configuration based on cave type
    switch (NewCaveType)
    {
        case EEnvArt_CaveVisualType::SmallCaveEntrance:
            CaveConfig.StalactiteCount = 3;
            CaveConfig.StalagmiteCount = 2;
            CaveConfig.bHasCrystalFormations = false;
            CaveConfig.bHasWaterFeatures = false;
            break;
            
        case EEnvArt_CaveVisualType::MediumCaveEntrance:
            CaveConfig.StalactiteCount = 5;
            CaveConfig.StalagmiteCount = 3;
            CaveConfig.bHasCrystalFormations = false;
            CaveConfig.bHasWaterFeatures = FMath::RandBool();
            break;
            
        case EEnvArt_CaveVisualType::LargeCaveEntrance:
            CaveConfig.StalactiteCount = 8;
            CaveConfig.StalagmiteCount = 5;
            CaveConfig.bHasCrystalFormations = FMath::RandBool();
            CaveConfig.bHasWaterFeatures = true;
            break;
            
        case EEnvArt_CaveVisualType::CrystalCave:
            CaveConfig.StalactiteCount = 4;
            CaveConfig.StalagmiteCount = 3;
            CaveConfig.bHasCrystalFormations = true;
            CaveConfig.bHasWaterFeatures = false;
            break;
            
        case EEnvArt_CaveVisualType::WaterCave:
            CaveConfig.StalactiteCount = 6;
            CaveConfig.StalagmiteCount = 4;
            CaveConfig.bHasCrystalFormations = false;
            CaveConfig.bHasWaterFeatures = true;
            break;
            
        case EEnvArt_CaveVisualType::VolcanicCave:
            CaveConfig.StalactiteCount = 3;
            CaveConfig.StalagmiteCount = 2;
            CaveConfig.bHasCrystalFormations = false;
            CaveConfig.bHasWaterFeatures = false;
            break;
    }
    
    // Update lighting configuration
    CaveConfig.AmbientLightColor = GetCaveTypeColor(NewCaveType);
    CaveConfig.AmbientLightIntensity = GetCaveTypeLightIntensity(NewCaveType);
    
    // Reinitialize visuals
    InitializeCaveVisuals();
}

void AEnvArt_CaveVisualSystem::UpdateLightingBasedOnCaveType()
{
    if (!AmbientLightComponent) return;
    
    FLinearColor TypeColor = GetCaveTypeColor(CaveConfig.CaveType);
    float TypeIntensity = GetCaveTypeLightIntensity(CaveConfig.CaveType);
    
    AmbientLightComponent->SetLightColor(TypeColor);
    AmbientLightComponent->SetIntensity(TypeIntensity);
}

void AEnvArt_CaveVisualSystem::CleanupCaveVisuals()
{
    // Clear mesh arrays
    StalactiteMeshes.Empty();
    StalagmiteMeshes.Empty();
    CrystalFormations.Empty();
    WaterFeatures.Empty();
    
    // Remove existing mesh components
    TArray<UActorComponent*> MeshComponents = GetComponentsByClass(UStaticMeshComponent::StaticClass());
    for (UActorComponent* Component : MeshComponents)
    {
        if (Component != RootComponent && Component != AmbientLightComponent)
        {
            Component->DestroyComponent();
        }
    }
}

void AEnvArt_CaveVisualSystem::CreateMeshComponent(const FString& ComponentName, const FVector& RelativeLocation, const FVector& Scale)
{
    UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
    if (MeshComponent)
    {
        MeshComponent->SetupAttachment(RootComponent);
        MeshComponent->SetRelativeLocation(RelativeLocation);
        MeshComponent->SetRelativeScale3D(Scale);
        
        // Add random rotation for natural look
        FRotator RandomRotation = FRotator(
            FMath::RandRange(-15.0f, 15.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );
        MeshComponent->SetRelativeRotation(RandomRotation);
        
        // Store in appropriate array based on component name
        if (ComponentName.Contains(TEXT("Stalactite")))
        {
            StalactiteMeshes.Add(MeshComponent);
        }
        else if (ComponentName.Contains(TEXT("Stalagmite")))
        {
            StalagmiteMeshes.Add(MeshComponent);
        }
        else if (ComponentName.Contains(TEXT("Crystal")))
        {
            CrystalFormations.Add(MeshComponent);
        }
        else if (ComponentName.Contains(TEXT("Water")))
        {
            WaterFeatures.Add(MeshComponent);
        }
    }
}

void AEnvArt_CaveVisualSystem::SetupCaveTypeSpecificFeatures()
{
    switch (CaveConfig.CaveType)
    {
        case EEnvArt_CaveVisualType::VolcanicCave:
            // Add red-orange lighting for volcanic caves
            if (AmbientLightComponent)
            {
                AmbientLightComponent->SetLightColor(FLinearColor(1.0f, 0.3f, 0.1f, 1.0f));
                AmbientLightComponent->SetIntensity(800.0f);
            }
            break;
            
        case EEnvArt_CaveVisualType::CrystalCave:
            // Add blue-purple lighting for crystal caves
            if (AmbientLightComponent)
            {
                AmbientLightComponent->SetLightColor(FLinearColor(0.3f, 0.5f, 1.0f, 1.0f));
                AmbientLightComponent->SetIntensity(600.0f);
            }
            break;
            
        case EEnvArt_CaveVisualType::WaterCave:
            // Add blue-green lighting for water caves
            if (AmbientLightComponent)
            {
                AmbientLightComponent->SetLightColor(FLinearColor(0.2f, 0.8f, 0.6f, 1.0f));
                AmbientLightComponent->SetIntensity(400.0f);
            }
            break;
            
        default:
            // Standard cave lighting
            break;
    }
}

FLinearColor AEnvArt_CaveVisualSystem::GetCaveTypeColor(EEnvArt_CaveVisualType CaveType)
{
    switch (CaveType)
    {
        case EEnvArt_CaveVisualType::VolcanicCave:
            return FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);
        case EEnvArt_CaveVisualType::CrystalCave:
            return FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);
        case EEnvArt_CaveVisualType::WaterCave:
            return FLinearColor(0.2f, 0.8f, 0.6f, 1.0f);
        default:
            return FLinearColor(0.2f, 0.6f, 0.8f, 1.0f);
    }
}

float AEnvArt_CaveVisualSystem::GetCaveTypeLightIntensity(EEnvArt_CaveVisualType CaveType)
{
    switch (CaveType)
    {
        case EEnvArt_CaveVisualType::VolcanicCave:
            return 800.0f;
        case EEnvArt_CaveVisualType::CrystalCave:
            return 600.0f;
        case EEnvArt_CaveVisualType::WaterCave:
            return 400.0f;
        case EEnvArt_CaveVisualType::LargeCaveEntrance:
            return 700.0f;
        default:
            return 500.0f;
    }
}