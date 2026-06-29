#include "NightAtmosphereController.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ANightAtmosphereController::ANightAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f; // ~30fps tick for flicker
}

void ANightAtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    ApplyNightPalette();
}

void ANightAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickCampfireFlicker(DeltaTime);
}

void ANightAtmosphereController::SetNightProgress(float NormalizedProgress)
{
    CurrentNightTime = FMath::Clamp(NormalizedProgress, 0.0f, 1.0f);

    // Deep night = middle third of night cycle (0.33..0.66)
    bool bWasDeepNight = bIsDeepNight;
    bIsDeepNight = (CurrentNightTime > 0.33f && CurrentNightTime < 0.66f);

    if (bIsDeepNight && !bWasDeepNight)
    {
        OnDeepNightBegin();
    }
    else if (!bIsDeepNight && bWasDeepNight && CurrentNightTime > 0.66f)
    {
        OnDawnApproaching();
    }

    // Modulate moon intensity across night
    // Peak at deep night (0.5), fade at dawn/dusk
    float MoonCurve = FMath::Sin(CurrentNightTime * PI);
    float CurrentMoonIntensity = MoonIntensity * MoonCurve;

    // Update directional light (moon)
    TArray<AActor*> DirLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirLights);
    if (DirLights.Num() > 0)
    {
        ADirectionalLight* Moon = Cast<ADirectionalLight>(DirLights[0]);
        if (Moon)
        {
            UDirectionalLightComponent* MoonComp = Moon->GetComponentByClass<UDirectionalLightComponent>();
            if (MoonComp)
            {
                MoonComp->SetIntensity(CurrentMoonIntensity);
            }
        }
    }

    UpdateNightFog();
}

void ANightAtmosphereController::ApplyNightPalette()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Apply night fog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActors[0]);
        if (Fog)
        {
            UExponentialHeightFogComponent* FogComp = Fog->GetComponentByClass<UExponentialHeightFogComponent>();
            if (FogComp)
            {
                FogComp->SetFogDensity(NightFogDensity);
                FogComp->SetFogInscatteringColor(NightFogColor);
                FogComp->SetVolumetricFog(true);
            }
        }
    }

    // Apply moonlight to directional light
    TArray<AActor*> DirLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirLights);
    if (DirLights.Num() > 0)
    {
        ADirectionalLight* Sun = Cast<ADirectionalLight>(DirLights[0]);
        if (Sun)
        {
            FRotator CurrentRot = Sun->GetActorRotation();
            Sun->SetActorRotation(FRotator(MoonPitchDegrees, CurrentRot.Yaw, CurrentRot.Roll));

            UDirectionalLightComponent* SunComp = Sun->GetComponentByClass<UDirectionalLightComponent>();
            if (SunComp)
            {
                SunComp->SetIntensity(MoonIntensity);
                SunComp->SetLightColor(MoonColor);
            }
        }
    }
}

void ANightAtmosphereController::RegisterCampfireLight(UPointLightComponent* CampfireComp)
{
    if (CampfireComp && !CampfireLights.Contains(CampfireComp))
    {
        CampfireLights.Add(CampfireComp);
    }
}

void ANightAtmosphereController::SpawnBioluminescentPatch(FVector WorldLocation, float Radius)
{
    // Bioluminescent patches are spawned as PointLight actors at runtime
    // The Niagara VFX agent handles the particle effect; we handle the light
    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APointLight* BioLight = World->SpawnActor<APointLight>(
        APointLight::StaticClass(),
        WorldLocation + FVector(0, 0, 30.0f),
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (BioLight)
    {
        UPointLightComponent* PLComp = BioLight->GetComponentByClass<UPointLightComponent>();
        if (PLComp)
        {
            PLComp->SetIntensity(BioGlowIntensity);
            PLComp->SetLightColor(BioGlowColor);
            PLComp->SetAttenuationRadius(Radius > 0.0f ? Radius : BioGlowRadius);
            PLComp->SetCastShadows(false);
            BioLights.Add(PLComp);
        }
    }
}

void ANightAtmosphereController::TickCampfireFlicker(float DeltaTime)
{
    if (CampfireLights.Num() == 0) return;

    FlickerAccumulator += DeltaTime * CampfireFlickerSpeed;

    // Perlin-style flicker using multiple sine waves
    float Flicker = FMath::Sin(FlickerAccumulator * 1.3f)
                  + FMath::Sin(FlickerAccumulator * 2.7f) * 0.5f
                  + FMath::Sin(FlickerAccumulator * 5.1f) * 0.25f;
    Flicker /= 1.75f; // normalize to -1..1
    Flicker = Flicker * CampfireFlickerAmplitude + 1.0f; // 0.85..1.15 multiplier

    float FlickerIntensity = CampfireIntensity * Flicker;

    // Slight color shift — warmer when brighter, cooler when dimmer
    float ColorShift = (Flicker - 1.0f) * 0.1f;
    FLinearColor FlickerColor = FLinearColor(
        FMath::Clamp(CampfireColor.R + ColorShift, 0.8f, 1.0f),
        FMath::Clamp(CampfireColor.G - ColorShift * 0.5f, 0.3f, 0.6f),
        FMath::Clamp(CampfireColor.B, 0.0f, 0.1f),
        1.0f
    );

    for (UPointLightComponent* Light : CampfireLights)
    {
        if (IsValid(Light))
        {
            Light->SetIntensity(FlickerIntensity);
            Light->SetLightColor(FlickerColor);
        }
    }
}

void ANightAtmosphereController::UpdateNightFog()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Deep night = denser, darker fog
    float FogMultiplier = bIsDeepNight ? 1.2f : 1.0f;

    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(FogActors[0]);
        if (Fog)
        {
            UExponentialHeightFogComponent* FogComp = Fog->GetComponentByClass<UExponentialHeightFogComponent>();
            if (FogComp)
            {
                FogComp->SetFogDensity(NightFogDensity * FogMultiplier);
            }
        }
    }
}
