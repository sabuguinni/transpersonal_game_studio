#include "AtmosphericVFXController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "VFXSystemManager.h"

AAtmosphericVFXController::AAtmosphericVFXController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize Niagara components
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
    
    // Cache VFX Manager reference
    if (UWorld* World = GetWorld())
    {
        VFXManager = World->GetSubsystem<UVFXSystemManager>();
    }
    
    // Set initial atmospheric state
    CurrentSettings = GetSettingsForMood(CurrentMood);
    TargetSettings = CurrentSettings;
    ApplySettingsToComponents();
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

void AAtmosphericVFXController::InitializeComponents()
{
    // Set auto-activate to false initially
    if (DustParticles)
    {
        DustParticles->SetAutoActivate(false);
    }
    
    if (MistParticles)
    {
        MistParticles->SetAutoActivate(false);
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetAutoActivate(false);
    }
    
    if (LightShafts)
    {
        LightShafts->SetAutoActivate(false);
    }
}

void AAtmosphericVFXController::InitializeDefaultSettings()
{
    // Calm settings - peaceful forest atmosphere
    CalmSettings.DustIntensity = 0.2f;
    CalmSettings.MistIntensity = 0.3f;
    CalmSettings.PollenIntensity = 0.4f;
    CalmSettings.WindDirection = FVector(1, 0.2f, 0);
    CalmSettings.WindStrength = 0.3f;
    CalmSettings.WindVariation = 0.2f;
    CalmSettings.LightShaftIntensity = 0.6f;
    CalmSettings.AtmosphericTint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    
    // Tense settings - something is watching
    TenseSettings.DustIntensity = 0.4f;
    TenseSettings.MistIntensity = 0.5f;
    TenseSettings.PollenIntensity = 0.1f;
    TenseSettings.WindDirection = FVector(0.8f, 0.6f, 0);
    TenseSettings.WindStrength = 0.6f;
    TenseSettings.WindVariation = 0.4f;
    TenseSettings.LightShaftIntensity = 0.3f;
    TenseSettings.AtmosphericTint = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    
    // Threatening settings - predator nearby
    ThreateningSettings.DustIntensity = 0.7f;
    ThreateningSettings.MistIntensity = 0.8f;
    ThreateningSettings.PollenIntensity = 0.0f;
    ThreateningSettings.WindDirection = FVector(0.5f, 0.8f, 0.2f);
    ThreateningSettings.WindStrength = 0.8f;
    ThreateningSettings.WindVariation = 0.6f;
    ThreateningSettings.LightShaftIntensity = 0.1f;
    ThreateningSettings.AtmosphericTint = FLinearColor(1.0f, 0.8f, 0.7f, 1.0f);
    
    // Panic settings - immediate danger
    PanicSettings.DustIntensity = 1.0f;
    PanicSettings.MistIntensity = 0.3f;
    PanicSettings.PollenIntensity = 0.0f;
    PanicSettings.WindDirection = FVector(0, 1, 0.3f);
    PanicSettings.WindStrength = 1.0f;
    PanicSettings.WindVariation = 0.8f;
    PanicSettings.LightShaftIntensity = 0.0f;
    PanicSettings.AtmosphericTint = FLinearColor(1.0f, 0.7f, 0.6f, 1.0f);
}

void AAtmosphericVFXController::SetAtmosphericMood(EAtmosphericMood NewMood, bool bInstant)
{
    if (CurrentMood == NewMood)
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
    // Temporarily intensify atmospheric effects based on threat proximity
    float Distance = FVector::Dist(GetActorLocation(), ThreatLocation);
    float ThreatInfluence = FMath::Clamp(1.0f - (Distance / ThreatDetectionRadius), 0.0f, 1.0f);
    ThreatInfluence *= Intensity * ThreatResponseIntensity;
    
    // Modify current settings temporarily
    FAtmosphericSettings ThreatSettings = CurrentSettings;
    ThreatSettings.DustIntensity = FMath::Clamp(ThreatSettings.DustIntensity + ThreatInfluence, 0.0f, 2.0f);
    ThreatSettings.WindStrength = FMath::Clamp(ThreatSettings.WindStrength + ThreatInfluence * 0.5f, 0.0f, 2.0f);
    ThreatSettings.WindVariation = FMath::Clamp(ThreatSettings.WindVariation + ThreatInfluence * 0.3f, 0.0f, 1.0f);
    
    // Apply threat-modified settings temporarily
    CurrentSettings = ThreatSettings;
    ApplySettingsToComponents();
}

void AAtmosphericVFXController::SetWindDirection(FVector NewDirection, float Strength)
{
    TargetSettings.WindDirection = NewDirection.GetSafeNormal();
    
    if (Strength >= 0.0f)
    {
        TargetSettings.WindStrength = FMath::Clamp(Strength, 0.0f, 2.0f);
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
        else if (ThreatLevel > 0.5f)
        {
            NewMood = EAtmosphericMood::Threatening;
        }
        else if (ThreatLevel > 0.2f)
        {
            NewMood = EAtmosphericMood::Tense;
        }
        
        SetAtmosphericMood(NewMood, false);
    }
    else if (CurrentMood != EAtmosphericMood::Calm)
    {
        // Gradually return to calm if no threats detected
        SetAtmosphericMood(EAtmosphericMood::Calm, false);
    }
}

bool AAtmosphericVFXController::IsPlayerInDanger() const
{
    return CurrentMood == EAtmosphericMood::Threatening || CurrentMood == EAtmosphericMood::Panic;
}

void AAtmosphericVFXController::UpdateAtmosphericEffects(float DeltaTime)
{
    // Update transition progress
    if (TransitionProgress < 1.0f)
    {
        TransitionProgress = FMath::Clamp(TransitionProgress + (DeltaTime * TransitionSpeed), 0.0f, 1.0f);
        BlendToTargetSettings(DeltaTime);
    }
    
    // Update variations
    UpdateWindVariation(DeltaTime);
    UpdateDustVariation(DeltaTime);
    
    // Apply current settings to components
    ApplySettingsToComponents();
}

void AAtmosphericVFXController::BlendToTargetSettings(float DeltaTime)
{
    float Alpha = TransitionProgress;
    
    // Smooth transition curve
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    
    CurrentSettings.DustIntensity = FMath::Lerp(CurrentSettings.DustIntensity, TargetSettings.DustIntensity, Alpha);
    CurrentSettings.MistIntensity = FMath::Lerp(CurrentSettings.MistIntensity, TargetSettings.MistIntensity, Alpha);
    CurrentSettings.PollenIntensity = FMath::Lerp(CurrentSettings.PollenIntensity, TargetSettings.PollenIntensity, Alpha);
    CurrentSettings.WindStrength = FMath::Lerp(CurrentSettings.WindStrength, TargetSettings.WindStrength, Alpha);
    CurrentSettings.WindVariation = FMath::Lerp(CurrentSettings.WindVariation, TargetSettings.WindVariation, Alpha);
    CurrentSettings.LightShaftIntensity = FMath::Lerp(CurrentSettings.LightShaftIntensity, TargetSettings.LightShaftIntensity, Alpha);
    
    CurrentSettings.WindDirection = FMath::Lerp(CurrentSettings.WindDirection, TargetSettings.WindDirection, Alpha);
    CurrentSettings.AtmosphericTint = FMath::Lerp(CurrentSettings.AtmosphericTint, TargetSettings.AtmosphericTint, Alpha);
}

void AAtmosphericVFXController::ApplySettingsToComponents()
{
    // Apply dust settings
    if (DustParticles && DustParticles->GetAsset())
    {
        DustParticles->SetFloatParameter(TEXT("Intensity"), CurrentSettings.DustIntensity);
        DustParticles->SetVectorParameter(TEXT("WindDirection"), CurrentSettings.WindDirection);
        DustParticles->SetFloatParameter(TEXT("WindStrength"), CurrentSettings.WindStrength);
        DustParticles->SetColorParameter(TEXT("Tint"), CurrentSettings.AtmosphericTint);
        
        if (CurrentSettings.DustIntensity > 0.01f && !DustParticles->IsActive())
        {
            DustParticles->Activate();
        }
        else if (CurrentSettings.DustIntensity <= 0.01f && DustParticles->IsActive())
        {
            DustParticles->Deactivate();
        }
    }
    
    // Apply mist settings
    if (MistParticles && MistParticles->GetAsset())
    {
        MistParticles->SetFloatParameter(TEXT("Intensity"), CurrentSettings.MistIntensity);
        MistParticles->SetVectorParameter(TEXT("WindDirection"), CurrentSettings.WindDirection * 0.5f);
        MistParticles->SetColorParameter(TEXT("Tint"), CurrentSettings.AtmosphericTint);
        
        if (CurrentSettings.MistIntensity > 0.01f && !MistParticles->IsActive())
        {
            MistParticles->Activate();
        }
        else if (CurrentSettings.MistIntensity <= 0.01f && MistParticles->IsActive())
        {
            MistParticles->Deactivate();
        }
    }
    
    // Apply pollen settings
    if (PollenParticles && PollenParticles->GetAsset())
    {
        PollenParticles->SetFloatParameter(TEXT("Intensity"), CurrentSettings.PollenIntensity);
        PollenParticles->SetVectorParameter(TEXT("WindDirection"), CurrentSettings.WindDirection * 0.3f);
        
        if (CurrentSettings.PollenIntensity > 0.01f && !PollenParticles->IsActive())
        {
            PollenParticles->Activate();
        }
        else if (CurrentSettings.PollenIntensity <= 0.01f && PollenParticles->IsActive())
        {
            PollenParticles->Deactivate();
        }
    }
    
    // Apply light shaft settings
    if (LightShafts && LightShafts->GetAsset())
    {
        LightShafts->SetFloatParameter(TEXT("Intensity"), CurrentSettings.LightShaftIntensity);
        LightShafts->SetColorParameter(TEXT("Tint"), CurrentSettings.AtmosphericTint);
        
        if (CurrentSettings.LightShaftIntensity > 0.01f && !LightShafts->IsActive())
        {
            LightShafts->Activate();
        }
        else if (CurrentSettings.LightShaftIntensity <= 0.01f && LightShafts->IsActive())
        {
            LightShafts->Deactivate();
        }
    }
}

void AAtmosphericVFXController::UpdateWindVariation(float DeltaTime)
{
    WindVariationTimer += DeltaTime;
    
    // Create subtle wind variation using sine waves
    float WindVariationMultiplier = 1.0f + (FMath::Sin(WindVariationTimer * 0.5f) * CurrentSettings.WindVariation * 0.5f);
    float WindDirectionVariation = FMath::Sin(WindVariationTimer * 0.3f) * CurrentSettings.WindVariation * 0.2f;
    
    FVector VariedWindDirection = CurrentSettings.WindDirection;
    VariedWindDirection.Y += WindDirectionVariation;
    VariedWindDirection = VariedWindDirection.GetSafeNormal();
    
    // Apply variations to active components
    if (DustParticles && DustParticles->IsActive())
    {
        DustParticles->SetFloatParameter(TEXT("WindStrength"), CurrentSettings.WindStrength * WindVariationMultiplier);
        DustParticles->SetVectorParameter(TEXT("WindDirection"), VariedWindDirection);
    }
}

void AAtmosphericVFXController::UpdateDustVariation(float DeltaTime)
{
    DustVariationTimer += DeltaTime * 0.7f;
    
    // Create subtle dust intensity variation
    float DustVariationMultiplier = 1.0f + (FMath::Sin(DustVariationTimer) * 0.2f);
    
    if (DustParticles && DustParticles->IsActive())
    {
        float VariedDustIntensity = CurrentSettings.DustIntensity * DustVariationMultiplier;
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
            Settings.LightShaftIntensity *= 0.5f;
            Settings.AtmosphericTint = FMath::Lerp(Settings.AtmosphericTint, FLinearColor(0.8f, 0.9f, 1.0f, 1.0f), 0.5f);
            break;
            
        case EWeatherState::Overcast:
            Settings.LightShaftIntensity *= 0.2f;
            Settings.DustIntensity *= 0.7f;
            Settings.AtmosphericTint = FMath::Lerp(Settings.AtmosphericTint, FLinearColor(0.7f, 0.7f, 0.8f, 1.0f), 0.7f);
            break;
            
        case EWeatherState::Storm:
            Settings.WindStrength *= 2.0f;
            Settings.WindVariation *= 1.5f;
            Settings.DustIntensity *= 1.5f;
            Settings.MistIntensity *= 0.3f;
            Settings.LightShaftIntensity = 0.0f;
            Settings.AtmosphericTint = FMath::Lerp(Settings.AtmosphericTint, FLinearColor(0.5f, 0.6f, 0.7f, 1.0f), 0.8f);
            break;
    }
}

TArray<AActor*> AAtmosphericVFXController::DetectNearbyThreats() const
{
    TArray<AActor*> Threats;
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsWithTag(World, FName("Predator"), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor && FVector::Dist(GetActorLocation(), Actor->GetActorLocation()) <= ThreatDetectionRadius)
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
    
    float TotalThreatLevel = 0.0f;
    FVector MyLocation = GetActorLocation();
    
    for (AActor* Threat : Threats)
    {
        if (!Threat)
        {
            continue;
        }
        
        float Distance = FVector::Dist(MyLocation, Threat->GetActorLocation());
        float DistanceInfluence = 1.0f - FMath::Clamp(Distance / ThreatDetectionRadius, 0.0f, 1.0f);
        
        // Base threat level (could be modified by creature size, aggression, etc.)
        float BaseThreatLevel = 1.0f;
        
        TotalThreatLevel += BaseThreatLevel * DistanceInfluence;
    }
    
    return FMath::Clamp(TotalThreatLevel, 0.0f, 1.0f);
}