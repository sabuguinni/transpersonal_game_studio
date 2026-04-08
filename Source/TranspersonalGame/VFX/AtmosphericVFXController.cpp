#include "AtmosphericVFXController.h"
#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AAtmosphericVFXController::AAtmosphericVFXController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize Niagara components
    DustParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustParticles"));
    DustParticles->SetupAttachment(RootComponent);
    DustParticles->bAutoActivate = false;
    
    MistParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistParticles"));
    MistParticles->SetupAttachment(RootComponent);
    MistParticles->bAutoActivate = false;
    
    PollenParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PollenParticles"));
    PollenParticles->SetupAttachment(RootComponent);
    PollenParticles->bAutoActivate = false;
    
    LightShafts = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightShafts"));
    LightShafts->SetupAttachment(RootComponent);
    LightShafts->bAutoActivate = false;
    
    // Set default values
    CurrentMood = EAtmosphericMood::Calm;
    CurrentWeather = EWeatherState::Clear;
    TransitionSpeed = 2.0f;
    ThreatDetectionRadius = 2000.0f;
    ThreatResponseIntensity = 1.5f;
    bAutoDetectThreats = true;
    
    // Initialize settings
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
    // Load default Niagara systems - these would be set in Blueprint or loaded from assets
    // For now, we'll set them up to be configured in the editor
    
    if (DustParticles)
    {
        DustParticles->SetVariableFloat(TEXT("Intensity"), 0.3f);
        DustParticles->SetVariableLinearColor(TEXT("Color"), FLinearColor::White);
    }
    
    if (MistParticles)
    {
        MistParticles->SetVariableFloat(TEXT("Intensity"), 0.2f);
        MistParticles->SetVariableLinearColor(TEXT("Color"), FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetVariableFloat(TEXT("Intensity"), 0.1f);
        PollenParticles->SetVariableLinearColor(TEXT("Color"), FLinearColor(1.0f, 1.0f, 0.8f, 0.8f));
    }
    
    if (LightShafts)
    {
        LightShafts->SetVariableFloat(TEXT("Intensity"), 0.4f);
        LightShafts->SetVariableLinearColor(TEXT("Color"), FLinearColor::White);
    }
}

void AAtmosphericVFXController::UpdateAtmosphericEffects(float DeltaTime)
{
    // Blend to target settings
    BlendToTargetSettings(DeltaTime);
    
    // Update wind and dust variations
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
        
        // Blend atmospheric settings
        CurrentSettings.DustIntensity = FMath::Lerp(CurrentSettings.DustIntensity, TargetSettings.DustIntensity, TransitionProgress);
        CurrentSettings.MistIntensity = FMath::Lerp(CurrentSettings.MistIntensity, TargetSettings.MistIntensity, TransitionProgress);
        CurrentSettings.PollenIntensity = FMath::Lerp(CurrentSettings.PollenIntensity, TargetSettings.PollenIntensity, TransitionProgress);
        CurrentSettings.WindStrength = FMath::Lerp(CurrentSettings.WindStrength, TargetSettings.WindStrength, TransitionProgress);
        CurrentSettings.WindVariation = FMath::Lerp(CurrentSettings.WindVariation, TargetSettings.WindVariation, TransitionProgress);
        CurrentSettings.LightShaftIntensity = FMath::Lerp(CurrentSettings.LightShaftIntensity, TargetSettings.LightShaftIntensity, TransitionProgress);
        CurrentSettings.AtmosphericTint = FLinearColor::LerpUsingHSV(CurrentSettings.AtmosphericTint, TargetSettings.AtmosphericTint, TransitionProgress);
        CurrentSettings.WindDirection = FMath::Lerp(CurrentSettings.WindDirection, TargetSettings.WindDirection, TransitionProgress);
    }
}

void AAtmosphericVFXController::ApplySettingsToComponents()
{
    if (DustParticles)
    {
        DustParticles->SetVariableFloat(TEXT("Intensity"), CurrentSettings.DustIntensity);
        DustParticles->SetVariableVector(TEXT("WindDirection"), CurrentSettings.WindDirection);
        DustParticles->SetVariableFloat(TEXT("WindStrength"), CurrentSettings.WindStrength);
        DustParticles->SetVariableLinearColor(TEXT("Tint"), CurrentSettings.AtmosphericTint);
    }
    
    if (MistParticles)
    {
        MistParticles->SetVariableFloat(TEXT("Intensity"), CurrentSettings.MistIntensity);
        MistParticles->SetVariableVector(TEXT("WindDirection"), CurrentSettings.WindDirection);
        MistParticles->SetVariableLinearColor(TEXT("Tint"), CurrentSettings.AtmosphericTint);
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetVariableFloat(TEXT("Intensity"), CurrentSettings.PollenIntensity);
        PollenParticles->SetVariableVector(TEXT("WindDirection"), CurrentSettings.WindDirection);
        PollenParticles->SetVariableFloat(TEXT("WindStrength"), CurrentSettings.WindStrength * 0.5f); // Pollen is lighter
    }
    
    if (LightShafts)
    {
        LightShafts->SetVariableFloat(TEXT("Intensity"), CurrentSettings.LightShaftIntensity);
        LightShafts->SetVariableLinearColor(TEXT("Color"), CurrentSettings.AtmosphericTint);
    }
}

void AAtmosphericVFXController::UpdateWindVariation(float DeltaTime)
{
    WindVariationTimer += DeltaTime;
    
    // Create subtle wind variations using sine waves
    float WindVariationMultiplier = 1.0f + (FMath::Sin(WindVariationTimer * 0.5f) * CurrentSettings.WindVariation);
    float CurrentWindStrength = CurrentSettings.WindStrength * WindVariationMultiplier;
    
    // Apply wind variations to dust and pollen
    if (DustParticles)
    {
        DustParticles->SetVariableFloat(TEXT("WindStrength"), CurrentWindStrength);
    }
    
    if (PollenParticles)
    {
        PollenParticles->SetVariableFloat(TEXT("WindStrength"), CurrentWindStrength * 0.5f);
    }
}

void AAtmosphericVFXController::UpdateDustVariation(float DeltaTime)
{
    DustVariationTimer += DeltaTime;
    
    // Create dust intensity variations
    float DustVariationMultiplier = 1.0f + (FMath::Sin(DustVariationTimer * 0.3f) * 0.2f);
    float CurrentDustIntensity = CurrentSettings.DustIntensity * DustVariationMultiplier;
    
    if (DustParticles)
    {
        DustParticles->SetVariableFloat(TEXT("Intensity"), CurrentDustIntensity);
    }
}

void AAtmosphericVFXController::SetAtmosphericMood(EAtmosphericMood NewMood, bool bInstant)
{
    if (CurrentMood != NewMood)
    {
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
    }
}

void AAtmosphericVFXController::SetWeatherState(EWeatherState NewWeather, bool bInstant)
{
    if (CurrentWeather != NewWeather)
    {
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
    }
}

void AAtmosphericVFXController::TriggerThreatResponse(FVector ThreatLocation, float Intensity)
{
    // Temporarily increase atmospheric tension
    FAtmosphericSettings ThreatSettings = CurrentSettings;
    
    // Enhance dust and reduce visibility slightly
    ThreatSettings.DustIntensity *= (1.0f + Intensity * ThreatResponseIntensity);
    ThreatSettings.MistIntensity *= (1.0f + Intensity * 0.5f);
    
    // Darken atmospheric tint
    ThreatSettings.AtmosphericTint = FLinearColor::LerpUsingHSV(
        ThreatSettings.AtmosphericTint, 
        FLinearColor(0.8f, 0.7f, 0.6f, 1.0f), 
        Intensity * 0.3f
    );
    
    // Apply threat settings temporarily
    TargetSettings = ThreatSettings;
    TransitionProgress = 0.0f;
}

void AAtmosphericVFXController::SetWindDirection(FVector NewDirection, float Strength)
{
    TargetSettings.WindDirection = NewDirection.GetSafeNormal();
    
    if (Strength >= 0.0f)
    {
        TargetSettings.WindStrength = Strength;
    }
    
    TransitionProgress = 0.0f;
}

void AAtmosphericVFXController::UpdateThreatDetection()
{
    TArray<AActor*> NearbyThreats = DetectNearbyThreats();
    float ThreatLevel = CalculateThreatLevel(NearbyThreats);
    
    if (ThreatLevel > 0.1f)
    {
        EAtmosphericMood ThreatMood = EAtmosphericMood::Calm;
        
        if (ThreatLevel > 0.8f)
        {
            ThreatMood = EAtmosphericMood::Panic;
        }
        else if (ThreatLevel > 0.5f)
        {
            ThreatMood = EAtmosphericMood::Threatening;
        }
        else if (ThreatLevel > 0.2f)
        {
            ThreatMood = EAtmosphericMood::Tense;
        }
        
        SetAtmosphericMood(ThreatMood, false);
    }
}

bool AAtmosphericVFXController::IsPlayerInDanger() const
{
    TArray<AActor*> NearbyThreats = DetectNearbyThreats();
    return CalculateThreatLevel(NearbyThreats) > 0.3f;
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
            Settings.LightShaftIntensity *= 0.3f;
            Settings.AtmosphericTint = FLinearColor::LerpUsingHSV(Settings.AtmosphericTint, FLinearColor(0.7f, 0.7f, 0.8f, 1.0f), 0.5f);
            break;
            
        case EWeatherState::Storm:
            Settings.DustIntensity *= 3.0f;
            Settings.WindStrength *= 2.5f;
            Settings.WindVariation *= 2.0f;
            Settings.LightShaftIntensity *= 0.1f;
            Settings.AtmosphericTint = FLinearColor::LerpUsingHSV(Settings.AtmosphericTint, FLinearColor(0.5f, 0.5f, 0.6f, 1.0f), 0.7f);
            break;
    }
}

TArray<AActor*> AAtmosphericVFXController::DetectNearbyThreats() const
{
    TArray<AActor*> Threats;
    
    if (UWorld* World = GetWorld())
    {
        // Get player location
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                
                // Find all actors with "Predator" or "Threat" tags within detection radius
                TArray<AActor*> AllActors;
                UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
                
                for (AActor* Actor : AllActors)
                {
                    if (Actor && Actor != PlayerPawn)
                    {
                        // Check if actor has threat tags
                        if (Actor->Tags.Contains(TEXT("Predator")) || 
                            Actor->Tags.Contains(TEXT("Threat")) ||
                            Actor->Tags.Contains(TEXT("Carnivore")))
                        {
                            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                            if (Distance <= ThreatDetectionRadius)
                            {
                                Threats.Add(Actor);
                            }
                        }
                    }
                }
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
    
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                
                for (AActor* Threat : Threats)
                {
                    if (Threat)
                    {
                        float Distance = FVector::Dist(PlayerLocation, Threat->GetActorLocation());
                        float ProximityThreat = 1.0f - (Distance / ThreatDetectionRadius);
                        
                        // Scale threat based on creature size/type
                        float SizeThreat = 1.0f;
                        if (Threat->Tags.Contains(TEXT("Large")))
                        {
                            SizeThreat = 2.0f;
                        }
                        else if (Threat->Tags.Contains(TEXT("Massive")))
                        {
                            SizeThreat = 3.0f;
                        }
                        
                        TotalThreat += ProximityThreat * SizeThreat;
                    }
                }
            }
        }
    }
    
    return FMath::Clamp(TotalThreat / 5.0f, 0.0f, 1.0f); // Normalize to 0-1 range
}

void AAtmosphericVFXController::InitializeDefaultSettings()
{
    // Calm settings
    CalmSettings.DustIntensity = 0.2f;
    CalmSettings.MistIntensity = 0.1f;
    CalmSettings.PollenIntensity = 0.15f;
    CalmSettings.WindDirection = FVector(1, 0, 0);
    CalmSettings.WindStrength = 0.3f;
    CalmSettings.WindVariation = 0.2f;
    CalmSettings.LightShaftIntensity = 0.6f;
    CalmSettings.AtmosphericTint = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    
    // Tense settings
    TenseSettings.DustIntensity = 0.4f;
    TenseSettings.MistIntensity = 0.2f;
    TenseSettings.PollenIntensity = 0.05f;
    TenseSettings.WindDirection = FVector(1, 0, 0);
    TenseSettings.WindStrength = 0.5f;
    TenseSettings.WindVariation = 0.4f;
    TenseSettings.LightShaftIntensity = 0.4f;
    TenseSettings.AtmosphericTint = FLinearColor(0.9f, 0.85f, 0.8f, 1.0f);
    
    // Threatening settings
    ThreateningSettings.DustIntensity = 0.7f;
    ThreateningSettings.MistIntensity = 0.4f;
    ThreateningSettings.PollenIntensity = 0.0f;
    ThreateningSettings.WindDirection = FVector(1, 0, 0);
    ThreateningSettings.WindStrength = 0.8f;
    ThreateningSettings.WindVariation = 0.6f;
    ThreateningSettings.LightShaftIntensity = 0.2f;
    ThreateningSettings.AtmosphericTint = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    
    // Panic settings
    PanicSettings.DustIntensity = 1.0f;
    PanicSettings.MistIntensity = 0.6f;
    PanicSettings.PollenIntensity = 0.0f;
    PanicSettings.WindDirection = FVector(1, 0, 0);
    PanicSettings.WindStrength = 1.2f;
    PanicSettings.WindVariation = 0.8f;
    PanicSettings.LightShaftIntensity = 0.1f;
    PanicSettings.AtmosphericTint = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
}