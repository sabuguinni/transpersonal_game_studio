#include "AtmosphericVFXController.h"
#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"

AAtmosphericVFXController::AAtmosphericVFXController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Create Niagara components
    DustParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustParticles"));
    DustParticles->SetupAttachment(RootComponent);
    
    MistParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistParticles"));
    MistParticles->SetupAttachment(RootComponent);
    
    PollenParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PollenParticles"));
    PollenParticles->SetupAttachment(RootComponent);
    
    LightShafts = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightShafts"));
    LightShafts->SetupAttachment(RootComponent);
    
    // Initialize default settings
    InitializeDefaultSettings();
}

void AAtmosphericVFXController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    
    // Find VFX Manager
    VFXManager = Cast<AVFXSystemManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVFXSystemManager::StaticClass()));
    if (!VFXManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericVFXController: VFXManager not found"));
    }
    
    // Set initial atmospheric settings
    SetAtmosphericMood(CurrentMood, true);
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
    CalmSettings.PollenIntensity = 0.05f;
    CalmSettings.WindStrength = 0.3f;
    CalmSettings.LightShaftIntensity = 0.4f;
    CalmSettings.AtmosphericTint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    
    // Tense settings
    TenseSettings.DustIntensity = 0.4f;
    TenseSettings.MistIntensity = 0.3f;
    TenseSettings.PollenIntensity = 0.1f;
    TenseSettings.WindStrength = 0.6f;
    TenseSettings.LightShaftIntensity = 0.3f;
    TenseSettings.AtmosphericTint = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    
    // Threatening settings
    ThreateningSettings.DustIntensity = 0.7f;
    ThreateningSettings.MistIntensity = 0.5f;
    ThreateningSettings.PollenIntensity = 0.2f;
    ThreateningSettings.WindStrength = 0.8f;
    ThreateningSettings.LightShaftIntensity = 0.2f;
    ThreateningSettings.AtmosphericTint = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    
    // Panic settings
    PanicSettings.DustIntensity = 1.0f;
    PanicSettings.MistIntensity = 0.8f;
    PanicSettings.PollenIntensity = 0.3f;
    PanicSettings.WindStrength = 1.2f;
    PanicSettings.LightShaftIntensity = 0.1f;
    PanicSettings.AtmosphericTint = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
}

void AAtmosphericVFXController::InitializeComponents()
{
    // Initialize Niagara components with default systems
    // These would be set in Blueprint or loaded from assets
    if (DustParticles)
    {
        DustParticles->SetAutoActivate(true);
    }
    
    if (MistParticles)
    {
        MistParticles->SetAutoActivate(true);
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetAutoActivate(true);
    }
    
    if (LightShafts)
    {
        LightShafts->SetAutoActivate(true);
    }
}

void AAtmosphericVFXController::SetAtmosphericMood(EAtmosphericMood NewMood, bool bInstant)
{
    if (CurrentMood == NewMood && TransitionProgress >= 1.0f)
    {
        return;
    }
    
    CurrentMood = NewMood;
    TargetSettings = GetSettingsForMood(NewMood);
    ModifySettingsForWeather(TargetSettings);
    
    if (bInstant)
    {
        CurrentSettings = TargetSettings;
        TransitionProgress = 1.0f;
        ApplySettingsToComponents();
    }
    else
    {
        TransitionProgress = 0.0f;
    }
}

void AAtmosphericVFXController::SetWeatherState(EWeatherState NewWeather, bool bInstant)
{
    if (CurrentWeather == NewWeather)
    {
        return;
    }
    
    CurrentWeather = NewWeather;
    TargetSettings = GetSettingsForMood(CurrentMood);
    ModifySettingsForWeather(TargetSettings);
    
    if (bInstant)
    {
        CurrentSettings = TargetSettings;
        TransitionProgress = 1.0f;
        ApplySettingsToComponents();
    }
    else
    {
        TransitionProgress = 0.0f;
    }
}

void AAtmosphericVFXController::TriggerThreatResponse(FVector ThreatLocation, float Intensity)
{
    // Temporarily increase atmospheric tension
    FAtmosphericSettings ThreatSettings = CurrentSettings;
    ThreatSettings.DustIntensity *= (1.0f + Intensity * ThreatResponseIntensity);
    ThreatSettings.WindStrength *= (1.0f + Intensity * 0.5f);
    ThreatSettings.AtmosphericTint = FLinearColor::LerpUsingHSV(
        ThreatSettings.AtmosphericTint,
        FLinearColor(0.6f, 0.4f, 0.3f, 1.0f),
        Intensity * 0.3f
    );
    
    // Apply threat settings temporarily
    CurrentSettings = ThreatSettings;
    ApplySettingsToComponents();
    
    // Schedule return to normal
    GetWorld()->GetTimerManager().SetTimer(
        ThreatResponseTimer,
        [this]() {
            SetAtmosphericMood(CurrentMood, false);
        },
        5.0f,
        false
    );
}

void AAtmosphericVFXController::SetWindDirection(FVector NewDirection, float Strength)
{
    TargetSettings.WindDirection = NewDirection.GetSafeNormal();
    if (Strength >= 0.0f)
    {
        TargetSettings.WindStrength = Strength;
    }
}

void AAtmosphericVFXController::UpdateThreatDetection()
{
    TArray<AActor*> NearbyThreats = DetectNearbyThreats();
    
    if (NearbyThreats.Num() > 0)
    {
        float ThreatLevel = CalculateThreatLevel(NearbyThreats);
        
        if (ThreatLevel > 0.7f)
        {
            SetAtmosphericMood(EAtmosphericMood::Panic, false);
        }
        else if (ThreatLevel > 0.4f)
        {
            SetAtmosphericMood(EAtmosphericMood::Threatening, false);
        }
        else if (ThreatLevel > 0.2f)
        {
            SetAtmosphericMood(EAtmosphericMood::Tense, false);
        }
    }
    else
    {
        SetAtmosphericMood(EAtmosphericMood::Calm, false);
    }
}

bool AAtmosphericVFXController::IsPlayerInDanger() const
{
    TArray<AActor*> Threats = DetectNearbyThreats();
    return Threats.Num() > 0 && CalculateThreatLevel(Threats) > 0.3f;
}

void AAtmosphericVFXController::UpdateAtmosphericEffects(float DeltaTime)
{
    // Blend to target settings
    if (TransitionProgress < 1.0f)
    {
        BlendToTargetSettings(DeltaTime);
    }
    
    // Update wind variation
    UpdateWindVariation(DeltaTime);
    
    // Update dust variation
    UpdateDustVariation(DeltaTime);
}

void AAtmosphericVFXController::BlendToTargetSettings(float DeltaTime)
{
    TransitionProgress = FMath::Clamp(TransitionProgress + DeltaTime * TransitionSpeed, 0.0f, 1.0f);
    
    // Lerp all settings
    CurrentSettings.DustIntensity = FMath::Lerp(CurrentSettings.DustIntensity, TargetSettings.DustIntensity, TransitionProgress);
    CurrentSettings.MistIntensity = FMath::Lerp(CurrentSettings.MistIntensity, TargetSettings.MistIntensity, TransitionProgress);
    CurrentSettings.PollenIntensity = FMath::Lerp(CurrentSettings.PollenIntensity, TargetSettings.PollenIntensity, TransitionProgress);
    CurrentSettings.WindStrength = FMath::Lerp(CurrentSettings.WindStrength, TargetSettings.WindStrength, TransitionProgress);
    CurrentSettings.LightShaftIntensity = FMath::Lerp(CurrentSettings.LightShaftIntensity, TargetSettings.LightShaftIntensity, TransitionProgress);
    CurrentSettings.AtmosphericTint = FLinearColor::LerpUsingHSV(CurrentSettings.AtmosphericTint, TargetSettings.AtmosphericTint, TransitionProgress);
    
    ApplySettingsToComponents();
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
        LightShafts->SetColorParameter(TEXT("Tint"), CurrentSettings.AtmosphericTint);
    }
}

void AAtmosphericVFXController::UpdateWindVariation(float DeltaTime)
{
    WindVariationTimer += DeltaTime;
    
    // Create subtle wind variation
    float WindVariation = FMath::Sin(WindVariationTimer * 0.5f) * CurrentSettings.WindVariation;
    FVector VariedWindDirection = CurrentSettings.WindDirection + FVector(WindVariation, WindVariation * 0.5f, 0.0f);
    VariedWindDirection.Normalize();
    
    if (DustParticles)
    {
        DustParticles->SetVectorParameter(TEXT("WindDirection"), VariedWindDirection);
    }
}

void AAtmosphericVFXController::UpdateDustVariation(float DeltaTime)
{
    DustVariationTimer += DeltaTime;
    
    // Create subtle dust intensity variation
    float DustVariation = 1.0f + FMath::Sin(DustVariationTimer * 0.3f) * 0.1f;
    float VariedDustIntensity = CurrentSettings.DustIntensity * DustVariation;
    
    if (DustParticles)
    {
        DustParticles->SetFloatParameter(TEXT("Intensity"), VariedDustIntensity);
    }
}

FAtmosphericSettings AAtmosphericVFXController::GetSettingsForMood(EAtmosphericMood Mood) const
{
    switch (Mood)
    {
        case EAtmosphericMood::Calm:
            return CalmSettings;
        case EAtmosphericMood::Tense:
            return TenseSettings;
        case EAtmosphericMood::Threatening:
            return ThreateningSettings;
        case EAtmosphericMood::Panic:
            return PanicSettings;
        default:
            return CalmSettings;
    }
}

void AAtmosphericVFXController::ModifySettingsForWeather(FAtmosphericSettings& Settings) const
{
    switch (CurrentWeather)
    {
        case EWeatherState::Clear:
            // No modifications needed
            break;
        case EWeatherState::Misty:
            Settings.MistIntensity *= 2.0f;
            Settings.LightShaftIntensity *= 0.7f;
            break;
        case EWeatherState::Overcast:
            Settings.LightShaftIntensity *= 0.4f;
            Settings.AtmosphericTint = FLinearColor::LerpUsingHSV(Settings.AtmosphericTint, FLinearColor(0.6f, 0.6f, 0.7f, 1.0f), 0.5f);
            break;
        case EWeatherState::Storm:
            Settings.WindStrength *= 2.0f;
            Settings.DustIntensity *= 1.5f;
            Settings.LightShaftIntensity *= 0.2f;
            break;
    }
}

TArray<AActor*> AAtmosphericVFXController::DetectNearbyThreats() const
{
    TArray<AActor*> Threats;
    
    // Find all actors with threat tags within detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->ActorHasTag(TEXT("Predator")))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= ThreatDetectionRadius)
            {
                Threats.Add(Actor);
            }
        }
    }
    
    return Threats;
}

float AAtmosphericVFXController::CalculateThreatLevel(const TArray<AActor*>& Threats) const
{
    if (Threats.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalThreat = 0.0f;
    
    for (AActor* Threat : Threats)
    {
        float Distance = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());
        float DistanceRatio = 1.0f - (Distance / ThreatDetectionRadius);
        
        // Base threat level
        float ThreatLevel = 0.5f;
        
        // Modify based on threat type
        if (Threat->ActorHasTag(TEXT("LargePredator")))
        {
            ThreatLevel = 1.0f;
        }
        else if (Threat->ActorHasTag(TEXT("SmallPredator")))
        {
            ThreatLevel = 0.3f;
        }
        
        TotalThreat += ThreatLevel * DistanceRatio;
    }
    
    return FMath::Clamp(TotalThreat, 0.0f, 1.0f);
}