#include "Light_AtmosphericManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"

ULight_AtmosphericManager::ULight_AtmosphericManager()
{
    SunLight = nullptr;
    SkyAtmosphereActor = nullptr;
    HeightFogActor = nullptr;
    PostProcessVolume = nullptr;
    
    bEnableDayNightCycle = true;
    DayDurationMinutes = 20.0f;
    CurrentTimeOfDay = 0.5f;
    CurrentTimePhase = ELight_TimeOfDay::Midday;
}

void ULight_AtmosphericManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Initializing Cretaceous atmospheric lighting system"));
    
    // Initialize time of day settings
    InitializeTimeOfDaySettings();
    
    // Find existing lighting actors in the world
    FindLightingActors();
    
    // Setup the initial Cretaceous atmosphere
    SetupCretaceousAtmosphere();
}

void ULight_AtmosphericManager::Deinitialize()
{
    SunLight = nullptr;
    SkyAtmosphereActor = nullptr;
    HeightFogActor = nullptr;
    PostProcessVolume = nullptr;
    FillLights.Empty();
    
    Super::Deinitialize();
}

void ULight_AtmosphericManager::SetupCretaceousAtmosphere()
{
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Setting up Cretaceous period atmosphere"));
    
    ConfigureDirectionalLight();
    ConfigureSkyAtmosphere();
    ConfigureHeightFog();
    ConfigurePostProcessing();
    SpawnAtmosphericFillLights();
    
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Cretaceous atmosphere setup complete"));
}

void ULight_AtmosphericManager::SetTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    CurrentTimePhase = TimeOfDay;
    
    if (TimeOfDaySettings.Contains(TimeOfDay))
    {
        const FLight_TimeOfDaySettings& Settings = TimeOfDaySettings[TimeOfDay];
        InterpolateToTimeSettings(Settings, 1.0f);
    }
}

void ULight_AtmosphericManager::UpdateAtmosphericLighting(float DeltaTime)
{
    if (!bEnableDayNightCycle)
    {
        return;
    }
    
    // Update time of day
    float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
    CurrentTimeOfDay += TimeIncrement;
    
    if (CurrentTimeOfDay >= 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
    }
    
    // Calculate current time phase
    ELight_TimeOfDay NewPhase = CalculateTimePhase(CurrentTimeOfDay);
    if (NewPhase != CurrentTimePhase)
    {
        CurrentTimePhase = NewPhase;
        UE_LOG(LogTemp, Log, TEXT("Light_AtmosphericManager: Time phase changed to %d"), (int32)CurrentTimePhase);
    }
    
    // Get current time settings and apply them
    FLight_TimeOfDaySettings CurrentSettings = GetCurrentTimeSettings();
    InterpolateToTimeSettings(CurrentSettings, 0.1f); // Smooth transition
}

void ULight_AtmosphericManager::ConfigureDirectionalLight()
{
    if (!SunLight)
    {
        return;
    }
    
    UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
    if (SunComponent)
    {
        // Cretaceous period: warmer, more intense sunlight
        SunComponent->SetIntensity(8.5f);
        SunComponent->SetLightColor(FLinearColor(1.0f, 0.92f, 0.8f, 1.0f));
        SunComponent->SetTemperature(5800.0f);
        SunComponent->SetCastShadows(true);
        SunComponent->SetCastVolumetricShadow(true);
        
        SunLight->SetActorRotation(FRotator(-45.0f, 135.0f, 0.0f));
        
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Directional light configured for Cretaceous atmosphere"));
    }
}

void ULight_AtmosphericManager::ConfigureSkyAtmosphere()
{
    if (!SkyAtmosphereActor)
    {
        return;
    }
    
    USkyAtmosphereComponent* SkyComponent = SkyAtmosphereActor->GetComponent();
    if (SkyComponent)
    {
        // Cretaceous atmosphere: higher CO2, different composition
        SkyComponent->AerialPespectiveViewDistanceScale = 1.2f;
        SkyComponent->HeightFogContribution = 0.8f;
        SkyComponent->TransmittanceMinLightElevationAngle = -90.0f;
        SkyComponent->MultiScatteringFactor = 1.2f;
        
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Sky atmosphere configured for prehistoric period"));
    }
}

void ULight_AtmosphericManager::ConfigureHeightFog()
{
    if (!HeightFogActor)
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComponent = HeightFogActor->GetComponent();
    if (FogComponent)
    {
        // Prehistoric atmospheric conditions
        FogComponent->FogDensity = 0.02f;
        FogComponent->FogHeightFalloff = 0.2f;
        FogComponent->FogInscatteringColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
        FogComponent->DirectionalInscatteringExponent = 4.0f;
        FogComponent->DirectionalInscatteringStartDistance = 10000.0f;
        FogComponent->DirectionalInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        FogComponent->VolumetricFog = true;
        FogComponent->VolumetricFogScatteringDistribution = 0.2f;
        FogComponent->VolumetricFogAlbedo = FLinearColor(0.95f, 0.9f, 0.8f, 1.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Height fog configured for Cretaceous atmosphere"));
    }
}

void ULight_AtmosphericManager::ConfigurePostProcessing()
{
    if (!PostProcessVolume)
    {
        return;
    }
    
    UPostProcessComponent* PPComponent = PostProcessVolume->GetComponent();
    if (PPComponent)
    {
        // Cinematic prehistoric look
        PostProcessVolume->bUnbound = true;
        
        // Configure post process settings for Cretaceous period
        PPComponent->Settings.BloomIntensity = 0.8f;
        PPComponent->Settings.AutoExposureBias = 0.2f;
        PPComponent->Settings.ColorGradingIntensity = 1.1f;
        
        UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Post process volume configured for cinematic Cretaceous look"));
    }
}

void ULight_AtmosphericManager::SpawnAtmosphericFillLights()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Clear existing fill lights
    FillLights.Empty();
    
    // Spawn atmospheric fill lights
    TArray<FVector> FillLightLocations = {
        FVector(10000.0f, 10000.0f, 2000.0f),   // Northeast fill
        FVector(-10000.0f, 10000.0f, 2000.0f),  // Northwest fill
        FVector(10000.0f, -10000.0f, 2000.0f),  // Southeast fill
        FVector(-10000.0f, -10000.0f, 2000.0f)  // Southwest fill
    };
    
    for (int32 i = 0; i < FillLightLocations.Num(); i++)
    {
        APointLight* FillLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), FillLightLocations[i], FRotator::ZeroRotator);
        if (FillLight)
        {
            UPointLightComponent* LightComponent = FillLight->GetPointLightComponent();
            if (LightComponent)
            {
                LightComponent->SetIntensity(0.5f);
                LightComponent->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
                LightComponent->SetAttenuationRadius(50000.0f);
                LightComponent->SetCastShadows(false);
            }
            
            FillLight->SetActorLabel(FString::Printf(TEXT("AtmosphericFill_%d"), i + 1));
            FillLights.Add(FillLight);
            
            UE_LOG(LogTemp, Log, TEXT("Light_AtmosphericManager: Atmospheric fill light %d spawned"), i + 1);
        }
    }
}

void ULight_AtmosphericManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find sky atmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }
    
    // Find height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
    
    // Find post process volume
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    if (PostProcessVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphericManager: Found lighting actors - Sun: %s, Sky: %s, Fog: %s, PPV: %s"), 
           SunLight ? TEXT("Yes") : TEXT("No"),
           SkyAtmosphereActor ? TEXT("Yes") : TEXT("No"),
           HeightFogActor ? TEXT("Yes") : TEXT("No"),
           PostProcessVolume ? TEXT("Yes") : TEXT("No"));
}

void ULight_AtmosphericManager::InitializeTimeOfDaySettings()
{
    // Dawn settings
    FLight_TimeOfDaySettings DawnSettings;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SunTemperature = 4500.0f;
    DawnSettings.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.9f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);
    
    // Midday settings (Cretaceous default)
    FLight_TimeOfDaySettings MiddaySettings;
    MiddaySettings.SunIntensity = 8.5f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.92f, 0.8f, 1.0f);
    MiddaySettings.SunTemperature = 5800.0f;
    MiddaySettings.SunRotation = FRotator(-80.0f, 135.0f, 0.0f);
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);
    
    // Sunset settings
    FLight_TimeOfDaySettings SunsetSettings;
    SunsetSettings.SunIntensity = 4.0f;
    SunsetSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    SunsetSettings.SunTemperature = 3500.0f;
    SunsetSettings.SunRotation = FRotator(-5.0f, 270.0f, 0.0f);
    SunsetSettings.FogDensity = 0.04f;
    SunsetSettings.FogColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Sunset, SunsetSettings);
    
    // Night settings
    FLight_TimeOfDaySettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunTemperature = 8000.0f;
    NightSettings.SunRotation = FRotator(30.0f, 225.0f, 0.0f);
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Night, NightSettings);
}

FLight_TimeOfDaySettings ULight_AtmosphericManager::GetCurrentTimeSettings() const
{
    if (TimeOfDaySettings.Contains(CurrentTimePhase))
    {
        return TimeOfDaySettings[CurrentTimePhase];
    }
    
    // Return default midday settings
    FLight_TimeOfDaySettings DefaultSettings;
    return DefaultSettings;
}

void ULight_AtmosphericManager::InterpolateToTimeSettings(const FLight_TimeOfDaySettings& TargetSettings, float Alpha)
{
    if (!SunLight || !HeightFogActor)
    {
        return;
    }
    
    UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
    UExponentialHeightFogComponent* FogComponent = HeightFogActor->GetComponent();
    
    if (SunComponent)
    {
        // Interpolate sun properties
        float CurrentIntensity = SunComponent->Intensity;
        float NewIntensity = FMath::Lerp(CurrentIntensity, TargetSettings.SunIntensity, Alpha);
        SunComponent->SetIntensity(NewIntensity);
        
        FLinearColor CurrentColor = SunComponent->GetLightColor();
        FLinearColor NewColor = FMath::Lerp(CurrentColor, TargetSettings.SunColor, Alpha);
        SunComponent->SetLightColor(NewColor);
        
        SunComponent->SetTemperature(TargetSettings.SunTemperature);
        
        FRotator CurrentRotation = SunLight->GetActorRotation();
        FRotator NewRotation = FMath::Lerp(CurrentRotation, TargetSettings.SunRotation, Alpha);
        SunLight->SetActorRotation(NewRotation);
    }
    
    if (FogComponent)
    {
        // Interpolate fog properties
        float CurrentDensity = FogComponent->FogDensity;
        float NewDensity = FMath::Lerp(CurrentDensity, TargetSettings.FogDensity, Alpha);
        FogComponent->FogDensity = NewDensity;
        
        FLinearColor CurrentFogColor = FogComponent->FogInscatteringColor;
        FLinearColor NewFogColor = FMath::Lerp(CurrentFogColor, TargetSettings.FogColor, Alpha);
        FogComponent->FogInscatteringColor = NewFogColor;
    }
}

ELight_TimeOfDay ULight_AtmosphericManager::CalculateTimePhase(float TimeOfDay) const
{
    if (TimeOfDay < 0.125f) return ELight_TimeOfDay::Night;
    if (TimeOfDay < 0.25f) return ELight_TimeOfDay::Dawn;
    if (TimeOfDay < 0.375f) return ELight_TimeOfDay::Morning;
    if (TimeOfDay < 0.625f) return ELight_TimeOfDay::Midday;
    if (TimeOfDay < 0.75f) return ELight_TimeOfDay::Afternoon;
    if (TimeOfDay < 0.875f) return ELight_TimeOfDay::Sunset;
    if (TimeOfDay < 1.0f) return ELight_TimeOfDay::Twilight;
    
    return ELight_TimeOfDay::Night;
}