#include "EnvArt_CretaceousForestManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_CretaceousForestManager::AEnvArt_CretaceousForestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize forest configuration
    ForestCenter = FVector(25000.0f, 25000.0f, 0.0f);
    ForestRadius = 15000.0f;
    MaxVegetationCount = 500;
    MaxClusterCount = 12;
    MaxActorLimit = 20000;
    CurrentActorCount = 0;
    
    // Initialize atmosphere settings
    AtmosphereSettings = FEnvArt_ForestAtmosphereSettings();
    
    // Initialize plant types
    InitializeCretaceousPlantTypes();
}

void AEnvArt_CretaceousForestManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the forest biome on game start
    InitializeForestBiome();
}

void AEnvArt_CretaceousForestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void AEnvArt_CretaceousForestManager::InitializeForestBiome()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Cretaceous Forest Biome"));
    
    // Validate actor limits before proceeding
    ValidateActorLimits();
    
    // Setup atmospheric lighting
    SetupAtmosphericLighting();
    
    // Create volumetric fog
    CreateVolumetricFog();
    
    // Generate vegetation clusters
    GenerateVegetationClusters();
    
    // Optimize performance
    OptimizeForestPerformance();
}

void AEnvArt_CretaceousForestManager::GenerateVegetationClusters()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating Cretaceous vegetation clusters"));
    
    // Clear existing clusters
    VegetationClusters.Empty();
    
    // Create vegetation clusters
    for (int32 ClusterIndex = 0; ClusterIndex < MaxClusterCount; ClusterIndex++)
    {
        FVector ClusterCenter = GetRandomClusterPosition();
        FEnvArt_ForestClusterData NewCluster;
        NewCluster.ClusterCenter = ClusterCenter;
        NewCluster.ClusterRadius = FMath::RandRange(800.0f, 1500.0f);
        
        // Spawn plants for each type in this cluster
        for (const FEnvArt_CretaceousPlantData& PlantType : CretaceousPlantTypes)
        {
            SpawnPlantCluster(PlantType, ClusterCenter);
        }
        
        VegetationClusters.Add(NewCluster);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d vegetation clusters"), VegetationClusters.Num());
}

void AEnvArt_CretaceousForestManager::SpawnPlantCluster(const FEnvArt_CretaceousPlantData& PlantData, const FVector& ClusterCenter)
{
    if (GetCurrentVegetationCount() >= MaxVegetationCount)
    {
        return;
    }
    
    int32 PlantsToSpawn = FMath::Min(PlantData.MaxInstancesPerCluster, MaxVegetationCount - GetCurrentVegetationCount());
    
    for (int32 i = 0; i < PlantsToSpawn; i++)
    {
        // Random position within cluster
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(0.0f, PlantData.ClusterRadius);
        
        FVector PlantPosition = ClusterCenter + FVector(
            Distance * FMath::Cos(FMath::DegreesToRadians(Angle)),
            Distance * FMath::Sin(FMath::DegreesToRadians(Angle)),
            0.0f
        );
        
        if (IsValidPlantPosition(PlantPosition))
        {
            // Spawn static mesh actor as plant placeholder
            AStaticMeshActor* PlantActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                PlantPosition,
                FRotator::ZeroRotator
            );
            
            if (PlantActor)
            {
                // Scale based on plant height range
                float Height = FMath::RandRange(PlantData.HeightRange.X, PlantData.HeightRange.Y);
                float Scale = Height / 100.0f;
                PlantActor->SetActorScale3D(FVector(Scale * 0.5f, Scale * 0.5f, Scale));
                
                // Set label for identification
                FString PlantLabel = FString::Printf(TEXT("%s_Cluster_%d"), *PlantData.PlantName, VegetationClusters.Num());
                PlantActor->SetActorLabel(PlantLabel);
                
                // Add to cluster data
                if (VegetationClusters.IsValidIndex(VegetationClusters.Num() - 1))
                {
                    VegetationClusters[VegetationClusters.Num() - 1].PlantActors.Add(PlantActor);
                    VegetationClusters[VegetationClusters.Num() - 1].PlantCount++;
                }
            }
        }
    }
}

void AEnvArt_CretaceousForestManager::SetupAtmosphericLighting()
{
    UE_LOG(LogTemp, Warning, TEXT("Setting up atmospheric lighting"));
    
    // Find directional light (sun)
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        if (SunLight)
        {
            ConfigureGoldenHourLighting();
        }
    }
    
    // Create ambient light points
    CreateAmbientLightPoints();
}

void AEnvArt_CretaceousForestManager::ConfigureGoldenHourLighting()
{
    if (!SunLight)
    {
        return;
    }
    
    // Set golden hour sun angle and color
    SunLight->SetActorRotation(AtmosphereSettings.SunRotation);
    
    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    if (LightComponent)
    {
        LightComponent->SetLightColor(AtmosphereSettings.SunColor);
        LightComponent->SetIntensity(AtmosphereSettings.SunIntensity);
        LightComponent->SetCastShadows(true);
        
        UE_LOG(LogTemp, Warning, TEXT("Golden hour lighting configured"));
    }
}

void AEnvArt_CretaceousForestManager::CreateAmbientLightPoints()
{
    const int32 MaxAmbientLights = 12;
    
    for (int32 i = 0; i < MaxAmbientLights; i++)
    {
        FVector LightPosition = ForestCenter + FVector(
            FMath::RandRange(-ForestRadius * 0.8f, ForestRadius * 0.8f),
            FMath::RandRange(-ForestRadius * 0.8f, ForestRadius * 0.8f),
            FMath::RandRange(200.0f, 800.0f)
        );
        
        APointLight* AmbientLight = GetWorld()->SpawnActor<APointLight>(
            APointLight::StaticClass(),
            LightPosition,
            FRotator::ZeroRotator
        );
        
        if (AmbientLight)
        {
            UPointLightComponent* LightComponent = AmbientLight->GetLightComponent();
            if (LightComponent)
            {
                // Warm forest lighting
                LightComponent->SetLightColor(FLinearColor(0.9f, 0.7f, 0.5f, 1.0f));
                LightComponent->SetIntensity(FMath::RandRange(500.0f, 1500.0f));
                LightComponent->SetAttenuationRadius(FMath::RandRange(2000.0f, 5000.0f));
                LightComponent->SetCastShadows(false); // Performance optimization
            }
            
            FString LightLabel = FString::Printf(TEXT("ForestAmbientLight_%d"), i);
            AmbientLight->SetActorLabel(LightLabel);
            AmbientLights.Add(AmbientLight);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d ambient light points"), AmbientLights.Num());
}

void AEnvArt_CretaceousForestManager::CreateVolumetricFog()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating volumetric fog"));
    
    FVector FogPosition = ForestCenter + FVector(0.0f, 0.0f, 500.0f);
    
    ForestFog = GetWorld()->SpawnActor<AExponentialHeightFog>(
        AExponentialHeightFog::StaticClass(),
        FogPosition,
        FRotator::ZeroRotator
    );
    
    if (ForestFog)
    {
        UExponentialHeightFogComponent* FogComponent = ForestFog->GetComponent();
        if (FogComponent)
        {
            SetForestFogParameters(AtmosphereSettings.FogDensity, AtmosphereSettings.FogColor, AtmosphereSettings.FogHeightFalloff);
        }
        
        ForestFog->SetActorLabel(TEXT("CretaceousForestFog"));
        UE_LOG(LogTemp, Warning, TEXT("Volumetric fog created"));
    }
}

void AEnvArt_CretaceousForestManager::SetForestFogParameters(float Density, const FLinearColor& Color, float HeightFalloff)
{
    if (!ForestFog)
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComponent = ForestFog->GetComponent();
    if (FogComponent)
    {
        FogComponent->SetFogDensity(Density);
        FogComponent->SetFogHeightFalloff(HeightFalloff);
        FogComponent->SetFogInscatteringColor(Color);
        FogComponent->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
    }
}

void AEnvArt_CretaceousForestManager::OptimizeForestPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing forest performance"));
    
    // Remove excess vegetation if over limits
    RemoveExcessVegetation();
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void AEnvArt_CretaceousForestManager::RemoveExcessVegetation()
{
    int32 CurrentVegetation = GetCurrentVegetationCount();
    
    if (CurrentVegetation > MaxVegetationCount)
    {
        int32 ToRemove = CurrentVegetation - MaxVegetationCount;
        int32 Removed = 0;
        
        for (FEnvArt_ForestClusterData& Cluster : VegetationClusters)
        {
            while (Cluster.PlantActors.Num() > 0 && Removed < ToRemove)
            {
                AActor* ActorToRemove = Cluster.PlantActors.Pop();
                if (ActorToRemove)
                {
                    ActorToRemove->Destroy();
                    Cluster.PlantCount--;
                    Removed++;
                }
            }
            
            if (Removed >= ToRemove)
            {
                break;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Removed %d excess vegetation actors"), Removed);
    }
}

void AEnvArt_CretaceousForestManager::ValidateActorLimits()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentActorCount = AllActors.Num();
    
    if (CurrentActorCount > MaxActorLimit)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %d actors exceed limit of %d"), CurrentActorCount, MaxActorLimit);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count: %d / %d"), CurrentActorCount, MaxActorLimit);
    }
}

int32 AEnvArt_CretaceousForestManager::GetCurrentVegetationCount() const
{
    int32 TotalVegetation = 0;
    
    for (const FEnvArt_ForestClusterData& Cluster : VegetationClusters)
    {
        TotalVegetation += Cluster.PlantCount;
    }
    
    return TotalVegetation;
}

void AEnvArt_CretaceousForestManager::UpdateTimeOfDayLighting(float TimeOfDay)
{
    if (!SunLight)
    {
        return;
    }
    
    // Update sun angle based on time of day (0.0 = midnight, 0.5 = noon)
    float SunAngle = (TimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    FRotator NewRotation = FRotator(SunAngle, AtmosphereSettings.SunRotation.Yaw, 0.0f);
    SunLight->SetActorRotation(NewRotation);
    
    // Update sun intensity based on time
    float SunIntensity = FMath::Max(0.0f, FMath::Cos(FMath::DegreesToRadians(SunAngle))) * AtmosphereSettings.SunIntensity;
    
    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    if (LightComponent)
    {
        LightComponent->SetIntensity(SunIntensity);
    }
}

void AEnvArt_CretaceousForestManager::UpdateForestAtmosphere(const FEnvArt_ForestAtmosphereSettings& Settings)
{
    AtmosphereSettings = Settings;
    
    // Update lighting
    ConfigureGoldenHourLighting();
    
    // Update fog
    SetForestFogParameters(Settings.FogDensity, Settings.FogColor, Settings.FogHeightFalloff);
}

void AEnvArt_CretaceousForestManager::InitializeCretaceousPlantTypes()
{
    CretaceousPlantTypes.Empty();
    
    // Conifer trees (dominant in Cretaceous)
    FEnvArt_CretaceousPlantData ConiferData;
    ConiferData.PlantName = TEXT("ConiferTree");
    ConiferData.DensityWeight = 0.4f;
    ConiferData.HeightRange = FVector2D(800.0f, 2000.0f);
    ConiferData.ClusterRadius = 1200.0f;
    ConiferData.MaxInstancesPerCluster = 15;
    CretaceousPlantTypes.Add(ConiferData);
    
    // Fern clusters
    FEnvArt_CretaceousPlantData FernData;
    FernData.PlantName = TEXT("FernCluster");
    FernData.DensityWeight = 0.3f;
    FernData.HeightRange = FVector2D(50.0f, 200.0f);
    FernData.ClusterRadius = 800.0f;
    FernData.MaxInstancesPerCluster = 25;
    CretaceousPlantTypes.Add(FernData);
    
    // Cycad palms
    FEnvArt_CretaceousPlantData CycadData;
    CycadData.PlantName = TEXT("CycadPalm");
    CycadData.DensityWeight = 0.2f;
    CycadData.HeightRange = FVector2D(300.0f, 800.0f);
    CycadData.ClusterRadius = 1000.0f;
    CycadData.MaxInstancesPerCluster = 10;
    CretaceousPlantTypes.Add(CycadData);
    
    // Horsetail rushes
    FEnvArt_CretaceousPlantData HorsetailData;
    HorsetailData.PlantName = TEXT("HorsetailRush");
    HorsetailData.DensityWeight = 0.1f;
    HorsetailData.HeightRange = FVector2D(100.0f, 400.0f);
    HorsetailData.ClusterRadius = 600.0f;
    HorsetailData.MaxInstancesPerCluster = 20;
    CretaceousPlantTypes.Add(HorsetailData);
}

FVector AEnvArt_CretaceousForestManager::GetRandomClusterPosition() const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(0.0f, ForestRadius);
    
    return ForestCenter + FVector(
        Distance * FMath::Cos(FMath::DegreesToRadians(Angle)),
        Distance * FMath::Sin(FMath::DegreesToRadians(Angle)),
        0.0f
    );
}

bool AEnvArt_CretaceousForestManager::IsValidPlantPosition(const FVector& Position) const
{
    // Check if position is within forest bounds
    float DistanceFromCenter = FVector::Dist2D(Position, ForestCenter);
    return DistanceFromCenter <= ForestRadius;
}

void AEnvArt_CretaceousForestManager::CleanupOldVegetation()
{
    for (FEnvArt_ForestClusterData& Cluster : VegetationClusters)
    {
        // Remove invalid actors
        Cluster.PlantActors.RemoveAll([](AActor* Actor) {
            return !IsValid(Actor);
        });
        
        Cluster.PlantCount = Cluster.PlantActors.Num();
    }
}

void AEnvArt_CretaceousForestManager::UpdatePerformanceMetrics()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentActorCount = AllActors.Num();
}