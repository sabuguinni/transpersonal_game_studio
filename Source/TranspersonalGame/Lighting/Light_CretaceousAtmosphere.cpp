#include "Light_CretaceousAtmosphere.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_CretaceousAtmosphere::ALight_CretaceousAtmosphere()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    CurrentWeather = ELight_WeatherType::Clear;
    PreviousTimeOfDay = CurrentTimeOfDay;
    PreviousWeather = CurrentWeather;

    // Initialize settings maps
    InitializeTimeOfDaySettings();
    InitializeWeatherSettings();
}

void ALight_CretaceousAtmosphere::BeginPlay()
{
    Super::BeginPlay();

    // Find atmospheric components in the world
    FindAtmosphericComponents();

    // Apply initial settings
    SetupCretaceousAtmosphere();
    UpdateFoundationLighting();
}

void ALight_CretaceousAtmosphere::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        TimeAccumulator += DeltaTime;
        
        // Update day time
        CurrentDayTime += DeltaTime / DayDuration;
        if (CurrentDayTime >= 1.0f)
        {
            CurrentDayTime -= 1.0f;
        }

        // Determine time of day based on CurrentDayTime
        ELight_TimeOfDay NewTimeOfDay = ELight_TimeOfDay::Night;
        if (CurrentDayTime >= 0.05f && CurrentDayTime < 0.15f)
            NewTimeOfDay = ELight_TimeOfDay::Dawn;
        else if (CurrentDayTime >= 0.15f && CurrentDayTime < 0.35f)
            NewTimeOfDay = ELight_TimeOfDay::Morning;
        else if (CurrentDayTime >= 0.35f && CurrentDayTime < 0.65f)
            NewTimeOfDay = ELight_TimeOfDay::Midday;
        else if (CurrentDayTime >= 0.65f && CurrentDayTime < 0.85f)
            NewTimeOfDay = ELight_TimeOfDay::Afternoon;
        else if (CurrentDayTime >= 0.85f && CurrentDayTime < 0.95f)
            NewTimeOfDay = ELight_TimeOfDay::Dusk;

        if (NewTimeOfDay != CurrentTimeOfDay)
        {
            SetTimeOfDay(NewTimeOfDay);
        }

        // Update atmospheric systems every frame for smooth transitions
        UpdateSunPosition();
        UpdateAtmosphericLighting();
    }
}

void ALight_CretaceousAtmosphere::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        PreviousTimeOfDay = CurrentTimeOfDay;
        CurrentTimeOfDay = NewTimeOfDay;
        
        // Apply new settings
        if (TimeOfDaySettings.Contains(CurrentTimeOfDay))
        {
            ApplyAtmosphericSettings(TimeOfDaySettings[CurrentTimeOfDay]);
        }
    }
}

void ALight_CretaceousAtmosphere::SetWeatherType(ELight_WeatherType NewWeatherType)
{
    if (CurrentWeather != NewWeatherType)
    {
        PreviousWeather = CurrentWeather;
        CurrentWeather = NewWeatherType;

        // Apply weather-specific modifications
        if (WeatherSettings.Contains(CurrentWeather))
        {
            FLight_AtmosphericSettings WeatherMod = WeatherSettings[CurrentWeather];
            FLight_AtmosphericSettings BaseSettings = TimeOfDaySettings.Contains(CurrentTimeOfDay) ? 
                TimeOfDaySettings[CurrentTimeOfDay] : FLight_AtmosphericSettings();

            // Blend weather modifications with time of day
            FLight_AtmosphericSettings FinalSettings = BaseSettings;
            FinalSettings.SunIntensity *= WeatherMod.SunIntensity;
            FinalSettings.FogDensity += WeatherMod.FogDensity;
            FinalSettings.FogHeightFalloff += WeatherMod.FogHeightFalloff;

            ApplyAtmosphericSettings(FinalSettings);
        }
    }
}

void ALight_CretaceousAtmosphere::SetDayTime(float NewDayTime)
{
    CurrentDayTime = FMath::Clamp(NewDayTime, 0.0f, 1.0f);
    UpdateSunPosition();
}

void ALight_CretaceousAtmosphere::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    // Apply to DirectionalLight
    if (MainDirectionalLight && MainDirectionalLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLightComp = MainDirectionalLight->GetLightComponent();
        DirLightComp->SetIntensity(Settings.SunIntensity);
        DirLightComp->SetLightColor(Settings.SunColor);
        DirLightComp->SetCastShadows(true);
        DirLightComp->SetCastVolumetricShadow(true);
    }

    // Apply to SkyLight
    if (MainSkyLight && MainSkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyLightComp = MainSkyLight->GetLightComponent();
        SkyLightComp->SetIntensity(Settings.SkyLightIntensity);
        SkyLightComp->SetLightColor(Settings.SkyLightColor);
        SkyLightComp->SetCastShadows(true);
    }

    // Apply to ExponentialHeightFog
    if (MainHeightFog && MainHeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = MainHeightFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
        FogComp->SetVolumetricFog(true);
    }
}

void ALight_CretaceousAtmosphere::UpdateFoundationLighting()
{
    // Find all foundation actors in the world
    TArray<AActor*> FoundationActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Foundation"), FoundationActors);

    // Clear existing foundation lights
    for (APointLight* Light : FoundationLights)
    {
        if (IsValid(Light))
        {
            Light->Destroy();
        }
    }
    FoundationLights.Empty();

    // Create new foundation lights
    int32 LightIndex = 0;
    for (AActor* Foundation : FoundationActors)
    {
        if (IsValid(Foundation) && LightIndex < 5) // Limit to 5 foundation lights
        {
            FVector FoundationLocation = Foundation->GetActorLocation();
            FVector LightLocation = FoundationLocation + FVector(200.0f, 200.0f, 300.0f);
            
            CreateFoundationLight(LightLocation, FString::Printf(TEXT("FoundationLight_%d"), LightIndex));
            LightIndex++;
        }
    }
}

void ALight_CretaceousAtmosphere::CreateFoundationLight(FVector Location, const FString& LightName)
{
    if (UWorld* World = GetWorld())
    {
        APointLight* NewLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator);
        if (NewLight && NewLight->GetLightComponent())
        {
            UPointLightComponent* LightComp = NewLight->GetLightComponent();
            LightComp->SetIntensity(FoundationLightIntensity);
            LightComp->SetLightColor(FoundationLightColor);
            LightComp->SetAttenuationRadius(FoundationLightRadius);
            LightComp->SetCastShadows(true);
            LightComp->SetCastVolumetricShadow(true);

            NewLight->SetActorLabel(LightName);
            FoundationLights.Add(NewLight);
        }
    }
}

void ALight_CretaceousAtmosphere::SetupCretaceousAtmosphere()
{
    FindAtmosphericComponents();

    // Apply current time of day settings
    if (TimeOfDaySettings.Contains(CurrentTimeOfDay))
    {
        ApplyAtmosphericSettings(TimeOfDaySettings[CurrentTimeOfDay]);
    }

    // Update sun position
    UpdateSunPosition();
}

void ALight_CretaceousAtmosphere::ResetToDefaultSettings()
{
    CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    CurrentWeather = ELight_WeatherType::Clear;
    CurrentDayTime = 0.5f;
    
    InitializeTimeOfDaySettings();
    InitializeWeatherSettings();
    SetupCretaceousAtmosphere();
}

void ALight_CretaceousAtmosphere::UpdateSunPosition()
{
    if (MainDirectionalLight)
    {
        // Calculate sun angle based on time of day
        float SunAngle = (CurrentDayTime - 0.5f) * 180.0f; // -90 to +90 degrees
        FRotator SunRotation = FRotator(SunAngle, 30.0f, 0.0f); // 30 degree azimuth offset
        
        MainDirectionalLight->SetActorRotation(SunRotation);
    }
}

void ALight_CretaceousAtmosphere::UpdateAtmosphericLighting()
{
    // This is called every frame for smooth transitions
    // Could implement interpolation between settings here
}

void ALight_CretaceousAtmosphere::UpdateFogSettings()
{
    // Additional fog updates if needed
}

void ALight_CretaceousAtmosphere::UpdateSkyAtmosphere()
{
    // Additional sky atmosphere updates if needed
}

void ALight_CretaceousAtmosphere::TransitionBetweenSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha)
{
    FLight_AtmosphericSettings BlendedSettings;
    
    BlendedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    BlendedSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    BlendedSettings.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    BlendedSettings.SkyLightColor = FMath::Lerp(From.SkyLightColor, To.SkyLightColor, Alpha);
    BlendedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    BlendedSettings.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    BlendedSettings.FogInscatteringColor = FMath::Lerp(From.FogInscatteringColor, To.FogInscatteringColor, Alpha);

    ApplyAtmosphericSettings(BlendedSettings);
}

void ALight_CretaceousAtmosphere::InitializeTimeOfDaySettings()
{
    // Dawn settings
    FLight_AtmosphericSettings DawnSettings;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.SkyLightColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    // Morning settings
    FLight_AtmosphericSettings MorningSettings;
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    MorningSettings.SkyLightIntensity = 1.2f;
    MorningSettings.SkyLightColor = FLinearColor(0.85f, 0.8f, 0.75f, 1.0f);
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Morning, MorningSettings);

    // Midday settings (default Cretaceous)
    FLight_AtmosphericSettings MiddaySettings;
    MiddaySettings.SunIntensity = 5.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.5f;
    MiddaySettings.SkyLightColor = FLinearColor(0.86f, 0.78f, 0.71f, 1.0f);
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogInscatteringColor = FLinearColor(0.71f, 0.63f, 0.47f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);

    // Afternoon settings
    FLight_AtmosphericSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 4.5f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.65f, 1.0f);
    AfternoonSettings.SkyLightIntensity = 1.3f;
    AfternoonSettings.SkyLightColor = FLinearColor(0.9f, 0.75f, 0.65f, 1.0f);
    AfternoonSettings.FogDensity = 0.025f;
    AfternoonSettings.FogInscatteringColor = FLinearColor(0.8f, 0.65f, 0.5f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk settings
    FLight_AtmosphericSettings DuskSettings;
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.9f;
    DuskSettings.SkyLightColor = FLinearColor(0.8f, 0.6f, 0.5f, 1.0f);
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogInscatteringColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    // Night settings
    FLight_AtmosphericSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.SkyLightColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogInscatteringColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Night, NightSettings);
}

void ALight_CretaceousAtmosphere::InitializeWeatherSettings()
{
    // Clear weather (no modifications)
    FLight_AtmosphericSettings ClearWeather;
    ClearWeather.SunIntensity = 1.0f; // Multiplier
    ClearWeather.FogDensity = 0.0f; // Additive
    WeatherSettings.Add(ELight_WeatherType::Clear, ClearWeather);

    // Cloudy weather
    FLight_AtmosphericSettings CloudyWeather;
    CloudyWeather.SunIntensity = 0.7f;
    CloudyWeather.FogDensity = 0.01f;
    WeatherSettings.Add(ELight_WeatherType::Cloudy, CloudyWeather);

    // Overcast weather
    FLight_AtmosphericSettings OvercastWeather;
    OvercastWeather.SunIntensity = 0.4f;
    OvercastWeather.FogDensity = 0.02f;
    WeatherSettings.Add(ELight_WeatherType::Overcast, OvercastWeather);

    // Foggy weather
    FLight_AtmosphericSettings FoggyWeather;
    FoggyWeather.SunIntensity = 0.3f;
    FoggyWeather.FogDensity = 0.08f;
    FoggyWeather.FogHeightFalloff = 0.1f;
    WeatherSettings.Add(ELight_WeatherType::Foggy, FoggyWeather);

    // Stormy weather
    FLight_AtmosphericSettings StormyWeather;
    StormyWeather.SunIntensity = 0.2f;
    StormyWeather.FogDensity = 0.04f;
    WeatherSettings.Add(ELight_WeatherType::Stormy, StormyWeather);
}

void ALight_CretaceousAtmosphere::FindAtmosphericComponents()
{
    if (UWorld* World = GetWorld())
    {
        // Find DirectionalLight
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            MainDirectionalLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }

        // Find SkyLight
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            MainSkyLight = Cast<ASkyLight>(SkyLights[0]);
        }

        // Find ExponentialHeightFog
        TArray<AActor*> FogActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
        if (FogActors.Num() > 0)
        {
            MainHeightFog = Cast<AExponentialHeightFog>(FogActors[0]);
        }

        // Find SkyAtmosphere
        TArray<AActor*> SkyAtmospheres;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
        if (SkyAtmospheres.Num() > 0)
        {
            MainSkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
        }
    }
}