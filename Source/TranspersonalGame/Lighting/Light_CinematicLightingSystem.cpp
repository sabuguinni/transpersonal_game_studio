#include "Light_CinematicLightingSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

ULight_CinematicLightingSystem::ULight_CinematicLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for lighting updates

    CurrentMood = ELight_CinematicMood::Midday;
    TransitionSpeed = 1.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;

    MainSunLight = nullptr;
    AtmosphericFog = nullptr;
    CinematicPostProcess = nullptr;
}

void ULight_CinematicLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingPresets();
    FindLightingActors();
    
    // Apply default mood
    SetCinematicMood(CurrentMood);
}

void ULight_CinematicLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
}

void ULight_CinematicLightingSystem::InitializeLightingPresets()
{
    // Dawn preset - soft golden hour
    FLight_CinematicLightingPreset DawnPreset;
    DawnPreset.PresetName = TEXT("Dawn");
    DawnPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnPreset.SunIntensity = 2.0f;
    DawnPreset.SunRotation = FRotator(-15.0f, 75.0f, 0.0f);
    DawnPreset.FogColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    DawnPreset.FogDensity = 0.04f;
    DawnPreset.ContrastBoost = 1.1f;
    DawnPreset.SaturationMultiplier = 1.2f;
    MoodPresets.Add(ELight_CinematicMood::Dawn, DawnPreset);

    // Midday preset - harsh bright
    FLight_CinematicLightingPreset MiddayPreset;
    MiddayPreset.PresetName = TEXT("Midday");
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    MiddayPreset.SunIntensity = 4.0f;
    MiddayPreset.SunRotation = FRotator(-80.0f, 180.0f, 0.0f);
    MiddayPreset.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MiddayPreset.FogDensity = 0.01f;
    MiddayPreset.ContrastBoost = 1.4f;
    MiddayPreset.SaturationMultiplier = 0.9f;
    MoodPresets.Add(ELight_CinematicMood::Midday, MiddayPreset);

    // Dusk preset - warm orange
    FLight_CinematicLightingPreset DuskPreset;
    DuskPreset.PresetName = TEXT("Dusk");
    DuskPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskPreset.SunIntensity = 1.5f;
    DuskPreset.SunRotation = FRotator(-10.0f, 285.0f, 0.0f);
    DuskPreset.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskPreset.FogDensity = 0.06f;
    DuskPreset.ContrastBoost = 1.3f;
    DuskPreset.SaturationMultiplier = 1.4f;
    MoodPresets.Add(ELight_CinematicMood::Dusk, DuskPreset);

    // Night preset - cool blue
    FLight_CinematicLightingPreset NightPreset;
    NightPreset.PresetName = TEXT("Night");
    NightPreset.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SunRotation = FRotator(15.0f, 0.0f, 0.0f);
    NightPreset.FogColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightPreset.FogDensity = 0.08f;
    NightPreset.ContrastBoost = 1.6f;
    NightPreset.SaturationMultiplier = 0.7f;
    MoodPresets.Add(ELight_CinematicMood::Night, NightPreset);

    // Storm preset - dark dramatic
    FLight_CinematicLightingPreset StormPreset;
    StormPreset.PresetName = TEXT("Storm");
    StormPreset.SunColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    StormPreset.SunIntensity = 1.0f;
    StormPreset.SunRotation = FRotator(-30.0f, 200.0f, 0.0f);
    StormPreset.FogColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
    StormPreset.FogDensity = 0.12f;
    StormPreset.ContrastBoost = 1.8f;
    StormPreset.SaturationMultiplier = 0.6f;
    MoodPresets.Add(ELight_CinematicMood::Storm, StormPreset);

    // Fire preset - warm flickering
    FLight_CinematicLightingPreset FirePreset;
    FirePreset.PresetName = TEXT("Fire");
    FirePreset.SunColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    FirePreset.SunIntensity = 0.8f;
    FirePreset.SunRotation = FRotator(-20.0f, 45.0f, 0.0f);
    FirePreset.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    FirePreset.FogDensity = 0.05f;
    FirePreset.ContrastBoost = 1.5f;
    FirePreset.SaturationMultiplier = 1.3f;
    MoodPresets.Add(ELight_CinematicMood::Fire, FirePreset);
}

void ULight_CinematicLightingSystem::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find main directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        MainSunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find atmospheric fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Find post-process volume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CinematicPostProcess = Cast<APostProcessVolume>(FoundActors[0]);
    }

    // Find dynamic point lights
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APointLight::StaticClass(), FoundActors);
    DynamicLights.Empty();
    for (AActor* Actor : FoundActors)
    {
        if (APointLight* PointLight = Cast<APointLight>(Actor))
        {
            DynamicLights.Add(PointLight);
        }
    }
}

void ULight_CinematicLightingSystem::SetCinematicMood(ELight_CinematicMood NewMood)
{
    if (MoodPresets.Contains(NewMood))
    {
        CurrentMood = NewMood;
        ApplyLightingPreset(MoodPresets[NewMood]);
    }
}

void ULight_CinematicLightingSystem::ApplyLightingPreset(const FLight_CinematicLightingPreset& Preset)
{
    ApplyPresetToActors(Preset);
}

void ULight_CinematicLightingSystem::ApplyPresetToActors(const FLight_CinematicLightingPreset& Preset)
{
    // Apply to main sun light
    if (MainSunLight && MainSunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComp = MainSunLight->GetLightComponent();
        SunComp->SetLightColor(Preset.SunColor);
        SunComp->SetIntensity(Preset.SunIntensity);
        MainSunLight->SetActorRotation(Preset.SunRotation);
    }

    // Apply to atmospheric fog
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogInscatteringColor(Preset.FogColor);
        FogComp->SetFogDensity(Preset.FogDensity);
    }
}

void ULight_CinematicLightingSystem::CreateDramaticShadows(float ShadowIntensity)
{
    if (MainSunLight && MainSunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComp = MainSunLight->GetLightComponent();
        SunComp->SetCastShadows(true);
        SunComp->SetCastVolumetricShadow(true);
        
        // Enhance shadow quality
        SunComp->SetShadowResolutionScale(ShadowIntensity);
        SunComp->SetShadowBias(0.1f / ShadowIntensity);
    }
}

void ULight_CinematicLightingSystem::SetupThreePointLighting(FVector SubjectLocation)
{
    CreateThreePointLightSetup(SubjectLocation);
}

void ULight_CinematicLightingSystem::CreateThreePointLightSetup(FVector SubjectLocation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Key light (main illumination)
    FVector KeyLightPos = SubjectLocation + FVector(-500, -500, 300);
    APointLight* KeyLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), KeyLightPos, FRotator::ZeroRotator);
    if (KeyLight && KeyLight->GetLightComponent())
    {
        KeyLight->SetActorLabel(TEXT("KeyLight_ThreePoint"));
        UPointLightComponent* KeyComp = KeyLight->GetLightComponent();
        KeyComp->SetIntensity(1000.0f);
        KeyComp->SetLightColor(FLinearColor(1.0f, 0.9f, 0.8f, 1.0f));
        KeyComp->SetAttenuationRadius(1000.0f);
        KeyComp->SetCastShadows(true);
    }

    // Fill light (shadow softening)
    FVector FillLightPos = SubjectLocation + FVector(300, -300, 200);
    APointLight* FillLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), FillLightPos, FRotator::ZeroRotator);
    if (FillLight && FillLight->GetLightComponent())
    {
        FillLight->SetActorLabel(TEXT("FillLight_ThreePoint"));
        UPointLightComponent* FillComp = FillLight->GetLightComponent();
        FillComp->SetIntensity(400.0f);
        FillComp->SetLightColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
        FillComp->SetAttenuationRadius(800.0f);
        FillComp->SetCastShadows(false);
    }

    // Rim light (edge definition)
    FVector RimLightPos = SubjectLocation + FVector(400, 600, 250);
    ASpotLight* RimLight = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), RimLightPos, FRotator::ZeroRotator);
    if (RimLight && RimLight->GetLightComponent())
    {
        RimLight->SetActorLabel(TEXT("RimLight_ThreePoint"));
        USpotLightComponent* RimComp = RimLight->GetLightComponent();
        RimComp->SetIntensity(600.0f);
        RimComp->SetLightColor(FLinearColor(0.7f, 0.8f, 1.0f, 1.0f));
        RimComp->SetAttenuationRadius(600.0f);
        RimComp->SetInnerConeAngle(15.0f);
        RimComp->SetOuterConeAngle(30.0f);
        RimComp->SetCastShadows(false);
        
        // Point rim light at subject
        FVector Direction = (SubjectLocation - RimLightPos).GetSafeNormal();
        RimLight->SetActorRotation(Direction.Rotation());
    }
}

void ULight_CinematicLightingSystem::EnableVolumetricLighting(bool bEnable)
{
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetVolumetricFog(bEnable);
        if (bEnable)
        {
            FogComp->SetVolumetricFogScatteringDistribution(0.2f);
            FogComp->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
        }
    }
}

void ULight_CinematicLightingSystem::CreateFireLighting(FVector FireLocation, float Intensity)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APointLight* FireLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), FireLocation, FRotator::ZeroRotator);
    if (FireLight && FireLight->GetLightComponent())
    {
        FireLight->SetActorLabel(TEXT("FireLight_Dynamic"));
        UPointLightComponent* FireComp = FireLight->GetLightComponent();
        FireComp->SetIntensity(Intensity);
        FireComp->SetLightColor(FLinearColor(1.0f, 0.7f, 0.3f, 1.0f));
        FireComp->SetAttenuationRadius(600.0f);
        FireComp->SetCastShadows(true);
        FireComp->SetCastVolumetricShadow(true);
        
        DynamicLights.Add(FireLight);
        AnimateFlickeringTorch(FireLight);
    }
}

void ULight_CinematicLightingSystem::AnimateFlickeringTorch(APointLight* TorchLight)
{
    if (!TorchLight || !TorchLight->GetLightComponent()) return;

    // Start flickering animation timer
    GetWorld()->GetTimerManager().SetTimer(FlickerTimerHandle, [this, TorchLight]()
    {
        if (TorchLight && TorchLight->GetLightComponent())
        {
            UPointLightComponent* LightComp = TorchLight->GetLightComponent();
            float BaseIntensity = 800.0f;
            float FlickerAmount = FMath::RandRange(0.7f, 1.3f);
            LightComp->SetIntensity(BaseIntensity * FlickerAmount);
            
            // Slight color variation for realism
            float ColorVariation = FMath::RandRange(0.9f, 1.1f);
            FLinearColor BaseColor(1.0f, 0.7f, 0.3f, 1.0f);
            LightComp->SetLightColor(BaseColor * ColorVariation);
        }
    }, 0.1f, true);
}

void ULight_CinematicLightingSystem::TransitionToMood(ELight_CinematicMood TargetMood, float TransitionTime)
{
    if (!MoodPresets.Contains(TargetMood) || bIsTransitioning) return;

    TransitionTarget = TargetMood;
    TransitionStartPreset = MoodPresets[CurrentMood];
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    TransitionSpeed = 1.0f / TransitionTime;
}

void ULight_CinematicLightingSystem::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;

    TransitionProgress += TransitionSpeed * DeltaTime;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentMood = TransitionTarget;
    }

    // Interpolate between start and target presets
    FLight_CinematicLightingPreset CurrentPreset = InterpolateLightingPresets(
        TransitionStartPreset, 
        MoodPresets[TransitionTarget], 
        TransitionProgress
    );
    
    ApplyPresetToActors(CurrentPreset);
}

FLight_CinematicLightingPreset ULight_CinematicLightingSystem::InterpolateLightingPresets(
    const FLight_CinematicLightingPreset& A, 
    const FLight_CinematicLightingPreset& B, 
    float Alpha)
{
    FLight_CinematicLightingPreset Result;
    
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.ContrastBoost = FMath::Lerp(A.ContrastBoost, B.ContrastBoost, Alpha);
    Result.SaturationMultiplier = FMath::Lerp(A.SaturationMultiplier, B.SaturationMultiplier, Alpha);
    
    return Result;
}