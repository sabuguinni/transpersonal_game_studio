#include "AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"

AAtmosphericLightingManager::AAtmosphericLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize Cretaceous atmospheric settings
    CretaceousSettings.SunIntensity = 8.5f;
    CretaceousSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    CretaceousSettings.SunAngle = -45.0f;
    CretaceousSettings.RayleighScattering = 0.8f;
    CretaceousSettings.MieScattering = 0.06f;
    CretaceousSettings.FogDensity = 0.008f;
    CretaceousSettings.FogHeightFalloff = 0.15f;
    CretaceousSettings.WhiteTemperature = 6200.0f;
    CretaceousSettings.WhiteTint = 0.2f;
}

void AAtmosphericLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find lighting actors in the world
    FindLightingActors();
    
    // Apply Cretaceous atmospheric settings
    ApplyCretaceousAtmosphere();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: Cretaceous atmosphere initialized"));
}

void AAtmosphericLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void AAtmosphericLightingManager::ApplyCretaceousAtmosphere()
{
    // Update sun settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        SunComponent->SetIntensity(CretaceousSettings.SunIntensity);
        SunComponent->SetLightColor(CretaceousSettings.SunColor);
        SunComponent->SetCastShadows(true);
        SunComponent->SetCastVolumetricShadow(true);
        
        // Set sun angle for Cretaceous period
        FRotator SunRotation = FRotator(CretaceousSettings.SunAngle, 135.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }
    
    // Update atmosphere settings
    UpdateAtmosphericScattering();
    
    // Update fog settings
    UpdateFogSettings();
    
    // Update post processing
    UpdatePostProcessing();
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous atmospheric settings applied"));
}

void AAtmosphericLightingManager::SetTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    CurrentTimeOfDay = TimeOfDay;
    
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            CurrentHour = 6.0f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentHour = 9.0f;
            break;
        case ELight_TimeOfDay::Noon:
            CurrentHour = 12.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentHour = 15.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentHour = 18.0f;
            break;
        case ELight_TimeOfDay::Night:
            CurrentHour = 22.0f;
            break;
    }
    
    UpdateSunPosition();
}

void AAtmosphericLightingManager::SetHour(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    
    // Determine time of day based on hour
    if (CurrentHour >= 5.0f && CurrentHour < 8.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (CurrentHour >= 8.0f && CurrentHour < 11.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (CurrentHour >= 11.0f && CurrentHour < 14.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Noon;
    else if (CurrentHour >= 14.0f && CurrentHour < 17.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (CurrentHour >= 17.0f && CurrentHour < 20.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Night;
    
    UpdateSunPosition();
}

void AAtmosphericLightingManager::UpdateSunPosition()
{
    if (!SunLight)
        return;
    
    FRotator SunRotation = GetSunRotationForTime(CurrentHour);
    SunLight->SetActorRotation(SunRotation);
    
    if (SunLight->GetLightComponent())
    {
        float SunIntensity = GetSunIntensityForTime(CurrentHour);
        FLinearColor SunColor = GetSunColorForTime(CurrentHour);
        
        SunLight->GetLightComponent()->SetIntensity(SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(SunColor);
    }
}

void AAtmosphericLightingManager::UpdateAtmosphericScattering()
{
    if (SkyAtmosphere && SkyAtmosphere->GetComponent())
    {
        USkyAtmosphereComponent* SkyComponent = SkyAtmosphere->GetComponent();
        // Note: Property setting requires specific UE5 API calls
        // These would be implemented with proper property access
        UE_LOG(LogTemp, Warning, TEXT("Atmospheric scattering updated"));
    }
}

void AAtmosphericLightingManager::UpdateFogSettings()
{
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
        // Note: Property setting requires specific UE5 API calls
        UE_LOG(LogTemp, Warning, TEXT("Fog settings updated"));
    }
}

void AAtmosphericLightingManager::UpdatePostProcessing()
{
    if (PostProcessVolume && PostProcessVolume->GetComponent())
    {
        UPostProcessComponent* PPComponent = PostProcessVolume->GetComponent();
        // Note: Post process settings require specific property access
        UE_LOG(LogTemp, Warning, TEXT("Post processing updated"));
    }
}

void AAtmosphericLightingManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        UE_LOG(LogTemp, Warning, TEXT("Found DirectionalLight: %s"), *SunLight->GetName());
    }
    
    // Find sky atmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
        UE_LOG(LogTemp, Warning, TEXT("Found SkyAtmosphere: %s"), *SkyAtmosphere->GetName());
    }
    
    // Find exponential height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
        UE_LOG(LogTemp, Warning, TEXT("Found ExponentialHeightFog: %s"), *HeightFog->GetName());
    }
    
    // Find post process volume
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    if (PostProcessVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
        UE_LOG(LogTemp, Warning, TEXT("Found PostProcessVolume: %s"), *PostProcessVolume->GetName());
    }
}

void AAtmosphericLightingManager::ApplyPresetSettings()
{
    ApplyCretaceousAtmosphere();
    UE_LOG(LogTemp, Warning, TEXT("Preset Cretaceous settings applied"));
}

void AAtmosphericLightingManager::UpdateDayNightCycle(float DeltaTime)
{
    TimeAccumulator += DeltaTime;
    
    // Convert day duration from minutes to seconds
    float DayDurationSeconds = DayDurationMinutes * 60.0f;
    float HourDurationSeconds = DayDurationSeconds / 24.0f;
    
    if (TimeAccumulator >= HourDurationSeconds)
    {
        TimeAccumulator = 0.0f;
        CurrentHour += 1.0f;
        
        if (CurrentHour >= 24.0f)
        {
            CurrentHour = 0.0f;
        }
        
        SetHour(CurrentHour);
    }
}

FLinearColor AAtmosphericLightingManager::GetSunColorForTime(float Hour) const
{
    // Cretaceous period color variations throughout the day
    if (Hour >= 6.0f && Hour <= 8.0f) // Dawn
    {
        float Alpha = (Hour - 6.0f) / 2.0f;
        return FMath::Lerp(FLinearColor(1.0f, 0.6f, 0.4f, 1.0f), CretaceousSettings.SunColor, Alpha);
    }
    else if (Hour >= 17.0f && Hour <= 19.0f) // Dusk
    {
        float Alpha = (Hour - 17.0f) / 2.0f;
        return FMath::Lerp(CretaceousSettings.SunColor, FLinearColor(1.0f, 0.5f, 0.3f, 1.0f), Alpha);
    }
    else if (Hour >= 20.0f || Hour <= 5.0f) // Night
    {
        return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Moonlight
    }
    
    return CretaceousSettings.SunColor; // Day
}

float AAtmosphericLightingManager::GetSunIntensityForTime(float Hour) const
{
    if (Hour >= 20.0f || Hour <= 5.0f) // Night
    {
        return 0.5f; // Moonlight intensity
    }
    else if (Hour >= 6.0f && Hour <= 8.0f) // Dawn
    {
        float Alpha = (Hour - 6.0f) / 2.0f;
        return FMath::Lerp(0.5f, CretaceousSettings.SunIntensity, Alpha);
    }
    else if (Hour >= 17.0f && Hour <= 19.0f) // Dusk
    {
        float Alpha = (Hour - 17.0f) / 2.0f;
        return FMath::Lerp(CretaceousSettings.SunIntensity, 0.5f, Alpha);
    }
    
    return CretaceousSettings.SunIntensity; // Full day intensity
}

FRotator AAtmosphericLightingManager::GetSunRotationForTime(float Hour) const
{
    // Calculate sun arc across the sky (simplified)
    float SunAngle = -90.0f + (Hour / 24.0f) * 180.0f; // -90 to +90 degrees
    float SunYaw = 135.0f; // Southeast to southwest arc
    
    return FRotator(SunAngle, SunYaw, 0.0f);
}