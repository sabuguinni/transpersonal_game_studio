#include "Arch_StructuralManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"

AArch_StructuralManager::AArch_StructuralManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    BiomeInfluenceRadius = 5000.0f;
    bEnableAtmosphericEffects = true;
    AtmosphericDensity = 0.02f;
    AtmosphericColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Initialize lighting configuration with Cretaceous-appropriate values
    LightingConfig = FArch_LightingConfiguration();
}

void AArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeArchitecturalSystems();
    ConfigureArchitecturalLighting();
    
    if (bEnableAtmosphericEffects)
    {
        UpdateAtmosphericEffects();
    }
}

void AArch_StructuralManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update structural weathering over time
    UpdateStructuralWeathering();
}

void AArch_StructuralManager::SpawnStructuralElement(const FArch_StructuralElement& Element)
{
    if (!ValidateElementPlacement(Element))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid structural element placement: %s"), *Element.ElementName);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world context for spawning structural element"));
        return;
    }

    // Spawn static mesh actor for the structural element
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Element.Location,
        Element.Rotation,
        SpawnParams
    );

    if (StructureActor)
    {
        StructureActor->SetActorScale3D(Element.Scale);
        StructureActor->SetActorLabel(Element.ElementName);
        
        // Configure mesh component if available
        UStaticMeshComponent* MeshComponent = StructureActor->GetStaticMeshComponent();
        if (MeshComponent)
        {
            // Apply biome-specific materials and properties
            ConfigureBiomeSpecificArchitecture(Element.AssociatedBiome);
        }

        SpawnedStructures.Add(StructureActor);
        UE_LOG(LogTemp, Log, TEXT("Successfully spawned structural element: %s"), *Element.ElementName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn structural element: %s"), *Element.ElementName);
    }
}

void AArch_StructuralManager::ConfigureArchitecturalLighting()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn directional light for main architectural illumination
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    ADirectionalLight* MainLight = World->SpawnActor<ADirectionalLight>(
        ADirectionalLight::StaticClass(),
        GetActorLocation() + FVector(0, 0, 500),
        LightingConfig.DirectionalRotation,
        SpawnParams
    );

    if (MainLight)
    {
        UDirectionalLightComponent* LightComponent = MainLight->GetLightComponent();
        if (LightComponent)
        {
            LightComponent->SetIntensity(LightingConfig.DirectionalIntensity);
            LightComponent->SetLightColor(LightingConfig.DirectionalColor);
            LightComponent->SetCastShadows(true);
        }
        
        MainLight->SetActorLabel(TEXT("Arch_MainDirectionalLight"));
        DirectionalLights.Add(MainLight);
    }

    // Spawn point lights for interior/accent lighting
    for (int32 i = 0; i < LightingConfig.PointLightLocations.Num(); i++)
    {
        APointLight* PointLight = World->SpawnActor<APointLight>(
            APointLight::StaticClass(),
            GetActorLocation() + LightingConfig.PointLightLocations[i],
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (PointLight)
        {
            UPointLightComponent* PointComponent = PointLight->GetLightComponent();
            if (PointComponent)
            {
                PointComponent->SetIntensity(LightingConfig.PointLightIntensity);
                PointComponent->SetAttenuationRadius(LightingConfig.PointLightRadius);
                PointComponent->SetLightColor(LightingConfig.DirectionalColor);
            }
            
            PointLight->SetActorLabel(FString::Printf(TEXT("Arch_PointLight_%d"), i));
            PointLights.Add(PointLight);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Architectural lighting configured with %d directional and %d point lights"), 
           DirectionalLights.Num(), PointLights.Num());
}

void AArch_StructuralManager::UpdateAtmosphericEffects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find or create exponential height fog
    AExponentialHeightFog* HeightFog = nullptr;
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);

    if (FoundFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
    }
    else
    {
        // Spawn new height fog
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        HeightFog = World->SpawnActor<AExponentialHeightFog>(
            AExponentialHeightFog::StaticClass(),
            GetActorLocation(),
            FRotator::ZeroRotator,
            SpawnParams
        );
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(AtmosphericDensity);
            FogComponent->SetFogHeightFalloff(0.2f);
            FogComponent->SetFogInscatteringColor(AtmosphericColor);
            FogComponent->SetStartDistance(1000.0f);
        }
        
        HeightFog->SetActorLabel(TEXT("Arch_AtmosphericFog"));
        UE_LOG(LogTemp, Log, TEXT("Atmospheric effects updated for architectural ambiance"));
    }
}

void AArch_StructuralManager::SetBiomeArchitecture(EBiomeType BiomeType)
{
    ConfigureBiomeSpecificArchitecture(BiomeType);
    
    // Update existing structures to match new biome
    for (AActor* Structure : SpawnedStructures)
    {
        if (Structure)
        {
            // Apply biome-specific modifications
            UE_LOG(LogTemp, Log, TEXT("Updated structure %s for biome type"), *Structure->GetName());
        }
    }
}

TArray<FArch_StructuralElement> AArch_StructuralManager::GetStructuresInRadius(FVector Center, float Radius)
{
    TArray<FArch_StructuralElement> StructuresInRange;
    
    for (const FArch_StructuralElement& Element : StructuralElements)
    {
        float Distance = FVector::Dist(Element.Location, Center);
        if (Distance <= Radius)
        {
            StructuresInRange.Add(Element);
        }
    }
    
    return StructuresInRange;
}

void AArch_StructuralManager::ApplyWeatheringEffects(float WeatheringIntensity)
{
    for (FArch_StructuralElement& Element : StructuralElements)
    {
        // Reduce structural integrity based on weathering
        Element.StructuralIntegrity = FMath::Max(0.0f, 
            Element.StructuralIntegrity - (WeatheringIntensity * 0.1f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied weathering effects with intensity: %f"), WeatheringIntensity);
}

void AArch_StructuralManager::GenerateArchitecturalLayout()
{
    // Clear existing elements
    StructuralElements.Empty();
    
    // Generate basic Cretaceous architectural layout
    FVector BaseLocation = GetActorLocation();
    
    // Create main structure
    FArch_StructuralElement MainStructure;
    MainStructure.ElementName = TEXT("CretaceousMainHall");
    MainStructure.Location = BaseLocation;
    MainStructure.Scale = FVector(2.0f, 2.0f, 1.5f);
    MainStructure.AssociatedBiome = EBiomeType::Temperate;
    MainStructure.StructuralIntegrity = 85.0f;
    StructuralElements.Add(MainStructure);
    
    // Create supporting pillars
    for (int32 i = 0; i < 4; i++)
    {
        FArch_StructuralElement Pillar;
        Pillar.ElementName = FString::Printf(TEXT("CretaceousPillar_%d"), i);
        
        float Angle = (i * 90.0f) * PI / 180.0f;
        FVector PillarOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0) * 1000.0f;
        Pillar.Location = BaseLocation + PillarOffset;
        Pillar.Scale = FVector(0.5f, 0.5f, 2.0f);
        Pillar.AssociatedBiome = EBiomeType::Temperate;
        Pillar.StructuralIntegrity = 70.0f;
        StructuralElements.Add(Pillar);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Generated architectural layout with %d elements"), StructuralElements.Num());
}

void AArch_StructuralManager::ValidateStructuralIntegrity()
{
    int32 ValidStructures = 0;
    int32 DamagedStructures = 0;
    
    for (const FArch_StructuralElement& Element : StructuralElements)
    {
        if (Element.StructuralIntegrity > 50.0f)
        {
            ValidStructures++;
        }
        else
        {
            DamagedStructures++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Structural integrity validation: %d valid, %d damaged structures"), 
           ValidStructures, DamagedStructures);
}

void AArch_StructuralManager::InitializeArchitecturalSystems()
{
    // Initialize point light locations for interior lighting
    LightingConfig.PointLightLocations.Empty();
    LightingConfig.PointLightLocations.Add(FVector(500, 0, 150));
    LightingConfig.PointLightLocations.Add(FVector(-500, 0, 150));
    LightingConfig.PointLightLocations.Add(FVector(0, 500, 150));
    LightingConfig.PointLightLocations.Add(FVector(0, -500, 150));
    
    UE_LOG(LogTemp, Log, TEXT("Architectural systems initialized"));
}

void AArch_StructuralManager::ConfigureBiomeSpecificArchitecture(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Temperate:
            AtmosphericColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
            AtmosphericDensity = 0.02f;
            break;
        case EBiomeType::Arid:
            AtmosphericColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            AtmosphericDensity = 0.015f;
            break;
        case EBiomeType::Tropical:
            AtmosphericColor = FLinearColor(0.7f, 1.0f, 0.8f, 1.0f);
            AtmosphericDensity = 0.025f;
            break;
        default:
            // Keep default values
            break;
    }
}

void AArch_StructuralManager::UpdateStructuralWeathering()
{
    // Gradual weathering simulation
    static float WeatheringTimer = 0.0f;
    WeatheringTimer += GetWorld()->GetDeltaSeconds();
    
    if (WeatheringTimer >= 10.0f) // Update every 10 seconds
    {
        ApplyWeatheringEffects(0.1f);
        WeatheringTimer = 0.0f;
    }
}

bool AArch_StructuralManager::ValidateElementPlacement(const FArch_StructuralElement& Element)
{
    // Check for overlapping structures
    for (const FArch_StructuralElement& ExistingElement : StructuralElements)
    {
        float Distance = FVector::Dist(Element.Location, ExistingElement.Location);
        if (Distance < 500.0f) // Minimum distance between structures
        {
            return false;
        }
    }
    
    // Validate structural integrity
    if (Element.StructuralIntegrity <= 0.0f)
    {
        return false;
    }
    
    return true;
}