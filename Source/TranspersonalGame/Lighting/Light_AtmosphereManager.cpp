#include "Light_AtmosphereManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    DayDuration = 1200.0f; // 20 minutes real time = 24 hours game time
    CurrentTimeOfDay = 0.6f; // Start at late afternoon
    CurrentPeriod = ELight_TimeOfDay::Afternoon;
    CretaceousTemperature = 3200.0f;
    CretaceousAmbientColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);
    bEnableVolumetricFog = true;
    bEnableLumenGI = true;

    // Initialize biome atmosphere settings
    BiomeSettings.Empty();
    
    // Savana
    FLight_BiomeAtmosphere Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    Savana.AtmosphericColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    Savana.FogDensity = 0.1f;
    Savana.LightIntensity = 2.5f;
    BiomeSettings.Add(Savana);

    // Pantano
    FLight_BiomeAtmosphere Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    Pantano.AtmosphericColor = FLinearColor(0.6f, 0.8f, 0.5f, 1.0f);
    Pantano.FogDensity = 0.25f;
    Pantano.LightIntensity = 1.8f;
    BiomeSettings.Add(Pantano);

    // Floresta
    FLight_BiomeAtmosphere Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    Floresta.AtmosphericColor = FLinearColor(0.5f, 0.9f, 0.4f, 1.0f);
    Floresta.FogDensity = 0.15f;
    Floresta.LightIntensity = 2.0f;
    BiomeSettings.Add(Floresta);

    // Deserto
    FLight_BiomeAtmosphere Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    Deserto.AtmosphericColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    Deserto.FogDensity = 0.05f;
    Deserto.LightIntensity = 3.2f;
    BiomeSettings.Add(Deserto);

    // Montanha
    FLight_BiomeAtmosphere Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
    Montanha.AtmosphericColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f);
    Montanha.FogDensity = 0.2f;
    Montanha.LightIntensity = 2.2f;
    BiomeSettings.Add(Montanha);
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAtmosphere();
    SetCretaceousLighting();
    ConfigureLumenSettings();
    SpawnBiomeAtmosphericLights();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateTimeOfDay(DeltaTime);
    UpdateSunPosition();
    UpdateAtmosphericProperties();
}

void ALight_AtmosphereManager::InitializeBiomeAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphereManager: World is null"));
        return;
    }

    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        ADirectionalLight* DirectionalLightActor = Cast<ADirectionalLight>(FoundActors[0]);
        if (DirectionalLightActor)
        {
            SunLight = DirectionalLightActor->GetLightComponent();
            UE_LOG(LogTemp, Log, TEXT("Light_AtmosphereManager: Found directional light"));
        }
    }
}

void ALight_AtmosphereManager::UpdateTimeOfDay(float DeltaTime)
{
    CurrentTimeOfDay += DeltaTime / DayDuration;
    
    if (CurrentTimeOfDay >= 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
    }
    
    CurrentPeriod = CalculateTimeOfDay(CurrentTimeOfDay);
}

void ALight_AtmosphereManager::SetCretaceousLighting()
{
    if (!SunLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphereManager: SunLight is null"));
        return;
    }

    // Configure Cretaceous period lighting
    SunLight->SetIntensity(8.5f);
    SunLight->SetLightColor(CretaceousAmbientColor);
    SunLight->SetTemperature(CretaceousTemperature);
    SunLight->SetUseTemperature(true);
    SunLight->SetCastVolumetricShadow(true);
    
    UE_LOG(LogTemp, Log, TEXT("Light_AtmosphereManager: Cretaceous lighting configured"));
}

void ALight_AtmosphereManager::ConfigureLumenSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (bEnableLumenGI)
    {
        // Configure Lumen Global Illumination via console commands
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.GlobalIllumination 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.Reflections 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.ScreenProbeGather 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.RadianceCache 1"));
    }

    if (bEnableVolumetricFog)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricFog 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricFog.GridPixelSize 8"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Fog 1"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Light_AtmosphereManager: Lumen and fog settings configured"));
}

void ALight_AtmosphereManager::SpawnBiomeAtmosphericLights()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    BiomeAtmosphericLights.Empty();

    for (const FLight_BiomeAtmosphere& Biome : BiomeSettings)
    {
        for (int32 i = 0; i < 3; i++)
        {
            FVector SpawnLocation = Biome.BiomeCenter;
            SpawnLocation.X += FMath::RandRange(-8000.0f, 8000.0f);
            SpawnLocation.Y += FMath::RandRange(-8000.0f, 8000.0f);
            SpawnLocation.Z += FMath::RandRange(500.0f, 1500.0f);

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            APointLight* PointLightActor = World->SpawnActor<APointLight>(APointLight::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
            
            if (PointLightActor)
            {
                UPointLightComponent* PointLightComp = PointLightActor->GetPointLightComponent();
                if (PointLightComp)
                {
                    PointLightComp->SetIntensity(Biome.LightIntensity);
                    PointLightComp->SetLightColor(Biome.AtmosphericColor);
                    PointLightComp->SetAttenuationRadius(5000.0f);
                    PointLightComp->SetCastVolumetricShadow(true);
                    
                    BiomeAtmosphericLights.Add(PointLightComp);
                }
                
                PointLightActor->SetActorLabel(FString::Printf(TEXT("AtmosphericLight_%s_%d"), *Biome.BiomeName, i));
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Light_AtmosphereManager: Spawned %d atmospheric lights"), BiomeAtmosphericLights.Num());
}

FLight_BiomeAtmosphere ALight_AtmosphereManager::GetBiomeAtmosphereByLocation(FVector Location)
{
    float MinDistance = FLT_MAX;
    FLight_BiomeAtmosphere ClosestBiome;

    for (const FLight_BiomeAtmosphere& Biome : BiomeSettings)
    {
        float Distance = FVector::Dist(Location, Biome.BiomeCenter);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome;
        }
    }

    return ClosestBiome;
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight)
    {
        return;
    }

    // Calculate sun position based on time of day
    float SunAngle = CurrentTimeOfDay * 360.0f - 90.0f; // -90 to start at dawn
    float SunPitch = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunYaw = 45.0f; // Fixed yaw for consistent shadows

    // Clamp pitch for realistic sun movement
    SunPitch = FMath::Clamp(SunPitch, -25.0f, 85.0f);

    FRotator NewRotation(SunPitch, SunYaw, 0.0f);
    GetOwner()->SetActorRotation(NewRotation);
}

void ALight_AtmosphereManager::UpdateAtmosphericProperties()
{
    if (!SunLight)
    {
        return;
    }

    // Adjust sun intensity based on time of day
    float IntensityMultiplier = 1.0f;
    
    switch (CurrentPeriod)
    {
        case ELight_TimeOfDay::Dawn:
            IntensityMultiplier = 0.3f;
            break;
        case ELight_TimeOfDay::Morning:
            IntensityMultiplier = 0.7f;
            break;
        case ELight_TimeOfDay::Noon:
            IntensityMultiplier = 1.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            IntensityMultiplier = 0.8f;
            break;
        case ELight_TimeOfDay::Dusk:
            IntensityMultiplier = 0.4f;
            break;
        case ELight_TimeOfDay::Night:
            IntensityMultiplier = 0.1f;
            break;
    }

    SunLight->SetIntensity(8.5f * IntensityMultiplier);
}

ELight_TimeOfDay ALight_AtmosphereManager::CalculateTimeOfDay(float TimeValue)
{
    if (TimeValue < 0.125f) return ELight_TimeOfDay::Night;
    if (TimeValue < 0.25f) return ELight_TimeOfDay::Dawn;
    if (TimeValue < 0.5f) return ELight_TimeOfDay::Morning;
    if (TimeValue < 0.625f) return ELight_TimeOfDay::Noon;
    if (TimeValue < 0.75f) return ELight_TimeOfDay::Afternoon;
    if (TimeValue < 0.875f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}