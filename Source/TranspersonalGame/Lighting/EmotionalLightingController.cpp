#include "EmotionalLightingController.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEmotionalLightingController::UEmotionalLightingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / UpdateFrequency;
    
    // Initialize default emotional profiles
    InitializeDefaultProfiles();
}

void UEmotionalLightingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find lighting components in the world if not assigned
    if (!SunLight)
    {
        SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass()));
    }
    
    if (!SkyLight)
    {
        SkyLight = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(GetWorld(), ASkyLight::StaticClass()));
    }
    
    if (!HeightFog)
    {
        HeightFog = Cast<AExponentialHeightFog>(UGameplayStatics::GetActorOfClass(GetWorld(), AExponentialHeightFog::StaticClass()));
    }
    
    if (!PostProcessVolume)
    {
        PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass()));
    }
    
    // Set initial state
    CurrentProfile = EmotionalProfiles[CurrentEmotionalState];
    TargetProfile = CurrentProfile;
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalLightingController: Initialized with state %d"), (int32)CurrentEmotionalState);
}

void UEmotionalLightingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Distance-based update optimization
    if (bUseDistanceBasedUpdates)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
            if (Distance > MaxUpdateDistance)
            {
                return; // Skip update if too far
            }
        }
    }
    
    // Blend current profile towards target
    BlendToTargetProfile(DeltaTime);
    
    // Apply dynamic effects
    ApplyDynamicEffects(DeltaTime);
    
    // Apply the current profile to lighting components
    ApplyLightingProfile(CurrentProfile);
}

void UEmotionalLightingController::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (NewState != CurrentEmotionalState)
    {
        CurrentEmotionalState = NewState;
        
        if (EmotionalProfiles.Contains(NewState))
        {
            TargetProfile = EmotionalProfiles[NewState];
            TransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
            
            UE_LOG(LogTemp, Log, TEXT("EmotionalLightingController: Transitioning to emotional state %d"), (int32)NewState);
            
            // Broadcast state change event
            OnEmotionalStateChanged.Broadcast(NewState, TransitionTime);
        }
    }
}

void UEmotionalLightingController::SetPredatorProximity(EPredatorProximity Proximity, float TransitionTime)
{
    if (Proximity != PredatorProximity)
    {
        PredatorProximity = Proximity;
        
        if (PredatorProfiles.Contains(Proximity))
        {
            // Blend predator profile with current emotional profile
            FEmotionalLightingProfile BlendedProfile = BlendProfiles(
                EmotionalProfiles[CurrentEmotionalState],
                PredatorProfiles[Proximity],
                0.7f // Predator influence weight
            );
            
            TargetProfile = BlendedProfile;
            TransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
            
            UE_LOG(LogTemp, Log, TEXT("EmotionalLightingController: Predator proximity changed to %d"), (int32)Proximity);
            
            // Update tension level based on proximity
            float NewTensionLevel = BaseTensionLevel;
            switch (Proximity)
            {
                case EPredatorProximity::Safe:
                    NewTensionLevel = BaseTensionLevel;
                    break;
                case EPredatorProximity::Distant:
                    NewTensionLevel = BaseTensionLevel + 0.2f;
                    break;
                case EPredatorProximity::Near:
                    NewTensionLevel = BaseTensionLevel + 0.5f;
                    break;
                case EPredatorProximity::Stalking:
                    NewTensionLevel = BaseTensionLevel + 0.8f;
                    break;
                case EPredatorProximity::Imminent:
                    NewTensionLevel = MaxTensionLevel;
                    break;
            }
            
            CurrentTensionLevel = FMath::Clamp(NewTensionLevel, 0.0f, MaxTensionLevel);
        }
    }
}

void UEmotionalLightingController::ModifyTensionLevel(float TensionDelta, float TransitionTime)
{
    float NewTensionLevel = FMath::Clamp(CurrentTensionLevel + TensionDelta, 0.0f, MaxTensionLevel);
    
    if (FMath::Abs(NewTensionLevel - CurrentTensionLevel) > 0.01f)
    {
        CurrentTensionLevel = NewTensionLevel;
        
        // Modify current profile based on tension
        ModifyProfileForTension(TargetProfile, CurrentTensionLevel);
        TransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
        
        UE_LOG(LogTemp, Log, TEXT("EmotionalLightingController: Tension level modified to %f"), CurrentTensionLevel);
    }
}

void UEmotionalLightingController::TriggerTensionSpike(float Intensity, float Duration)
{
    // Create temporary high-tension profile
    FEmotionalLightingProfile SpikeProfile = CurrentProfile;
    
    // Amplify tension effects
    SpikeProfile.ShadowIntensityMultiplier *= (1.0f + Intensity);
    SpikeProfile.ShadowContrastBoost += Intensity * 0.5f;
    SpikeProfile.VisibilityReduction += Intensity * 0.3f;
    SpikeProfile.ContrastMultiplier *= (1.0f + Intensity * 0.5f);
    SpikeProfile.TensionPulseIntensity = Intensity;
    SpikeProfile.TensionPulseSpeed = 2.0f; // Fast heartbeat
    
    // Apply spike profile temporarily
    TargetProfile = SpikeProfile;
    TransitionSpeed = 5.0f; // Fast transition in
    
    // Schedule return to normal after duration
    GetWorld()->GetTimerManager().SetTimer(
        TensionSpikeTimer,
        [this]()
        {
            // Return to base emotional state
            TargetProfile = EmotionalProfiles[CurrentEmotionalState];
            TransitionSpeed = 0.5f; // Slower transition out
        },
        Duration,
        false
    );
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalLightingController: Tension spike triggered with intensity %f"), Intensity);
}

void UEmotionalLightingController::ApplyEmotionalProfile(const FEmotionalLightingProfile& Profile, float BlendWeight)
{
    if (BlendWeight >= 1.0f)
    {
        TargetProfile = Profile;
    }
    else
    {
        TargetProfile = BlendProfiles(CurrentProfile, Profile, BlendWeight);
    }
    
    TransitionSpeed = Profile.TransitionSpeed;
}

float UEmotionalLightingController::GetVisibilityFactor() const
{
    // Return visibility factor for AI systems
    return 1.0f - CurrentProfile.VisibilityReduction;
}

void UEmotionalLightingController::ResetToBaseline(float TransitionTime)
{
    CurrentEmotionalState = EEmotionalState::Wonder;
    PredatorProximity = EPredatorProximity::Safe;
    CurrentTensionLevel = BaseTensionLevel;
    
    TargetProfile = EmotionalProfiles[EEmotionalState::Wonder];
    TransitionSpeed = 1.0f / FMath::Max(TransitionTime, 0.1f);
    
    UE_LOG(LogTemp, Log, TEXT("EmotionalLightingController: Reset to baseline"));
}

void UEmotionalLightingController::InitializeDefaultProfiles()
{
    // Wonder state - bright, warm, inviting
    FEmotionalLightingProfile WonderProfile;
    WonderProfile.ProfileName = TEXT("Wonder");
    WonderProfile.SunIntensityMultiplier = 1.2f;
    WonderProfile.SkyLightMultiplier = 1.1f;
    WonderProfile.ColorTint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    WonderProfile.SaturationMultiplier = 1.1f;
    WonderProfile.TemperatureShift = 0.2f; // Warmer
    EmotionalProfiles.Add(EEmotionalState::Wonder, WonderProfile);
    
    // Isolation state - desaturated, cooler
    FEmotionalLightingProfile IsolationProfile;
    IsolationProfile.ProfileName = TEXT("Isolation");
    IsolationProfile.SkyLightMultiplier = 0.8f;
    IsolationProfile.SaturationMultiplier = 0.7f;
    IsolationProfile.TemperatureShift = -0.3f; // Cooler
    IsolationProfile.ColorTint = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    IsolationProfile.AmbientOcclusionStrength = 1.3f;
    EmotionalProfiles.Add(EEmotionalState::Isolation, IsolationProfile);
    
    // Tension state - increased contrast, sharper shadows
    FEmotionalLightingProfile TensionProfile;
    TensionProfile.ProfileName = TEXT("Tension");
    TensionProfile.ShadowIntensityMultiplier = 1.3f;
    TensionProfile.ShadowContrastBoost = 0.3f;
    TensionProfile.ContrastMultiplier = 1.2f;
    TensionProfile.VisibilityReduction = 0.1f;
    TensionProfile.LightFlickerIntensity = 0.1f;
    TensionProfile.TensionPulseIntensity = 0.2f;
    EmotionalProfiles.Add(EEmotionalState::Tension, TensionProfile);
    
    // Fear state - dark, high contrast, reduced visibility
    FEmotionalLightingProfile FearProfile;
    FearProfile.ProfileName = TEXT("Fear");
    FearProfile.SunIntensityMultiplier = 0.7f;
    FearProfile.SkyLightMultiplier = 0.6f;
    FearProfile.ShadowIntensityMultiplier = 1.5f;
    FearProfile.ShadowContrastBoost = 0.5f;
    FearProfile.VisibilityReduction = 0.3f;
    FearProfile.FogDensityMultiplier = 1.5f;
    FearProfile.ContrastMultiplier = 1.4f;
    FearProfile.SaturationMultiplier = 0.8f;
    FearProfile.TemperatureShift = -0.2f;
    FearProfile.LightFlickerIntensity = 0.2f;
    FearProfile.TensionPulseIntensity = 0.4f;
    FearProfile.TensionPulseSpeed = 1.5f;
    EmotionalProfiles.Add(EEmotionalState::Fear, FearProfile);
    
    // Terror state - maximum darkness and chaos
    FEmotionalLightingProfile TerrorProfile;
    TerrorProfile.ProfileName = TEXT("Terror");
    TerrorProfile.SunIntensityMultiplier = 0.5f;
    TerrorProfile.SkyLightMultiplier = 0.4f;
    TerrorProfile.ShadowIntensityMultiplier = 2.0f;
    TerrorProfile.ShadowContrastBoost = 0.8f;
    TerrorProfile.VisibilityReduction = 0.5f;
    TerrorProfile.FogDensityMultiplier = 2.0f;
    TerrorProfile.ContrastMultiplier = 1.6f;
    TerrorProfile.SaturationMultiplier = 0.6f;
    TerrorProfile.TemperatureShift = -0.4f;
    TerrorProfile.ColorTint = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
    TerrorProfile.LightFlickerIntensity = 0.4f;
    TerrorProfile.TensionPulseIntensity = 0.8f;
    TerrorProfile.TensionPulseSpeed = 2.0f;
    EmotionalProfiles.Add(EEmotionalState::Terror, TerrorProfile);
    
    // Relief state - soft, warm, peaceful
    FEmotionalLightingProfile ReliefProfile;
    ReliefProfile.ProfileName = TEXT("Relief");
    ReliefProfile.SunIntensityMultiplier = 1.1f;
    ReliefProfile.SkyLightMultiplier = 1.2f;
    ReliefProfile.ShadowIntensityMultiplier = 0.8f;
    ReliefProfile.TemperatureShift = 0.3f;
    ReliefProfile.ColorTint = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    ReliefProfile.SaturationMultiplier = 1.0f;
    ReliefProfile.ContrastMultiplier = 0.9f;
    EmotionalProfiles.Add(EEmotionalState::Relief, ReliefProfile);
    
    // Despair state - muted, grey, hopeless
    FEmotionalLightingProfile DespairProfile;
    DespairProfile.ProfileName = TEXT("Despair");
    DespairProfile.SunIntensityMultiplier = 0.6f;
    DespairProfile.SkyLightMultiplier = 0.7f;
    DespairProfile.SaturationMultiplier = 0.5f;
    DespairProfile.ContrastMultiplier = 0.8f;
    DespairProfile.ColorTint = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    DespairProfile.VisibilityReduction = 0.2f;
    DespairProfile.FogDensityMultiplier = 1.3f;
    DespairProfile.AmbientOcclusionStrength = 1.4f;
    EmotionalProfiles.Add(EEmotionalState::Despair, DespairProfile);
    
    // Initialize predator proximity profiles
    InitializePredatorProfiles();
}

void UEmotionalLightingController::InitializePredatorProfiles()
{
    // Safe - normal lighting
    FEmotionalLightingProfile SafeProfile;
    SafeProfile.ProfileName = TEXT("Safe");
    PredatorProfiles.Add(EPredatorProximity::Safe, SafeProfile);
    
    // Distant - slight tension
    FEmotionalLightingProfile DistantProfile;
    DistantProfile.ProfileName = TEXT("Distant Threat");
    DistantProfile.ShadowIntensityMultiplier = 1.1f;
    DistantProfile.LightFlickerIntensity = 0.05f;
    PredatorProfiles.Add(EPredatorProximity::Distant, DistantProfile);
    
    // Near - increased shadows and contrast
    FEmotionalLightingProfile NearProfile;
    NearProfile.ProfileName = TEXT("Near Threat");
    NearProfile.ShadowIntensityMultiplier = 1.3f;
    NearProfile.ShadowContrastBoost = 0.2f;
    NearProfile.VisibilityReduction = 0.1f;
    NearProfile.LightFlickerIntensity = 0.1f;
    NearProfile.TensionPulseIntensity = 0.2f;
    PredatorProfiles.Add(EPredatorProximity::Near, NearProfile);
    
    // Stalking - dramatic shadows, reduced visibility
    FEmotionalLightingProfile StalkingProfile;
    StalkingProfile.ProfileName = TEXT("Stalking");
    StalkingProfile.SunIntensityMultiplier = 0.8f;
    StalkingProfile.ShadowIntensityMultiplier = 1.6f;
    StalkingProfile.ShadowContrastBoost = 0.4f;
    StalkingProfile.VisibilityReduction = 0.25f;
    StalkingProfile.FogDensityMultiplier = 1.3f;
    StalkingProfile.LightFlickerIntensity = 0.2f;
    StalkingProfile.TensionPulseIntensity = 0.5f;
    StalkingProfile.TensionPulseSpeed = 1.3f;
    PredatorProfiles.Add(EPredatorProximity::Stalking, StalkingProfile);
    
    // Imminent - maximum threat lighting
    FEmotionalLightingProfile ImminentProfile;
    ImminentProfile.ProfileName = TEXT("Imminent Attack");
    ImminentProfile.SunIntensityMultiplier = 0.6f;
    ImminentProfile.SkyLightMultiplier = 0.7f;
    ImminentProfile.ShadowIntensityMultiplier = 2.0f;
    ImminentProfile.ShadowContrastBoost = 0.7f;
    ImminentProfile.VisibilityReduction = 0.4f;
    ImminentProfile.FogDensityMultiplier = 1.8f;
    ImminentProfile.ContrastMultiplier = 1.5f;
    ImminentProfile.SaturationMultiplier = 0.7f;
    ImminentProfile.TemperatureShift = -0.3f;
    ImminentProfile.LightFlickerIntensity = 0.3f;
    ImminentProfile.TensionPulseIntensity = 0.8f;
    ImminentProfile.TensionPulseSpeed = 2.0f;
    PredatorProfiles.Add(EPredatorProximity::Imminent, ImminentProfile);
}

void UEmotionalLightingController::BlendToTargetProfile(float DeltaTime)
{
    float BlendSpeed = TransitionSpeed * DeltaTime;
    
    // Blend all profile properties
    CurrentProfile.TensionLevel = FMath::FInterpTo(CurrentProfile.TensionLevel, TargetProfile.TensionLevel, DeltaTime, BlendSpeed);
    CurrentProfile.ShadowIntensityMultiplier = FMath::FInterpTo(CurrentProfile.ShadowIntensityMultiplier, TargetProfile.ShadowIntensityMultiplier, DeltaTime, BlendSpeed);
    CurrentProfile.ShadowContrastBoost = FMath::FInterpTo(CurrentProfile.ShadowContrastBoost, TargetProfile.ShadowContrastBoost, DeltaTime, BlendSpeed);
    CurrentProfile.VisibilityReduction = FMath::FInterpTo(CurrentProfile.VisibilityReduction, TargetProfile.VisibilityReduction, DeltaTime, BlendSpeed);
    CurrentProfile.FogDensityMultiplier = FMath::FInterpTo(CurrentProfile.FogDensityMultiplier, TargetProfile.FogDensityMultiplier, DeltaTime, BlendSpeed);
    CurrentProfile.SaturationMultiplier = FMath::FInterpTo(CurrentProfile.SaturationMultiplier, TargetProfile.SaturationMultiplier, DeltaTime, BlendSpeed);
    CurrentProfile.ContrastMultiplier = FMath::FInterpTo(CurrentProfile.ContrastMultiplier, TargetProfile.ContrastMultiplier, DeltaTime, BlendSpeed);
    CurrentProfile.SunIntensityMultiplier = FMath::FInterpTo(CurrentProfile.SunIntensityMultiplier, TargetProfile.SunIntensityMultiplier, DeltaTime, BlendSpeed);
    CurrentProfile.SkyLightMultiplier = FMath::FInterpTo(CurrentProfile.SkyLightMultiplier, TargetProfile.SkyLightMultiplier, DeltaTime, BlendSpeed);
    CurrentProfile.TemperatureShift = FMath::FInterpTo(CurrentProfile.TemperatureShift, TargetProfile.TemperatureShift, DeltaTime, BlendSpeed);
    
    // Blend color properties
    CurrentProfile.ColorTint = FMath::CInterpTo(CurrentProfile.ColorTint, TargetProfile.ColorTint, DeltaTime, BlendSpeed);
    CurrentProfile.FogTintShift = FMath::CInterpTo(CurrentProfile.FogTintShift, TargetProfile.FogTintShift, DeltaTime, BlendSpeed);
}

void UEmotionalLightingController::ApplyDynamicEffects(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Apply light flicker
    if (bEnableLightFlicker && CurrentProfile.LightFlickerIntensity > 0.0f)
    {
        float FlickerNoise = FMath::PerlinNoise1D(CurrentTime * CurrentProfile.LightFlickerSpeed * FlickerNoiseScale);
        LightFlickerValue = FlickerNoise * CurrentProfile.LightFlickerIntensity;
    }
    else
    {
        LightFlickerValue = 0.0f;
    }
    
    // Apply tension pulse
    if (bEnableTensionPulse && CurrentProfile.TensionPulseIntensity > 0.0f)
    {
        float PulseNoise = FMath::Sin(CurrentTime * CurrentProfile.TensionPulseSpeed * PulseNoiseScale) * 0.5f + 0.5f;
        TensionPulseValue = PulseNoise * CurrentProfile.TensionPulseIntensity;
    }
    else
    {
        TensionPulseValue = 0.0f;
    }
}

void UEmotionalLightingController::ApplyLightingProfile(const FEmotionalLightingProfile& Profile)
{
    // Apply to sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        float FinalIntensity = SunComponent->Intensity * Profile.SunIntensityMultiplier * (1.0f + LightFlickerValue);
        SunComponent->SetIntensity(FinalIntensity);
    }
    
    // Apply to sky light
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLight->GetLightComponent();
        float FinalIntensity = SkyComponent->Intensity * Profile.SkyLightMultiplier;
        SkyComponent->SetIntensity(FinalIntensity);
    }
    
    // Apply to height fog
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
        float BaseDensity = FogComponent->FogDensity;
        FogComponent->SetFogDensity(BaseDensity * Profile.FogDensityMultiplier);
        
        FLinearColor FogColor = FogComponent->FogInscatteringColor;
        FLinearColor ModifiedFogColor = FogColor * Profile.FogTintShift;
        FogComponent->SetFogInscatteringColor(ModifiedFogColor);
    }
    
    // Apply to post process volume
    if (PostProcessVolume)
    {
        FPostProcessSettings& Settings = PostProcessVolume->Settings;
        
        // Color grading
        Settings.ColorSaturation = FVector4(Profile.SaturationMultiplier);
        Settings.ColorContrast = FVector4(Profile.ContrastMultiplier * (1.0f + TensionPulseValue * 0.1f));
        Settings.ColorGamma = FVector4(1.0f);
        Settings.ColorGain = FVector4(Profile.ColorTint.R, Profile.ColorTint.G, Profile.ColorTint.B, 1.0f);
        
        // Temperature and tint
        Settings.WhiteTemp = 6500.0f + (Profile.TemperatureShift * 2000.0f);
        
        // Ambient occlusion
        Settings.AmbientOcclusionIntensity = Profile.AmbientOcclusionStrength;
        
        PostProcessVolume->bEnabled = true;
    }
}

FEmotionalLightingProfile UEmotionalLightingController::BlendProfiles(const FEmotionalLightingProfile& ProfileA, const FEmotionalLightingProfile& ProfileB, float BlendWeight)
{
    FEmotionalLightingProfile BlendedProfile;
    
    BlendedProfile.TensionLevel = FMath::Lerp(ProfileA.TensionLevel, ProfileB.TensionLevel, BlendWeight);
    BlendedProfile.ShadowIntensityMultiplier = FMath::Lerp(ProfileA.ShadowIntensityMultiplier, ProfileB.ShadowIntensityMultiplier, BlendWeight);
    BlendedProfile.ShadowContrastBoost = FMath::Lerp(ProfileA.ShadowContrastBoost, ProfileB.ShadowContrastBoost, BlendWeight);
    BlendedProfile.VisibilityReduction = FMath::Lerp(ProfileA.VisibilityReduction, ProfileB.VisibilityReduction, BlendWeight);
    BlendedProfile.FogDensityMultiplier = FMath::Lerp(ProfileA.FogDensityMultiplier, ProfileB.FogDensityMultiplier, BlendWeight);
    BlendedProfile.SaturationMultiplier = FMath::Lerp(ProfileA.SaturationMultiplier, ProfileB.SaturationMultiplier, BlendWeight);
    BlendedProfile.ContrastMultiplier = FMath::Lerp(ProfileA.ContrastMultiplier, ProfileB.ContrastMultiplier, BlendWeight);
    BlendedProfile.SunIntensityMultiplier = FMath::Lerp(ProfileA.SunIntensityMultiplier, ProfileB.SunIntensityMultiplier, BlendWeight);
    BlendedProfile.SkyLightMultiplier = FMath::Lerp(ProfileA.SkyLightMultiplier, ProfileB.SkyLightMultiplier, BlendWeight);
    BlendedProfile.TemperatureShift = FMath::Lerp(ProfileA.TemperatureShift, ProfileB.TemperatureShift, BlendWeight);
    BlendedProfile.LightFlickerIntensity = FMath::Lerp(ProfileA.LightFlickerIntensity, ProfileB.LightFlickerIntensity, BlendWeight);
    BlendedProfile.TensionPulseIntensity = FMath::Lerp(ProfileA.TensionPulseIntensity, ProfileB.TensionPulseIntensity, BlendWeight);
    
    BlendedProfile.ColorTint = FMath::Lerp(ProfileA.ColorTint, ProfileB.ColorTint, BlendWeight);
    BlendedProfile.FogTintShift = FMath::Lerp(ProfileA.FogTintShift, ProfileB.FogTintShift, BlendWeight);
    
    return BlendedProfile;
}

void UEmotionalLightingController::ModifyProfileForTension(FEmotionalLightingProfile& Profile, float TensionLevel)
{
    float TensionFactor = TensionLevel / MaxTensionLevel;
    
    // Increase shadows and contrast with tension
    Profile.ShadowIntensityMultiplier *= (1.0f + TensionFactor * 0.5f);
    Profile.ShadowContrastBoost += TensionFactor * 0.3f;
    Profile.ContrastMultiplier *= (1.0f + TensionFactor * 0.3f);
    
    // Reduce visibility with high tension
    Profile.VisibilityReduction += TensionFactor * 0.2f;
    Profile.FogDensityMultiplier *= (1.0f + TensionFactor * 0.5f);
    
    // Desaturate with tension
    Profile.SaturationMultiplier *= (1.0f - TensionFactor * 0.3f);
    
    // Add dynamic effects
    Profile.LightFlickerIntensity += TensionFactor * 0.2f;
    Profile.TensionPulseIntensity += TensionFactor * 0.4f;
    Profile.TensionPulseSpeed = 1.0f + TensionFactor * 1.5f; // Faster pulse with more tension
}