#include "Light_AtmosphericManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricClouds.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Configurações padrão para diferentes momentos
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SkyTint = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SkyIntensity = 0.5f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.CloudCoverage = 0.3f;

    NoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    NoonSettings.SunIntensity = 10.0f;
    NoonSettings.SkyTint = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);
    NoonSettings.SkyIntensity = 1.0f;
    NoonSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    NoonSettings.FogDensity = 0.02f;
    NoonSettings.CloudCoverage = 0.4f;

    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SkyTint = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    DuskSettings.SkyIntensity = 0.3f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.CloudCoverage = 0.6f;

    NightSettings.SunColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SkyTint = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    NightSettings.SkyIntensity = 0.1f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.2f, 0.4f, 1.0f);
    NightSettings.FogDensity = 0.08f;
    NightSettings.CloudCoverage = 0.7f;
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    UpdateSunPosition();
    UpdateAtmosphericSettings();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDynamicTimeEnabled && DayDurationMinutes > 0.0f)
    {
        // Avançar o tempo: 24 horas em DayDurationMinutes minutos
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        TimeOfDayHours += TimeIncrement;
        
        if (TimeOfDayHours >= 24.0f)
        {
            TimeOfDayHours -= 24.0f;
        }

        // Actualizar iluminação a cada 0.1 segundos para performance
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= 0.1f)
        {
            UpdateSunPosition();
            UpdateAtmosphericSettings();
            ApplyWeatherEffects();
            LastUpdateTime = 0.0f;
        }
    }
}

void ALight_AtmosphericManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Encontrar actores de iluminação existentes
    TArray<AActor*> FoundActors;
    
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunActor = Cast<ADirectionalLight>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphereActor = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricClouds::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CloudsActor = Cast<AVolumetricClouds>(FoundActors[0]);
    }

    UE_LOG(LogTemp, Warning, TEXT("AtmosphericManager found: Sun=%s, Sky=%s, Atmosphere=%s, Fog=%s, Clouds=%s"),
        SunActor ? TEXT("Yes") : TEXT("No"),
        SkyLightActor ? TEXT("Yes") : TEXT("No"),
        AtmosphereActor ? TEXT("Yes") : TEXT("No"),
        FogActor ? TEXT("Yes") : TEXT("No"),
        CloudsActor ? TEXT("Yes") : TEXT("No"));
}

void ALight_AtmosphericManager::UpdateSunPosition()
{
    if (!SunActor) return;

    float SunAngle = CalculateSunAngle(TimeOfDayHours);
    
    // Calcular rotação do sol baseada na hora
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // Sol nasce a leste, põe-se a oeste
    SunRotation.Roll = 0.0f;

    SunActor->SetActorRotation(SunRotation);

    // Ajustar intensidade baseada na altura do sol
    if (SunActor->GetLightComponent())
    {
        float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));
        float IntensityMultiplier = FMath::Max(0.1f, SunHeight);
        
        FLight_AtmosphericSettings CurrentSettings = GetSettingsForTime(TimeOfDayHours);
        SunActor->GetLightComponent()->SetIntensity(CurrentSettings.SunIntensity * IntensityMultiplier);
    }
}

void ALight_AtmosphericManager::UpdateAtmosphericSettings()
{
    FLight_AtmosphericSettings CurrentSettings = GetSettingsForTime(TimeOfDayHours);

    // Aplicar configurações ao sol
    if (SunActor && SunActor->GetLightComponent())
    {
        SunActor->GetLightComponent()->SetLightColor(CurrentSettings.SunColor);
    }

    // Aplicar configurações à sky light
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->SetIntensity(CurrentSettings.SkyIntensity);
        SkyLightActor->GetLightComponent()->RecaptureSky();
    }

    // Aplicar configurações ao fog
    if (FogActor && FogActor->GetComponent())
    {
        FogActor->GetComponent()->SetFogInscatteringColor(CurrentSettings.FogColor);
        FogActor->GetComponent()->SetFogDensity(CurrentSettings.FogDensity);
    }

    // Aplicar configurações às nuvens
    if (CloudsActor && CloudsActor->GetVolumetricCloudComponent())
    {
        CloudsActor->GetVolumetricCloudComponent()->SetLayerBottomAltitude(1.0f);
        CloudsActor->GetVolumetricCloudComponent()->SetLayerHeight(4.0f);
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(float Hours)
{
    TimeOfDayHours = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateAtmosphericSettings();
}

void ALight_AtmosphericManager::SetWeather(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    ApplyWeatherEffects();
}

void ALight_AtmosphericManager::ApplyBiomeLighting(const FString& BiomeName)
{
    // Reset all biome flags
    bUseSwampLighting = false;
    bUseForestLighting = false;
    bUseDesertLighting = false;
    bUseMountainLighting = false;

    if (BiomeName.Contains(TEXT("Swamp")) || BiomeName.Contains(TEXT("Pântano")))
    {
        bUseSwampLighting = true;
        // Ajustar fog para ser mais denso e verde
        if (FogActor && FogActor->GetComponent())
        {
            FogActor->GetComponent()->SetFogInscatteringColor(FLinearColor(0.4f, 0.6f, 0.4f, 1.0f));
            FogActor->GetComponent()->SetFogDensity(0.08f);
        }
    }
    else if (BiomeName.Contains(TEXT("Forest")) || BiomeName.Contains(TEXT("Floresta")))
    {
        bUseForestLighting = true;
        // Reduzir intensidade da luz para simular sombra das árvores
        if (SunActor && SunActor->GetLightComponent())
        {
            SunActor->GetLightComponent()->SetIntensity(SunActor->GetLightComponent()->Intensity * 0.7f);
        }
    }
    else if (BiomeName.Contains(TEXT("Desert")) || BiomeName.Contains(TEXT("Deserto")))
    {
        bUseDesertLighting = true;
        // Aumentar intensidade e tornar mais quente
        if (SunActor && SunActor->GetLightComponent())
        {
            SunActor->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
            SunActor->GetLightComponent()->SetIntensity(SunActor->GetLightComponent()->Intensity * 1.3f);
        }
    }
    else if (BiomeName.Contains(TEXT("Mountain")) || BiomeName.Contains(TEXT("Montanha")))
    {
        bUseMountainLighting = true;
        // Ar mais claro, menos fog
        if (FogActor && FogActor->GetComponent())
        {
            FogActor->GetComponent()->SetFogDensity(0.01f);
        }
    }
}

ELight_TimeOfDay ALight_AtmosphericManager::GetCurrentTimeOfDay() const
{
    if (TimeOfDayHours >= 5.0f && TimeOfDayHours < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (TimeOfDayHours >= 7.0f && TimeOfDayHours < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (TimeOfDayHours >= 11.0f && TimeOfDayHours < 15.0f)
        return ELight_TimeOfDay::Noon;
    else if (TimeOfDayHours >= 15.0f && TimeOfDayHours < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (TimeOfDayHours >= 18.0f && TimeOfDayHours < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphericManager::EnableDynamicTimeProgression(bool bEnable)
{
    bDynamicTimeEnabled = bEnable;
}

void ALight_AtmosphericManager::InterpolateSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha)
{
    // Implementar interpolação suave entre configurações
    // Esta função pode ser expandida para transições mais suaves
}

void ALight_AtmosphericManager::ApplyWeatherEffects()
{
    if (!FogActor || !FogActor->GetComponent()) return;

    switch (CurrentWeather)
    {
        case ELight_WeatherType::Clear:
            FogActor->GetComponent()->SetFogDensity(0.02f);
            break;
        case ELight_WeatherType::Cloudy:
            FogActor->GetComponent()->SetFogDensity(0.04f);
            if (SunActor && SunActor->GetLightComponent())
            {
                SunActor->GetLightComponent()->SetIntensity(SunActor->GetLightComponent()->Intensity * 0.8f);
            }
            break;
        case ELight_WeatherType::Stormy:
            FogActor->GetComponent()->SetFogDensity(0.06f);
            FogActor->GetComponent()->SetFogInscatteringColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
            break;
        case ELight_WeatherType::Foggy:
            FogActor->GetComponent()->SetFogDensity(0.15f);
            break;
        case ELight_WeatherType::Dusty:
            FogActor->GetComponent()->SetFogDensity(0.08f);
            FogActor->GetComponent()->SetFogInscatteringColor(FLinearColor(0.8f, 0.7f, 0.5f, 1.0f));
            break;
    }
}

FLight_AtmosphericSettings ALight_AtmosphericManager::GetSettingsForTime(float Hours) const
{
    if (Hours >= 5.0f && Hours < 7.0f)
    {
        // Dawn
        return DawnSettings;
    }
    else if (Hours >= 7.0f && Hours < 18.0f)
    {
        // Day (interpolate between dawn, noon, dusk)
        if (Hours < 12.0f)
        {
            float Alpha = (Hours - 7.0f) / 5.0f; // 7-12 hours
            // Interpolar entre dawn e noon
            return NoonSettings; // Simplificado
        }
        else
        {
            float Alpha = (Hours - 12.0f) / 6.0f; // 12-18 hours
            // Interpolar entre noon e dusk
            return NoonSettings; // Simplificado
        }
    }
    else if (Hours >= 18.0f && Hours < 20.0f)
    {
        // Dusk
        return DuskSettings;
    }
    else
    {
        // Night
        return NightSettings;
    }
}

float ALight_AtmosphericManager::CalculateSunAngle(float Hours) const
{
    // Sol nasce às 6h (90°), meio-dia às 12h (0°), põe-se às 18h (-90°)
    // Mapear 0-24h para ângulo do sol
    float NormalizedHour = FMath::Fmod(Hours + 6.0f, 24.0f); // Offset para nascer às 6h
    float SunAngle = (NormalizedHour / 24.0f) * 360.0f - 90.0f;
    
    return SunAngle;
}