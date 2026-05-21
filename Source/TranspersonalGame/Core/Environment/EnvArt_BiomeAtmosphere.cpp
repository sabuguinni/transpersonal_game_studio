#include "EnvArt_BiomeAtmosphere.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UEnvArt_BiomeAtmosphere::UEnvArt_BiomeAtmosphere()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UEnvArt_BiomeAtmosphere::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeSettings();
    CreateAtmosphereActors();
    SetBiome(CurrentBiome);
}

void UEnvArt_BiomeAtmosphere::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsTransitioning && TargetTransitionTime > 0.0f)
    {
        CurrentTransitionTime += DeltaTime;
        float Alpha = FMath::Clamp(CurrentTransitionTime / TargetTransitionTime, 0.0f, 1.0f);
        
        // Interpolate atmosphere settings
        FEnvArt_AtmosphereSettings CurrentSettings;
        CurrentSettings.LightColor = FMath::Lerp(StartSettings.LightColor, TargetSettings.LightColor, Alpha);
        CurrentSettings.LightIntensity = FMath::Lerp(StartSettings.LightIntensity, TargetSettings.LightIntensity, Alpha);
        CurrentSettings.FogDensity = FMath::Lerp(StartSettings.FogDensity, TargetSettings.FogDensity, Alpha);
        CurrentSettings.FogColor = FMath::Lerp(StartSettings.FogColor, TargetSettings.FogColor, Alpha);
        CurrentSettings.FogHeightFalloff = FMath::Lerp(StartSettings.FogHeightFalloff, TargetSettings.FogHeightFalloff, Alpha);
        
        ApplyAtmosphereSettings(CurrentSettings);
        
        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            CurrentBiome = TransitionTargetBiome;
        }
    }
}

void UEnvArt_BiomeAtmosphere::SetBiome(EEnvArt_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    
    if (BiomeSettings.Contains(NewBiome))
    {
        ApplyAtmosphereSettings(BiomeSettings[NewBiome]);
    }
}

void UEnvArt_BiomeAtmosphere::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings)
{
    UpdateLighting();
    UpdateFog();
    
    // Apply lighting settings
    if (BiomeLight && BiomeLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = BiomeLight->GetLightComponent();
        LightComp->SetLightColor(Settings.LightColor);
        LightComp->SetIntensity(Settings.LightIntensity * WeatherIntensity);
        
        // Adjust for time of day
        float TimeMultiplier = 1.0f;
        if (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 20.0f)
        {
            TimeMultiplier = 0.1f; // Night time
        }
        else if (CurrentTimeOfDay < 8.0f || CurrentTimeOfDay > 18.0f)
        {
            TimeMultiplier = 0.5f; // Dawn/Dusk
        }
        
        LightComp->SetIntensity(Settings.LightIntensity * WeatherIntensity * TimeMultiplier);
        BiomeLight->SetActorRotation(Settings.LightRotation);
    }
    
    // Apply fog settings
    if (BiomeFog && BiomeFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = BiomeFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity * WeatherIntensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }
}

FEnvArt_AtmosphereSettings UEnvArt_BiomeAtmosphere::GetCurrentAtmosphereSettings() const
{
    if (BiomeSettings.Contains(CurrentBiome))
    {
        return BiomeSettings[CurrentBiome];
    }
    return FEnvArt_AtmosphereSettings();
}

void UEnvArt_BiomeAtmosphere::TransitionToBiome(EEnvArt_BiomeType TargetBiome, float TransitionDuration)
{
    if (TargetBiome == CurrentBiome || !BiomeSettings.Contains(TargetBiome))
    {
        return;
    }
    
    TransitionTargetBiome = TargetBiome;
    StartSettings = GetCurrentAtmosphereSettings();
    TargetSettings = BiomeSettings[TargetBiome];
    
    CurrentTransitionTime = 0.0f;
    TargetTransitionTime = TransitionDuration;
    bIsTransitioning = true;
}

void UEnvArt_BiomeAtmosphere::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 24.0f);
    
    // Reapply current settings to update lighting based on time
    if (BiomeSettings.Contains(CurrentBiome))
    {
        ApplyAtmosphereSettings(BiomeSettings[CurrentBiome]);
    }
}

void UEnvArt_BiomeAtmosphere::SetWeatherIntensity(float Intensity)
{
    WeatherIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
    
    // Reapply current settings to update with new weather intensity
    if (BiomeSettings.Contains(CurrentBiome))
    {
        ApplyAtmosphereSettings(BiomeSettings[CurrentBiome]);
    }
}

void UEnvArt_BiomeAtmosphere::InitializeBiomeSettings()
{
    // Savana - Warm golden grasslands
    FEnvArt_AtmosphereSettings SavanaSettings;
    SavanaSettings.LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    SavanaSettings.LightIntensity = 5.0f;
    SavanaSettings.LightRotation = FRotator(-45.0f, 45.0f, 0.0f);
    SavanaSettings.FogDensity = 0.003f;
    SavanaSettings.FogColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    SavanaSettings.FogHeightFalloff = 0.15f;
    SavanaSettings.TemperatureVariation = 1.2f;
    SavanaSettings.HumidityLevel = 0.3f;
    BiomeSettings.Add(EEnvArt_BiomeType::Savana, SavanaSettings);
    
    // Floresta - Green filtered forest light
    FEnvArt_AtmosphereSettings FlorestaSettings;
    FlorestaSettings.LightColor = FLinearColor(0.7f, 1.0f, 0.8f, 1.0f);
    FlorestaSettings.LightIntensity = 3.0f;
    FlorestaSettings.LightRotation = FRotator(-60.0f, 30.0f, 0.0f);
    FlorestaSettings.FogDensity = 0.008f;
    FlorestaSettings.FogColor = FLinearColor(0.8f, 1.0f, 0.9f, 1.0f);
    FlorestaSettings.FogHeightFalloff = 0.25f;
    FlorestaSettings.TemperatureVariation = 0.8f;
    FlorestaSettings.HumidityLevel = 0.8f;
    BiomeSettings.Add(EEnvArt_BiomeType::Floresta, FlorestaSettings);
    
    // Deserto - Hot orange desert sun
    FEnvArt_AtmosphereSettings DesertoSettings;
    DesertoSettings.LightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DesertoSettings.LightIntensity = 8.0f;
    DesertoSettings.LightRotation = FRotator(-30.0f, 0.0f, 0.0f);
    DesertoSettings.FogDensity = 0.002f;
    DesertoSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DesertoSettings.FogHeightFalloff = 0.1f;
    DesertoSettings.TemperatureVariation = 1.8f;
    DesertoSettings.HumidityLevel = 0.1f;
    BiomeSettings.Add(EEnvArt_BiomeType::Deserto, DesertoSettings);
    
    // Pantano - Misty blue-green swamp
    FEnvArt_AtmosphereSettings PantanoSettings;
    PantanoSettings.LightColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    PantanoSettings.LightIntensity = 2.5f;
    PantanoSettings.LightRotation = FRotator(-70.0f, 60.0f, 0.0f);
    PantanoSettings.FogDensity = 0.02f;
    PantanoSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.6f, 1.0f);
    PantanoSettings.FogHeightFalloff = 0.3f;
    PantanoSettings.TemperatureVariation = 0.9f;
    PantanoSettings.HumidityLevel = 0.95f;
    BiomeSettings.Add(EEnvArt_BiomeType::Pantano, PantanoSettings);
    
    // Montanha - Cool blue mountain light
    FEnvArt_AtmosphereSettings MontanhaSettings;
    MontanhaSettings.LightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MontanhaSettings.LightIntensity = 4.0f;
    MontanhaSettings.LightRotation = FRotator(-50.0f, -30.0f, 0.0f);
    MontanhaSettings.FogDensity = 0.012f;
    MontanhaSettings.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MontanhaSettings.FogHeightFalloff = 0.2f;
    MontanhaSettings.TemperatureVariation = 0.5f;
    MontanhaSettings.HumidityLevel = 0.6f;
    BiomeSettings.Add(EEnvArt_BiomeType::Montanha, MontanhaSettings);
}

void UEnvArt_BiomeAtmosphere::CreateAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find or create directional light
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        BiomeLight = Cast<ADirectionalLight>(FoundLights[0]);
    }
    else
    {
        FVector SpawnLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 1000);
        BiomeLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (BiomeLight)
        {
            BiomeLight->SetActorLabel(TEXT("BiomeDirectionalLight"));
        }
    }
    
    // Find or create exponential height fog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    
    if (FoundFogs.Num() > 0)
    {
        BiomeFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
    }
    else
    {
        FVector SpawnLocation = GetOwner()->GetActorLocation();
        BiomeFog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (BiomeFog)
        {
            BiomeFog->SetActorLabel(TEXT("BiomeFog"));
        }
    }
}

void UEnvArt_BiomeAtmosphere::UpdateLighting()
{
    // Lighting updates are handled in ApplyAtmosphereSettings
}

void UEnvArt_BiomeAtmosphere::UpdateFog()
{
    // Fog updates are handled in ApplyAtmosphereSettings
}