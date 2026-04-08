#include "AtmosphericLightingSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkyLight.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtmosphericLighting, Log, All);

UAtmosphericLightingSystem::UAtmosphericLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CurrentTimeOfDay = 12.0f; // Noon
    TimeSpeed = 1.0f;
    bAutoAdvanceTime = true;
    
    // Atmosphere defaults
    AtmosphereHeight = 60.0f;
    RayleighScatteringScale = 0.0331f;
    MieScatteringScale = 0.004f;
    
    // Cloud defaults
    CloudCoverage = 0.4f;
    CloudDensity = 0.5f;
    CloudSpeed = 0.1f;
    
    // Sun defaults
    SunIntensity = 10.0f;
    SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    SunTemperature = 5500.0f;
    
    // Sky defaults
    SkyLightIntensity = 1.0f;
    SkyTint = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
}

void UAtmosphericLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find or create atmospheric components
    InitializeAtmosphericComponents();
    
    // Apply initial lighting state
    UpdateAtmosphericLighting();
    
    UE_LOG(LogAtmosphericLighting, Log, TEXT("Atmospheric Lighting System initialized"));
}

void UAtmosphericLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoAdvanceTime)
    {
        // Advance time of day
        CurrentTimeOfDay += (DeltaTime * TimeSpeed) / 3600.0f; // Convert seconds to hours
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay = 0.0f;
        }
    }
    
    // Update atmospheric lighting
    UpdateAtmosphericLighting();
    
    // Update clouds
    UpdateVolumetricClouds(DeltaTime);
}

void UAtmosphericLightingSystem::InitializeAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find existing directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirectionalLight = *ActorItr;
        if (DirectionalLight && DirectionalLight->GetLightComponent())
        {
            SunLight = DirectionalLight;
            break;
        }
    }
    
    // Find existing sky light
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ASkyLight* SkyLight = *ActorItr;
        if (SkyLight && SkyLight->GetLightComponent())
        {
            SkyLightComponent = SkyLight;
            break;
        }
    }
    
    // Find sky atmosphere component
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkyAtmosphere = Owner->FindComponentByClass<USkyAtmosphereComponent>();
        VolumetricClouds = Owner->FindComponentByClass<UVolumetricCloudComponent>();
    }
    
    // Log what we found
    UE_LOG(LogAtmosphericLighting, Log, TEXT("Found components - Sun: %s, Sky: %s, Atmosphere: %s, Clouds: %s"),
        SunLight ? TEXT("Yes") : TEXT("No"),
        SkyLightComponent ? TEXT("Yes") : TEXT("No"),
        SkyAtmosphere ? TEXT("Yes") : TEXT("No"),
        VolumetricClouds ? TEXT("Yes") : TEXT("No"));
}

void UAtmosphericLightingSystem::UpdateAtmosphericLighting()
{
    // Calculate sun position based on time of day
    float SunAngle = CalculateSunAngle(CurrentTimeOfDay);
    FRotator SunRotation = FRotator(SunAngle, 180.0f, 0.0f);
    
    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        
        // Set sun rotation
        SunLight->SetActorRotation(SunRotation);
        
        // Calculate sun intensity based on angle
        float SunIntensityMultiplier = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(SunAngle + 90.0f)), 0.0f, 1.0f);
        SunComponent->SetIntensity(SunIntensity * SunIntensityMultiplier);
        
        // Calculate sun color based on angle
        FLinearColor CurrentSunColor = CalculateSunColor(SunAngle);
        SunComponent->SetLightColor(CurrentSunColor);
        
        // Set atmosphere sun light properties
        SunComponent->SetAtmosphereSunLight(true);
        SunComponent->SetAtmosphereSunLightIndex(0);
        
        // Enable cloud shadows
        SunComponent->SetCastCloudShadows(true);
        SunComponent->SetCloudShadowStrength(0.7f);
    }
    
    // Update sky light
    if (SkyLightComponent && SkyLightComponent->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLightComponent->GetLightComponent();
        
        // Calculate sky intensity based on sun angle
        float SkyIntensityMultiplier = FMath::Clamp(0.3f + (1.0f - FMath::Abs(SunAngle) / 90.0f) * 0.7f, 0.1f, 1.0f);
        SkyComponent->SetIntensity(SkyLightIntensity * SkyIntensityMultiplier);
        
        // Set sky color
        FLinearColor CurrentSkyColor = CalculateSkyColor(SunAngle);
        SkyComponent->SetLightColor(CurrentSkyColor);
        
        // Enable real-time capture for dynamic lighting
        SkyComponent->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
        SkyComponent->SetRealTimeCaptureEnabled(true);
    }
    
    // Update sky atmosphere
    if (SkyAtmosphere)
    {
        // Set atmosphere properties
        SkyAtmosphere->SetAtmosphereHeight(AtmosphereHeight);
        SkyAtmosphere->SetRayleighScatteringScale(RayleighScatteringScale);
        SkyAtmosphere->SetMieScatteringScale(MieScatteringScale);
        
        // Adjust sky luminance based on time of day
        float LuminanceFactor = FMath::Clamp(0.5f + (1.0f - FMath::Abs(SunAngle) / 90.0f) * 0.5f, 0.2f, 1.0f);
        SkyAtmosphere->SetSkyLuminanceFactor(LuminanceFactor);
    }
}

void UAtmosphericLightingSystem::UpdateVolumetricClouds(float DeltaTime)
{
    if (!VolumetricClouds)
    {
        return;
    }
    
    // Update cloud coverage and density
    VolumetricClouds->SetLayerBottomAltitude(1.5f); // 1.5km altitude
    VolumetricClouds->SetLayerHeight(4.0f); // 4km thick
    
    // Animate clouds based on wind
    CloudOffset += FVector2D(CloudSpeed * DeltaTime, CloudSpeed * 0.5f * DeltaTime);
    
    // Apply cloud parameters through material parameters if available
    // This would typically be done through a material parameter collection
}

float UAtmosphericLightingSystem::CalculateSunAngle(float TimeHours) const
{
    // Convert 24-hour time to angle
    // 6 AM = -90 degrees (sunrise)
    // 12 PM = 0 degrees (noon)
    // 6 PM = 90 degrees (sunset)
    // 12 AM = 180 degrees (midnight)
    
    float NormalizedTime = (TimeHours - 6.0f) / 12.0f; // Normalize around noon
    return NormalizedTime * 180.0f - 90.0f;
}

FLinearColor UAtmosphericLightingSystem::CalculateSunColor(float SunAngle) const
{
    // Calculate color based on sun angle
    float AbsAngle = FMath::Abs(SunAngle);
    
    if (AbsAngle > 90.0f)
    {
        // Night time - no sun
        return FLinearColor::Black;
    }
    
    // Interpolate between noon color and sunset color
    float SunsetFactor = FMath::Clamp(AbsAngle / 90.0f, 0.0f, 1.0f);
    
    FLinearColor NoonColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    FLinearColor SunsetColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    
    return FMath::Lerp(NoonColor, SunsetColor, SunsetFactor);
}

FLinearColor UAtmosphericLightingSystem::CalculateSkyColor(float SunAngle) const
{
    float AbsAngle = FMath::Abs(SunAngle);
    
    if (AbsAngle > 90.0f)
    {
        // Night time - dark blue
        return FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    }
    
    // Interpolate between day and sunset sky colors
    float SunsetFactor = FMath::Clamp(AbsAngle / 90.0f, 0.0f, 1.0f);
    
    FLinearColor DayColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    FLinearColor SunsetColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    
    return FMath::Lerp(DayColor, SunsetColor, SunsetFactor);
}

void UAtmosphericLightingSystem::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateAtmosphericLighting();
}

void UAtmosphericLightingSystem::SetCloudCoverage(float Coverage)
{
    CloudCoverage = FMath::Clamp(Coverage, 0.0f, 1.0f);
    
    if (VolumetricClouds)
    {
        // Update cloud material parameters
        // This would typically update a material parameter collection
    }
}

void UAtmosphericLightingSystem::SetAtmosphereParameters(float Height, float RayleighScale, float MieScale)
{
    AtmosphereHeight = Height;
    RayleighScatteringScale = RayleighScale;
    MieScatteringScale = MieScale;
    
    UpdateAtmosphericLighting();
}

void UAtmosphericLightingSystem::SetSunParameters(float Intensity, FLinearColor Color, float Temperature)
{
    SunIntensity = Intensity;
    SunColor = Color;
    SunTemperature = Temperature;
    
    UpdateAtmosphericLighting();
}

void UAtmosphericLightingSystem::SetSkyLightParameters(float Intensity, FLinearColor Tint)
{
    SkyLightIntensity = Intensity;
    SkyTint = Tint;
    
    UpdateAtmosphericLighting();
}

EJurassicTimeOfDay UAtmosphericLightingSystem::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 4.0f && CurrentTimeOfDay < 5.5f)
        return EJurassicTimeOfDay::PreDawn;
    else if (CurrentTimeOfDay >= 5.5f && CurrentTimeOfDay < 7.0f)
        return EJurassicTimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 10.0f)
        return EJurassicTimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 14.0f)
        return EJurassicTimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
        return EJurassicTimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f)
        return EJurassicTimeOfDay::Dusk;
    else if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 20.5f)
        return EJurassicTimeOfDay::Twilight;
    else
        return EJurassicTimeOfDay::Night;
}

float UAtmosphericLightingSystem::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

void UAtmosphericLightingSystem::SetTimeSpeed(float Speed)
{
    TimeSpeed = FMath::Max(0.0f, Speed);
}

void UAtmosphericLightingSystem::SetAutoAdvanceTime(bool bEnabled)
{
    bAutoAdvanceTime = bEnabled;
}