#include "EnvironmentalAtmosphere.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

UEnvironmentalAtmosphere::UEnvironmentalAtmosphere()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second

    // Initialize default settings
    AtmosphereType = EEnvArt_AtmosphereType::Forest;
    TimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    
    // Create particle components
    DustParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    PollenParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PollenParticles"));
    MistParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MistParticles"));
    
    // Create audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    
    // Initialize particle components
    if (DustParticles)
    {
        DustParticles->bAutoActivate = false;
        DustParticles->SetupAttachment(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
    }
    
    if (PollenParticles)
    {
        PollenParticles->bAutoActivate = false;
        PollenParticles->SetupAttachment(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
    }
    
    if (MistParticles)
    {
        MistParticles->bAutoActivate = false;
        MistParticles->SetupAttachment(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
    }
    
    if (AmbientAudio)
    {
        AmbientAudio->bAutoActivate = true;
        AmbientAudio->SetupAttachment(GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
    }
}

void UEnvironmentalAtmosphere::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    CreateParticleEffects();
    SetupAmbientAudio();
    ApplyAtmosphereSettings();
}

void UEnvironmentalAtmosphere::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic atmosphere updates can go here
    // For now, we'll keep it simple and only update when explicitly called
}

void UEnvironmentalAtmosphere::SetAtmosphereType(EEnvArt_AtmosphereType NewType)
{
    if (AtmosphereType != NewType)
    {
        AtmosphereType = NewType;
        AtmosphereSettings = GetPresetSettings(AtmosphereType, TimeOfDay);
        ApplyAtmosphereSettings();
    }
}

void UEnvironmentalAtmosphere::SetTimeOfDay(EEnvArt_TimeOfDay NewTime)
{
    if (TimeOfDay != NewTime)
    {
        TimeOfDay = NewTime;
        AtmosphereSettings = GetPresetSettings(AtmosphereType, TimeOfDay);
        ApplyAtmosphereSettings();
    }
}

void UEnvironmentalAtmosphere::ApplyAtmosphereSettings()
{
    UpdateLighting();
    UpdateFog();
    UpdateParticleEffects();
    UpdateAmbientAudio();
}

void UEnvironmentalAtmosphere::UpdateLighting()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(AtmosphereSettings.SunColor);
        LightComp->SetIntensity(AtmosphereSettings.SunIntensity);
        SunLight->SetActorRotation(AtmosphereSettings.SunRotation);
        
        UE_LOG(LogTemp, Log, TEXT("Updated sun lighting: Color(%f,%f,%f), Intensity(%f), Rotation(%s)"),
            AtmosphereSettings.SunColor.R, AtmosphereSettings.SunColor.G, AtmosphereSettings.SunColor.B,
            AtmosphereSettings.SunIntensity, *AtmosphereSettings.SunRotation.ToString());
    }
}

void UEnvironmentalAtmosphere::UpdateFog()
{
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        FogComp->SetFogDensity(AtmosphereSettings.FogDensity);
        FogComp->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
        FogComp->SetStartDistance(AtmosphereSettings.FogStartDistance);
        
        UE_LOG(LogTemp, Log, TEXT("Updated fog: Color(%f,%f,%f), Density(%f)"),
            AtmosphereSettings.FogColor.R, AtmosphereSettings.FogColor.G, AtmosphereSettings.FogColor.B,
            AtmosphereSettings.FogDensity);
    }
}

void UEnvironmentalAtmosphere::UpdateParticleEffects()
{
    if (!AtmosphereSettings.bEnableParticleEffects)
    {
        if (DustParticles && DustParticles->IsActive())
            DustParticles->Deactivate();
        if (PollenParticles && PollenParticles->IsActive())
            PollenParticles->Deactivate();
        if (MistParticles && MistParticles->IsActive())
            MistParticles->Deactivate();
        return;
    }

    float Intensity = AtmosphereSettings.ParticleIntensity;
    
    // Update particle effects based on atmosphere type
    switch (AtmosphereType)
    {
        case EEnvArt_AtmosphereType::Forest:
            if (PollenParticles && !PollenParticles->IsActive())
                PollenParticles->Activate();
            if (DustParticles && DustParticles->IsActive())
                DustParticles->Deactivate();
            break;
            
        case EEnvArt_AtmosphereType::Volcanic:
            if (DustParticles && !DustParticles->IsActive())
                DustParticles->Activate();
            if (PollenParticles && PollenParticles->IsActive())
                PollenParticles->Deactivate();
            break;
            
        case EEnvArt_AtmosphereType::Swamp:
            if (MistParticles && !MistParticles->IsActive())
                MistParticles->Activate();
            break;
            
        case EEnvArt_AtmosphereType::Grassland:
            if (DustParticles && !DustParticles->IsActive())
                DustParticles->Activate();
            break;
            
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated particle effects for atmosphere type: %d"), (int32)AtmosphereType);
}

void UEnvironmentalAtmosphere::UpdateAmbientAudio()
{
    if (AmbientAudio)
    {
        // Audio updates would go here
        // For now, just log that we're updating audio
        UE_LOG(LogTemp, Log, TEXT("Updated ambient audio for atmosphere type: %d"), (int32)AtmosphereType);
    }
}

FEnvArt_AtmosphereSettings UEnvironmentalAtmosphere::GetPresetSettings(EEnvArt_AtmosphereType Type, EEnvArt_TimeOfDay Time)
{
    switch (Type)
    {
        case EEnvArt_AtmosphereType::Forest:
            return GetForestSettings(Time);
        case EEnvArt_AtmosphereType::Volcanic:
            return GetVolcanicSettings(Time);
        case EEnvArt_AtmosphereType::Swamp:
            return GetSwampSettings(Time);
        case EEnvArt_AtmosphereType::Grassland:
            return GetGrasslandSettings(Time);
        default:
            return FEnvArt_AtmosphereSettings();
    }
}

void UEnvironmentalAtmosphere::RefreshAtmosphere()
{
    FindLightingActors();
    ApplyAtmosphereSettings();
}

void UEnvironmentalAtmosphere::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break; // Use the first one found
    }

    // Find height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        HeightFog = *ActorItr;
        break;
    }

    // Find sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphere = *ActorItr;
        break;
    }

    // Find volumetric clouds
    for (TActorIterator<AVolumetricCloud> ActorItr(World); ActorItr; ++ActorItr)
    {
        VolumetricClouds = *ActorItr;
        break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Found lighting actors - Sun: %s, Fog: %s, Sky: %s, Clouds: %s"),
        SunLight ? TEXT("Yes") : TEXT("No"),
        HeightFog ? TEXT("Yes") : TEXT("No"),
        SkyAtmosphere ? TEXT("Yes") : TEXT("No"),
        VolumetricClouds ? TEXT("Yes") : TEXT("No"));
}

void UEnvironmentalAtmosphere::CreateParticleEffects()
{
    // Particle effects would be loaded from assets here
    // For now, we'll just ensure the components are properly initialized
    UE_LOG(LogTemp, Log, TEXT("Particle effects components initialized"));
}

void UEnvironmentalAtmosphere::SetupAmbientAudio()
{
    // Audio setup would go here
    // For now, just log that we're setting up audio
    UE_LOG(LogTemp, Log, TEXT("Ambient audio component initialized"));
}

FEnvArt_AtmosphereSettings UEnvironmentalAtmosphere::GetForestSettings(EEnvArt_TimeOfDay Time)
{
    FEnvArt_AtmosphereSettings Settings;
    
    switch (Time)
    {
        case EEnvArt_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            Settings.SunIntensity = 2.0f;
            Settings.SunRotation = FRotator(-75.0f, 45.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
            Settings.FogDensity = 0.05f;
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunIntensity = 3.0f;
            Settings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
            Settings.FogDensity = 0.02f;
            break;
        case EEnvArt_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            Settings.SunIntensity = 1.5f;
            Settings.SunRotation = FRotator(-85.0f, 45.0f, 0.0f);
            Settings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
            Settings.FogDensity = 0.08f;
            break;
        default:
            // Use default settings
            break;
    }
    
    Settings.bEnableParticleEffects = true;
    Settings.ParticleIntensity = 1.0f;
    
    return Settings;
}

FEnvArt_AtmosphereSettings UEnvironmentalAtmosphere::GetVolcanicSettings(EEnvArt_TimeOfDay Time)
{
    FEnvArt_AtmosphereSettings Settings;
    
    Settings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    Settings.SunIntensity = 2.5f;
    Settings.SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
    Settings.FogColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
    Settings.FogDensity = 0.1f;
    Settings.bEnableParticleEffects = true;
    Settings.ParticleIntensity = 2.0f;
    
    return Settings;
}

FEnvArt_AtmosphereSettings UEnvironmentalAtmosphere::GetSwampSettings(EEnvArt_TimeOfDay Time)
{
    FEnvArt_AtmosphereSettings Settings;
    
    Settings.SunColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f);
    Settings.SunIntensity = 2.0f;
    Settings.SunRotation = FRotator(-60.0f, 45.0f, 0.0f);
    Settings.FogColor = FLinearColor(0.4f, 0.5f, 0.4f, 1.0f);
    Settings.FogDensity = 0.15f;
    Settings.bEnableParticleEffects = true;
    Settings.ParticleIntensity = 1.5f;
    
    return Settings;
}

FEnvArt_AtmosphereSettings UEnvironmentalAtmosphere::GetGrasslandSettings(EEnvArt_TimeOfDay Time)
{
    FEnvArt_AtmosphereSettings Settings;
    
    switch (Time)
    {
        case EEnvArt_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
            Settings.SunIntensity = 2.5f;
            Settings.SunRotation = FRotator(-80.0f, 45.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            Settings.SunIntensity = 4.0f;
            Settings.SunRotation = FRotator(-20.0f, 45.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
            Settings.SunIntensity = 1.8f;
            Settings.SunRotation = FRotator(-88.0f, 45.0f, 0.0f);
            break;
        default:
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            Settings.SunIntensity = 3.5f;
            Settings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
            break;
    }
    
    Settings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    Settings.FogDensity = 0.01f;
    Settings.bEnableParticleEffects = true;
    Settings.ParticleIntensity = 0.8f;
    
    return Settings;
}