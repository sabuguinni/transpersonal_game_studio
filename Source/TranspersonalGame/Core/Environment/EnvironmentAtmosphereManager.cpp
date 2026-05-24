#include "EnvironmentAtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AEnvironmentAtmosphereManager::AEnvironmentAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default biome atmospheres
    BiomeAtmospheres.SetNum(5);
    
    // Savana - Golden grasslands
    BiomeAtmospheres[0].BiomeType = EBiomeType::Savana;
    BiomeAtmospheres[0].GoldenHour.SunElevation = -15.0f;
    BiomeAtmospheres[0].GoldenHour.SunAzimuth = 45.0f;
    BiomeAtmospheres[0].GoldenHour.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    BiomeAtmospheres[0].GoldenHour.SunIntensity = 3.0f;
    BiomeAtmospheres[0].GoldenHour.FogDensity = 0.01f;
    BiomeAtmospheres[0].VolumeFogDensity = 0.005f;
    
    // Pantano - Misty swamplands
    BiomeAtmospheres[1].BiomeType = EBiomeType::Pantano;
    BiomeAtmospheres[1].GoldenHour.SunColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f);
    BiomeAtmospheres[1].GoldenHour.FogDensity = 0.05f;
    BiomeAtmospheres[1].VolumeFogDensity = 0.03f;
    BiomeAtmospheres[1].VolumeFogColor = FLinearColor(0.6f, 0.7f, 0.6f, 1.0f);
    
    // Floresta - Dense forest canopy
    BiomeAtmospheres[2].BiomeType = EBiomeType::Floresta;
    BiomeAtmospheres[2].GoldenHour.SunColor = FLinearColor(0.7f, 0.8f, 0.6f, 1.0f);
    BiomeAtmospheres[2].GoldenHour.SunIntensity = 2.0f;
    BiomeAtmospheres[2].VolumeFogDensity = 0.02f;
    BiomeAtmospheres[2].VolumeFogColor = FLinearColor(0.4f, 0.6f, 0.4f, 1.0f);
    
    // Deserto - Harsh desert sun
    BiomeAtmospheres[3].BiomeType = EBiomeType::Deserto;
    BiomeAtmospheres[3].GoldenHour.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    BiomeAtmospheres[3].GoldenHour.SunIntensity = 4.0f;
    BiomeAtmospheres[3].GoldenHour.FogDensity = 0.005f;
    BiomeAtmospheres[3].VolumeFogDensity = 0.001f;
    
    // Montanha - High altitude clarity
    BiomeAtmospheres[4].BiomeType = EBiomeType::Montanha;
    BiomeAtmospheres[4].GoldenHour.SunColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    BiomeAtmospheres[4].GoldenHour.SunIntensity = 3.5f;
    BiomeAtmospheres[4].GoldenHour.FogDensity = 0.003f;
    BiomeAtmospheres[4].VolumeFogDensity = 0.008f;
}

void AEnvironmentAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingActors();
    ApplyGoldenHourLighting();
}

void AEnvironmentAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day
    CurrentTimeOfDay += DeltaTime * TimeSpeed / 3600.0f; // Convert to hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
    
    UpdateSunPosition();
    UpdateSkyColor();
    UpdateFogSettings();
}

void AEnvironmentAtmosphereManager::InitializeLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find or create directional light
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }
    else
    {
        // Create new directional light
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SunLight");
        SunLight = World->SpawnActor<ADirectionalLight>(SpawnParams);
    }
    
    // Find or create sky light
    TArray<AActor*> FoundSkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkyLights);
    
    if (FoundSkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundSkyLights[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SkyLight");
        SkyLight = World->SpawnActor<ASkyLight>(SpawnParams);
    }
    
    // Find or create height fog
    TArray<AActor*> FoundFog;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFog);
    
    if (FoundFog.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundFog[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("HeightFog");
        HeightFog = World->SpawnActor<AExponentialHeightFog>(SpawnParams);
    }
}

void AEnvironmentAtmosphereManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateSkyColor();
    UpdateFogSettings();
}

void AEnvironmentAtmosphereManager::SetBiomeAtmosphere(EBiomeType BiomeType)
{
    CurrentBiome = BiomeType;
    
    // Find matching biome atmosphere
    for (const FEnvArt_BiomeAtmosphere& BiomeAtmo : BiomeAtmospheres)
    {
        if (BiomeAtmo.BiomeType == BiomeType)
        {
            SetupVolumeFog(BiomeAtmo.VolumeFogDensity, BiomeAtmo.VolumeFogColor);
            break;
        }
    }
}

void AEnvironmentAtmosphereManager::ApplyGoldenHourLighting()
{
    if (!SunLight) return;
    
    FEnvArt_TimeOfDaySettings GoldenHourSettings = GetCurrentTimeSettings();
    
    // Set sun rotation for golden hour
    FRotator SunRotation;
    SunRotation.Pitch = GoldenHourSettings.SunElevation;
    SunRotation.Yaw = GoldenHourSettings.SunAzimuth;
    SunRotation.Roll = 0.0f;
    SunLight->SetActorRotation(SunRotation);
    
    // Configure directional light component
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    if (LightComp)
    {
        LightComp->SetLightColor(GoldenHourSettings.SunColor);
        LightComp->SetIntensity(GoldenHourSettings.SunIntensity);
        LightComp->SetCastShadows(true);
        LightComp->SetCastVolumetricShadow(true);
    }
    
    // Update sky light
    if (SkyLight)
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(1.0f);
            SkyComp->RecaptureSky();
        }
    }
}

void AEnvironmentAtmosphereManager::SetupVolumeFog(float Density, FLinearColor Color)
{
    if (!HeightFog) return;
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(Density);
        FogComp->SetFogInscatteringColor(Color);
        FogComp->SetFogHeightFalloff(0.2f);
        FogComp->SetFogMaxOpacity(1.0f);
        FogComp->SetStartDistance(1000.0f);
        FogComp->SetFogCutoffDistance(100000.0f);
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.2f);
        FogComp->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
    }
}

void AEnvironmentAtmosphereManager::UpdateAtmosphericScattering()
{
    // Update atmospheric scattering based on sun position and biome
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            // Enable atmospheric sun disk
            LightComp->SetAtmosphereSunDisk(true);
            LightComp->SetAtmosphereSunDiskColorScale(FLinearColor(1.0f, 1.0f, 1.0f));
        }
    }
}

void AEnvironmentAtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun position based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    
    FRotator SunRotation;
    SunRotation.Pitch = SunElevation;
    SunRotation.Yaw = SunAngle;
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
}

void AEnvironmentAtmosphereManager::UpdateSkyColor()
{
    // Update sky light color based on time of day and biome
    if (SkyLight)
    {
        FEnvArt_TimeOfDaySettings CurrentSettings = GetCurrentTimeSettings();
        
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetLightColor(CurrentSettings.SkyColor);
            SkyComp->RecaptureSky();
        }
    }
}

void AEnvironmentAtmosphereManager::UpdateFogSettings()
{
    if (!HeightFog) return;
    
    FEnvArt_TimeOfDaySettings CurrentSettings = GetCurrentTimeSettings();
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(CurrentSettings.FogDensity);
        FogComp->SetFogInscatteringColor(CurrentSettings.FogColor);
    }
}

FEnvArt_TimeOfDaySettings AEnvironmentAtmosphereManager::GetCurrentTimeSettings()
{
    // Find current biome settings
    for (const FEnvArt_BiomeAtmosphere& BiomeAtmo : BiomeAtmospheres)
    {
        if (BiomeAtmo.BiomeType == CurrentBiome)
        {
            // For now, return golden hour settings
            // TODO: Interpolate between different times based on CurrentTimeOfDay
            return BiomeAtmo.GoldenHour;
        }
    }
    
    // Default fallback
    FEnvArt_TimeOfDaySettings DefaultSettings;
    return DefaultSettings;
}