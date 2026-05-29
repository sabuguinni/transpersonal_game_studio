#include "Light_AtmosphericLightingSystem.h"
#include "Engine/World.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphericLightingSystem::ALight_AtmosphericLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default Cretaceous period settings
    TimeOfDaySettings.SunAngle = -35.0f;
    TimeOfDaySettings.SunAzimuth = 120.0f;
    TimeOfDaySettings.SunIntensity = 8.5f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    
    TimeOfDaySettings.AtmosphereHeight = 80.0f;
    TimeOfDaySettings.RayleighScattering = 0.8f;
    TimeOfDaySettings.MieScattering = 0.04f;
    
    TimeOfDaySettings.FogDensity = 0.008f;
    TimeOfDaySettings.FogHeightFalloff = 0.15f;
    TimeOfDaySettings.FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    
    RimLightSettings.RimLightIntensity = 15000.0f;
    RimLightSettings.RimLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    RimLightSettings.RimLightRadius = 2500.0f;
    RimLightSettings.RimLightOffset = FVector(-800.0f, 400.0f, 600.0f);
    
    bEnableDynamicTimeOfDay = false;
    TimeOfDaySpeed = 1.0f;
    bEnableAutomaticRimLighting = true;
    CurrentTimeOfDay = 14.0f; // 2 PM late afternoon
}

void ALight_AtmosphericLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndConfigureLightingActors();
    ApplyCretaceousLighting();
    
    if (bEnableAutomaticRimLighting)
    {
        // Find dinosaur actors and create rim lighting
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("rex")) || 
                Actor->GetName().Contains(TEXT("raptor")) || 
                Actor->GetName().Contains(TEXT("brachio")))
            {
                CreateRimLightingForActor(Actor);
            }
        }
    }
}

void ALight_AtmosphericLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDynamicTimeOfDay)
    {
        CurrentTimeOfDay += (TimeOfDaySpeed * DeltaTime) / 3600.0f; // Convert to hours
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay = 0.0f;
        }
        
        SetTimeOfDay(CurrentTimeOfDay);
    }
}

void ALight_AtmosphericLightingSystem::ApplyCretaceousLighting()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        
        // Apply Cretaceous period late afternoon lighting
        SunLight->SetActorRotation(FRotator(TimeOfDaySettings.SunAngle, TimeOfDaySettings.SunAzimuth, 0.0f));
        LightComp->SetIntensity(TimeOfDaySettings.SunIntensity);
        LightComp->SetLightColor(TimeOfDaySettings.SunColor);
        LightComp->SetCastShadows(true);
        LightComp->SetCastVolumetricShadow(true);
    }
    
    UpdateAtmosphericFog();
    UpdateSkyAtmosphere();
    EnableLumenGlobalIllumination();
}

void ALight_AtmosphericLightingSystem::SetTimeOfDay(float HourOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(HourOfDay, 0.0f, 24.0f);
    
    // Calculate sun position based on time
    float SunAngle = -90.0f + (CurrentTimeOfDay / 24.0f) * 180.0f;
    float SunAzimuth = (CurrentTimeOfDay / 24.0f) * 360.0f;
    
    TimeOfDaySettings.SunAngle = SunAngle;
    TimeOfDaySettings.SunAzimuth = SunAzimuth;
    TimeOfDaySettings.SunColor = CalculateSunColorFromTime(CurrentTimeOfDay);
    TimeOfDaySettings.SunIntensity = CalculateSunIntensityFromTime(CurrentTimeOfDay);
    
    UpdateSunPosition();
}

void ALight_AtmosphericLightingSystem::CreateRimLightingForActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }
    
    FVector ActorLocation = TargetActor->GetActorLocation();
    FVector RimLightLocation = ActorLocation + RimLightSettings.RimLightOffset;
    
    APointLight* RimLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), RimLightLocation, FRotator::ZeroRotator);
    if (RimLight && RimLight->GetLightComponent())
    {
        UPointLightComponent* LightComp = RimLight->GetLightComponent();
        LightComp->SetIntensity(RimLightSettings.RimLightIntensity);
        LightComp->SetLightColor(RimLightSettings.RimLightColor);
        LightComp->SetAttenuationRadius(RimLightSettings.RimLightRadius);
        LightComp->SetCastShadows(false); // Rim lights don't cast shadows
        
        RimLight->SetActorLabel(FString::Printf(TEXT("RimLight_%s"), *TargetActor->GetName()));
        RimLights.Add(RimLight);
    }
}

void ALight_AtmosphericLightingSystem::UpdateAtmosphericSettings(const FLight_TimeOfDaySettings& NewSettings)
{
    TimeOfDaySettings = NewSettings;
    ApplyCretaceousLighting();
}

void ALight_AtmosphericLightingSystem::EnableLumenGlobalIllumination()
{
    if (UWorld* World = GetWorld())
    {
        // Enable Lumen Global Illumination
        GEngine->Exec(World, TEXT("r.DynamicGlobalIlluminationMethod 1"));
        GEngine->Exec(World, TEXT("r.ReflectionMethod 1"));
        GEngine->Exec(World, TEXT("r.Lumen.GlobalIllumination.MaxLuminance 100"));
        GEngine->Exec(World, TEXT("r.Lumen.Reflections.MaxLuminance 100"));
    }
}

void ALight_AtmosphericLightingSystem::FindAndConfigureLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light (sun)
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }
        
        // Find exponential height fog
        TArray<AActor*> FogActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
        if (FogActors.Num() > 0)
        {
            AtmosphericFog = Cast<AExponentialHeightFog>(FogActors[0]);
        }
        
        // Find sky atmosphere
        TArray<AActor*> SkyActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyActors);
        if (SkyActors.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(SkyActors[0]);
        }
    }
}

void ALight_AtmosphericLightingSystem::ApplyPrehistoricAtmosphere()
{
    FindAndConfigureLightingActors();
    ApplyCretaceousLighting();
}

void ALight_AtmosphericLightingSystem::UpdateSunPosition()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->SetActorRotation(FRotator(TimeOfDaySettings.SunAngle, TimeOfDaySettings.SunAzimuth, 0.0f));
        SunLight->GetLightComponent()->SetIntensity(TimeOfDaySettings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(TimeOfDaySettings.SunColor);
    }
}

void ALight_AtmosphericLightingSystem::UpdateAtmosphericFog()
{
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(TimeOfDaySettings.FogDensity);
        FogComp->SetFogHeightFalloff(TimeOfDaySettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(TimeOfDaySettings.FogInscatteringColor);
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.6f);
    }
}

void ALight_AtmosphericLightingSystem::UpdateSkyAtmosphere()
{
    if (SkyAtmosphere)
    {
        USkyAtmosphereComponent* AtmComp = SkyAtmosphere->GetAtmosphereComponent();
        if (AtmComp)
        {
            AtmComp->SetAtmosphereHeight(TimeOfDaySettings.AtmosphereHeight);
            AtmComp->SetRayleighScatteringScale(TimeOfDaySettings.RayleighScattering);
            AtmComp->SetMieScatteringScale(TimeOfDaySettings.MieScattering);
        }
    }
}

void ALight_AtmosphericLightingSystem::CleanupRimLights()
{
    for (APointLight* RimLight : RimLights)
    {
        if (RimLight && IsValid(RimLight))
        {
            RimLight->Destroy();
        }
    }
    RimLights.Empty();
}

FLinearColor ALight_AtmosphericLightingSystem::CalculateSunColorFromTime(float HourOfDay)
{
    // Cretaceous period color variations throughout the day
    if (HourOfDay >= 6.0f && HourOfDay <= 8.0f) // Dawn
    {
        return FLinearColor(1.0f, 0.6f, 0.4f, 1.0f); // Orange dawn
    }
    else if (HourOfDay >= 8.0f && HourOfDay <= 16.0f) // Day
    {
        return FLinearColor(1.0f, 0.85f, 0.6f, 1.0f); // Warm daylight
    }
    else if (HourOfDay >= 16.0f && HourOfDay <= 18.0f) // Dusk
    {
        return FLinearColor(1.0f, 0.5f, 0.3f, 1.0f); // Red dusk
    }
    else // Night
    {
        return FLinearColor(0.3f, 0.4f, 0.8f, 1.0f); // Cool moonlight
    }
}

float ALight_AtmosphericLightingSystem::CalculateSunIntensityFromTime(float HourOfDay)
{
    // Intensity variations throughout the day
    if (HourOfDay >= 6.0f && HourOfDay <= 18.0f) // Day
    {
        float DayProgress = (HourOfDay - 6.0f) / 12.0f;
        float IntensityCurve = FMath::Sin(DayProgress * PI);
        return FMath::Lerp(2.0f, 10.0f, IntensityCurve);
    }
    else // Night
    {
        return 0.5f; // Minimal moonlight
    }
}