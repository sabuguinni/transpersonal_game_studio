#include "WeatherSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWeatherSystem::AWeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    CurrentWeatherState = ELight_WeatherState::Clear;
    TargetWeatherState = ELight_WeatherState::Clear;
    LightningStrikeChancePerSecond = 0.05f;
    WeatherChangeCooldown = 120.f;
    RainIntensity = 0.f;
    TransitionProgress = 0.f;
    TransitionDurationTotal = 5.f;
    TransitionElapsed = 0.f;
    bIsTransitioning = false;
    LightningCooldown = 0.f;
    WeatherCooldownRemaining = 0.f;
}

void AWeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find scene actors if not assigned
    if (!SunLight)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Found);
        if (Found.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(Found[0]);
        }
    }

    if (!HeightFog)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), Found);
        if (Found.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(Found[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), Found);
        if (Found.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(Found[0]);
        }
    }

    CurrentPalette = GetPaletteForState(CurrentWeatherState);
    ApplyPalette(CurrentPalette);
}

void AWeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (WeatherCooldownRemaining > 0.f)
    {
        WeatherCooldownRemaining -= DeltaTime;
    }

    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }

    if (CurrentWeatherState == ELight_WeatherState::Storm)
    {
        UpdateLightning(DeltaTime);
    }

    UpdateRainIntensity();
}

void AWeatherSystem::SetWeatherState(ELight_WeatherState NewState, float TransitionDuration)
{
    if (NewState == CurrentWeatherState)
    {
        return;
    }

    TargetWeatherState = NewState;
    TargetPalette = GetPaletteForState(NewState);
    CurrentPalette = GetPaletteForState(CurrentWeatherState);

    TransitionDurationTotal = FMath::Max(0.1f, TransitionDuration);
    TransitionElapsed = 0.f;
    TransitionProgress = 0.f;
    bIsTransitioning = true;

    WeatherCooldownRemaining = WeatherChangeCooldown;
}

void AWeatherSystem::TriggerLightningStrike()
{
    if (CurrentWeatherState != ELight_WeatherState::Storm)
    {
        return;
    }

    // Brief flash: temporarily boost sun intensity
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (LightComp)
        {
            float OriginalIntensity = LightComp->Intensity;
            LightComp->SetIntensity(50.f);

            // Reset after 0.1s via timer
            FTimerHandle FlashTimer;
            GetWorld()->GetTimerManager().SetTimer(FlashTimer, [LightComp, OriginalIntensity]()
            {
                if (LightComp)
                {
                    LightComp->SetIntensity(OriginalIntensity);
                }
            }, 0.1f, false);
        }
    }

    LightningCooldown = 3.f + FMath::FRandRange(0.f, 7.f);
}

FLight_WeatherPalette AWeatherSystem::GetPaletteForState(ELight_WeatherState State) const
{
    FLight_WeatherPalette Palette;

    switch (State)
    {
    case ELight_WeatherState::Clear:
        Palette.SunColor = FLinearColor(1.f, 0.95f, 0.82f, 1.f);
        Palette.SunIntensity = 10.f;
        Palette.FogInscatteringColor = FLinearColor(0.55f, 0.70f, 0.90f, 1.f);
        Palette.FogDensity = 0.02f;
        Palette.SkyLightColor = FLinearColor(0.85f, 0.90f, 1.f, 1.f);
        Palette.SkyLightIntensity = 1.2f;
        break;

    case ELight_WeatherState::Overcast:
        Palette.SunColor = FLinearColor(0.72f, 0.78f, 0.85f, 1.f);
        Palette.SunIntensity = 1.8f;
        Palette.FogInscatteringColor = FLinearColor(0.38f, 0.42f, 0.50f, 1.f);
        Palette.FogDensity = 0.05f;
        Palette.SkyLightColor = FLinearColor(0.55f, 0.60f, 0.70f, 1.f);
        Palette.SkyLightIntensity = 0.7f;
        break;

    case ELight_WeatherState::Storm:
        Palette.SunColor = FLinearColor(0.55f, 0.60f, 0.68f, 1.f);
        Palette.SunIntensity = 0.8f;
        Palette.FogInscatteringColor = FLinearColor(0.28f, 0.32f, 0.38f, 1.f);
        Palette.FogDensity = 0.08f;
        Palette.SkyLightColor = FLinearColor(0.40f, 0.45f, 0.55f, 1.f);
        Palette.SkyLightIntensity = 0.4f;
        break;

    case ELight_WeatherState::Fog:
        Palette.SunColor = FLinearColor(0.80f, 0.82f, 0.85f, 1.f);
        Palette.SunIntensity = 2.5f;
        Palette.FogInscatteringColor = FLinearColor(0.70f, 0.72f, 0.75f, 1.f);
        Palette.FogDensity = 0.15f;
        Palette.SkyLightColor = FLinearColor(0.75f, 0.78f, 0.82f, 1.f);
        Palette.SkyLightIntensity = 0.5f;
        break;

    case ELight_WeatherState::Drizzle:
        Palette.SunColor = FLinearColor(0.68f, 0.72f, 0.78f, 1.f);
        Palette.SunIntensity = 3.5f;
        Palette.FogInscatteringColor = FLinearColor(0.45f, 0.50f, 0.58f, 1.f);
        Palette.FogDensity = 0.04f;
        Palette.SkyLightColor = FLinearColor(0.60f, 0.65f, 0.72f, 1.f);
        Palette.SkyLightIntensity = 0.8f;
        break;

    default:
        break;
    }

    return Palette;
}

void AWeatherSystem::ApplyPalette(const FLight_WeatherPalette& Palette)
{
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (LightComp)
        {
            LightComp->SetIntensity(Palette.SunIntensity);
            LightComp->SetLightColor(Palette.SunColor);
        }
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogInscatteringColor);
        }
    }

    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
            SkyComp->SetLightColor(Palette.SkyLightColor);
        }
    }
}

void AWeatherSystem::UpdateTransition(float DeltaTime)
{
    TransitionElapsed += DeltaTime;
    TransitionProgress = FMath::Clamp(TransitionElapsed / TransitionDurationTotal, 0.f, 1.f);

    FLight_WeatherPalette BlendedPalette;
    BlendedPalette.SunColor = FMath::Lerp(CurrentPalette.SunColor, TargetPalette.SunColor, TransitionProgress);
    BlendedPalette.SunIntensity = FMath::Lerp(CurrentPalette.SunIntensity, TargetPalette.SunIntensity, TransitionProgress);
    BlendedPalette.FogInscatteringColor = FMath::Lerp(CurrentPalette.FogInscatteringColor, TargetPalette.FogInscatteringColor, TransitionProgress);
    BlendedPalette.FogDensity = FMath::Lerp(CurrentPalette.FogDensity, TargetPalette.FogDensity, TransitionProgress);
    BlendedPalette.SkyLightColor = FMath::Lerp(CurrentPalette.SkyLightColor, TargetPalette.SkyLightColor, TransitionProgress);
    BlendedPalette.SkyLightIntensity = FMath::Lerp(CurrentPalette.SkyLightIntensity, TargetPalette.SkyLightIntensity, TransitionProgress);

    ApplyPalette(BlendedPalette);

    if (TransitionProgress >= 1.f)
    {
        CurrentWeatherState = TargetWeatherState;
        CurrentPalette = TargetPalette;
        bIsTransitioning = false;
        TransitionProgress = 1.f;
    }
}

void AWeatherSystem::UpdateLightning(float DeltaTime)
{
    if (LightningCooldown > 0.f)
    {
        LightningCooldown -= DeltaTime;
        return;
    }

    float RollChance = LightningStrikeChancePerSecond * DeltaTime;
    if (FMath::FRand() < RollChance)
    {
        TriggerLightningStrike();
    }
}

void AWeatherSystem::UpdateRainIntensity()
{
    switch (CurrentWeatherState)
    {
    case ELight_WeatherState::Storm:
        RainIntensity = bIsTransitioning ? TransitionProgress : 1.f;
        break;
    case ELight_WeatherState::Drizzle:
        RainIntensity = bIsTransitioning ? TransitionProgress * 0.3f : 0.3f;
        break;
    case ELight_WeatherState::Overcast:
        RainIntensity = 0.f;
        break;
    default:
        RainIntensity = 0.f;
        break;
    }
}
