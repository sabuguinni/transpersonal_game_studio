#include "Light_DynamicLightingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialParameterCollectionInstance.h"

ULight_DynamicLightingSystem::ULight_DynamicLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default time settings
    CurrentTime = FLight_TimeOfDay();
    CurrentTime.Hours = 12.0f; // Start at noon
    TimeScale = 1.0f;
    DayDurationMinutes = 24.0f; // 24 real minutes = 1 game day
    
    // Default weather
    CurrentWeather = FLight_WeatherState();
    WeatherTransitionSpeed = 1.0f;
    bIsTransitioningWeather = false;
    WeatherTransitionProgress = 0.0f;
    
    // Atmosphere colors
    DawnColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);    // Orange dawn
    DayColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);    // Warm daylight
    DuskColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);    // Red dusk
    NightColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);  // Blue night
    
    // Fire lighting defaults
    FireLightIntensity = 2000.0f;
    FireLightRadius = 800.0f;
    FireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f); // Orange fire
    
    // Cave lighting defaults
    CaveAmbientIntensity = 0.5f;
    
    // Initialize pointers
    SunLight = nullptr;
    SkyLightActor = nullptr;
    LightingParameters = nullptr;
}

void ULight_DynamicLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingSystem();
    
    // Set up weather update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(WeatherUpdateTimer, this, 
            &ULight_DynamicLightingSystem::UpdateWeatherEffects, 5.0f, true);
        
        World->GetTimerManager().SetTimer(FireFlickerTimer, this,
            &ULight_DynamicLightingSystem::UpdateFireFlicker, 0.1f, true);
    }
}

void ULight_DynamicLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTimeOfDay(DeltaTime);
    UpdateSunPosition();
    UpdateAtmosphereColors();
    UpdateFireLighting();
    UpdateCaveLighting();
    
    if (bIsTransitioningWeather)
    {
        WeatherTransitionProgress += DeltaTime * WeatherTransitionSpeed;
        if (WeatherTransitionProgress >= 1.0f)
        {
            WeatherTransitionProgress = 1.0f;
            bIsTransitioningWeather = false;
            CurrentWeather = TargetWeather;
        }
        else
        {
            // Interpolate weather values
            CurrentWeather.CloudCoverage = FMath::Lerp(StartWeather.CloudCoverage, TargetWeather.CloudCoverage, WeatherTransitionProgress);
            CurrentWeather.RainIntensity = FMath::Lerp(StartWeather.RainIntensity, TargetWeather.RainIntensity, WeatherTransitionProgress);
            CurrentWeather.FogDensity = FMath::Lerp(StartWeather.FogDensity, TargetWeather.FogDensity, WeatherTransitionProgress);
            CurrentWeather.WindStrength = FMath::Lerp(StartWeather.WindStrength, TargetWeather.WindStrength, WeatherTransitionProgress);
        }
    }
}

void ULight_DynamicLightingSystem::SetTimeOfDay(float Hours, float Minutes, float Seconds)
{
    CurrentTime.Hours = FMath::Fmod(Hours, 24.0f);
    CurrentTime.Minutes = FMath::Fmod(Minutes, 60.0f);
    CurrentTime.Seconds = FMath::Fmod(Seconds, 60.0f);
}

float ULight_DynamicLightingSystem::GetTimeOfDayNormalized() const
{
    return CurrentTime.GetTotalHours() / 24.0f;
}

void ULight_DynamicLightingSystem::SetWeatherState(const FLight_WeatherState& NewWeather)
{
    CurrentWeather = NewWeather;
    bIsTransitioningWeather = false;
}

void ULight_DynamicLightingSystem::TransitionToWeather(const FLight_WeatherState& TargetWeatherState, float TransitionTime)
{
    StartWeather = CurrentWeather;
    TargetWeather = TargetWeatherState;
    WeatherTransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
    WeatherTransitionProgress = 0.0f;
    bIsTransitioningWeather = true;
}

void ULight_DynamicLightingSystem::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    float SunAngle = CalculateSunAngle();
    
    // Calculate sun rotation (east to west arc)
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // Due south at noon
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
    
    // Adjust sun intensity based on angle
    float SunIntensity = FMath::Max(0.0f, FMath::Cos(FMath::DegreesToRadians(SunAngle + 90.0f)));
    SunIntensity = FMath::Pow(SunIntensity, 0.5f); // Soften the curve
    
    // Apply weather effects
    SunIntensity *= (1.0f - CurrentWeather.CloudCoverage * 0.7f);
    
    SunLight->GetLightComponent()->SetIntensity(SunIntensity * 5.0f);
}

void ULight_DynamicLightingSystem::UpdateAtmosphereColors()
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    FLinearColor CurrentColor = InterpolateAtmosphereColor();
    
    // Apply weather tinting
    if (CurrentWeather.RainIntensity > 0.0f)
    {
        CurrentColor = FLinearColor::LerpUsingHSV(CurrentColor, 
            FLinearColor(0.6f, 0.7f, 0.8f, 1.0f), CurrentWeather.RainIntensity * 0.5f);
    }
    
    SunLight->GetLightComponent()->SetLightColor(CurrentColor);
    
    // Update sky light
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        float SkyIntensity = FMath::Lerp(0.2f, 1.0f, FMath::Max(0.0f, FMath::Sin(FMath::DegreesToRadians(CalculateSunAngle() + 90.0f))));
        SkyIntensity *= (1.0f - CurrentWeather.CloudCoverage * 0.3f);
        SkyLightActor->GetLightComponent()->SetIntensity(SkyIntensity);
    }
}

void ULight_DynamicLightingSystem::UpdateWeatherEffects()
{
    // Random weather changes
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance every 5 seconds
    {
        FLight_WeatherState NewWeather;
        NewWeather.CloudCoverage = FMath::RandRange(0.0f, 0.8f);
        NewWeather.RainIntensity = (NewWeather.CloudCoverage > 0.5f) ? FMath::RandRange(0.0f, 0.6f) : 0.0f;
        NewWeather.FogDensity = FMath::RandRange(0.0f, 0.4f);
        NewWeather.WindStrength = FMath::RandRange(0.2f, 1.0f);
        
        TransitionToWeather(NewWeather, FMath::RandRange(30.0f, 120.0f));
    }
}

void ULight_DynamicLightingSystem::RegisterFirePit(APointLight* FireLight)
{
    if (FireLight && !FirePitLights.Contains(FireLight))
    {
        FirePitLights.Add(FireLight);
        
        // Configure fire light properties
        if (UPointLightComponent* LightComp = FireLight->GetLightComponent())
        {
            LightComp->SetIntensity(FireLightIntensity);
            LightComp->SetAttenuationRadius(FireLightRadius);
            LightComp->SetLightColor(FireLightColor);
            LightComp->SetCastShadows(true);
        }
    }
}

void ULight_DynamicLightingSystem::UnregisterFirePit(APointLight* FireLight)
{
    FirePitLights.Remove(FireLight);
}

void ULight_DynamicLightingSystem::UpdateFireLighting()
{
    // Remove null references
    FirePitLights.RemoveAll([](APointLight* Light) { return !IsValid(Light); });
    
    for (APointLight* FireLight : FirePitLights)
    {
        if (UPointLightComponent* LightComp = FireLight->GetLightComponent())
        {
            // Fire flicker effect
            float FlickerIntensity = FireLightIntensity * FMath::RandRange(0.8f, 1.2f);
            LightComp->SetIntensity(FlickerIntensity);
            
            // Slight color variation
            FLinearColor FlickerColor = FireLightColor;
            FlickerColor.R *= FMath::RandRange(0.9f, 1.1f);
            FlickerColor.G *= FMath::RandRange(0.8f, 1.0f);
            LightComp->SetLightColor(FlickerColor);
        }
    }
}

void ULight_DynamicLightingSystem::RegisterCaveLight(APointLight* CaveLight)
{
    if (CaveLight && !CaveLights.Contains(CaveLight))
    {
        CaveLights.Add(CaveLight);
        
        // Configure cave ambient light
        if (UPointLightComponent* LightComp = CaveLight->GetLightComponent())
        {
            LightComp->SetIntensity(CaveAmbientIntensity);
            LightComp->SetAttenuationRadius(1000.0f);
            LightComp->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f)); // Cool blue-white
            LightComp->SetCastShadows(false);
        }
    }
}

void ULight_DynamicLightingSystem::UpdateCaveLighting()
{
    // Remove null references
    CaveLights.RemoveAll([](APointLight* Light) { return !IsValid(Light); });
    
    // Cave lights remain constant but can be affected by time of day
    float TimeBasedIntensity = CaveAmbientIntensity;
    
    // Slightly brighter during day, dimmer at night
    float TimeOfDayFactor = GetTimeOfDayNormalized();
    if (TimeOfDayFactor > 0.25f && TimeOfDayFactor < 0.75f) // Day time
    {
        TimeBasedIntensity *= 1.2f;
    }
    else // Night time
    {
        TimeBasedIntensity *= 0.8f;
    }
    
    for (APointLight* CaveLight : CaveLights)
    {
        if (UPointLightComponent* LightComp = CaveLight->GetLightComponent())
        {
            LightComp->SetIntensity(TimeBasedIntensity);
        }
    }
}

void ULight_DynamicLightingSystem::InitializeLightingSystem()
{
    FindLightingActors();
    FixAtmosphereSettings();
}

void ULight_DynamicLightingSystem::FixAtmosphereSettings()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find and configure directional light (sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            SunLight = *ActorItr;
            break;
        }
    }
    
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(5.0f);
        SunLight->GetLightComponent()->SetLightColor(DayColor);
        SunLight->GetLightComponent()->SetCastShadows(true);
    }
    
    // Find and configure sky light
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
        {
            SkyLightActor = *ActorItr;
            break;
        }
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->SetIntensity(1.0f);
        SkyLightActor->GetLightComponent()->RecaptureSky();
    }
}

void ULight_DynamicLightingSystem::UpdateTimeOfDay(float DeltaTime)
{
    float GameSecondsPerRealSecond = (24.0f * 3600.0f) / (DayDurationMinutes * 60.0f);
    float TimeIncrement = DeltaTime * TimeScale * GameSecondsPerRealSecond;
    
    CurrentTime.Seconds += TimeIncrement;
    
    if (CurrentTime.Seconds >= 60.0f)
    {
        CurrentTime.Minutes += FMath::FloorToInt(CurrentTime.Seconds / 60.0f);
        CurrentTime.Seconds = FMath::Fmod(CurrentTime.Seconds, 60.0f);
    }
    
    if (CurrentTime.Minutes >= 60.0f)
    {
        CurrentTime.Hours += FMath::FloorToInt(CurrentTime.Minutes / 60.0f);
        CurrentTime.Minutes = FMath::Fmod(CurrentTime.Minutes, 60.0f);
    }
    
    if (CurrentTime.Hours >= 24.0f)
    {
        CurrentTime.Hours = FMath::Fmod(CurrentTime.Hours, 24.0f);
    }
}

void ULight_DynamicLightingSystem::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }
    
    // Find sky light
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyLightActor = *ActorItr;
        break;
    }
    
    // Find existing fire pit lights (tagged with "FirePit")
    for (TActorIterator<APointLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        APointLight* PointLight = *ActorItr;
        if (PointLight->Tags.Contains(FName("FirePit")))
        {
            RegisterFirePit(PointLight);
        }
        else if (PointLight->Tags.Contains(FName("Cave")))
        {
            RegisterCaveLight(PointLight);
        }
    }
}

FLinearColor ULight_DynamicLightingSystem::InterpolateAtmosphereColor() const
{
    float TimeNormalized = GetTimeOfDayNormalized();
    
    // Define time periods
    const float DawnStart = 0.2f;   // 4:48 AM
    const float DawnEnd = 0.3f;     // 7:12 AM
    const float DuskStart = 0.75f;  // 6:00 PM
    const float DuskEnd = 0.85f;    // 8:24 PM
    
    if (TimeNormalized < DawnStart || TimeNormalized > DuskEnd)
    {
        return NightColor; // Night
    }
    else if (TimeNormalized >= DawnStart && TimeNormalized <= DawnEnd)
    {
        // Dawn transition
        float Alpha = (TimeNormalized - DawnStart) / (DawnEnd - DawnStart);
        return FLinearColor::LerpUsingHSV(NightColor, DawnColor, Alpha);
    }
    else if (TimeNormalized > DawnEnd && TimeNormalized < DuskStart)
    {
        // Day transition from dawn to day
        if (TimeNormalized < 0.5f) // Morning
        {
            float Alpha = (TimeNormalized - DawnEnd) / (0.5f - DawnEnd);
            return FLinearColor::LerpUsingHSV(DawnColor, DayColor, Alpha);
        }
        else // Afternoon
        {
            return DayColor;
        }
    }
    else if (TimeNormalized >= DuskStart && TimeNormalized <= DuskEnd)
    {
        // Dusk transition
        float Alpha = (TimeNormalized - DuskStart) / (DuskEnd - DuskStart);
        return FLinearColor::LerpUsingHSV(DayColor, DuskColor, Alpha);
    }
    else
    {
        // Evening to night
        float Alpha = (TimeNormalized - DuskEnd) / (1.0f - DuskEnd);
        return FLinearColor::LerpUsingHSV(DuskColor, NightColor, Alpha);
    }
}

float ULight_DynamicLightingSystem::CalculateSunAngle() const
{
    float TimeNormalized = GetTimeOfDayNormalized();
    
    // Sun angle: -90 degrees at midnight, 0 degrees at noon, 90 degrees at midnight
    float SunAngle = (TimeNormalized - 0.5f) * 180.0f;
    
    return SunAngle;
}

void ULight_DynamicLightingSystem::ApplyWeatherToLighting()
{
    // Weather effects are applied in UpdateSunPosition and UpdateAtmosphereColors
    // This function can be extended for additional weather-based lighting effects
}

void ULight_DynamicLightingSystem::UpdateFireFlicker()
{
    // Fire flicker is handled in UpdateFireLighting
    // This timer-based function ensures consistent flicker timing
}