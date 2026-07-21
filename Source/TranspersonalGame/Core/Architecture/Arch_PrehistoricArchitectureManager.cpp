#include "Arch_PrehistoricArchitectureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Materials/MaterialInstanceDynamic.h"

AArch_PrehistoricArchitectureManager::AArch_PrehistoricArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MonumentSpawnRadius = 10000.0f;
    MaxMonumentsPerBiome = 3;
    WeatheredStoneMaterial = nullptr;
    MossyStoneMaterial = nullptr;
    StandingStoneBaseMesh = nullptr;
    DolmenCapMesh = nullptr;
}

void AArch_PrehistoricArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultAssets();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Prehistoric Architecture Manager initialized"));
    }
}

void AArch_PrehistoricArchitectureManager::InitializeDefaultAssets()
{
    // Load default stone mesh (using engine cube as fallback)
    if (!StandingStoneBaseMesh)
    {
        StandingStoneBaseMesh = LoadObject<UStaticMesh>(nullptr, 
            TEXT("/Engine/BasicShapes/Cube.Cube"));
    }
    
    if (!DolmenCapMesh)
    {
        DolmenCapMesh = LoadObject<UStaticMesh>(nullptr, 
            TEXT("/Engine/BasicShapes/Cube.Cube"));
    }
    
    // Load default materials
    if (!WeatheredStoneMaterial)
    {
        WeatheredStoneMaterial = LoadObject<UMaterialInterface>(nullptr, 
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    }
    
    if (!MossyStoneMaterial)
    {
        MossyStoneMaterial = LoadObject<UMaterialInterface>(nullptr, 
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    }
}

void AArch_PrehistoricArchitectureManager::GenerateMonumentAtLocation(const FVector& Location, EArch_MonumentType MonumentType)
{
    FArch_MonumentProperties Properties;
    Properties.MonumentType = MonumentType;
    Properties.CircleRadius = FMath::RandRange(300.0f, 800.0f);
    Properties.StoneCount = FMath::RandRange(8, 16);
    Properties.WeatheringIntensity = FMath::RandRange(0.5f, 1.0f);
    Properties.bHasMossGrowth = FMath::RandBool();
    Properties.bIsAncientRuin = FMath::RandRange(0.0f, 1.0f) > 0.7f;

    switch (MonumentType)
    {
        case EArch_MonumentType::StoneCircle:
            CreateStoneCircle(Location, Properties);
            break;
        case EArch_MonumentType::MegalithicDolmen:
            CreateMegalithicDolmen(Location, Properties);
            break;
        case EArch_MonumentType::CairnPyramid:
            CreateCairnPyramid(Location, Properties);
            break;
        case EArch_MonumentType::StandingStones:
            CreateStoneCircle(Location, Properties); // Use circle logic with fewer stones
            break;
        case EArch_MonumentType::RockFormation:
            CreateCairnPyramid(Location, Properties); // Use cairn logic for natural formation
            break;
    }

    RegisterMonument(Properties);
}

void AArch_PrehistoricArchitectureManager::CreateStoneCircle(const FVector& CenterLocation, const FArch_MonumentProperties& Properties)
{
    if (!StandingStoneBaseMesh)
    {
        return;
    }

    float AngleStep = 360.0f / Properties.StoneCount;
    
    for (int32 i = 0; i < Properties.StoneCount; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector StoneLocation = CenterLocation + FVector(
            FMath::Cos(RadianAngle) * Properties.CircleRadius,
            FMath::Sin(RadianAngle) * Properties.CircleRadius,
            0.0f
        );
        
        // Add some random variation
        StoneLocation += FVector(
            FMath::RandRange(-50.0f, 50.0f),
            FMath::RandRange(-50.0f, 50.0f),
            0.0f
        );
        
        FVector StoneScale = Properties.StoneScale;
        StoneScale.Z *= FMath::RandRange(0.8f, 1.5f); // Vary height
        
        UStaticMeshComponent* StoneComponent = CreateStoneComponent(StoneLocation, StoneScale, Angle);
        if (StoneComponent)
        {
            ApplyWeatheringEffects(StoneComponent, Properties.WeatheringIntensity);
            ApplyMaterialToComponent(StoneComponent, Properties.bHasMossGrowth);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
            FString::Printf(TEXT("Created stone circle with %d stones at %s"), 
                Properties.StoneCount, *CenterLocation.ToString()));
    }
}

void AArch_PrehistoricArchitectureManager::CreateMegalithicDolmen(const FVector& Location, const FArch_MonumentProperties& Properties)
{
    if (!StandingStoneBaseMesh || !DolmenCapMesh)
    {
        return;
    }

    // Create vertical support stones
    FVector LeftSupport = Location + FVector(-200.0f, 0.0f, 0.0f);
    FVector RightSupport = Location + FVector(200.0f, 0.0f, 0.0f);
    FVector BackSupport = Location + FVector(0.0f, -150.0f, 0.0f);
    
    // Create support stones
    TArray<FVector> SupportLocations = {LeftSupport, RightSupport, BackSupport};
    
    for (const FVector& SupportLoc : SupportLocations)
    {
        UStaticMeshComponent* SupportComponent = CreateStoneComponent(
            SupportLoc, 
            FVector(0.8f, 0.8f, 2.5f), 
            FMath::RandRange(0.0f, 15.0f)
        );
        
        if (SupportComponent)
        {
            ApplyWeatheringEffects(SupportComponent, Properties.WeatheringIntensity);
            ApplyMaterialToComponent(SupportComponent, Properties.bHasMossGrowth);
        }
    }
    
    // Create capstone
    FVector CapLocation = Location + FVector(0.0f, 0.0f, 300.0f);
    UStaticMeshComponent* CapComponent = CreateStoneComponent(
        CapLocation, 
        FVector(3.0f, 2.0f, 0.5f), 
        FMath::RandRange(0.0f, 30.0f)
    );
    
    if (CapComponent)
    {
        CapComponent->SetStaticMesh(DolmenCapMesh);
        ApplyWeatheringEffects(CapComponent, Properties.WeatheringIntensity);
        ApplyMaterialToComponent(CapComponent, Properties.bHasMossGrowth);
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
            FString::Printf(TEXT("Created megalithic dolmen at %s"), *Location.ToString()));
    }
}

void AArch_PrehistoricArchitectureManager::CreateCairnPyramid(const FVector& Location, const FArch_MonumentProperties& Properties)
{
    if (!StandingStoneBaseMesh)
    {
        return;
    }

    int32 Layers = FMath::RandRange(4, 7);
    float BaseRadius = Properties.CircleRadius * 0.3f;
    
    for (int32 Layer = 0; Layer < Layers; Layer++)
    {
        float LayerRadius = BaseRadius * (1.0f - (float)Layer / Layers);
        float LayerHeight = Layer * 80.0f;
        int32 StonesInLayer = FMath::Max(1, FMath::RoundToInt(LayerRadius / 50.0f));
        
        if (Layer == Layers - 1)
        {
            StonesInLayer = 1; // Top stone
        }
        
        float AngleStep = StonesInLayer > 1 ? 360.0f / StonesInLayer : 0.0f;
        
        for (int32 i = 0; i < StonesInLayer; i++)
        {
            FVector StoneLocation = Location;
            
            if (StonesInLayer > 1)
            {
                float Angle = i * AngleStep;
                float RadianAngle = FMath::DegreesToRadians(Angle);
                StoneLocation += FVector(
                    FMath::Cos(RadianAngle) * LayerRadius,
                    FMath::Sin(RadianAngle) * LayerRadius,
                    LayerHeight
                );
            }
            else
            {
                StoneLocation.Z += LayerHeight;
            }
            
            FVector StoneScale = FVector(
                FMath::RandRange(0.5f, 1.0f),
                FMath::RandRange(0.5f, 1.0f),
                FMath::RandRange(0.3f, 0.8f)
            );
            
            UStaticMeshComponent* StoneComponent = CreateStoneComponent(
                StoneLocation, 
                StoneScale, 
                FMath::RandRange(0.0f, 360.0f)
            );
            
            if (StoneComponent)
            {
                ApplyWeatheringEffects(StoneComponent, Properties.WeatheringIntensity);
                ApplyMaterialToComponent(StoneComponent, Properties.bHasMossGrowth);
            }
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
            FString::Printf(TEXT("Created cairn pyramid with %d layers at %s"), Layers, *Location.ToString()));
    }
}

UStaticMeshComponent* AArch_PrehistoricArchitectureManager::CreateStoneComponent(const FVector& Location, const FVector& Scale, float RotationYaw)
{
    UStaticMeshComponent* StoneComponent = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("Stone_%d"), GetComponents<UStaticMeshComponent>().Num())
    );
    
    if (StoneComponent && StandingStoneBaseMesh)
    {
        StoneComponent->SetStaticMesh(StandingStoneBaseMesh);
        StoneComponent->SetWorldLocation(Location);
        StoneComponent->SetWorldScale3D(Scale);
        StoneComponent->SetWorldRotation(FRotator(0.0f, RotationYaw, 0.0f));
        StoneComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        
        return StoneComponent;
    }
    
    return nullptr;
}

void AArch_PrehistoricArchitectureManager::ApplyMaterialToComponent(UStaticMeshComponent* Component, bool bUseMossyVariant)
{
    if (!Component)
    {
        return;
    }
    
    UMaterialInterface* MaterialToUse = bUseMossyVariant ? MossyStoneMaterial : WeatheredStoneMaterial;
    
    if (MaterialToUse)
    {
        Component->SetMaterial(0, MaterialToUse);
    }
}

void AArch_PrehistoricArchitectureManager::ApplyWeatheringEffects(UStaticMeshComponent* MeshComponent, float WeatheringIntensity)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Create dynamic material instance for weathering effects
    UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply weathering parameters (if material supports them)
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), WeatheringIntensity);
            DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), WeatheringIntensity * 0.8f);
            DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.8f + WeatheringIntensity * 0.2f);
            
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AArch_PrehistoricArchitectureManager::RegisterMonument(const FArch_MonumentProperties& Monument)
{
    ActiveMonuments.Add(Monument);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
            FString::Printf(TEXT("Registered monument. Total active: %d"), ActiveMonuments.Num()));
    }
}

TArray<FArch_MonumentProperties> AArch_PrehistoricArchitectureManager::GetNearbyMonuments(const FVector& Location, float SearchRadius) const
{
    TArray<FArch_MonumentProperties> NearbyMonuments;
    
    for (const FArch_MonumentProperties& Monument : ActiveMonuments)
    {
        // Note: For full implementation, we'd need to store monument locations
        // This is a simplified version for the architecture system
        NearbyMonuments.Add(Monument);
    }
    
    return NearbyMonuments;
}

void AArch_PrehistoricArchitectureManager::GenerateTestMonuments()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Generate test monuments around the origin
    TArray<FVector> TestLocations = {
        FVector(1000.0f, 0.0f, 0.0f),
        FVector(-1000.0f, 500.0f, 0.0f),
        FVector(0.0f, -1200.0f, 0.0f),
        FVector(800.0f, 800.0f, 0.0f)
    };
    
    TArray<EArch_MonumentType> MonumentTypes = {
        EArch_MonumentType::StoneCircle,
        EArch_MonumentType::MegalithicDolmen,
        EArch_MonumentType::CairnPyramid,
        EArch_MonumentType::StandingStones
    };
    
    for (int32 i = 0; i < TestLocations.Num(); i++)
    {
        EArch_MonumentType MonumentType = MonumentTypes[i % MonumentTypes.Num()];
        GenerateMonumentAtLocation(TestLocations[i], MonumentType);
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Generated test prehistoric monuments"));
    }
}