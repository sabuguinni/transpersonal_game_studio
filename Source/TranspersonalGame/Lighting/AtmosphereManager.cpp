#include "AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

AAtmosphereManager::AAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create components
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    RootComponent = SkyAtmosphere;
    
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    
    // Initialize default values
    TimeOfDayHours = 12.0f;
    DayDurationMinutes = 20.0f;
    bAutoAdvanceTime = true;
    CurrentWeather = EWeatherState::Clear;
    TargetWeather = EWeatherState::Clear;
    WeatherTransitionSpeed = 1.0f;
    CurrentEmotionalTone = EEmotionalTone::Peaceful;
    EmotionalIntensity = 1.0f;
    LastTimeOfDay = ETimeOfDay::Midday;
    WeatherBlendAlpha = 0.0f;
    EmotionalBlendAlpha = 0.0f;
    EmotionalTransitionDuration = 2.0f;
    EmotionalTransitionTimer = 0.0f;
}

void AAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    FindOrCreateLightingActors();
    SetupDefaultPresets();
    
    // Set initial atmosphere state
    BlendAtmosphereSettings();
    UpdateSunPosition();
    UpdateMoonPosition();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Initialized with time %.2f hours"), TimeOfDayHours);
}

void AAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }
    
    UpdateWeatherTransition(DeltaTime);
    UpdateEmotionalLighting(DeltaTime);
    BlendAtmosphereSettings();
}

void AAtmosphereManager::InitializeComponents()
{
    if (SkyAtmosphere)
    {
        // Set up sky atmosphere with Earth-like parameters
        SkyAtmosphere->SetAtmosphereHeight(60.0f);
        SkyAtmosphere->SetRayleighScatteringScale(1.0f);
        SkyAtmosphere->SetMieScatteringScale(1.0f);
        SkyAtmosphere->SetRayleighExponentialDistribution(8.0f);
        SkyAtmosphere->SetMieExponentialDistribution(1.2f);
        SkyAtmosphere->SetOtherAbsorptionScale(1.0f);
        SkyAtmosphere->SetSkyLuminanceFactor(1.0f);
        SkyAtmosphere->SetGroundAlbedo(FLinearColor(0.3f, 0.25f, 0.2f)); // Jurassic earth tones
    }
    
    if (VolumetricClouds)
    {
        // Set up volumetric clouds for prehistoric atmosphere
        VolumetricClouds->SetLayerBottomAltitude(1.5f);
        VolumetricClouds->SetLayerHeight(4.0f);
        VolumetricClouds->SetTracingStartMaxDistance(350.0f);
        VolumetricClouds->SetTracingMaxDistance(50.0f);
        VolumetricClouds->SetPlanetRadius(6360.0f);
        VolumetricClouds->SetGroundAlbedo(FLinearColor(0.3f, 0.25f, 0.2f));
    }
    
    if (HeightFog)
    {
        // Set up exponential height fog for atmospheric depth
        HeightFog->SetFogDensity(0.02f);
        HeightFog->SetFogHeightFalloff(0.2f);
        HeightFog->SetFogMaxOpacity(1.0f);
        HeightFog->SetStartDistance(0.0f);
        HeightFog->SetFogCutoffDistance(50000.0f);
        HeightFog->SetFogInscatteringColor(FLinearColor(0.7f, 0.8f, 0.9f));
        HeightFog->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.9f, 0.7f));
        HeightFog->SetDirectionalInscatteringExponent(4.0f);
        HeightFog->SetVolumetricFog(true);
        HeightFog->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFog->SetVolumetricFogExtinctionScale(1.0f);
    }
}

void AAtmosphereManager::FindOrCreateLightingActors()
{
    // Find existing sun light or create one
    SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass()));
    if (!SunLight)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SunLight");
        SunLight = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SpawnParams);
        if (SunLight && SunLight->GetLightComponent())
        {
            SunLight->GetLightComponent()->SetAtmosphereSunLight(true);
            SunLight->GetLightComponent()->SetAtmosphereSunLightIndex(0);
            SunLight->GetLightComponent()->SetCastShadows(true);
            SunLight->GetLightComponent()->SetCastCloudShadows(true);
            SunLight->GetLightComponent()->SetCloudScatteringLuminanceScale(FLinearColor::White);
            SunLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        }
    }
    
    // Find existing sky light or create one
    SkyLight = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ASkyLight::StaticClass()));
    if (!SkyLight)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SkyLight");
        SkyLight = GetWorld()->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), SpawnParams);
        if (SkyLight && SkyLight->GetLightComponent())
        {
            SkyLight->GetLightComponent()->SetRealTimeCapture(true);
            SkyLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
            SkyLight->GetLightComponent()->SetIntensity(1.0f);
            SkyLight->GetLightComponent()->RecaptureSky();
        }
    }
    
    // Create moon light (secondary directional light)
    if (!MoonLight)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("MoonLight");
        MoonLight = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SpawnParams);
        if (MoonLight && MoonLight->GetLightComponent())
        {
            MoonLight->GetLightComponent()->SetAtmosphereSunLight(true);
            MoonLight->GetLightComponent()->SetAtmosphereSunLightIndex(1);
            MoonLight->GetLightComponent()->SetIntensity(0.1f);
            MoonLight->GetLightComponent()->SetLightColor(FLinearColor(0.7f, 0.8f, 1.0f));
            MoonLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
            MoonLight->GetLightComponent()->SetVisibility(false); // Start hidden during day
        }
    }
}

void AAtmosphereManager::SetupDefaultPresets()
{
    // Time of day presets
    FAtmosphereSettings DawnSettings;
    DawnSettings.SunAngle = 10.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.5f);
    DawnSettings.CloudCoverage = 0.3f;
    TimeOfDayPresets.Add(ETimeOfDay::Dawn, DawnSettings);
    
    FAtmosphereSettings MiddaySettings;
    MiddaySettings.SunAngle = 80.0f;
    MiddaySettings.SunIntensity = 10.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f);
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    MiddaySettings.CloudCoverage = 0.5f;
    TimeOfDayPresets.Add(ETimeOfDay::Midday, MiddaySettings);
    
    FAtmosphereSettings DuskSettings;
    DuskSettings.SunAngle = 5.0f;
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f);
    DuskSettings.CloudCoverage = 0.4f;
    TimeOfDayPresets.Add(ETimeOfDay::Dusk, DuskSettings);
    
    FAtmosphereSettings NightSettings;
    NightSettings.SunAngle = -30.0f;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f);
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f);
    NightSettings.CloudCoverage = 0.6f;
    TimeOfDayPresets.Add(ETimeOfDay::Night, NightSettings);
    
    // Weather presets
    FAtmosphereSettings StormSettings;
    StormSettings.CloudCoverage = 0.9f;
    StormSettings.CloudOpacity = 1.0f;
    StormSettings.FogDensity = 0.08f;
    StormSettings.SunIntensity = 0.3f;
    WeatherPresets.Add(EWeatherState::Storm, StormSettings);
    
    // Emotional presets
    FAtmosphereSettings TenseSettings;
    TenseSettings.FogDensity = 0.06f;
    TenseSettings.FogColor = FLinearColor(0.6f, 0.6f, 0.7f);
    TenseSettings.CloudCoverage = 0.7f;
    EmotionalPresets.Add(EEmotionalTone::Tense, TenseSettings);
}

void AAtmosphereManager::UpdateTimeOfDay(float DeltaTime)
{
    if (DayDurationMinutes <= 0.0f) return;
    
    // Advance time
    float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
    TimeOfDayHours += HoursPerSecond * DeltaTime;
    
    // Wrap around 24 hours
    if (TimeOfDayHours >= 24.0f)
    {
        TimeOfDayHours -= 24.0f;
    }
    
    // Check for time of day transitions
    ETimeOfDay NewTimeOfDay = GetCurrentTimeOfDayEnum();
    if (NewTimeOfDay != LastTimeOfDay)
    {
        LastTimeOfDay = NewTimeOfDay;
        OnTimeOfDayChanged.Broadcast(NewTimeOfDay);
        UE_LOG(LogTemp, Log, TEXT("AtmosphereManager: Time of day changed to %s"), 
               *UEnum::GetValueAsString(NewTimeOfDay));
    }
    
    UpdateSunPosition();
    UpdateMoonPosition();
}

void AAtmosphereManager::UpdateWeatherTransition(float DeltaTime)
{
    if (CurrentWeather != TargetWeather)
    {
        WeatherBlendAlpha += WeatherTransitionSpeed * DeltaTime;
        if (WeatherBlendAlpha >= 1.0f)
        {
            WeatherBlendAlpha = 1.0f;
            CurrentWeather = TargetWeather;
            OnWeatherChanged.Broadcast(CurrentWeather);
            UE_LOG(LogTemp, Log, TEXT("AtmosphereManager: Weather transitioned to %s"), 
                   *UEnum::GetValueAsString(CurrentWeather));
        }
    }
}

void AAtmosphereManager::UpdateEmotionalLighting(float DeltaTime)
{
    if (EmotionalTransitionTimer > 0.0f)
    {
        EmotionalTransitionTimer -= DeltaTime;
        EmotionalBlendAlpha = FMath::Clamp(1.0f - (EmotionalTransitionTimer / EmotionalTransitionDuration), 0.0f, 1.0f);
        
        if (EmotionalTransitionTimer <= 0.0f)
        {
            EmotionalBlendAlpha = 1.0f;
            EmotionalTransitionTimer = 0.0f;
        }
    }
}

void AAtmosphereManager::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent()) return;
    
    // Calculate sun position based on time of day
    float SunAngle = (TimeOfDayHours - 6.0f) * 15.0f - 90.0f; // 6 AM = horizon, 12 PM = zenith
    FRotator SunRotation = FRotator(SunAngle, 180.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
    
    // Adjust sun intensity based on angle
    float SunIntensityMultiplier = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunAngle + 90.0f)), 0.0f, 1.0f);
    
    // Get base intensity from current time preset
    ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    if (TimeOfDayPresets.Contains(CurrentTimeEnum))
    {
        float BaseIntensity = TimeOfDayPresets[CurrentTimeEnum].SunIntensity;
        SunLight->GetLightComponent()->SetIntensity(BaseIntensity * SunIntensityMultiplier);
        SunLight->GetLightComponent()->SetLightColor(TimeOfDayPresets[CurrentTimeEnum].SunColor);
    }
    
    // Hide sun when below horizon
    SunLight->GetLightComponent()->SetVisibility(SunAngle > -10.0f);
}

void AAtmosphereManager::UpdateMoonPosition()
{
    if (!MoonLight || !MoonLight->GetLightComponent()) return;
    
    // Moon is opposite to sun
    float MoonAngle = (TimeOfDayHours - 18.0f) * 15.0f - 90.0f; // 6 PM = horizon, 12 AM = zenith
    if (MoonAngle < -180.0f) MoonAngle += 360.0f;
    
    FRotator MoonRotation = FRotator(MoonAngle, 0.0f, 0.0f);
    MoonLight->SetActorRotation(MoonRotation);
    
    // Moon is visible when sun is down
    bool bMoonVisible = (TimeOfDayHours >= 19.0f || TimeOfDayHours <= 5.0f);
    MoonLight->GetLightComponent()->SetVisibility(bMoonVisible && MoonAngle > -10.0f);
    
    if (bMoonVisible)
    {
        float MoonIntensityMultiplier = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(MoonAngle + 90.0f)), 0.0f, 1.0f);
        MoonLight->GetLightComponent()->SetIntensity(0.1f * MoonIntensityMultiplier);
    }
}

void AAtmosphereManager::BlendAtmosphereSettings()
{
    ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    if (!TimeOfDayPresets.Contains(CurrentTimeEnum)) return;
    
    FAtmosphereSettings FinalSettings = TimeOfDayPresets[CurrentTimeEnum];
    
    // Blend with weather settings
    if (WeatherPresets.Contains(CurrentWeather))
    {
        FAtmosphereSettings WeatherSettings = WeatherPresets[CurrentWeather];
        FinalSettings.CloudCoverage = FMath::Lerp(FinalSettings.CloudCoverage, WeatherSettings.CloudCoverage, WeatherBlendAlpha);
        FinalSettings.CloudOpacity = FMath::Lerp(FinalSettings.CloudOpacity, WeatherSettings.CloudOpacity, WeatherBlendAlpha);
        FinalSettings.FogDensity = FMath::Lerp(FinalSettings.FogDensity, WeatherSettings.FogDensity, WeatherBlendAlpha);
    }
    
    // Blend with emotional settings
    if (EmotionalPresets.Contains(CurrentEmotionalTone))
    {
        FAtmosphereSettings EmotionalSettings = EmotionalPresets[CurrentEmotionalTone];
        float EmotionalBlend = EmotionalBlendAlpha * EmotionalIntensity;
        FinalSettings.FogDensity = FMath::Lerp(FinalSettings.FogDensity, EmotionalSettings.FogDensity, EmotionalBlend);
        FinalSettings.FogColor = FLinearColor::LerpUsingHSV(FinalSettings.FogColor, EmotionalSettings.FogColor, EmotionalBlend);
        FinalSettings.CloudCoverage = FMath::Lerp(FinalSettings.CloudCoverage, EmotionalSettings.CloudCoverage, EmotionalBlend);
    }
    
    ApplyAtmosphereSettings(FinalSettings);
}

void AAtmosphereManager::SetTimeOfDay(float Hours)
{
    TimeOfDayHours = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateMoonPosition();
    BlendAtmosphereSettings();
}

ETimeOfDay AAtmosphereManager::GetCurrentTimeOfDayEnum() const
{
    if (TimeOfDayHours >= 5.0f && TimeOfDayHours < 7.0f)
        return ETimeOfDay::Dawn;
    else if (TimeOfDayHours >= 7.0f && TimeOfDayHours < 11.0f)
        return ETimeOfDay::Morning;
    else if (TimeOfDayHours >= 11.0f && TimeOfDayHours < 14.0f)
        return ETimeOfDay::Midday;
    else if (TimeOfDayHours >= 14.0f && TimeOfDayHours < 18.0f)
        return ETimeOfDay::Afternoon;
    else if (TimeOfDayHours >= 18.0f && TimeOfDayHours < 20.0f)
        return ETimeOfDay::Dusk;
    else
        return ETimeOfDay::Night;
}

void AAtmosphereManager::TransitionToWeather(EWeatherState NewWeather, float TransitionDuration)
{
    if (NewWeather != CurrentWeather)
    {
        TargetWeather = NewWeather;
        WeatherBlendAlpha = 0.0f;
        WeatherTransitionSpeed = (TransitionDuration > 0.0f) ? (1.0f / TransitionDuration) : 1.0f;
        
        UE_LOG(LogTemp, Log, TEXT("AtmosphereManager: Starting weather transition to %s over %.1f seconds"), 
               *UEnum::GetValueAsString(NewWeather), TransitionDuration);
    }
}

void AAtmosphereManager::SetEmotionalTone(EEmotionalTone NewTone, float Intensity, float TransitionDuration)
{
    CurrentEmotionalTone = NewTone;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    EmotionalTransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    EmotionalTransitionTimer = EmotionalTransitionDuration;
    EmotionalBlendAlpha = 0.0f;
    
    OnEmotionalToneChanged.Broadcast(NewTone, EmotionalIntensity);
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphereManager: Setting emotional tone to %s with intensity %.2f"), 
           *UEnum::GetValueAsString(NewTone), EmotionalIntensity);
}

void AAtmosphereManager::ApplyAtmosphereSettings(const FAtmosphereSettings& Settings, float BlendWeight)
{
    // Apply fog settings
    if (HeightFog)
    {
        HeightFog->SetFogDensity(Settings.FogDensity * BlendWeight);
        HeightFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(Settings.FogColor);
    }
    
    // Apply sky atmosphere settings
    if (SkyAtmosphere)
    {
        SkyAtmosphere->SetAtmosphereHeight(Settings.AtmosphereHeight);
        SkyAtmosphere->SetRayleighScatteringScale(Settings.RayleighScatteringScale);
        SkyAtmosphere->SetMieScatteringScale(Settings.MieScatteringScale);
    }
    
    // Apply cloud settings
    if (VolumetricClouds)
    {
        // Note: Volumetric cloud parameters would need to be set via material parameters
        // This is a simplified version - full implementation would require material parameter collection
    }
}