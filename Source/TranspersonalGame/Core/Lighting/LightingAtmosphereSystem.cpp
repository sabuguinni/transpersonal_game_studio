#include "LightingAtmosphereSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"

ULightingAtmosphereSystem::ULightingAtmosphereSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    // Initialize default settings
    SetupDefaultPresets();
}

void ULightingAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find or create atmosphere components
    InitializeAtmosphereComponents();
    
    // Apply initial settings
    ApplyAtmosphereSettings(AtmosphereSettings, true);
    ApplyCloudSettings(CloudSettings, true);
    ApplyFogSettings(FogSettings, true);
    
    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereSystem: Initialized with preset %s"), 
           *UEnum::GetValueAsString(CurrentPreset));
}

void ULightingAtmosphereSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
}

void ULightingAtmosphereSystem::SetAtmospherePreset(EAtmospherePreset NewPreset, bool bImmediate)
{
    if (NewPreset == CurrentPreset && !bIsTransitioning)
    {
        return;
    }
    
    EAtmospherePreset OldPreset = CurrentPreset;
    CurrentPreset = NewPreset;
    
    // Get preset settings
    FAtmosphereSettings* NewAtmosphereSettings = AtmospherePresets.Find(NewPreset);
    FCloudSettings* NewCloudSettings = CloudPresets.Find(NewPreset);
    FFogSettings* NewFogSettings = FogPresets.Find(NewPreset);
    
    if (NewAtmosphereSettings && NewCloudSettings && NewFogSettings)
    {
        if (bImmediate || !bSmoothTransitions)
        {
            // Apply immediately
            AtmosphereSettings = *NewAtmosphereSettings;
            CloudSettings = *NewCloudSettings;
            FogSettings = *NewFogSettings;
            
            ApplyAtmosphereSettings(AtmosphereSettings, true);
            ApplyCloudSettings(CloudSettings, true);
            ApplyFogSettings(FogSettings, true);
        }
        else
        {
            // Start smooth transition
            StartTransition(*NewAtmosphereSettings, *NewCloudSettings, *NewFogSettings);
        }
        
        OnAtmosphereChanged.Broadcast(NewPreset);
        
        UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereSystem: Changed preset from %s to %s"), 
               *UEnum::GetValueAsString(OldPreset), *UEnum::GetValueAsString(NewPreset));
    }
}

void ULightingAtmosphereSystem::SetCustomAtmosphereSettings(const FAtmosphereSettings& NewSettings, bool bImmediate)
{
    if (bImmediate || !bSmoothTransitions)
    {
        AtmosphereSettings = NewSettings;
        ApplyAtmosphereSettings(AtmosphereSettings, true);
    }
    else
    {
        StartAtmosphereTransition(NewSettings);
    }
}

void ULightingAtmosphereSystem::SetCloudSettings(const FCloudSettings& NewSettings, bool bImmediate)
{
    if (bImmediate || !bSmoothTransitions)
    {
        CloudSettings = NewSettings;
        ApplyCloudSettings(CloudSettings, true);
    }
    else
    {
        StartCloudTransition(NewSettings);
    }
}

void ULightingAtmosphereSystem::SetFogSettings(const FFogSettings& NewSettings, bool bImmediate)
{
    if (bImmediate || !bSmoothTransitions)
    {
        FogSettings = NewSettings;
        ApplyFogSettings(FogSettings, true);
    }
    else
    {
        StartFogTransition(NewSettings);
    }
}

void ULightingAtmosphereSystem::InitializeAtmosphereComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find existing Sky Atmosphere
    TArray<AActor*> SkyAtmosphereActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmosphereActors);
    
    if (SkyAtmosphereActors.Num() > 0)
    {
        ASkyAtmosphere* SkyAtmosphereActor = Cast<ASkyAtmosphere>(SkyAtmosphereActors[0]);
        if (SkyAtmosphereActor)
        {
            SkyAtmosphereComponent = SkyAtmosphereActor->GetSkyAtmosphereComponent();
        }
    }
    
    // Find existing Volumetric Clouds
    TArray<AActor*> VolumetricCloudActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), VolumetricCloudActors);
    
    if (VolumetricCloudActors.Num() > 0)
    {
        AVolumetricCloud* VolumetricCloudActor = Cast<AVolumetricCloud>(VolumetricCloudActors[0]);
        if (VolumetricCloudActor)
        {
            VolumetricCloudComponent = VolumetricCloudActor->GetVolumetricCloudComponent();
        }
    }
    
    // Find existing Height Fog
    TArray<AActor*> HeightFogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogActors);
    
    if (HeightFogActors.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(HeightFogActors[0]);
    }
}

void ULightingAtmosphereSystem::ApplyAtmosphereSettings(const FAtmosphereSettings& Settings, bool bImmediate)
{
    if (!SkyAtmosphereComponent)
    {
        return;
    }
    
    // Apply Rayleigh scattering
    SkyAtmosphereComponent->SetRayleighScattering(Settings.RayleighScattering);
    SkyAtmosphereComponent->SetRayleighScatteringScale(Settings.RayleighScatteringScale);
    SkyAtmosphereComponent->SetRayleighExponentialDistribution(Settings.RayleighExponentialDistribution);
    
    // Apply Mie scattering
    SkyAtmosphereComponent->SetMieScattering(Settings.MieScattering);
    SkyAtmosphereComponent->SetMieScatteringScale(Settings.MieScatteringScale);
    SkyAtmosphereComponent->SetMieAbsorption(Settings.MieAbsorption);
    SkyAtmosphereComponent->SetMieAbsorptionScale(Settings.MieAbsorptionScale);
    SkyAtmosphereComponent->SetMieAnisotropy(Settings.MieAnisotropy);
    SkyAtmosphereComponent->SetMieExponentialDistribution(Settings.MieExponentialDistribution);
    
    // Apply absorption
    SkyAtmosphereComponent->SetAbsorption(Settings.Absorption);
    SkyAtmosphereComponent->SetAbsorptionScale(Settings.AbsorptionScale);
    
    // Apply planet properties
    SkyAtmosphereComponent->SetGroundRadius(Settings.GroundRadius);
    SkyAtmosphereComponent->SetAtmosphereHeight(Settings.AtmosphereHeight);
    SkyAtmosphereComponent->SetGroundAlbedo(Settings.GroundAlbedo);
    
    // Apply art direction
    SkyAtmosphereComponent->SetSkyLuminanceFactor(Settings.SkyLuminanceFactor);
    SkyAtmosphereComponent->SetAerialPerspectiveDistanceScale(Settings.AerialPerspectiveDistanceScale);
    SkyAtmosphereComponent->SetHeightFogContribution(Settings.HeightFogContribution);
    SkyAtmosphereComponent->SetTransmittanceMinLightElevationAngle(Settings.TransmittanceMinLightElevationAngle);
}

void ULightingAtmosphereSystem::ApplyCloudSettings(const FCloudSettings& Settings, bool bImmediate)
{
    if (!VolumetricCloudComponent)
    {
        return;
    }
    
    // Apply cloud coverage and density
    VolumetricCloudComponent->SetCloudCoverage(Settings.CloudCoverage);
    VolumetricCloudComponent->SetCloudType(Settings.CloudType);
    VolumetricCloudComponent->SetCloudDensity(Settings.CloudDensity);
    VolumetricCloudComponent->SetCloudOpacity(Settings.CloudOpacity);
    
    // Apply altitude settings
    VolumetricCloudComponent->SetLayerBottomAltitude(Settings.CloudLayerBottomAltitude);
    VolumetricCloudComponent->SetLayerHeight(Settings.CloudLayerHeight);
    
    // Apply wind
    VolumetricCloudComponent->SetWindDirection(Settings.CloudWindDirection);
    VolumetricCloudComponent->SetWindSpeed(Settings.CloudWindSpeed);
    
    // Apply lighting
    VolumetricCloudComponent->SetScatteringLuminanceScale(Settings.CloudScatteringLuminanceScale);
    VolumetricCloudComponent->SetScatteringLuminanceColor(Settings.CloudScatteringLuminanceColor);
}

void ULightingAtmosphereSystem::ApplyFogSettings(const FFogSettings& Settings, bool bImmediate)
{
    if (!HeightFogActor)
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComponent = HeightFogActor->GetComponent();
    if (!FogComponent)
    {
        return;
    }
    
    // Apply density settings
    FogComponent->SetFogDensity(Settings.FogDensity);
    FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
    
    // Apply color settings
    FogComponent->SetFogInscatteringColor(Settings.FogInscatteringColor);
    FogComponent->SetDirectionalInscatteringColor(Settings.DirectionalInscatteringColor);
    FogComponent->SetDirectionalInscatteringExponent(Settings.DirectionalInscatteringExponent);
    FogComponent->SetDirectionalInscatteringStartDistance(Settings.DirectionalInscatteringStartDistance);
    
    // Apply distance settings
    FogComponent->SetFogMaxOpacity(Settings.FogMaxOpacity);
    FogComponent->SetStartDistance(Settings.StartDistance);
    FogComponent->SetFogCutoffDistance(Settings.FogCutoffDistance);
    
    // Apply volumetric fog
    FogComponent->SetVolumetricFog(Settings.bVolumetricFog);
    FogComponent->SetVolumetricFogScatteringDistribution(Settings.VolumetricFogScatteringDistribution);
    FogComponent->SetVolumetricFogAlbedo(Settings.VolumetricFogAlbedo);
    FogComponent->SetVolumetricFogEmissive(Settings.VolumetricFogEmissive);
    FogComponent->SetVolumetricFogExtinctionScale(Settings.VolumetricFogExtinctionScale);
}

void ULightingAtmosphereSystem::StartTransition(const FAtmosphereSettings& NewAtmosphere, 
                                               const FCloudSettings& NewClouds, 
                                               const FFogSettings& NewFog)
{
    SourceAtmosphere = AtmosphereSettings;
    TargetAtmosphere = NewAtmosphere;
    SourceClouds = CloudSettings;
    TargetClouds = NewClouds;
    SourceFog = FogSettings;
    TargetFog = NewFog;
    
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
}

void ULightingAtmosphereSystem::StartAtmosphereTransition(const FAtmosphereSettings& NewSettings)
{
    SourceAtmosphere = AtmosphereSettings;
    TargetAtmosphere = NewSettings;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
}

void ULightingAtmosphereSystem::StartCloudTransition(const FCloudSettings& NewSettings)
{
    SourceClouds = CloudSettings;
    TargetClouds = NewSettings;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
}

void ULightingAtmosphereSystem::StartFogTransition(const FFogSettings& NewSettings)
{
    SourceFog = FogSettings;
    TargetFog = NewSettings;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
}

void ULightingAtmosphereSystem::UpdateTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    // Apply smooth curve
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    
    // Blend atmosphere settings
    AtmosphereSettings = BlendAtmosphereSettings(SourceAtmosphere, TargetAtmosphere, Alpha);
    ApplyAtmosphereSettings(AtmosphereSettings, false);
    
    // Blend cloud settings
    CloudSettings = BlendCloudSettings(SourceClouds, TargetClouds, Alpha);
    ApplyCloudSettings(CloudSettings, false);
    
    // Blend fog settings
    FogSettings = BlendFogSettings(SourceFog, TargetFog, Alpha);
    ApplyFogSettings(FogSettings, false);
    
    // Check if transition is complete
    if (Alpha >= 1.0f)
    {
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereSystem: Transition completed"));
    }
}

FAtmosphereSettings ULightingAtmosphereSystem::BlendAtmosphereSettings(const FAtmosphereSettings& A, const FAtmosphereSettings& B, float Alpha)
{
    FAtmosphereSettings Result;
    
    Result.RayleighScattering = FMath::Lerp(A.RayleighScattering, B.RayleighScattering, Alpha);
    Result.RayleighScatteringScale = FMath::Lerp(A.RayleighScatteringScale, B.RayleighScatteringScale, Alpha);
    Result.RayleighExponentialDistribution = FMath::Lerp(A.RayleighExponentialDistribution, B.RayleighExponentialDistribution, Alpha);
    
    Result.MieScattering = FMath::Lerp(A.MieScattering, B.MieScattering, Alpha);
    Result.MieScatteringScale = FMath::Lerp(A.MieScatteringScale, B.MieScatteringScale, Alpha);
    Result.MieAbsorption = FMath::Lerp(A.MieAbsorption, B.MieAbsorption, Alpha);
    Result.MieAbsorptionScale = FMath::Lerp(A.MieAbsorptionScale, B.MieAbsorptionScale, Alpha);
    Result.MieAnisotropy = FMath::Lerp(A.MieAnisotropy, B.MieAnisotropy, Alpha);
    Result.MieExponentialDistribution = FMath::Lerp(A.MieExponentialDistribution, B.MieExponentialDistribution, Alpha);
    
    Result.Absorption = FMath::Lerp(A.Absorption, B.Absorption, Alpha);
    Result.AbsorptionScale = FMath::Lerp(A.AbsorptionScale, B.AbsorptionScale, Alpha);
    
    Result.GroundRadius = FMath::Lerp(A.GroundRadius, B.GroundRadius, Alpha);
    Result.AtmosphereHeight = FMath::Lerp(A.AtmosphereHeight, B.AtmosphereHeight, Alpha);
    Result.GroundAlbedo = FMath::Lerp(A.GroundAlbedo, B.GroundAlbedo, Alpha);
    
    Result.SkyLuminanceFactor = FMath::Lerp(A.SkyLuminanceFactor, B.SkyLuminanceFactor, Alpha);
    Result.AerialPerspectiveDistanceScale = FMath::Lerp(A.AerialPerspectiveDistanceScale, B.AerialPerspectiveDistanceScale, Alpha);
    Result.HeightFogContribution = FMath::Lerp(A.HeightFogContribution, B.HeightFogContribution, Alpha);
    Result.TransmittanceMinLightElevationAngle = FMath::Lerp(A.TransmittanceMinLightElevationAngle, B.TransmittanceMinLightElevationAngle, Alpha);
    
    return Result;
}

FCloudSettings ULightingAtmosphereSystem::BlendCloudSettings(const FCloudSettings& A, const FCloudSettings& B, float Alpha)
{
    FCloudSettings Result;
    
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.CloudType = FMath::Lerp(A.CloudType, B.CloudType, Alpha);
    Result.CloudDensity = FMath::Lerp(A.CloudDensity, B.CloudDensity, Alpha);
    Result.CloudOpacity = FMath::Lerp(A.CloudOpacity, B.CloudOpacity, Alpha);
    
    Result.CloudLayerBottomAltitude = FMath::Lerp(A.CloudLayerBottomAltitude, B.CloudLayerBottomAltitude, Alpha);
    Result.CloudLayerHeight = FMath::Lerp(A.CloudLayerHeight, B.CloudLayerHeight, Alpha);
    
    Result.CloudWindDirection = FMath::Lerp(A.CloudWindDirection, B.CloudWindDirection, Alpha);
    Result.CloudWindSpeed = FMath::Lerp(A.CloudWindSpeed, B.CloudWindSpeed, Alpha);
    
    Result.CloudScatteringLuminanceScale = FMath::Lerp(A.CloudScatteringLuminanceScale, B.CloudScatteringLuminanceScale, Alpha);
    Result.CloudScatteringLuminanceColor = FMath::Lerp(A.CloudScatteringLuminanceColor, B.CloudScatteringLuminanceColor, Alpha);
    
    return Result;
}

FFogSettings ULightingAtmosphereSystem::BlendFogSettings(const FFogSettings& A, const FFogSettings& B, float Alpha)
{
    FFogSettings Result;
    
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.DirectionalInscatteringColor = FMath::Lerp(A.DirectionalInscatteringColor, B.DirectionalInscatteringColor, Alpha);
    Result.DirectionalInscatteringExponent = FMath::Lerp(A.DirectionalInscatteringExponent, B.DirectionalInscatteringExponent, Alpha);
    Result.DirectionalInscatteringStartDistance = FMath::Lerp(A.DirectionalInscatteringStartDistance, B.DirectionalInscatteringStartDistance, Alpha);
    
    Result.FogMaxOpacity = FMath::Lerp(A.FogMaxOpacity, B.FogMaxOpacity, Alpha);
    Result.StartDistance = FMath::Lerp(A.StartDistance, B.StartDistance, Alpha);
    Result.FogCutoffDistance = FMath::Lerp(A.FogCutoffDistance, B.FogCutoffDistance, Alpha);
    
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    Result.VolumetricFogScatteringDistribution = FMath::Lerp(A.VolumetricFogScatteringDistribution, B.VolumetricFogScatteringDistribution, Alpha);
    Result.VolumetricFogAlbedo = FMath::Lerp(A.VolumetricFogAlbedo, B.VolumetricFogAlbedo, Alpha);
    Result.VolumetricFogEmissive = FMath::Lerp(A.VolumetricFogEmissive, B.VolumetricFogEmissive, Alpha);
    Result.VolumetricFogExtinctionScale = FMath::Lerp(A.VolumetricFogExtinctionScale, B.VolumetricFogExtinctionScale, Alpha);
    
    return Result;
}

void ULightingAtmosphereSystem::SetupDefaultPresets()
{
    // Earth-like preset (default)
    FAtmosphereSettings EarthAtmosphere;
    EarthAtmosphere.RayleighScattering = FLinearColor(0.005802f, 0.013558f, 0.033100f);
    EarthAtmosphere.RayleighScatteringScale = 1.0f;
    EarthAtmosphere.MieScattering = FLinearColor(0.003996f, 0.003996f, 0.003996f);
    EarthAtmosphere.MieAnisotropy = 0.8f;
    EarthAtmosphere.GroundRadius = 6360.0f;
    EarthAtmosphere.AtmosphereHeight = 60.0f;
    AtmospherePresets.Add(EAtmospherePreset::EarthLike, EarthAtmosphere);
    
    FCloudSettings EarthClouds;
    EarthClouds.CloudCoverage = 0.5f;
    EarthClouds.CloudDensity = 1.0f;
    EarthClouds.CloudLayerBottomAltitude = 1.5f;
    EarthClouds.CloudLayerHeight = 4.0f;
    CloudPresets.Add(EAtmospherePreset::EarthLike, EarthClouds);
    
    FFogSettings EarthFog;
    EarthFog.FogDensity = 0.02f;
    EarthFog.FogHeightFalloff = 0.2f;
    EarthFog.bVolumetricFog = true;
    FogPresets.Add(EAtmospherePreset::EarthLike, EarthFog);
    
    // Prehistoric preset - thicker atmosphere, more dramatic
    FAtmosphereSettings PrehistoricAtmosphere = EarthAtmosphere;
    PrehistoricAtmosphere.RayleighScatteringScale = 1.3f;
    PrehistoricAtmosphere.MieScatteringScale = 1.5f;
    PrehistoricAtmosphere.AtmosphereHeight = 80.0f;
    PrehistoricAtmosphere.SkyLuminanceFactor = 1.2f;
    AtmospherePresets.Add(EAtmospherePreset::Prehistoric, PrehistoricAtmosphere);
    
    FCloudSettings PrehistoricClouds = EarthClouds;
    PrehistoricClouds.CloudCoverage = 0.7f;
    PrehistoricClouds.CloudDensity = 1.3f;
    PrehistoricClouds.CloudOpacity = 0.9f;
    CloudPresets.Add(EAtmospherePreset::Prehistoric, PrehistoricClouds);
    
    FFogSettings PrehistoricFog = EarthFog;
    PrehistoricFog.FogDensity = 0.035f;
    PrehistoricFog.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.8f);
    FogPresets.Add(EAtmospherePreset::Prehistoric, PrehistoricFog);
    
    // Mystical preset - ethereal, otherworldly
    FAtmosphereSettings MysticalAtmosphere = EarthAtmosphere;
    MysticalAtmosphere.RayleighScattering = FLinearColor(0.008f, 0.015f, 0.025f);
    MysticalAtmosphere.MieScattering = FLinearColor(0.006f, 0.004f, 0.008f);
    MysticalAtmosphere.SkyLuminanceFactor = 0.8f;
    MysticalAtmosphere.AerialPerspectiveDistanceScale = 1.5f;
    AtmospherePresets.Add(EAtmospherePreset::Mystical, MysticalAtmosphere);
    
    FCloudSettings MysticalClouds = EarthClouds;
    MysticalClouds.CloudCoverage = 0.3f;
    MysticalClouds.CloudType = 0.8f; // More wispy
    MysticalClouds.CloudOpacity = 0.6f;
    MysticalClouds.CloudScatteringLuminanceColor = FLinearColor(0.9f, 0.95f, 1.0f);
    CloudPresets.Add(EAtmospherePreset::Mystical, MysticalClouds);
    
    FFogSettings MysticalFog = EarthFog;
    MysticalFog.FogDensity = 0.025f;
    MysticalFog.FogInscatteringColor = FLinearColor(0.8f, 0.85f, 0.95f);
    MysticalFog.VolumetricFogAlbedo = FLinearColor(0.9f, 0.95f, 1.0f);
    FogPresets.Add(EAtmospherePreset::Mystical, MysticalFog);
    
    // Dangerous preset - ominous, threatening
    FAtmosphereSettings DangerousAtmosphere = EarthAtmosphere;
    DangerousAtmosphere.RayleighScattering = FLinearColor(0.008f, 0.006f, 0.004f);
    DangerousAtmosphere.MieScatteringScale = 2.0f;
    DangerousAtmosphere.SkyLuminanceFactor = 0.6f;
    DangerousAtmosphere.GroundAlbedo = FLinearColor(0.2f, 0.15f, 0.1f);
    AtmospherePresets.Add(EAtmospherePreset::Dangerous, DangerousAtmosphere);
    
    FCloudSettings DangerousClouds = EarthClouds;
    DangerousClouds.CloudCoverage = 0.8f;
    DangerousClouds.CloudDensity = 1.5f;
    DangerousClouds.CloudOpacity = 0.95f;
    DangerousClouds.CloudScatteringLuminanceColor = FLinearColor(0.8f, 0.7f, 0.6f);
    CloudPresets.Add(EAtmospherePreset::Dangerous, DangerousClouds);
    
    FFogSettings DangerousFog = EarthFog;
    DangerousFog.FogDensity = 0.04f;
    DangerousFog.FogInscatteringColor = FLinearColor(0.6f, 0.5f, 0.4f);
    DangerousFog.DirectionalInscatteringColor = FLinearColor(0.8f, 0.6f, 0.4f);
    FogPresets.Add(EAtmospherePreset::Dangerous, DangerousFog);
    
    // Peaceful preset - serene, calming
    FAtmosphereSettings PeacefulAtmosphere = EarthAtmosphere;
    PeacefulAtmosphere.RayleighScattering = FLinearColor(0.004f, 0.012f, 0.035f);
    PeacefulAtmosphere.SkyLuminanceFactor = 1.1f;
    PeacefulAtmosphere.AerialPerspectiveDistanceScale = 0.8f;
    AtmospherePresets.Add(EAtmospherePreset::Peaceful, PeacefulAtmosphere);
    
    FCloudSettings PeacefulClouds = EarthClouds;
    PeacefulClouds.CloudCoverage = 0.3f;
    PeacefulClouds.CloudType = 0.3f; // More fluffy
    PeacefulClouds.CloudOpacity = 0.7f;
    CloudPresets.Add(EAtmospherePreset::Peaceful, PeacefulClouds);
    
    FFogSettings PeacefulFog = EarthFog;
    PeacefulFog.FogDensity = 0.015f;
    PeacefulFog.FogInscatteringColor = FLinearColor(0.5f, 0.7f, 0.9f);
    FogPresets.Add(EAtmospherePreset::Peaceful, PeacefulFog);
    
    // Set default values
    AtmosphereSettings = EarthAtmosphere;
    CloudSettings = EarthClouds;
    FogSettings = EarthFog;
}