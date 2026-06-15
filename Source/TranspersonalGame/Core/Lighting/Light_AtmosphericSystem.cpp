#include "Light_AtmosphericSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    CurrentTimeOfDay = 12.0f;
    DayDurationMinutes = 20.0f;
    CurrentWeather = ELight_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
    bEnableTimeProgression = true;
    bEnableWeatherChanges = true;
    AtmosphericIntensityMultiplier = 1.0f;
    
    // Initialize lighting references
    SunLight = nullptr;
    SkyLightActor = nullptr;
    FogActor = nullptr;
    CloudActor = nullptr;
    
    // Initialize transition system
    TransitionProgress = 0.0f;
    TransitionDuration = 2.0f;
    bIsTransitioning = false;
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePresets();
    FindLightingActors();
    CreateAtmosphericLighting();
    
    // Set initial lighting based on current time
    UpdateTimeBasedLighting();
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableTimeProgression)
    {
        // Progress time of day
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        UpdateTimeBasedLighting();
        UpdateSunPosition();
    }
    
    if (bIsTransitioning)
    {
        UpdateLightingTransition(DeltaTime);
    }
    
    if (bEnableWeatherChanges)
    {
        UpdateWeatherEffects();
    }
    
    UpdateFogSettings();
}

void ALight_AtmosphericSystem::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateTimeBasedLighting();
    UpdateSunPosition();
}

void ALight_AtmosphericSystem::SetWeatherType(ELight_WeatherType NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        
        // Start transition to new weather preset
        if (WeatherPresets.Contains(NewWeather))
        {
            TransitionToPreset(WeatherPresets[NewWeather], 3.0f);
        }
    }
}

void ALight_AtmosphericSystem::TransitionToPreset(const FLight_LightingPreset& TargetPreset, float TransitionDuration)
{
    this->TargetPreset = TargetPreset;
    this->TransitionDuration = TransitionDuration;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
}

ELight_TimeOfDay ALight_AtmosphericSystem::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Noon;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 21.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphericSystem::CreateAtmosphericLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create atmospheric fog if it doesn't exist
    if (!FogActor)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName("PrehistoricAtmosphericFog");
        FogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), 
            FVector(0, 0, 500), FRotator::ZeroRotator, SpawnParams);
        
        if (FogActor && FogActor->GetComponent())
        {
            UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
            FogComp->FogDensity = 0.02f;
            FogComp->FogHeightFalloff = 0.2f;
            FogComp->FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
            FogComp->FogMaxOpacity = 0.8f;
            FogComp->VolumetricFog = true;
            FogComp->VolumetricFogScatteringDistribution = 0.2f;
            FogComp->VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
        }
    }
    
    // Create volumetric clouds if they don't exist
    if (!CloudActor)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName("PrehistoricClouds");
        CloudActor = World->SpawnActor<AVolumetricCloud>(AVolumetricCloud::StaticClass(),
            FVector(0, 0, 2000), FRotator::ZeroRotator, SpawnParams);
        
        if (CloudActor && CloudActor->GetVolumetricCloudComponent())
        {
            UVolumetricCloudComponent* CloudComp = CloudActor->GetVolumetricCloudComponent();
            CloudComp->LayerBottomAltitude = 1.5f;
            CloudComp->LayerHeight = 4.0f;
            CloudComp->ViewSampleCountScale = 0.8f;
            CloudComp->ReflectionSampleCountScale = 0.5f;
            CloudComp->ShadowSampleCountScale = 0.5f;
        }
    }
}

void ALight_AtmosphericSystem::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun angle based on time of day (0-24 hours)
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
    
    // Create sun rotation (pitch based on time, yaw can be adjusted for seasonal variation)
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // Can be modified for seasonal sun path
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
    
    // Adjust sun intensity based on angle (dimmer when low on horizon)
    if (SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComp = SunLight->GetLightComponent();
        float IntensityMultiplier = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunAngle + 90.0f)), 0.1f, 1.0f);
        SunComp->SetIntensity(CurrentPreset.SunIntensity * IntensityMultiplier * AtmosphericIntensityMultiplier);
    }
}

void ALight_AtmosphericSystem::UpdateFogSettings()
{
    if (!FogActor || !FogActor->GetComponent()) return;
    
    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
    
    // Adjust fog based on weather and time
    float FogMultiplier = 1.0f;
    
    switch (CurrentWeather)
    {
        case ELight_WeatherType::Foggy:
            FogMultiplier = 3.0f;
            break;
        case ELight_WeatherType::Stormy:
            FogMultiplier = 2.0f;
            break;
        case ELight_WeatherType::Overcast:
            FogMultiplier = 1.5f;
            break;
        case ELight_WeatherType::Volcanic:
            FogMultiplier = 2.5f;
            FogComp->FogInscatteringColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
            break;
        default:
            FogMultiplier = 1.0f;
            break;
    }
    
    // Night fog is denser
    ELight_TimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
    if (TimeEnum == ELight_TimeOfDay::Night || TimeEnum == ELight_TimeOfDay::Dawn)
    {
        FogMultiplier *= 1.5f;
    }
    
    FogComp->FogDensity = CurrentPreset.FogDensity * FogMultiplier;
}

void ALight_AtmosphericSystem::CreateVolcanicAtmosphere()
{
    if (!bEnableVolcanicGlow) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Create volcanic glow lights
    for (int32 i = 0; i < 3; i++)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("VolcanicGlow_%d"), i + 1));
        
        FVector Location = FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(100.0f, 800.0f)
        );
        
        APointLight* VolcanicLight = World->SpawnActor<APointLight>(APointLight::StaticClass(),
            Location, FRotator::ZeroRotator, SpawnParams);
        
        if (VolcanicLight && VolcanicLight->GetLightComponent())
        {
            UPointLightComponent* LightComp = VolcanicLight->GetLightComponent();
            LightComp->SetIntensity(VolcanicGlowIntensity);
            LightComp->SetLightColor(VolcanicGlowColor);
            LightComp->SetAttenuationRadius(2500.0f);
            LightComp->SetCastShadows(false);
            LightComp->VolumetricScatteringIntensity = 2.0f;
        }
    }
}

void ALight_AtmosphericSystem::InitializePresets()
{
    // Initialize time-based presets
    FLight_LightingPreset DawnPreset;
    DawnPreset.SunIntensity = 2.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnPreset.SunTemperature = 3000.0f;
    DawnPreset.SkyLightIntensity = 0.5f;
    DawnPreset.FogDensity = 0.04f;
    DawnPreset.VolumetricScattering = 1.5f;
    TimePresets.Add(ELight_TimeOfDay::Dawn, DawnPreset);
    
    FLight_LightingPreset NoonPreset;
    NoonPreset.SunIntensity = 6.0f;
    NoonPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    NoonPreset.SunTemperature = 5800.0f;
    NoonPreset.SkyLightIntensity = 1.5f;
    NoonPreset.FogDensity = 0.015f;
    NoonPreset.VolumetricScattering = 1.0f;
    TimePresets.Add(ELight_TimeOfDay::Noon, NoonPreset);
    
    FLight_LightingPreset DuskPreset;
    DuskPreset.SunIntensity = 3.0f;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskPreset.SunTemperature = 2500.0f;
    DuskPreset.SkyLightIntensity = 0.8f;
    DuskPreset.FogDensity = 0.03f;
    DuskPreset.VolumetricScattering = 2.0f;
    TimePresets.Add(ELight_TimeOfDay::Dusk, DuskPreset);
    
    FLight_LightingPreset NightPreset;
    NightPreset.SunIntensity = 0.1f;
    NightPreset.SunColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f);
    NightPreset.SunTemperature = 8000.0f;
    NightPreset.SkyLightIntensity = 0.2f;
    NightPreset.FogDensity = 0.05f;
    NightPreset.VolumetricScattering = 0.5f;
    TimePresets.Add(ELight_TimeOfDay::Night, NightPreset);
    
    // Initialize weather presets
    FLight_LightingPreset StormyPreset;
    StormyPreset.SunIntensity = 1.5f;
    StormyPreset.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    StormyPreset.SkyLightIntensity = 0.4f;
    StormyPreset.FogDensity = 0.08f;
    StormyPreset.CloudCoverage = 0.9f;
    WeatherPresets.Add(ELight_WeatherType::Stormy, StormyPreset);
    
    FLight_LightingPreset VolcanicPreset;
    VolcanicPreset.SunIntensity = 2.0f;
    VolcanicPreset.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    VolcanicPreset.SkyLightIntensity = 0.6f;
    VolcanicPreset.FogDensity = 0.06f;
    VolcanicPreset.FogColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    VolcanicPreset.VolumetricScattering = 3.0f;
    WeatherPresets.Add(ELight_WeatherType::Volcanic, VolcanicPreset);
    
    // Set current preset to noon clear weather
    CurrentPreset = NoonPreset;
}

void ALight_AtmosphericSystem::UpdateLightingTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;
    
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentPreset = TargetPreset;
    }
    
    // Interpolate between current and target presets
    FLight_LightingPreset InterpolatedPreset = InterpolateLightingPresets(CurrentPreset, TargetPreset, TransitionProgress);
    ApplyLightingPreset(InterpolatedPreset);
}

void ALight_AtmosphericSystem::ApplyLightingPreset(const FLight_LightingPreset& Preset)
{
    // Apply to sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComp = SunLight->GetLightComponent();
        SunComp->SetLightColor(Preset.SunColor);
        SunComp->SetTemperature(Preset.SunTemperature);
        SunComp->VolumetricScatteringIntensity = Preset.VolumetricScattering;
    }
    
    // Apply to sky light
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        SkyComp->SetIntensity(Preset.SkyLightIntensity * AtmosphericIntensityMultiplier);
    }
    
    // Apply to fog
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->FogInscatteringColor = Preset.FogColor;
        FogComp->FogHeightFalloff = Preset.FogHeightFalloff;
    }
}

void ALight_AtmosphericSystem::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing lighting actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

FLight_LightingPreset ALight_AtmosphericSystem::InterpolateLightingPresets(const FLight_LightingPreset& A, const FLight_LightingPreset& B, float Alpha) const
{
    FLight_LightingPreset Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunTemperature = FMath::Lerp(A.SunTemperature, B.SunTemperature, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.VolumetricScattering = FMath::Lerp(A.VolumetricScattering, B.VolumetricScattering, Alpha);
    
    return Result;
}

void ALight_AtmosphericSystem::UpdateTimeBasedLighting()
{
    ELight_TimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    
    if (TimePresets.Contains(CurrentTimeEnum))
    {
        FLight_LightingPreset TimePreset = TimePresets[CurrentTimeEnum];
        
        // Blend with weather preset if active
        if (WeatherPresets.Contains(CurrentWeather))
        {
            FLight_LightingPreset WeatherPreset = WeatherPresets[CurrentWeather];
            TimePreset = InterpolateLightingPresets(TimePreset, WeatherPreset, 0.5f);
        }
        
        if (!bIsTransitioning)
        {
            TransitionToPreset(TimePreset, 1.0f);
        }
    }
}

void ALight_AtmosphericSystem::UpdateWeatherEffects()
{
    // Weather change logic can be implemented here
    // For now, weather changes are manual via SetWeatherType
}