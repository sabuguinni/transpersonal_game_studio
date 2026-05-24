#include "EnvironmentAtmosphereManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AEnvironmentAtmosphereManager::AEnvironmentAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default settings for Cretaceous period
    AtmosphereSettings = FEnvArt_AtmosphereSettings();
    TimeOfDay = 12.0f; // Start at noon
    DayDurationMinutes = 20.0f;
    
    SunLight = nullptr;
    SkyLightActor = nullptr;
    FogActor = nullptr;
}

void AEnvironmentAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing atmosphere actors in the level
    FindExistingAtmosphereActors();
    
    // If no actors found, create them
    if (!SunLight || !SkyLightActor || !FogActor)
    {
        CreateAtmosphereActors();
    }
    
    // Set initial Cretaceous atmosphere
    SetCretaceousAtmosphere();
}

void AEnvironmentAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day (optional - can be controlled externally)
    if (DayDurationMinutes > 0.0f)
    {
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        TimeOfDay += TimeIncrement;
        
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
        
        UpdateLightingBasedOnTime();
    }
}

void AEnvironmentAtmosphereManager::SetCretaceousAtmosphere()
{
    // Cretaceous period: warm, humid climate with high CO2
    // Lighting should be bright and warm
    AtmosphereSettings.SunIntensity = 3.5f;
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm white
    AtmosphereSettings.FogDensity = 0.015f; // Slight haze from humidity
    AtmosphereSettings.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f); // Warm fog
    AtmosphereSettings.SkyLightIntensity = 1.2f;
    
    UpdateSunPosition();
    UpdateFogSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentAtmosphereManager: Cretaceous atmosphere applied"));
}

void AEnvironmentAtmosphereManager::SetGoldenHourLighting()
{
    // Golden hour lighting for dramatic visuals
    AtmosphereSettings.SunIntensity = 2.5f;
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // Golden
    AtmosphereSettings.SunAngle = 15.0f; // Low angle
    AtmosphereSettings.FogDensity = 0.025f;
    AtmosphereSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Golden fog
    
    UpdateSunPosition();
    UpdateFogSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("EnvironmentAtmosphereManager: Golden hour lighting applied"));
}

void AEnvironmentAtmosphereManager::UpdateSunPosition()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        FRotator SunRotation = CalculateSunRotation(TimeOfDay);
        SunLight->SetActorRotation(SunRotation);
        
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CalculateSunIntensity(TimeOfDay));
        LightComp->SetLightColor(CalculateSunColor(TimeOfDay));
    }
}

void AEnvironmentAtmosphereManager::UpdateFogSettings()
{
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogDensity(AtmosphereSettings.FogDensity);
        FogComp->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        FogComp->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
    }
}

void AEnvironmentAtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateLightingBasedOnTime();
}

void AEnvironmentAtmosphereManager::CreateAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create Directional Light (Sun) if not found
    if (!SunLight)
    {
        FVector SunLocation = GetActorLocation() + FVector(0, 0, 1000);
        SunLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SunLocation, FRotator::ZeroRotator);
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("CretaceousSun"));
            UE_LOG(LogTemp, Warning, TEXT("EnvironmentAtmosphereManager: Created Directional Light"));
        }
    }
    
    // Create Sky Light if not found
    if (!SkyLightActor)
    {
        FVector SkyLocation = GetActorLocation() + FVector(0, 0, 500);
        SkyLightActor = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), SkyLocation, FRotator::ZeroRotator);
        if (SkyLightActor)
        {
            SkyLightActor->SetActorLabel(TEXT("CretaceousSky"));
            UE_LOG(LogTemp, Warning, TEXT("EnvironmentAtmosphereManager: Created Sky Light"));
        }
    }
    
    // Create Exponential Height Fog if not found
    if (!FogActor)
    {
        FVector FogLocation = GetActorLocation();
        FogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FogLocation, FRotator::ZeroRotator);
        if (FogActor)
        {
            FogActor->SetActorLabel(TEXT("CretaceousFog"));
            UE_LOG(LogTemp, Warning, TEXT("EnvironmentAtmosphereManager: Created Exponential Height Fog"));
        }
    }
}

void AEnvironmentAtmosphereManager::FindExistingAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find existing Directional Light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find existing Sky Light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
    }
    
    // Find existing Exponential Height Fog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
    }
}

void AEnvironmentAtmosphereManager::UpdateLightingBasedOnTime()
{
    UpdateSunPosition();
    UpdateFogSettings();
}

FRotator AEnvironmentAtmosphereManager::CalculateSunRotation(float TimeHours)
{
    // Convert time to sun angle (0-24 hours to 0-360 degrees)
    float SunAngleFromTime = (TimeHours / 24.0f) * 360.0f - 90.0f; // Offset so noon is overhead
    
    // Combine with manual sun angle setting
    float FinalPitch = SunAngleFromTime + AtmosphereSettings.SunAngle;
    
    return FRotator(FinalPitch, 0.0f, 0.0f);
}

FLinearColor AEnvironmentAtmosphereManager::CalculateSunColor(float TimeHours)
{
    // Vary sun color based on time of day
    if (TimeHours >= 6.0f && TimeHours <= 18.0f) // Daytime
    {
        if (TimeHours <= 8.0f || TimeHours >= 16.0f) // Golden hours
        {
            return FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // Golden
        }
        else // Midday
        {
            return AtmosphereSettings.SunColor; // Use settings color
        }
    }
    else // Night
    {
        return FLinearColor(0.1f, 0.1f, 0.2f, 1.0f); // Moonlight
    }
}

float AEnvironmentAtmosphereManager::CalculateSunIntensity(float TimeHours)
{
    // Vary sun intensity based on time of day
    if (TimeHours >= 6.0f && TimeHours <= 18.0f) // Daytime
    {
        float DaytimeProgress = (TimeHours - 6.0f) / 12.0f; // 0-1 over day
        float IntensityCurve = FMath::Sin(DaytimeProgress * PI); // Sine curve for natural falloff
        return AtmosphereSettings.SunIntensity * IntensityCurve;
    }
    else // Night
    {
        return 0.1f; // Minimal moonlight
    }
}