// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "AtmosphereController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AAtmosphereController::AAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    InitializeComponents();
    
    // Set default atmosphere settings
    HighConsciousnessSettings.SkyColor = FLinearColor(1.0f, 0.9f, 0.7f);
    HighConsciousnessSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f);
    HighConsciousnessSettings.SunIntensity = 4.0f;
    HighConsciousnessSettings.SkyIntensity = 1.5f;
    HighConsciousnessSettings.FogDensity = 0.01f;
    HighConsciousnessSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.6f);
    
    NormalConsciousnessSettings.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f);
    NormalConsciousnessSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    NormalConsciousnessSettings.SunIntensity = 3.0f;
    NormalConsciousnessSettings.SkyIntensity = 1.0f;
    NormalConsciousnessSettings.FogDensity = 0.02f;
    NormalConsciousnessSettings.FogColor = FLinearColor(0.5f, 0.7f, 1.0f);
    
    LowConsciousnessSettings.SkyColor = FLinearColor(0.3f, 0.4f, 0.6f);
    LowConsciousnessSettings.SunColor = FLinearColor(0.8f, 0.7f, 0.9f);
    LowConsciousnessSettings.SunIntensity = 2.0f;
    LowConsciousnessSettings.SkyIntensity = 0.7f;
    LowConsciousnessSettings.FogDensity = 0.04f;
    LowConsciousnessSettings.FogColor = FLinearColor(0.4f, 0.3f, 0.6f);
    
    CurrentSettings = NormalConsciousnessSettings;
    TargetSettings = NormalConsciousnessSettings;
}

void AAtmosphereController::InitializeComponents()
{
    // Create Sun Light Component
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    RootComponent = SunLight;
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f));
    SunLight->SetCastShadows(true);
    
    // Create Sky Light Component
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor(0.4f, 0.7f, 1.0f));
    
    // Create Post Process Component
    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
    PostProcessComponent->SetupAttachment(RootComponent);
    PostProcessComponent->bUnbound = true;
}

void AAtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and bind to consciousness system
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            ConsciousnessSystem = PlayerPawn->FindComponentByClass<UConsciousnessSystem>();
            if (ConsciousnessSystem)
            {
                ConsciousnessSystem->OnConsciousnessChanged.AddDynamic(this, &AAtmosphereController::OnConsciousnessChanged);
                CurrentConsciousnessLevel = ConsciousnessSystem->GetConsciousnessLevel();
                UpdateAtmosphere(CurrentConsciousnessLevel);
            }
        }
    }
}

void AAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSmoothTransitions)
    {
        InterpolateSettings(DeltaTime);
    }
    
    UpdateLightingComponents();
}

void AAtmosphereController::UpdateAtmosphere(float ConsciousnessLevel)
{
    CurrentConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.0f, 1.0f);
    TargetSettings = GetSettingsForConsciousnessLevel(CurrentConsciousnessLevel);
    
    if (!bSmoothTransitions)
    {
        CurrentSettings = TargetSettings;
        UpdateLightingComponents();
    }
    
    OnAtmosphereChanged(CurrentConsciousnessLevel);
}

void AAtmosphereController::SetAtmosphereSettings(const FAtmosphereSettings& NewSettings, bool bInstant)
{
    TargetSettings = NewSettings;
    
    if (bInstant)
    {
        CurrentSettings = TargetSettings;
        UpdateLightingComponents();
    }
}

void AAtmosphereController::UpdateLightingComponents()
{
    if (SunLight)
    {
        SunLight->SetIntensity(CurrentSettings.SunIntensity);
        SunLight->SetLightColor(CurrentSettings.SunColor);
    }
    
    if (SkyLight)
    {
        SkyLight->SetIntensity(CurrentSettings.SkyIntensity);
        SkyLight->SetLightColor(CurrentSettings.SkyColor);
        SkyLight->RecaptureSky();
    }
    
    if (PostProcessComponent)
    {
        // Update post process settings
        FPostProcessSettings& PPSettings = PostProcessComponent->Settings;
        
        // Fog settings
        PPSettings.bOverride_FogDensity = true;
        PPSettings.FogDensity = CurrentSettings.FogDensity;
        PPSettings.bOverride_FogInscatteringColor = true;
        PPSettings.FogInscatteringColor = CurrentSettings.FogColor;
        
        // Color grading based on consciousness level
        PPSettings.bOverride_ColorGamma = true;
        float ConsciousnessInfluence = (CurrentConsciousnessLevel - 0.5f) * 0.2f;
        PPSettings.ColorGamma = FVector4(1.0f + ConsciousnessInfluence, 1.0f, 1.0f - ConsciousnessInfluence * 0.5f, 1.0f);
        
        // Bloom intensity
        PPSettings.bOverride_BloomIntensity = true;
        PPSettings.BloomIntensity = 0.675f + (CurrentConsciousnessLevel * 0.5f);
    }
}

void AAtmosphereController::InterpolateSettings(float DeltaTime)
{
    float Alpha = FMath::Clamp(TransitionSpeed * DeltaTime, 0.0f, 1.0f);
    
    // Interpolate colors
    CurrentSettings.SkyColor = FMath::Lerp(CurrentSettings.SkyColor, TargetSettings.SkyColor, Alpha);
    CurrentSettings.SunColor = FMath::Lerp(CurrentSettings.SunColor, TargetSettings.SunColor, Alpha);
    CurrentSettings.FogColor = FMath::Lerp(CurrentSettings.FogColor, TargetSettings.FogColor, Alpha);
    
    // Interpolate scalars
    CurrentSettings.SunIntensity = FMath::Lerp(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, Alpha);
    CurrentSettings.SkyIntensity = FMath::Lerp(CurrentSettings.SkyIntensity, TargetSettings.SkyIntensity, Alpha);
    CurrentSettings.FogDensity = FMath::Lerp(CurrentSettings.FogDensity, TargetSettings.FogDensity, Alpha);
}

FAtmosphereSettings AAtmosphereController::GetSettingsForConsciousnessLevel(float Level)
{
    if (Level >= 0.7f)
    {
        // High consciousness - interpolate between normal and high
        float Alpha = (Level - 0.7f) / 0.3f;
        FAtmosphereSettings Result;
        
        Result.SkyColor = FMath::Lerp(NormalConsciousnessSettings.SkyColor, HighConsciousnessSettings.SkyColor, Alpha);
        Result.SunColor = FMath::Lerp(NormalConsciousnessSettings.SunColor, HighConsciousnessSettings.SunColor, Alpha);
        Result.SunIntensity = FMath::Lerp(NormalConsciousnessSettings.SunIntensity, HighConsciousnessSettings.SunIntensity, Alpha);
        Result.SkyIntensity = FMath::Lerp(NormalConsciousnessSettings.SkyIntensity, HighConsciousnessSettings.SkyIntensity, Alpha);
        Result.FogDensity = FMath::Lerp(NormalConsciousnessSettings.FogDensity, HighConsciousnessSettings.FogDensity, Alpha);
        Result.FogColor = FMath::Lerp(NormalConsciousnessSettings.FogColor, HighConsciousnessSettings.FogColor, Alpha);
        
        return Result;
    }
    else if (Level <= 0.3f)
    {
        // Low consciousness - interpolate between low and normal
        float Alpha = Level / 0.3f;
        FAtmosphereSettings Result;
        
        Result.SkyColor = FMath::Lerp(LowConsciousnessSettings.SkyColor, NormalConsciousnessSettings.SkyColor, Alpha);
        Result.SunColor = FMath::Lerp(LowConsciousnessSettings.SunColor, NormalConsciousnessSettings.SunColor, Alpha);
        Result.SunIntensity = FMath::Lerp(LowConsciousnessSettings.SunIntensity, NormalConsciousnessSettings.SunIntensity, Alpha);
        Result.SkyIntensity = FMath::Lerp(LowConsciousnessSettings.SkyIntensity, NormalConsciousnessSettings.SkyIntensity, Alpha);
        Result.FogDensity = FMath::Lerp(LowConsciousnessSettings.FogDensity, NormalConsciousnessSettings.FogDensity, Alpha);
        Result.FogColor = FMath::Lerp(LowConsciousnessSettings.FogColor, NormalConsciousnessSettings.FogColor, Alpha);
        
        return Result;
    }
    else
    {
        // Normal range - return normal settings
        return NormalConsciousnessSettings;
    }
}

void AAtmosphereController::OnConsciousnessChanged(float NewLevel, float PreviousLevel)
{
    UpdateAtmosphere(NewLevel);
}