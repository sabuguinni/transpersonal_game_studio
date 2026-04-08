#include "AtmosphericVFXController.h"
#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"

AAtmosphericVFXController::AAtmosphericVFXController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create VFX components
    DustParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustParticles"));
    DustParticles->SetupAttachment(RootComponent);
    DustParticles->SetAutoDestroy(false);

    MistParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistParticles"));
    MistParticles->SetupAttachment(RootComponent);
    MistParticles->SetAutoDestroy(false);

    PollenParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PollenParticles"));
    PollenParticles->SetupAttachment(RootComponent);
    PollenParticles->SetAutoDestroy(false);

    LightShafts = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightShafts"));
    LightShafts->SetupAttachment(RootComponent);
    LightShafts->SetAutoDestroy(false);

    // Initialize default settings
    InitializeDefaultSettings();
}

void AAtmosphericVFXController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    // Find VFX Manager
    VFXManager = Cast<AVFXSystemManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVFXSystemManager::StaticClass()));
    
    // Apply initial settings
    CurrentSettings = GetSettingsForMood(CurrentMood);
    ModifySettingsForWeather(CurrentSettings);
    TargetSettings = CurrentSettings;
    ApplySettingsToComponents();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericVFXController initialized with mood: %d"), (int32)CurrentMood);
}

void AAtmosphericVFXController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAtmosphericEffects(DeltaTime);
    
    if (bAutoDetectThreats)
    {
        UpdateThreatDetection();
    }
}

void AAtmosphericVFXController::InitializeDefaultSettings()
{
    // Calm settings
    CalmSettings.DustIntensity = 0.2f;
    CalmSettings.MistIntensity = 0.1f;
    CalmSettings.PollenIntensity = 0.3f;
    CalmSettings.WindStrength = 0.3f;
    CalmSettings.LightShaftIntensity = 0.6f;
    CalmSettings.AtmosphericTint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    // Tense settings
    TenseSettings.DustIntensity = 0.4f;
    TenseSettings.MistIntensity = 0.3f;
    TenseSettings.PollenIntensity = 0.1f;
    TenseSettings.WindStrength = 0.6f;
    TenseSettings.LightShaftIntensity = 0.4f;
    TenseSettings.AtmosphericTint = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    // Threatening settings
    ThreateningSettings.DustIntensity = 0.7f;
    ThreateningSettings.MistIntensity = 0.5f;
    ThreateningSettings.PollenIntensity = 0.05f;
    ThreateningSettings.WindStrength = 0.8f;
    ThreateningSettings.LightShaftIntensity = 0.2f;
    ThreateningSettings.AtmosphericTint = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);

    // Panic settings
    PanicSettings.DustIntensity = 1.0f;
    PanicSettings.MistIntensity = 0.8f;
    PanicSettings.PollenIntensity = 0.0f;
    PanicSettings.WindStrength = 1.2f;
    PanicSettings.LightShaftIntensity = 0.1f;
    PanicSettings.AtmosphericTint = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
}

void AAtmosphericVFXController::InitializeComponents()
{
    if (DustParticles)
    {
        DustParticles->SetVisibility(true);
        DustParticles->Activate();
    }
    
    if (MistParticles)
    {
        MistParticles->SetVisibility(true);
        MistParticles->Activate();
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetVisibility(true);
        PollenParticles->Activate();
    }
    
    if (LightShafts)
    {
        LightShafts->SetVisibility(true);
        LightShafts->Activate();
    }
}

void AAtmosphericVFXController::UpdateAtmosphericEffects(float DeltaTime)
{
    // Update transition between settings
    BlendToTargetSettings(DeltaTime);
    
    // Update variations
    UpdateWindVariation(DeltaTime);
    UpdateDustVariation(DeltaTime);
    
    // Apply current settings to components
    ApplySettingsToComponents();
}

void AAtmosphericVFXController::BlendToTargetSettings(float DeltaTime)
{
    if (TransitionProgress < 1.0f)
    {
        TransitionProgress = FMath::Clamp(TransitionProgress + (DeltaTime * TransitionSpeed), 0.0f, 1.0f);
        
        // Interpolate between current and target settings
        FAtmosphericSettings StartSettings = CurrentSettings;
        
        CurrentSettings.DustIntensity = FMath::Lerp(StartSettings.DustIntensity, TargetSettings.DustIntensity, TransitionProgress);
        CurrentSettings.MistIntensity = FMath::Lerp(StartSettings.MistIntensity, TargetSettings.MistIntensity, TransitionProgress);
        CurrentSettings.PollenIntensity = FMath::Lerp(StartSettings.PollenIntensity, TargetSettings.PollenIntensity, TransitionProgress);
        CurrentSettings.WindStrength = FMath::Lerp(StartSettings.WindStrength, TargetSettings.WindStrength, TransitionProgress);
        CurrentSettings.LightShaftIntensity = FMath::Lerp(StartSettings.LightShaftIntensity, TargetSettings.LightShaftIntensity, TransitionProgress);
        CurrentSettings.AtmosphericTint = FLinearColor::LerpUsingHSV(StartSettings.AtmosphericTint, TargetSettings.AtmosphericTint, TransitionProgress);
    }
}

void AAtmosphericVFXController::ApplySettingsToComponents()
{
    if (DustParticles)
    {
        DustParticles->SetFloatParameter(TEXT("Intensity"), CurrentSettings.DustIntensity);
        DustParticles->SetVectorParameter(TEXT("WindDirection"), CurrentSettings.WindDirection);
        DustParticles->SetFloatParameter(TEXT("WindStrength"), CurrentSettings.WindStrength);
    }
    
    if (MistParticles)
    {
        MistParticles->SetFloatParameter(TEXT("Intensity"), CurrentSettings.MistIntensity);
        MistParticles->SetColorParameter(TEXT("Tint"), CurrentSettings.AtmosphericTint);
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetFloatParameter(TEXT("Intensity"), CurrentSettings.PollenIntensity);
        PollenParticles->SetVectorParameter(TEXT("WindDirection"), CurrentSettings.WindDirection);
    }
    
    if (LightShafts)
    {
        LightShafts->SetFloatParameter(TEXT("Intensity"), CurrentSettings.LightShaftIntensity);
        LightShafts->SetColorParameter(TEXT("LightColor"), CurrentSettings.AtmosphericTint);
    }
}

void AAtmosphericVFXController::UpdateWindVariation(float DeltaTime)
{
    WindVariationTimer += DeltaTime;
    
    if (WindVariationTimer >= 2.0f) // Update wind every 2 seconds
    {
        WindVariationTimer = 0.0f;
        
        // Add variation to wind direction and strength
        FVector BaseDirection = TargetSettings.WindDirection;
        float VariationAngle = FMath::RandRange(-CurrentSettings.WindVariation * 45.0f, CurrentSettings.WindVariation * 45.0f);
        FRotator VariationRotation(0, VariationAngle, 0);
        
        CurrentSettings.WindDirection = VariationRotation.RotateVector(BaseDirection);
        
        float StrengthVariation = FMath::RandRange(-CurrentSettings.WindVariation * 0.3f, CurrentSettings.WindVariation * 0.3f);
        CurrentSettings.WindStrength = FMath::Clamp(TargetSettings.WindStrength + StrengthVariation, 0.0f, 2.0f);
    }
}

void AAtmosphericVFXController::UpdateDustVariation(float DeltaTime)
{
    DustVariationTimer += DeltaTime;
    
    if (DustVariationTimer >= 3.0f) // Update dust every 3 seconds
    {
        DustVariationTimer = 0.0f;
        
        // Add subtle variation to dust intensity
        float DustVariation = FMath::RandRange(-0.1f, 0.1f);
        CurrentSettings.DustIntensity = FMath::Clamp(TargetSettings.DustIntensity + DustVariation, 0.0f, 2.0f);
    }
}

void AAtmosphericVFXController::SetAtmosphericMood(EAtmosphericMood NewMood, bool bInstant)
{
    if (CurrentMood == NewMood) return;
    
    CurrentMood = NewMood;
    TargetSettings = GetSettingsForMood(NewMood);
    ModifySettingsForWeather(TargetSettings);
    
    if (bInstant)
    {
        CurrentSettings = TargetSettings;
        TransitionProgress = 1.0f;
    }
    else
    {
        TransitionProgress = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Atmospheric mood changed to: %d"), (int32)NewMood);
}

void AAtmosphericVFXController::SetWeatherState(EWeatherState NewWeather, bool bInstant)
{
    if (CurrentWeather == NewWeather) return;
    
    CurrentWeather = NewWeather;
    TargetSettings = GetSettingsForMood(CurrentMood);
    ModifySettingsForWeather(TargetSettings);
    
    if (bInstant)
    {
        CurrentSettings = TargetSettings;
        TransitionProgress = 1.0f;
    }
    else
    {
        TransitionProgress = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weather state changed to: %d"), (int32)NewWeather);
}

void AAtmosphericVFXController::TriggerThreatResponse(FVector ThreatLocation, float Intensity)
{
    // Temporarily intensify atmospheric effects
    FAtmosphericSettings ThreatSettings = CurrentSettings;
    ThreatSettings.DustIntensity *= (1.0f + Intensity * ThreatResponseIntensity);
    ThreatSettings.WindStrength *= (1.0f + Intensity * 0.5f);
    ThreatSettings.LightShaftIntensity *= (1.0f - Intensity * 0.3f);
    
    // Apply threat direction to wind
    FVector DirectionToThreat = (ThreatLocation - GetActorLocation()).GetSafeNormal();
    ThreatSettings.WindDirection = DirectionToThreat;
    
    TargetSettings = ThreatSettings;
    TransitionProgress = 0.0f;
    TransitionSpeed = 4.0f; // Faster transition for threats
    
    UE_LOG(LogTemp, Warning, TEXT("Threat response triggered with intensity: %f"), Intensity);
}

void AAtmosphericVFXController::SetWindDirection(FVector NewDirection, float Strength)
{
    CurrentSettings.WindDirection = NewDirection.GetSafeNormal();
    TargetSettings.WindDirection = CurrentSettings.WindDirection;
    
    if (Strength >= 0.0f)
    {
        CurrentSettings.WindStrength = Strength;
        TargetSettings.WindStrength = Strength;
    }
}

void AAtmosphericVFXController::UpdateThreatDetection()
{
    TArray<AActor*> NearbyThreats = DetectNearbyThreats();
    
    if (NearbyThreats.Num() > 0)
    {
        float ThreatLevel = CalculateThreatLevel(NearbyThreats);
        
        // Determine appropriate mood based on threat level
        EAtmosphericMood NewMood = CurrentMood;
        if (ThreatLevel > 0.8f)
        {
            NewMood = EAtmosphericMood::Panic;
        }
        else if (ThreatLevel > 0.6f)
        {
            NewMood = EAtmosphericMood::Threatening;
        }
        else if (ThreatLevel > 0.3f)
        {
            NewMood = EAtmosphericMood::Tense;
        }
        
        if (NewMood != CurrentMood)
        {
            SetAtmosphericMood(NewMood, false);
        }
    }
    else
    {
        // No threats detected, gradually return to calm
        if (CurrentMood != EAtmosphericMood::Calm)
        {
            SetAtmosphericMood(EAtmosphericMood::Calm, false);
        }
    }
}

bool AAtmosphericVFXController::IsPlayerInDanger() const
{
    TArray<AActor*> Threats = DetectNearbyThreats();
    return Threats.Num() > 0 && CalculateThreatLevel(Threats) > 0.5f;
}

FAtmosphericSettings AAtmosphericVFXController::GetSettingsForMood(EAtmosphericMood Mood) const
{
    switch (Mood)
    {
        case EAtmosphericMood::Tense:
            return TenseSettings;
        case EAtmosphericMood::Threatening:
            return ThreateningSettings;
        case EAtmosphericMood::Panic:
            return PanicSettings;
        case EAtmosphericMood::Calm:
        default:
            return CalmSettings;
    }
}

void AAtmosphericVFXController::ModifySettingsForWeather(FAtmosphericSettings& Settings) const
{
    switch (CurrentWeather)
    {
        case EWeatherState::Misty:
            Settings.MistIntensity *= 2.0f;
            Settings.LightShaftIntensity *= 0.7f;
            break;
        case EWeatherState::Overcast:
            Settings.LightShaftIntensity *= 0.3f;
            Settings.AtmosphericTint = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
            break;
        case EWeatherState::Storm:
            Settings.WindStrength *= 2.0f;
            Settings.DustIntensity *= 1.5f;
            Settings.LightShaftIntensity *= 0.1f;
            Settings.AtmosphericTint = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
            break;
        case EWeatherState::Clear:
        default:
            // No modifications for clear weather
            break;
    }
}

TArray<AActor*> AAtmosphericVFXController::DetectNearbyThreats() const
{
    TArray<AActor*> Threats;
    
    // Get all actors within detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;
        
        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= ThreatDetectionRadius)
        {
            // Check if actor has threat tags or is a predator
            if (Actor->Tags.Contains(TEXT("Predator")) || 
                Actor->Tags.Contains(TEXT("Threat")) ||
                Actor->Tags.Contains(TEXT("LargeDinosaur")))
            {
                Threats.Add(Actor);
            }
        }
    }
    
    return Threats;
}

float AAtmosphericVFXController::CalculateThreatLevel(const TArray<AActor*>& Threats) const
{
    if (Threats.Num() == 0) return 0.0f;
    
    float TotalThreat = 0.0f;
    
    for (AActor* Threat : Threats)
    {
        float Distance = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());
        float DistanceRatio = 1.0f - (Distance / ThreatDetectionRadius);
        
        float ThreatValue = 0.5f; // Base threat
        
        // Increase threat based on tags
        if (Threat->Tags.Contains(TEXT("Apex")))
        {
            ThreatValue = 1.0f;
        }
        else if (Threat->Tags.Contains(TEXT("LargePredator")))
        {
            ThreatValue = 0.8f;
        }
        
        TotalThreat += ThreatValue * DistanceRatio;
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}