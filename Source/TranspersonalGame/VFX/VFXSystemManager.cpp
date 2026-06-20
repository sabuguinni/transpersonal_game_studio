// VFXSystemManager.cpp
// VFX Agent #17 — Transpersonal Game Studio
// Prehistoric survival game — realistic VFX only (fire, dust, weather, blood, volcanic ash)

#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Campfire defaults
    CampfireData.FlameIntensity = 1.0f;
    CampfireData.SmokeOpacity = 0.6f;
    CampfireData.EmberCount = 40;
    CampfireData.LightRadius = 600.0f;
    CampfireData.LightColor = FLinearColor(1.0f, 0.45f, 0.1f, 1.0f);
    CampfireData.bIsActive = false;

    // Footstep defaults
    FootstepData.DustParticleCount = 20;
    FootstepData.DustRadius = 80.0f;
    FootstepData.DustLifetime = 1.2f;
    FootstepData.ImpactForce = 1.0f;
    FootstepData.bHeavyCreature = false;

    // Weather defaults
    CurrentWeather = EVFX_WeatherType::Clear;
    RainIntensity = 0.0f;
    FogDensity = 0.02f;
    bVolcanicAshActive = false;
    AshDensity = 0.0f;
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
}

void UVFX_SystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Flicker campfire light if active
    if (CampfireData.bIsActive && CampfireLightRef)
    {
        float Flicker = FMath::Sin(GetWorld()->GetTimeSeconds() * 8.0f) * 0.15f + 1.0f;
        UPointLightComponent* LightComp = Cast<UPointLightComponent>(
            CampfireLightRef->GetComponentByClass(UPointLightComponent::StaticClass())
        );
        if (LightComp)
        {
            LightComp->SetIntensity(3000.0f * Flicker * CampfireData.FlameIntensity);
        }
    }
}

void UVFX_SystemManager::SpawnCampfireVFX(FVector Location, float Intensity)
{
    if (!GetWorld()) return;

    CampfireData.FlameIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
    CampfireData.bIsActive = true;

    // Spawn a point light actor for campfire glow
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* LightActor = GetWorld()->SpawnActor<AActor>(
        AActor::StaticClass(), Location + FVector(0, 0, 50.0f), FRotator::ZeroRotator, SpawnParams
    );

    if (LightActor)
    {
        UPointLightComponent* PointLight = NewObject<UPointLightComponent>(LightActor, TEXT("CampfireLight"));
        if (PointLight)
        {
            PointLight->RegisterComponent();
            PointLight->SetIntensity(3000.0f * Intensity);
            PointLight->SetLightColor(CampfireData.LightColor);
            PointLight->SetAttenuationRadius(CampfireData.LightRadius);
            LightActor->AddInstanceComponent(PointLight);
        }
        CampfireLightRef = LightActor;
        UE_LOG(LogTemp, Log, TEXT("VFX_CAMPFIRE_SPAWNED at %s intensity=%.1f"), *Location.ToString(), Intensity);
    }
}

void UVFX_SystemManager::SpawnFootstepDust(FVector ImpactLocation, float CreatureWeight)
{
    if (!GetWorld()) return;

    FootstepData.ImpactForce = FMath::Clamp(CreatureWeight / 1000.0f, 0.1f, 5.0f);
    FootstepData.bHeavyCreature = CreatureWeight > 2000.0f;
    FootstepData.DustParticleCount = FootstepData.bHeavyCreature ? 60 : 20;
    FootstepData.DustRadius = FootstepData.bHeavyCreature ? 200.0f : 80.0f;

    // Log footstep event for Niagara system pickup
    UE_LOG(LogTemp, Log, TEXT("VFX_FOOTSTEP at %s weight=%.0fkg heavy=%d particles=%d"),
        *ImpactLocation.ToString(), CreatureWeight,
        FootstepData.bHeavyCreature ? 1 : 0,
        FootstepData.DustParticleCount);
}

void UVFX_SystemManager::SetWeatherState(EVFX_WeatherType WeatherType, float Intensity)
{
    CurrentWeather = WeatherType;
    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    switch (WeatherType)
    {
        case EVFX_WeatherType::Clear:
            RainIntensity = 0.0f;
            FogDensity = 0.01f;
            break;
        case EVFX_WeatherType::LightRain:
            RainIntensity = Intensity * 0.4f;
            FogDensity = 0.03f;
            break;
        case EVFX_WeatherType::HeavyRain:
            RainIntensity = Intensity;
            FogDensity = 0.06f;
            break;
        case EVFX_WeatherType::Fog:
            RainIntensity = 0.0f;
            FogDensity = Intensity * 0.15f;
            break;
        case EVFX_WeatherType::Storm:
            RainIntensity = 1.0f;
            FogDensity = 0.08f;
            break;
        case EVFX_WeatherType::Snow:
            RainIntensity = 0.0f;
            FogDensity = 0.04f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_WEATHER_SET type=%d rain=%.2f fog=%.3f"),
        (int32)WeatherType, RainIntensity, FogDensity);
}

void UVFX_SystemManager::SpawnBloodImpact(FVector HitLocation, FVector HitNormal, float DamageAmount)
{
    if (!GetWorld()) return;

    // Scale blood VFX by damage
    float BloodScale = FMath::Clamp(DamageAmount / 50.0f, 0.2f, 3.0f);
    int32 DropletCount = FMath::RoundToInt(BloodScale * 15.0f);

    UE_LOG(LogTemp, Log, TEXT("VFX_BLOOD_IMPACT at %s damage=%.0f scale=%.2f droplets=%d"),
        *HitLocation.ToString(), DamageAmount, BloodScale, DropletCount);
}

void UVFX_SystemManager::SetVolcanicAsh(bool bActive, float Density)
{
    bVolcanicAshActive = bActive;
    AshDensity = FMath::Clamp(Density, 0.0f, 1.0f);

    if (bActive)
    {
        // Adjust fog color to grey/brown for ash atmosphere
        FogDensity = FMath::Max(FogDensity, AshDensity * 0.05f);
        UE_LOG(LogTemp, Log, TEXT("VFX_VOLCANIC_ASH_ON density=%.2f"), AshDensity);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_VOLCANIC_ASH_OFF"));
    }
}
