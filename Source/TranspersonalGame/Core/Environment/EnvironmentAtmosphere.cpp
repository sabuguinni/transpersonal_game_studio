#include "EnvironmentAtmosphere.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

UEnvironmentAtmosphere::UEnvironmentAtmosphere()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    SunLight = nullptr;
    VolumeFog = nullptr;
    CurrentDayTime = 0.0f;
}

void UEnvironmentAtmosphere::BeginPlay()
{
    Super::BeginPlay();
    
    FindOrCreateLightingActors();
    UpdateAtmosphere();
}

void UEnvironmentAtmosphere::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableDynamicLighting && DayDurationSeconds > 0.0f)
    {
        CurrentDayTime += DeltaTime;
        if (CurrentDayTime >= DayDurationSeconds)
        {
            CurrentDayTime = 0.0f;
        }
        
        // Calculate time of day based on current day time
        float DayProgress = CurrentDayTime / DayDurationSeconds;
        int32 TimeIndex = FMath::FloorToInt(DayProgress * 6.0f); // 6 time periods
        EEnvArt_TimeOfDay NewTimeOfDay = static_cast<EEnvArt_TimeOfDay>(FMath::Clamp(TimeIndex, 0, 5));
        
        if (NewTimeOfDay != CurrentTimeOfDay)
        {
            SetTimeOfDay(NewTimeOfDay);
        }
    }
}

void UEnvironmentAtmosphere::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    AtmosphereSettings = GetSettingsForTimeOfDay(NewTimeOfDay);
    UpdateAtmosphere();
}

void UEnvironmentAtmosphere::UpdateAtmosphere()
{
    if (bEnableDynamicLighting)
    {
        UpdateDirectionalLight(AtmosphereSettings);
    }
    
    if (bEnableVolumetricFog && VolumeFog)
    {
        UExponentialHeightFogComponent* FogComponent = VolumeFog->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(AtmosphereSettings.FogDensity);
            FogComponent->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
            FogComponent->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        }
    }
}

void UEnvironmentAtmosphere::CreateVolumetricFog(FVector Location, const FEnvArt_AtmosphereSettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    VolumeFog = World->SpawnActor<AExponentialHeightFog>(Location, FRotator::ZeroRotator, SpawnParams);
    if (VolumeFog)
    {
        UExponentialHeightFogComponent* FogComponent = VolumeFog->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(Settings.FogDensity);
            FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
            FogComponent->SetFogInscatteringColor(Settings.FogColor);
        }
        
        VolumeFog->SetActorLabel(TEXT("Environment_VolumetricFog"));
    }
}

void UEnvironmentAtmosphere::UpdateDirectionalLight(const FEnvArt_AtmosphereSettings& Settings)
{
    if (!SunLight) return;
    
    SunLight->SetActorRotation(Settings.SunRotation);
    
    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    if (LightComponent)
    {
        LightComponent->SetIntensity(Settings.SunIntensity);
        LightComponent->SetLightColor(Settings.SunColor);
    }
}

void UEnvironmentAtmosphere::FindOrCreateLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing directional light
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }
    else
    {
        // Create new directional light if none exists
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        SunLight = World->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("Environment_Sun"));
        }
    }
    
    // Find existing fog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    
    if (FoundFogs.Num() > 0)
    {
        VolumeFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
    }
}

FEnvArt_AtmosphereSettings UEnvironmentAtmosphere::GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const
{
    FEnvArt_AtmosphereSettings Settings;
    
    switch (TimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            Settings.SunIntensity = 2.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
            Settings.SunRotation = FRotator(-80.0f, 90.0f, 0.0f);
            Settings.FogDensity = 0.05f;
            Settings.FogColor = FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Morning:
            Settings.SunIntensity = 3.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunRotation = FRotator(-45.0f, 60.0f, 0.0f);
            Settings.FogDensity = 0.03f;
            Settings.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Noon:
            Settings.SunIntensity = 4.0f;
            Settings.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
            Settings.SunRotation = FRotator(-10.0f, 0.0f, 0.0f);
            Settings.FogDensity = 0.01f;
            Settings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Afternoon:
            Settings.SunIntensity = 3.5f;
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
            Settings.FogDensity = 0.02f;
            Settings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Dusk:
            Settings.SunIntensity = 2.5f;
            Settings.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
            Settings.SunRotation = FRotator(-70.0f, -45.0f, 0.0f);
            Settings.FogDensity = 0.04f;
            Settings.FogColor = FLinearColor(0.9f, 0.6f, 0.5f, 1.0f);
            break;
            
        case EEnvArt_TimeOfDay::Night:
            Settings.SunIntensity = 0.5f;
            Settings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
            Settings.SunRotation = FRotator(-120.0f, 0.0f, 0.0f);
            Settings.FogDensity = 0.06f;
            Settings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
            break;
    }
    
    return Settings;
}