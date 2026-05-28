#include "EnvArt_AtmosphericManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentTimeOfDay.Hour = 12.0f;
    CurrentTimeOfDay.Minute = 0.0f;
    CurrentTimeOfDay.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    CurrentTimeOfDay.SunIntensity = 3.0f;
    CurrentTimeOfDay.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    FogSettings.FogDensity = 0.02f;
    FogSettings.FogHeightFalloff = 0.2f;
    FogSettings.FogInscatteringColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    FogSettings.StartDistance = 500.0f;

    SunLight = nullptr;
    SkyLight = nullptr;
    HeightFog = nullptr;
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndAssignLightingActors();
    UpdateSunLighting();
    UpdateFogSettings();
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Optional: Auto-advance time
    // CurrentTimeOfDay.Minute += DeltaTime * 0.1f; // 1 minute per 10 seconds
    // if (CurrentTimeOfDay.Minute >= 60.0f)
    // {
    //     CurrentTimeOfDay.Minute = 0.0f;
    //     CurrentTimeOfDay.Hour += 1.0f;
    //     if (CurrentTimeOfDay.Hour >= 24.0f)
    //     {
    //         CurrentTimeOfDay.Hour = 0.0f;
    //     }
    //     UpdateSunLighting();
    // }
}

void AEnvArt_AtmosphericManager::SetTimeOfDay(float Hour, float Minute)
{
    CurrentTimeOfDay.Hour = FMath::Clamp(Hour, 0.0f, 23.0f);
    CurrentTimeOfDay.Minute = FMath::Clamp(Minute, 0.0f, 59.0f);
    
    CalculateSunPosition();
    UpdateSunLighting();
}

void AEnvArt_AtmosphericManager::UpdateSunLighting()
{
    if (!SunLight)
    {
        return;
    }

    // Apply current settings
    SunLight->SetActorRotation(CurrentTimeOfDay.SunRotation);
    
    UDirectionalLightComponent* LightComp = SunLight->GetComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(CurrentTimeOfDay.SunIntensity);
        LightComp->SetLightColor(CurrentTimeOfDay.SunColor);
    }

    UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Updated sun lighting - Intensity: %f, Color: %s"), 
           CurrentTimeOfDay.SunIntensity, *CurrentTimeOfDay.SunColor.ToString());
}

void AEnvArt_AtmosphericManager::UpdateFogSettings()
{
    if (!HeightFog)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(FogSettings.FogDensity);
        FogComp->SetFogHeightFalloff(FogSettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(FogSettings.FogInscatteringColor);
        FogComp->SetStartDistance(FogSettings.StartDistance);
    }

    UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Updated fog settings - Density: %f"), FogSettings.FogDensity);
}

void AEnvArt_AtmosphericManager::SetGoldenHourLighting()
{
    CurrentTimeOfDay.Hour = 17.0f;
    CurrentTimeOfDay.Minute = 30.0f;
    CurrentTimeOfDay.SunColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    CurrentTimeOfDay.SunIntensity = 2.5f;
    CurrentTimeOfDay.SunRotation = FRotator(-15.0f, 60.0f, 0.0f);
    
    UpdateSunLighting();
    
    // Enhance fog for golden hour
    FogSettings.FogInscatteringColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    UpdateFogSettings();
}

void AEnvArt_AtmosphericManager::SetMidnightLighting()
{
    CurrentTimeOfDay.Hour = 0.0f;
    CurrentTimeOfDay.Minute = 0.0f;
    CurrentTimeOfDay.SunColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    CurrentTimeOfDay.SunIntensity = 0.1f;
    CurrentTimeOfDay.SunRotation = FRotator(-90.0f, 0.0f, 0.0f);
    
    UpdateSunLighting();
}

void AEnvArt_AtmosphericManager::FindAndAssignLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find DirectionalLight
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Found and assigned DirectionalLight"));
    }

    // Find SkyLight
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(SkyLights[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Found and assigned SkyLight"));
    }

    // Find ExponentialHeightFog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FogActors[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Found and assigned ExponentialHeightFog"));
    }
}

void AEnvArt_AtmosphericManager::CalculateSunPosition()
{
    float TotalMinutes = CurrentTimeOfDay.Hour * 60.0f + CurrentTimeOfDay.Minute;
    float TimeOfDayNormalized = TotalMinutes / (24.0f * 60.0f); // 0-1 range
    
    // Calculate sun elevation (-90 to +90 degrees)
    float SunElevation = FMath::Sin(TimeOfDayNormalized * 2.0f * PI - PI/2.0f) * 90.0f;
    
    // Calculate sun azimuth (0-360 degrees)
    float SunAzimuth = TimeOfDayNormalized * 360.0f;
    
    CurrentTimeOfDay.SunRotation = FRotator(SunElevation, SunAzimuth, 0.0f);
    
    // Adjust intensity based on elevation
    if (SunElevation > 0.0f)
    {
        CurrentTimeOfDay.SunIntensity = FMath::Clamp(SunElevation / 90.0f * 5.0f, 0.1f, 5.0f);
    }
    else
    {
        CurrentTimeOfDay.SunIntensity = 0.1f; // Night time
    }
}

void AEnvArt_AtmosphericManager::InterpolateLightingValues()
{
    // Future implementation for smooth transitions between time periods
}