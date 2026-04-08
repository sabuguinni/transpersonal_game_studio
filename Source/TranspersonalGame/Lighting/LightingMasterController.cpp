#include "LightingMasterController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ALightingMasterController::ALightingMasterController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize lighting components
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);
    SunLight->SetCastShadows(true);
    SunLight->SetCastVolumetricShadow(true);
    
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetRealTimeCapture(true);
    
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);
    
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);
    
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
}

void ALightingMasterController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    InitializePresets();
    
    // Find or create post process volume
    MainPostProcessVolume = Cast<APostProcessVolume>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass())
    );
    
    if (!MainPostProcessVolume)
    {
        // Create a global post process volume if none exists
        MainPostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
        MainPostProcessVolume->bUnbound = true;
    }
    
    UpdateLighting();
}

void ALightingMasterController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }
    
    if (bIsTransitioning)
    {
        InterpolateLighting(DeltaTime);
    }
}

void ALightingMasterController::InitializeComponents()
{
    // Configure Sky Atmosphere for Earth-like planet
    if (SkyAtmosphere)
    {
        SkyAtmosphere->SetGroundRadius(6360.0f); // Earth radius in km
        SkyAtmosphere->SetAtmosphereHeight(60.0f); // 60km atmosphere
        SkyAtmosphere->SetRayleighScatteringScale(1.0f);
        SkyAtmosphere->SetMieScatteringScale(1.0f);
        SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    }
    
    // Configure Height Fog for forest atmosphere
    if (HeightFog)
    {
        HeightFog->SetFogDensity(0.02f);
        HeightFog->SetFogHeightFalloff(0.2f);
        HeightFog->SetFogMaxOpacity(1.0f);
        HeightFog->SetStartDistance(0.0f);
        HeightFog->SetFogCutoffDistance(0.0f);
        HeightFog->SetVolumetricFog(true);
    }
    
    // Configure Volumetric Clouds
    if (VolumetricClouds)
    {
        VolumetricClouds->SetLayerBottomAltitude(1.5f);
        VolumetricClouds->SetLayerHeight(4.0f);
        VolumetricClouds->SetTracingStartMaxDistance(350.0f);
        VolumetricClouds->SetTracingMaxDistance(50.0f);
    }
}

void ALightingMasterController::InitializePresets()
{
    // Dawn Preset - Mysterious, low light
    FLightingPreset DawnPreset;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.7f, 0.4f); // Warm orange
    DawnPreset.SunIntensity = 1.5f;
    DawnPreset.SunAngle = 5.0f; // Low on horizon
    DawnPreset.SkyColor = FLinearColor(0.4f, 0.3f, 0.6f); // Purple-blue
    DawnPreset.SkyIntensity = 0.3f;
    DawnPreset.FogColor = FLinearColor(0.6f, 0.5f, 0.7f);
    DawnPreset.FogDensity = 0.05f; // Dense morning fog
    DawnPreset.Temperature = 4000.0f; // Warm
    DawnPreset.Saturation = 0.8f;
    DawnPreset.Contrast = 1.2f;
    TimePresets.Add(ETimeOfDay::Dawn, DawnPreset);
    
    // Morning Preset - Hopeful but cautious
    FLightingPreset MorningPreset;
    MorningPreset.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    MorningPreset.SunIntensity = 3.0f;
    MorningPreset.SunAngle = 25.0f;
    MorningPreset.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f);
    MorningPreset.SkyIntensity = 0.6f;
    MorningPreset.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    MorningPreset.FogDensity = 0.03f;
    MorningPreset.Temperature = 5500.0f;
    MorningPreset.Saturation = 1.0f;
    MorningPreset.Contrast = 1.1f;
    TimePresets.Add(ETimeOfDay::Morning, MorningPreset);
    
    // Midday Preset - Harsh, exposed feeling
    FLightingPreset MiddayPreset;
    MiddayPreset.SunColor = FLinearColor(1.0f, 1.0f, 0.95f);
    MiddayPreset.SunIntensity = 5.0f;
    MiddayPreset.SunAngle = 80.0f; // High overhead
    MiddayPreset.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);
    MiddayPreset.SkyIntensity = 1.0f;
    MiddayPreset.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MiddayPreset.FogDensity = 0.01f; // Clear
    MiddayPreset.Temperature = 6500.0f; // Neutral
    MiddayPreset.Saturation = 1.2f; // Vibrant
    MiddayPreset.Contrast = 1.3f; // High contrast
    TimePresets.Add(ETimeOfDay::Midday, MiddayPreset);
    
    // Afternoon Preset - Golden but ominous
    FLightingPreset AfternoonPreset;
    AfternoonPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.5f);
    AfternoonPreset.SunIntensity = 4.0f;
    AfternoonPreset.SunAngle = 45.0f;
    AfternoonPreset.SkyColor = FLinearColor(0.6f, 0.5f, 0.8f);
    AfternoonPreset.SkyIntensity = 0.8f;
    AfternoonPreset.FogColor = FLinearColor(0.8f, 0.7f, 0.6f);
    AfternoonPreset.FogDensity = 0.02f;
    AfternoonPreset.Temperature = 4500.0f; // Warm
    AfternoonPreset.Saturation = 1.1f;
    AfternoonPreset.Contrast = 1.2f;
    TimePresets.Add(ETimeOfDay::Afternoon, AfternoonPreset);
    
    // Dusk Preset - Beautiful but dangerous
    FLightingPreset DuskPreset;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f); // Deep orange
    DuskPreset.SunIntensity = 2.0f;
    DuskPreset.SunAngle = 10.0f; // Low
    DuskPreset.SkyColor = FLinearColor(0.8f, 0.3f, 0.5f); // Purple-pink
    DuskPreset.SkyIntensity = 0.4f;
    DuskPreset.FogColor = FLinearColor(0.7f, 0.4f, 0.5f);
    DuskPreset.FogDensity = 0.04f;
    DuskPreset.Temperature = 3500.0f; // Very warm
    DuskPreset.Saturation = 1.3f; // Very saturated
    DuskPreset.Contrast = 1.4f;
    TimePresets.Add(ETimeOfDay::Dusk, DuskPreset);
    
    // Night Preset - Terrifying darkness
    FLightingPreset NightPreset;
    NightPreset.SunColor = FLinearColor(0.1f, 0.1f, 0.3f); // Very dim blue
    NightPreset.SunIntensity = 0.1f; // Moonlight
    NightPreset.SunAngle = -30.0f; // Below horizon
    NightPreset.SkyColor = FLinearColor(0.05f, 0.05f, 0.15f); // Dark blue
    NightPreset.SkyIntensity = 0.1f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.1f, 0.2f);
    NightPreset.FogDensity = 0.06f; // Dense night fog
    NightPreset.Temperature = 8000.0f; // Cool
    NightPreset.Saturation = 0.6f; // Desaturated
    NightPreset.Contrast = 1.5f; // High contrast for drama
    TimePresets.Add(ETimeOfDay::Night, NightPreset);
    
    // Initialize Weather Presets
    InitializeWeatherPresets();
    InitializeMoodPresets();
}

void ALightingMasterController::InitializeWeatherPresets()
{
    // Clear Weather - Base values
    FLightingPreset ClearPreset;
    ClearPreset.SkyIntensity = 1.0f;
    ClearPreset.FogDensity = 0.02f;
    ClearPreset.Saturation = 1.0f;
    WeatherPresets.Add(EWeatherState::Clear, ClearPreset);
    
    // Overcast - Muted, oppressive
    FLightingPreset OvercastPreset;
    OvercastPreset.SunIntensity = 0.7f; // Dimmed sun
    OvercastPreset.SkyColor = FLinearColor(0.6f, 0.6f, 0.6f); // Gray
    OvercastPreset.SkyIntensity = 0.8f;
    OvercastPreset.FogDensity = 0.03f;
    OvercastPreset.Saturation = 0.8f; // Desaturated
    OvercastPreset.Temperature = 6000.0f; // Cooler
    WeatherPresets.Add(EWeatherState::Overcast, OvercastPreset);
    
    // Light Rain - Atmospheric
    FLightingPreset LightRainPreset;
    LightRainPreset.SunIntensity = 0.5f;
    LightRainPreset.SkyColor = FLinearColor(0.5f, 0.5f, 0.6f);
    LightRainPreset.SkyIntensity = 0.6f;
    LightRainPreset.FogDensity = 0.04f;
    LightRainPreset.Saturation = 0.9f;
    LightRainPreset.Temperature = 6500.0f;
    WeatherPresets.Add(EWeatherState::LightRain, LightRainPreset);
    
    // Heavy Rain - Dramatic
    FLightingPreset HeavyRainPreset;
    HeavyRainPreset.SunIntensity = 0.3f;
    HeavyRainPreset.SkyColor = FLinearColor(0.4f, 0.4f, 0.5f);
    HeavyRainPreset.SkyIntensity = 0.4f;
    HeavyRainPreset.FogDensity = 0.06f;
    HeavyRainPreset.Saturation = 0.7f;
    HeavyRainPreset.Contrast = 1.3f;
    HeavyRainPreset.Temperature = 7000.0f; // Cool
    WeatherPresets.Add(EWeatherState::HeavyRain, HeavyRainPreset);
    
    // Storm - Menacing
    FLightingPreset StormPreset;
    StormPreset.SunIntensity = 0.2f;
    StormPreset.SkyColor = FLinearColor(0.3f, 0.3f, 0.4f);
    StormPreset.SkyIntensity = 0.3f;
    StormPreset.FogDensity = 0.08f;
    StormPreset.Saturation = 0.6f;
    StormPreset.Contrast = 1.5f;
    StormPreset.Temperature = 7500.0f;
    WeatherPresets.Add(EWeatherState::Storm, StormPreset);
    
    // Fog - Mysterious and limiting
    FLightingPreset FogPreset;
    FogPreset.SunIntensity = 0.4f;
    FogPreset.SkyIntensity = 0.3f;
    FogPreset.FogDensity = 0.12f; // Very dense
    FogPreset.Saturation = 0.5f; // Very desaturated
    FogPreset.Contrast = 0.8f; // Low contrast
    FogPreset.Temperature = 6000.0f;
    WeatherPresets.Add(EWeatherState::Fog, FogPreset);
}

void ALightingMasterController::InitializeMoodPresets()
{
    // Peaceful - Warm, safe feeling
    FLightingPreset PeacefulPreset;
    PeacefulPreset.Temperature = 5000.0f; // Warm
    PeacefulPreset.Saturation = 1.1f;
    PeacefulPreset.Contrast = 0.9f; // Soft contrast
    PeacefulPreset.ColorGrading = FLinearColor(1.0f, 1.0f, 0.95f); // Slight warm tint
    MoodPresets.Add(EMoodState::Peaceful, PeacefulPreset);
    
    // Tense - Slightly desaturated, cooler
    FLightingPreset TensePreset;
    TensePreset.Temperature = 6000.0f;
    TensePreset.Saturation = 0.9f;
    TensePreset.Contrast = 1.1f;
    TensePreset.ColorGrading = FLinearColor(0.95f, 0.95f, 1.0f); // Cool tint
    MoodPresets.Add(EMoodState::Tense, TensePreset);
    
    // Dangerous - High contrast, dramatic
    FLightingPreset DangerousPreset;
    DangerousPreset.Temperature = 6500.0f;
    DangerousPreset.Saturation = 1.2f;
    DangerousPreset.Contrast = 1.3f;
    DangerousPreset.ColorGrading = FLinearColor(1.0f, 0.9f, 0.8f); // Orange tint
    MoodPresets.Add(EMoodState::Dangerous, DangerousPreset);
    
    // Terrifying - Desaturated, high contrast
    FLightingPreset TerrifyingPreset;
    TerrifyingPreset.Temperature = 7000.0f; // Cool
    TerrifyingPreset.Saturation = 0.7f; // Desaturated
    TerrifyingPreset.Contrast = 1.5f; // Very high contrast
    TerrifyingPreset.ColorGrading = FLinearColor(0.9f, 0.9f, 1.0f); // Cold blue tint
    MoodPresets.Add(EMoodState::Terrifying, TerrifyingPreset);
    
    // Discovery - Bright, saturated
    FLightingPreset DiscoveryPreset;
    DiscoveryPreset.Temperature = 5500.0f;
    DiscoveryPreset.Saturation = 1.3f; // Very saturated
    DiscoveryPreset.Contrast = 1.1f;
    DiscoveryPreset.ColorGrading = FLinearColor(1.0f, 1.0f, 0.9f); // Warm golden
    MoodPresets.Add(EMoodState::Discovery, DiscoveryPreset);
    
    // Safe - Warm, soft
    FLightingPreset SafePreset;
    SafePreset.Temperature = 4500.0f; // Very warm
    SafePreset.Saturation = 1.0f;
    SafePreset.Contrast = 0.8f; // Low contrast
    SafePreset.ColorGrading = FLinearColor(1.0f, 0.98f, 0.9f); // Warm cream
    MoodPresets.Add(EMoodState::Safe, SafePreset);
}

void ALightingMasterController::UpdateTimeOfDay(float DeltaTime)
{
    if (DayDurationMinutes > 0.0f)
    {
        float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
        TimeOfDayHours += HoursPerSecond * DeltaTime;
        
        if (TimeOfDayHours >= 24.0f)
        {
            TimeOfDayHours -= 24.0f;
        }
        
        UpdateLighting();
    }
}

void ALightingMasterController::CalculateTargetPreset()
{
    // Start with time-based preset
    FLightingPreset BasePreset = GetTimeBasedPreset();
    
    // Apply weather modifications
    FLightingPreset WeatherModified = GetWeatherModifiedPreset(BasePreset);
    
    // Apply mood modifications
    TargetPreset = GetMoodModifiedPreset(WeatherModified);
    
    // Start transition
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
}

FLightingPreset ALightingMasterController::GetTimeBasedPreset() const
{
    ETimeOfDay CurrentTime = GetCurrentTimeOfDay();
    
    if (TimePresets.Contains(CurrentTime))
    {
        return TimePresets[CurrentTime];
    }
    
    // Fallback to midday
    return TimePresets[ETimeOfDay::Midday];
}

ETimeOfDay ALightingMasterController::GetCurrentTimeOfDay() const
{
    if (TimeOfDayHours >= 5.0f && TimeOfDayHours < 8.0f)
        return ETimeOfDay::Dawn;
    else if (TimeOfDayHours >= 8.0f && TimeOfDayHours < 12.0f)
        return ETimeOfDay::Morning;
    else if (TimeOfDayHours >= 12.0f && TimeOfDayHours < 16.0f)
        return ETimeOfDay::Midday;
    else if (TimeOfDayHours >= 16.0f && TimeOfDayHours < 19.0f)
        return ETimeOfDay::Afternoon;
    else if (TimeOfDayHours >= 19.0f && TimeOfDayHours < 22.0f)
        return ETimeOfDay::Dusk;
    else
        return ETimeOfDay::Night;
}

FLightingPreset ALightingMasterController::GetWeatherModifiedPreset(const FLightingPreset& BasePreset) const
{
    if (!WeatherPresets.Contains(CurrentWeather))
    {
        return BasePreset;
    }
    
    const FLightingPreset& WeatherMod = WeatherPresets[CurrentWeather];
    FLightingPreset Result = BasePreset;
    
    // Apply weather modifications
    Result.SunIntensity *= WeatherMod.SunIntensity;
    Result.SkyIntensity *= WeatherMod.SkyIntensity;
    Result.FogDensity = FMath::Max(Result.FogDensity, WeatherMod.FogDensity);
    Result.Saturation *= WeatherMod.Saturation;
    
    if (WeatherMod.Temperature > 0.0f)
    {
        Result.Temperature = WeatherMod.Temperature;
    }
    
    return Result;
}

FLightingPreset ALightingMasterController::GetMoodModifiedPreset(const FLightingPreset& BasePreset) const
{
    if (!MoodPresets.Contains(CurrentMood))
    {
        return BasePreset;
    }
    
    const FLightingPreset& MoodMod = MoodPresets[CurrentMood];
    FLightingPreset Result = BasePreset;
    
    // Blend mood modifications based on influence strength
    float Alpha = MoodInfluenceStrength;
    
    Result.Temperature = FMath::Lerp(Result.Temperature, MoodMod.Temperature, Alpha);
    Result.Saturation = FMath::Lerp(Result.Saturation, MoodMod.Saturation, Alpha);
    Result.Contrast = FMath::Lerp(Result.Contrast, MoodMod.Contrast, Alpha);
    Result.ColorGrading = FLinearColor::LerpUsingHSV(Result.ColorGrading, MoodMod.ColorGrading, Alpha);
    
    return Result;
}

void ALightingMasterController::InterpolateLighting(float DeltaTime)
{
    TransitionProgress += DeltaTime * WeatherTransitionSpeed;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentPreset = TargetPreset;
    }
    else
    {
        CurrentPreset = BlendPresets(CurrentPreset, TargetPreset, TransitionProgress);
    }
    
    ApplyLightingPreset(CurrentPreset);
}

FLightingPreset ALightingMasterController::BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha)
{
    FLightingPreset Result;
    
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunAngle = FMath::Lerp(A.SunAngle, B.SunAngle, Alpha);
    Result.SkyColor = FLinearColor::LerpUsingHSV(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.Temperature = FMath::Lerp(A.Temperature, B.Temperature, Alpha);
    Result.Saturation = FMath::Lerp(A.Saturation, B.Saturation, Alpha);
    Result.Contrast = FMath::Lerp(A.Contrast, B.Contrast, Alpha);
    Result.ColorGrading = FLinearColor::LerpUsingHSV(A.ColorGrading, B.ColorGrading, Alpha);
    
    return Result;
}

void ALightingMasterController::ApplyLightingPreset(const FLightingPreset& Preset)
{
    // Apply to Sun Light
    if (SunLight)
    {
        SunLight->SetLightColor(Preset.SunColor);
        SunLight->SetIntensity(Preset.SunIntensity);
        
        // Set sun angle
        FRotator SunRotation = FRotator(-Preset.SunAngle, 0.0f, 0.0f);
        SunLight->SetWorldRotation(SunRotation);
    }
    
    // Apply to Sky Light
    if (SkyLight)
    {
        SkyLight->SetIntensity(Preset.SkyIntensity);
    }
    
    // Apply to Height Fog
    if (HeightFog)
    {
        HeightFog->SetFogInscatteringColor(Preset.FogColor);
        HeightFog->SetFogDensity(Preset.FogDensity);
        HeightFog->SetFogHeightFalloff(Preset.FogHeightFalloff);
    }
    
    // Apply to Post Process Volume
    if (MainPostProcessVolume)
    {
        FPostProcessSettings& Settings = MainPostProcessVolume->Settings;
        
        Settings.WhiteTemp = Preset.Temperature;
        Settings.ColorSaturation = FVector4(Preset.Saturation, Preset.Saturation, Preset.Saturation, 1.0f);
        Settings.ColorContrast = FVector4(Preset.Contrast, Preset.Contrast, Preset.Contrast, 1.0f);
        Settings.ColorGamma = FVector4(Preset.ColorGrading.R, Preset.ColorGrading.G, Preset.ColorGrading.B, 1.0f);
        
        // Enable overrides
        Settings.bOverride_WhiteTemp = true;
        Settings.bOverride_ColorSaturation = true;
        Settings.bOverride_ColorContrast = true;
        Settings.bOverride_ColorGamma = true;
    }
}

void ALightingMasterController::SetTimeOfDay(float Hours)
{
    TimeOfDayHours = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateLighting();
}

void ALightingMasterController::SetWeather(EWeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        UpdateLighting();
    }
}

void ALightingMasterController::SetMood(EMoodState NewMood)
{
    if (CurrentMood != NewMood)
    {
        CurrentMood = NewMood;
        UpdateLighting();
    }
}

void ALightingMasterController::UpdateLighting()
{
    CalculateTargetPreset();
}