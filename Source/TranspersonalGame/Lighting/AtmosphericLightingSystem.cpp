// Copyright Transpersonal Game Studio. All Rights Reserved.
// Atmospheric Lighting System Implementation
// Agent #8 - Lighting & Atmosphere Agent

#include "AtmosphericLightingSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UAtmosphericLightingSystem::UAtmosphericLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize default parameters
    AtmosphereParameters = FAtmosphereParameters();
    SunLightParameters = FSunLightParameters();
    SkyLightParameters = FSkyLightParameters();
    
    // Component references
    SkyAtmosphereComponent = nullptr;
    SunDirectionalLight = nullptr;
    SkyLightActor = nullptr;
    
    bIsInitialized = false;
    LastUpdateTime = 0.0f;
}

void UAtmosphericLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the atmospheric system
    InitializeAtmosphericSystem();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingSystem: Initialized successfully"));
}

void UAtmosphericLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
        return;
        
    LastUpdateTime += DeltaTime;
    
    // Update atmospheric parameters periodically for performance
    if (LastUpdateTime >= 0.1f) // Update 10 times per second
    {
        UpdateAtmosphericParameters();
        LastUpdateTime = 0.0f;
    }
}

void UAtmosphericLightingSystem::InitializeAtmosphericSystem()
{
    if (bIsInitialized)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("AtmosphericLightingSystem: No valid world found"));
        return;
    }
    
    // Create or find atmospheric components
    CreateAtmosphericComponents();
    
    // Configure all components
    ConfigureAtmosphereComponent();
    ConfigureSunLight();
    ConfigureSkyLight();
    
    // Apply quality settings
    ApplyQualitySettings();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingSystem: Atmospheric system initialized"));
}

void UAtmosphericLightingSystem::CreateAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    // Find or create Sky Atmosphere Component
    if (!SkyAtmosphereComponent)
    {
        // Look for existing sky atmosphere in the world
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (USkyAtmosphereComponent* ExistingComp = Actor->FindComponentByClass<USkyAtmosphereComponent>())
            {
                SkyAtmosphereComponent = ExistingComp;
                break;
            }
        }
        
        // Create new sky atmosphere if none found
        if (!SkyAtmosphereComponent)
        {
            AActor* AtmosphereActor = World->SpawnActor<AActor>();
            AtmosphereActor->SetActorLabel(TEXT("SkyAtmosphere"));
            SkyAtmosphereComponent = NewObject<USkyAtmosphereComponent>(AtmosphereActor);
            AtmosphereActor->AddInstanceComponent(SkyAtmosphereComponent);
            SkyAtmosphereComponent->RegisterComponent();
        }
    }
    
    // Find or create Sun Directional Light
    if (!SunDirectionalLight)
    {
        // Look for existing directional light marked as sun
        for (TActorIterator<ADirectionalLight> LightItr(World); LightItr; ++LightItr)
        {
            ADirectionalLight* Light = *LightItr;
            if (Light->GetLightComponent()->bAtmosphereSunLight)
            {
                SunDirectionalLight = Light;
                break;
            }
        }
        
        // Create new sun light if none found
        if (!SunDirectionalLight)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("SunLight");
            SunDirectionalLight = World->SpawnActor<ADirectionalLight>(SpawnParams);
            SunDirectionalLight->SetActorLabel(TEXT("SunLight"));
        }
    }
    
    // Find or create Sky Light
    if (!SkyLightActor)
    {
        // Look for existing sky light
        SkyLightActor = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(World, ASkyLight::StaticClass()));
        
        // Create new sky light if none found
        if (!SkyLightActor)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("SkyLight");
            SkyLightActor = World->SpawnActor<ASkyLight>(SpawnParams);
            SkyLightActor->SetActorLabel(TEXT("SkyLight"));
        }
    }
}

void UAtmosphericLightingSystem::ConfigureAtmosphereComponent()
{
    if (!SkyAtmosphereComponent)
        return;
        
    // Configure planet properties
    SkyAtmosphereComponent->GroundRadius = AtmosphereParameters.GroundRadius;
    SkyAtmosphereComponent->AtmosphereHeight = AtmosphereParameters.AtmosphereHeight;
    SkyAtmosphereComponent->GroundAlbedo = AtmosphereParameters.GroundAlbedo;
    
    // Configure Rayleigh scattering
    SkyAtmosphereComponent->RayleighScatteringScale = AtmosphereParameters.RayleighScatteringScale;
    SkyAtmosphereComponent->RayleighScattering = AtmosphereParameters.RayleighScattering;
    SkyAtmosphereComponent->RayleighExponentialDistribution = AtmosphereParameters.RayleighExponentialDistribution;
    
    // Configure Mie scattering
    SkyAtmosphereComponent->MieScatteringScale = AtmosphereParameters.MieScatteringScale;
    SkyAtmosphereComponent->MieScattering = AtmosphereParameters.MieScattering;
    SkyAtmosphereComponent->MieAnisotropy = AtmosphereParameters.MieAnisotropy;
    SkyAtmosphereComponent->MieExponentialDistribution = AtmosphereParameters.MieExponentialDistribution;
    
    // Configure absorption
    SkyAtmosphereComponent->AbsorptionScale = AtmosphereParameters.AbsorptionScale;
    SkyAtmosphereComponent->OtherAbsorption = AtmosphereParameters.Absorption;
    
    // Configure multi-scattering and sky luminance
    SkyAtmosphereComponent->MultiScatteringFactor = AtmosphereParameters.MultiScattering;
    SkyAtmosphereComponent->SkyLuminanceFactor = AtmosphereParameters.SkyLuminanceFactor;
    SkyAtmosphereComponent->AerialPespectiveViewDistanceScale = AtmosphereParameters.AerialPerspectiveDistanceScale;
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingSystem: Sky atmosphere configured"));
}

void UAtmosphericLightingSystem::ConfigureSunLight()
{
    if (!SunDirectionalLight || !SunDirectionalLight->GetLightComponent())
        return;
        
    UDirectionalLightComponent* SunComponent = SunDirectionalLight->GetLightComponent();
    
    // Mark as atmospheric sun light
    SunComponent->bAtmosphereSunLight = true;
    SunComponent->AtmosphereSunLightIndex = 0; // Primary sun
    
    // Configure light properties
    SunComponent->SetLightColor(SunLightParameters.SunColor);
    SunComponent->SetIntensity(SunLightParameters.SunIntensity);
    SunComponent->SourceAngle = SunLightParameters.SunSourceAngle;
    SunComponent->Temperature = SunLightParameters.SunTemperature;
    SunComponent->bUseTemperature = true;
    
    // Configure shadows
    SunComponent->bCastShadows = true;
    SunComponent->bCastCloudShadows = SunLightParameters.bCastCloudShadows;
    SunComponent->CloudShadowStrength = SunLightParameters.CloudShadowStrength;
    SunComponent->CloudShadowExtent = SunLightParameters.CloudShadowExtent;
    
    // Set mobility to movable for dynamic lighting
    SunComponent->SetMobility(EComponentMobility::Movable);
    
    // Set initial sun position
    SetSunPosition(SunLightParameters.SunElevationAngle, SunLightParameters.SunAzimuthAngle);
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingSystem: Sun light configured"));
}

void UAtmosphericLightingSystem::ConfigureSkyLight()
{
    if (!SkyLightActor || !SkyLightActor->GetLightComponent())
        return;
        
    USkyLightComponent* SkyComponent = SkyLightActor->GetLightComponent();
    
    // Configure sky light properties
    SkyComponent->SetIntensity(SkyLightParameters.SkyLightIntensity);
    SkyComponent->SetLightColor(SkyLightParameters.SkyLightColor);
    
    // Enable real-time capture for dynamic lighting
    SkyComponent->bRealTimeCapture = SkyLightParameters.bRealTimeCapture;
    SkyComponent->SkyDistanceThreshold = SkyLightParameters.CaptureDistance;
    
    // Configure cloud ambient occlusion
    SkyComponent->CloudAmbientOcclusionStrength = SkyLightParameters.CloudAmbientOcclusionStrength;
    SkyComponent->CloudAmbientOcclusionExtent = SkyLightParameters.CloudAmbientOcclusionExtent;
    SkyComponent->CloudAmbientOcclusionMapResolutionScale = SkyLightParameters.CloudAmbientOcclusionMapResolutionScale;
    
    // Set mobility to stationary for performance
    SkyComponent->SetMobility(EComponentMobility::Stationary);
    
    // Force initial capture
    SkyComponent->RecaptureSky();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingSystem: Sky light configured"));
}

void UAtmosphericLightingSystem::UpdateAtmosphericParameters()
{
    if (!bIsInitialized)
        return;
        
    // Update atmosphere component if parameters changed
    ConfigureAtmosphereComponent();
    
    // Update performance settings
    UpdatePerformanceSettings();
}

void UAtmosphericLightingSystem::SetAtmosphericQuality(EAtmosphericQuality Quality)
{
    AtmosphericQuality = Quality;
    ApplyQualitySettings();
}

void UAtmosphericLightingSystem::ApplyQualitySettings()
{
    // Configure console variables based on quality level
    switch (AtmosphericQuality)
    {
        case EAtmosphericQuality::Mobile:
            // Mobile optimized settings
            TraceSampleCountScale = 0.5f;
            bEnableFastSkyLUT = true;
            bEnableAerialPerspective = false;
            bEnableTemporalUpsampling = false;
            break;
            
        case EAtmosphericQuality::Console:
            // Console balanced settings
            TraceSampleCountScale = 0.75f;
            bEnableFastSkyLUT = true;
            bEnableAerialPerspective = true;
            bEnableTemporalUpsampling = true;
            break;
            
        case EAtmosphericQuality::PC_High:
            // High-end PC settings
            TraceSampleCountScale = 1.0f;
            bEnableFastSkyLUT = false;
            bEnableAerialPerspective = true;
            bEnableTemporalUpsampling = true;
            break;
            
        case EAtmosphericQuality::Cinematic:
            // Maximum quality settings
            TraceSampleCountScale = 1.5f;
            bEnableFastSkyLUT = false;
            bEnableAerialPerspective = true;
            bEnableTemporalUpsampling = true;
            break;
    }
    
    UpdatePerformanceSettings();
}

void UAtmosphericLightingSystem::UpdatePerformanceSettings()
{
    if (!SkyAtmosphereComponent)
        return;
        
    // Apply trace sample count scale
    SkyAtmosphereComponent->TraceSampleCountScale = TraceSampleCountScale;
    
    // Note: Console variables would be set here in a real implementation
    // For now, we configure component properties directly
}

void UAtmosphericLightingSystem::SetSunPosition(float ElevationDegrees, float AzimuthDegrees)
{
    if (!SunDirectionalLight)
        return;
        
    // Update parameters
    SunLightParameters.SunElevationAngle = ElevationDegrees;
    SunLightParameters.SunAzimuthAngle = AzimuthDegrees;
    
    // Calculate and apply rotation
    FRotator SunRotation = CalculateSunRotation(ElevationDegrees, AzimuthDegrees);
    SunDirectionalLight->SetActorRotation(SunRotation);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("AtmosphericLightingSystem: Sun position updated - Elevation: %.1f, Azimuth: %.1f"), 
           ElevationDegrees, AzimuthDegrees);
}

void UAtmosphericLightingSystem::UpdateSunLight(const FSunLightParameters& SunParams)
{
    SunLightParameters = SunParams;
    ConfigureSunLight();
}

FVector UAtmosphericLightingSystem::GetSunDirection() const
{
    if (!SunDirectionalLight)
        return FVector::ForwardVector;
        
    return SunDirectionalLight->GetActorForwardVector();
}

void UAtmosphericLightingSystem::UpdateAtmosphereScattering(const FAtmosphereParameters& AtmosphereParams)
{
    AtmosphereParameters = AtmosphereParams;
    ConfigureAtmosphereComponent();
}

void UAtmosphericLightingSystem::SetAtmosphereHaziness(float Haziness)
{
    // Adjust Mie scattering for haziness effect
    AtmosphereParameters.MieScatteringScale = FMath::Lerp(0.5f, 2.0f, Haziness);
    ConfigureAtmosphereComponent();
}

void UAtmosphericLightingSystem::UpdateSkyLight(const FSkyLightParameters& SkyParams)
{
    SkyLightParameters = SkyParams;
    ConfigureSkyLight();
}

void UAtmosphericLightingSystem::RecaptureSkyLight()
{
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->RecaptureSky();
    }
}

USkyAtmosphereComponent* UAtmosphericLightingSystem::GetSkyAtmosphereComponent()
{
    if (!SkyAtmosphereComponent)
    {
        CreateAtmosphericComponents();
    }
    return SkyAtmosphereComponent;
}

ADirectionalLight* UAtmosphericLightingSystem::GetSunLight()
{
    if (!SunDirectionalLight)
    {
        CreateAtmosphericComponents();
    }
    return SunDirectionalLight;
}

ASkyLight* UAtmosphericLightingSystem::GetSkyLight()
{
    if (!SkyLightActor)
    {
        CreateAtmosphericComponents();
    }
    return SkyLightActor;
}

FRotator UAtmosphericLightingSystem::CalculateSunRotation(float ElevationDegrees, float AzimuthDegrees) const
{
    // Convert elevation and azimuth to UE4 rotation
    // Elevation: 0 = horizon, 90 = zenith
    // Azimuth: 0 = north, 90 = east, 180 = south, 270 = west
    
    float Pitch = -ElevationDegrees; // Negative because UE4 pitch is inverted
    float Yaw = AzimuthDegrees - 90.0f; // Adjust for UE4 coordinate system
    
    return FRotator(Pitch, Yaw, 0.0f);
}