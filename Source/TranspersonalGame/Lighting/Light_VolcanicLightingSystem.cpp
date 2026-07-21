#include "Light_VolcanicLightingSystem.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

ULight_VolcanicLightingSystem::ULight_VolcanicLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize volcanic lighting state
    CurrentVolcanicState = ELight_VolcanicLightState::Dormant;
    CurrentLavaIntensity = 0.0f;
    CurrentTemperature = 20.0f; // Ambient temperature
    bIsLavaGlowActive = false;
    FlickerTime = 0.0f;
    BaseFlickerIntensity = 500.0f;
    bFlickerDirection = true;

    // Create lava light component
    LavaLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("LavaLightComponent"));
    if (LavaLightComponent)
    {
        LavaLightComponent->SetIntensity(0.0f);
        LavaLightComponent->SetLightColor(FLinearColor(1.0f, 0.3f, 0.1f, 1.0f));
        LavaLightComponent->SetAttenuationRadius(1000.0f);
        LavaLightComponent->SetCastShadows(true);
        LavaLightComponent->SetVolumetricScatteringIntensity(2.0f);
    }

    // Initialize volcanic config
    VolcanicConfig.BaseIntensity = 500.0f;
    VolcanicConfig.MaxIntensity = 2000.0f;
    VolcanicConfig.LavaColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);
    VolcanicConfig.FlickerSpeed = 2.0f;
    VolcanicConfig.HeatShimmerIntensity = 0.8f;
    VolcanicConfig.VolcanicAshDensity = 0.3f;
}

void ULight_VolcanicLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeVolcanicLighting();
}

void ULight_VolcanicLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsLavaGlowActive)
    {
        UpdateLavaFlicker(DeltaTime);
        UpdateHeatDistortion(DeltaTime);
        UpdateVolcanicAtmosphere();
    }
}

void ULight_VolcanicLightingSystem::InitializeVolcanicLighting()
{
    if (!LavaLightComponent)
        return;

    // Set initial lighting state
    LavaLightComponent->SetIntensity(0.0f);
    LavaLightComponent->SetLightColor(VolcanicConfig.LavaColor);
    
    // Create heat shimmer effect if Niagara is available
    if (GetOwner())
    {
        HeatShimmerComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            nullptr, // Will need to set actual Niagara system
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false
        );

        VolcanicAshComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            nullptr, // Will need to set actual Niagara system
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false
        );
    }
}

void ULight_VolcanicLightingSystem::SetVolcanicState(ELight_VolcanicLightState NewState)
{
    if (CurrentVolcanicState == NewState)
        return;

    CurrentVolcanicState = NewState;

    switch (NewState)
    {
        case ELight_VolcanicLightState::Dormant:
            DeactivateLavaGlow();
            CurrentTemperature = 20.0f;
            break;

        case ELight_VolcanicLightState::LowGlow:
            ActivateLavaGlow(VolcanicConfig.BaseIntensity * 0.3f);
            CurrentTemperature = 200.0f;
            break;

        case ELight_VolcanicLightState::ActiveGlow:
            ActivateLavaGlow(VolcanicConfig.BaseIntensity);
            CurrentTemperature = 500.0f;
            break;

        case ELight_VolcanicLightState::IntenseGlow:
            ActivateLavaGlow(VolcanicConfig.BaseIntensity * 1.5f);
            CurrentTemperature = 800.0f;
            SpawnVolcanicAshParticles();
            break;

        case ELight_VolcanicLightState::Erupting:
            ActivateLavaGlow(VolcanicConfig.MaxIntensity);
            CurrentTemperature = 1200.0f;
            SpawnVolcanicAshParticles();
            CreateHeatShimmerEffect(GetOwner()->GetActorLocation(), 1000.0f);
            break;
    }

    UpdateLightingForTemperature(CurrentTemperature);
}

void ULight_VolcanicLightingSystem::ActivateLavaGlow(float Intensity)
{
    if (!LavaLightComponent)
        return;

    bIsLavaGlowActive = true;
    CurrentLavaIntensity = FMath::Clamp(Intensity, 0.0f, VolcanicConfig.MaxIntensity);
    BaseFlickerIntensity = CurrentLavaIntensity;

    LavaLightComponent->SetIntensity(CurrentLavaIntensity);
    LavaLightComponent->SetVisibility(true);

    // Start flicker timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FlickerTimerHandle,
            [this]() { UpdateLavaFlicker(0.1f); },
            0.1f,
            true
        );
    }
}

void ULight_VolcanicLightingSystem::DeactivateLavaGlow()
{
    bIsLavaGlowActive = false;
    CurrentLavaIntensity = 0.0f;

    if (LavaLightComponent)
    {
        LavaLightComponent->SetIntensity(0.0f);
        LavaLightComponent->SetVisibility(false);
    }

    // Clear flicker timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(FlickerTimerHandle);
    }
}

void ULight_VolcanicLightingSystem::CreateHeatShimmerEffect(FVector Location, float Radius)
{
    if (!HeatShimmerComponent)
        return;

    // Set heat shimmer parameters
    HeatShimmerComponent->SetWorldLocation(Location);
    HeatShimmerComponent->SetFloatParameter(TEXT("ShimmerIntensity"), VolcanicConfig.HeatShimmerIntensity);
    HeatShimmerComponent->SetFloatParameter(TEXT("ShimmerRadius"), Radius);
    HeatShimmerComponent->SetVectorParameter(TEXT("ShimmerCenter"), Location);
}

void ULight_VolcanicLightingSystem::SpawnVolcanicAshParticles()
{
    if (!VolcanicAshComponent)
        return;

    VolcanicAshComponent->SetFloatParameter(TEXT("AshDensity"), VolcanicConfig.VolcanicAshDensity);
    VolcanicAshComponent->SetFloatParameter(TEXT("AshSpawnRate"), CurrentTemperature / 100.0f);
    VolcanicAshComponent->SetVectorParameter(TEXT("WindDirection"), FVector(1.0f, 0.0f, 0.5f));
}

void ULight_VolcanicLightingSystem::UpdateLightingForTemperature(float Temperature)
{
    CurrentTemperature = Temperature;
    
    if (!LavaLightComponent)
        return;

    // Calculate color based on temperature
    FLinearColor TemperatureColor = CalculateLavaColorForTemperature(Temperature);
    LavaLightComponent->SetLightColor(TemperatureColor);

    // Adjust intensity based on temperature
    float TemperatureIntensity = FMath::GetMappedRangeValueClamped(
        FVector2D(20.0f, 1200.0f),
        FVector2D(0.0f, VolcanicConfig.MaxIntensity),
        Temperature
    );

    if (bIsLavaGlowActive)
    {
        BaseFlickerIntensity = TemperatureIntensity;
    }
}

void ULight_VolcanicLightingSystem::UpdateLavaFlicker(float DeltaTime)
{
    if (!LavaLightComponent || !bIsLavaGlowActive)
        return;

    FlickerTime += DeltaTime * VolcanicConfig.FlickerSpeed;

    // Create realistic lava flicker using sine wave with noise
    float FlickerNoise = FMath::PerlinNoise1D(FlickerTime * 3.0f) * 0.3f;
    float FlickerSine = FMath::Sin(FlickerTime) * 0.2f;
    float FlickerVariation = FlickerNoise + FlickerSine;

    float FlickerIntensity = BaseFlickerIntensity + (BaseFlickerIntensity * FlickerVariation);
    FlickerIntensity = FMath::Clamp(FlickerIntensity, BaseFlickerIntensity * 0.7f, BaseFlickerIntensity * 1.3f);

    LavaLightComponent->SetIntensity(FlickerIntensity);
    CurrentLavaIntensity = FlickerIntensity;
}

void ULight_VolcanicLightingSystem::UpdateHeatDistortion(float DeltaTime)
{
    if (!HeatShimmerComponent)
        return;

    // Update heat shimmer based on temperature
    float ShimmerIntensity = FMath::GetMappedRangeValueClamped(
        FVector2D(20.0f, 1200.0f),
        FVector2D(0.0f, 2.0f),
        CurrentTemperature
    );

    HeatShimmerComponent->SetFloatParameter(TEXT("ShimmerIntensity"), ShimmerIntensity);
    HeatShimmerComponent->SetFloatParameter(TEXT("DistortionStrength"), ShimmerIntensity * 0.5f);
}

void ULight_VolcanicLightingSystem::UpdateVolcanicAtmosphere()
{
    // Update atmospheric effects based on volcanic state
    if (VolcanicAshComponent && CurrentVolcanicState >= ELight_VolcanicLightState::IntenseGlow)
    {
        float AshIntensity = static_cast<float>(CurrentVolcanicState) / 4.0f;
        VolcanicAshComponent->SetFloatParameter(TEXT("AshDensity"), AshIntensity);
    }
}

void ULight_VolcanicLightingSystem::ApplyTemperatureToLighting(float Temperature)
{
    UpdateLightingForTemperature(Temperature);
}

FLinearColor ULight_VolcanicLightingSystem::CalculateLavaColorForTemperature(float Temperature)
{
    // Temperature-based color calculation for realistic lava
    if (Temperature < 100.0f)
    {
        // Cool - dark red
        return FLinearColor(0.3f, 0.1f, 0.05f, 1.0f);
    }
    else if (Temperature < 500.0f)
    {
        // Warm - red-orange
        return FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
    }
    else if (Temperature < 800.0f)
    {
        // Hot - bright orange
        return FLinearColor(1.0f, 0.5f, 0.1f, 1.0f);
    }
    else if (Temperature < 1000.0f)
    {
        // Very hot - orange-yellow
        return FLinearColor(1.0f, 0.7f, 0.2f, 1.0f);
    }
    else
    {
        // Extremely hot - bright yellow-white
        return FLinearColor(1.0f, 0.9f, 0.6f, 1.0f);
    }
}