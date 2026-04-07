#include "ConsciousnessVFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PostProcessComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PostProcessVolume.h"

AConsciousnessVFXManager::AConsciousnessVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create post-process component
    ConsciousnessPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ConsciousnessPostProcess"));
    ConsciousnessPostProcess->SetupAttachment(RootComponent);
    ConsciousnessPostProcess->bUnbound = true;
    ConsciousnessPostProcess->Priority = 1.0f;

    // Initialize default values
    CurrentVFXState.AwarenessIntensity = 0.5f;
    CurrentVFXState.EmotionalResonance = 0.0f;
    CurrentVFXState.SpiritualEnergy = 0.0f;
    CurrentVFXState.TransformationProgress = 0.0f;
    CurrentVFXState.CurrentState = EConsciousnessState::Ordinary;
    CurrentVFXState.MeditationDepth = 0.0f;
    CurrentVFXState.bInTranscendentState = false;
}

void AConsciousnessVFXManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeVFXLayers();
    InitializeParticleSystems();
    InitializePostProcessMaterials();

    // Set initial post-process parameters
    UpdatePostProcessParameters();
}

void AConsciousnessVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateVFXLayerTransitions(DeltaTime);
    UpdatePostProcessParameters();
    UpdateParticleEffects();
    UpdateConsciousnessVisualization();
}

void AConsciousnessVFXManager::UpdateConsciousnessState(const FConsciousnessState& NewState)
{
    // Store previous state for transition
    EConsciousnessState PreviousState = CurrentVFXState.CurrentState;
    
    // Update VFX state
    CurrentVFXState.AwarenessIntensity = NewState.AwarenessLevel;
    CurrentVFXState.EmotionalResonance = NewState.EmotionalResonance;
    CurrentVFXState.SpiritualEnergy = NewState.SpiritualEnergy;
    CurrentVFXState.CurrentState = NewState.CurrentState;
    CurrentVFXState.ActiveArchetypes = NewState.ActiveArchetypes;

    // Trigger state transition effects
    if (PreviousState != NewState.CurrentState)
    {
        FString FromStateName = UEnum::GetValueAsString(PreviousState);
        FString ToStateName = UEnum::GetValueAsString(NewState.CurrentState);
        StartVFXTransition(FromStateName, ToStateName);
    }

    // Activate archetype-specific VFX
    for (const FString& Archetype : NewState.ActiveArchetypes)
    {
        ActivateArchetypeVFX(Archetype);
    }
}

void AConsciousnessVFXManager::SetAwarenessLevel(float AwarenessLevel)
{
    CurrentVFXState.AwarenessIntensity = FMath::Clamp(AwarenessLevel, 0.0f, 1.0f);
    
    // Update awareness-related VFX layers
    SetVFXLayerIntensity("Awareness", CurrentVFXState.AwarenessIntensity);
    
    // High awareness creates clarity effects
    if (CurrentVFXState.AwarenessIntensity > 0.8f)
    {
        ActivateVFXLayer("Clarity", CurrentVFXState.AwarenessIntensity);
    }
    else
    {
        DeactivateVFXLayer("Clarity");
    }
}

void AConsciousnessVFXManager::SetEmotionalResonance(float Resonance)
{
    CurrentVFXState.EmotionalResonance = FMath::Clamp(Resonance, -1.0f, 1.0f);
    
    // Positive resonance creates warm, flowing effects
    if (CurrentVFXState.EmotionalResonance > 0.0f)
    {
        ActivateVFXLayer("PositiveResonance", CurrentVFXState.EmotionalResonance);
        DeactivateVFXLayer("NegativeResonance");
    }
    // Negative resonance creates turbulent, chaotic effects
    else if (CurrentVFXState.EmotionalResonance < 0.0f)
    {
        ActivateVFXLayer("NegativeResonance", FMath::Abs(CurrentVFXState.EmotionalResonance));
        DeactivateVFXLayer("PositiveResonance");
    }
    // Neutral state
    else
    {
        DeactivateVFXLayer("PositiveResonance");
        DeactivateVFXLayer("NegativeResonance");
    }
}

void AConsciousnessVFXManager::SetSpiritualEnergy(float Energy)
{
    CurrentVFXState.SpiritualEnergy = FMath::Clamp(Energy, 0.0f, 1.0f);
    
    // Spiritual energy creates ethereal, luminous effects
    SetVFXLayerIntensity("SpiritualEnergy", CurrentVFXState.SpiritualEnergy);
    
    // High spiritual energy triggers transcendent effects
    if (CurrentVFXState.SpiritualEnergy > 0.9f)
    {
        ActivateVFXLayer("Transcendence", CurrentVFXState.SpiritualEnergy);
    }
}

void AConsciousnessVFXManager::TriggerTransformationEffect(float Intensity)
{
    CurrentVFXState.TransformationProgress = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Create transformation burst effect
    ActivateVFXLayer("TransformationBurst", Intensity);
    
    // Schedule deactivation after effect duration
    FTimerHandle TransformationTimer;
    GetWorld()->GetTimerManager().SetTimer(TransformationTimer, [this]()
    {
        DeactivateVFXLayer("TransformationBurst");
    }, 3.0f, false);
}

void AConsciousnessVFXManager::ActivateVFXLayer(const FString& LayerName, float TargetIntensity)
{
    if (VFXLayers.Contains(LayerName))
    {
        FVFXLayer& Layer = VFXLayers[LayerName];
        Layer.bIsActive = true;
        Layer.Intensity = TargetIntensity;
        
        // Activate associated particle system
        if (ParticleComponents.Contains(LayerName))
        {
            UParticleSystemComponent* ParticleComp = ParticleComponents[LayerName];
            if (ParticleComp && !ParticleComp->IsActive())
            {
                ParticleComp->ActivateSystem();
            }
        }
    }
}

void AConsciousnessVFXManager::DeactivateVFXLayer(const FString& LayerName)
{
    if (VFXLayers.Contains(LayerName))
    {
        FVFXLayer& Layer = VFXLayers[LayerName];
        Layer.bIsActive = false;
        Layer.Intensity = 0.0f;
        
        // Deactivate associated particle system
        if (ParticleComponents.Contains(LayerName))
        {
            UParticleSystemComponent* ParticleComp = ParticleComponents[LayerName];
            if (ParticleComp && ParticleComp->IsActive())
            {
                ParticleComp->DeactivateSystem();
            }
        }
    }
}

void AConsciousnessVFXManager::SetVFXLayerIntensity(const FString& LayerName, float Intensity)
{
    if (VFXLayers.Contains(LayerName))
    {
        VFXLayers[LayerName].Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    }
}

void AConsciousnessVFXManager::ActivateArchetypeVFX(const FString& ArchetypeName)
{
    // Each archetype has its own unique visual signature
    if (ArchetypeName == "Hero")
    {
        ActivateVFXLayer("HeroAura", 0.8f);
    }
    else if (ArchetypeName == "Sage")
    {
        ActivateVFXLayer("WisdomParticles", 0.7f);
    }
    else if (ArchetypeName == "Shadow")
    {
        ActivateVFXLayer("ShadowDistortion", 0.6f);
    }
    else if (ArchetypeName == "Anima" || ArchetypeName == "Animus")
    {
        ActivateVFXLayer("AnimaGlow", 0.5f);
    }
    else if (ArchetypeName == "Self")
    {
        ActivateVFXLayer("SelfMandala", 1.0f);
    }
}

void AConsciousnessVFXManager::DeactivateArchetypeVFX(const FString& ArchetypeName)
{
    if (ArchetypeName == "Hero")
    {
        DeactivateVFXLayer("HeroAura");
    }
    else if (ArchetypeName == "Sage")
    {
        DeactivateVFXLayer("WisdomParticles");
    }
    else if (ArchetypeName == "Shadow")
    {
        DeactivateVFXLayer("ShadowDistortion");
    }
    else if (ArchetypeName == "Anima" || ArchetypeName == "Animus")
    {
        DeactivateVFXLayer("AnimaGlow");
    }
    else if (ArchetypeName == "Self")
    {
        DeactivateVFXLayer("SelfMandala");
    }
}

void AConsciousnessVFXManager::StartMeditationVFX(float Depth)
{
    CurrentVFXState.MeditationDepth = FMath::Clamp(Depth, 0.0f, 1.0f);
    
    ActivateVFXLayer("MeditationAura", CurrentVFXState.MeditationDepth);
    ActivateVFXLayer("BreathingRhythm", 0.5f);
    
    // Gradually reduce environmental noise in VFX
    SetVFXLayerIntensity("EnvironmentalNoise", 1.0f - CurrentVFXState.MeditationDepth);
}

void AConsciousnessVFXManager::UpdateMeditationDepth(float Depth)
{
    CurrentVFXState.MeditationDepth = FMath::Clamp(Depth, 0.0f, 1.0f);
    
    SetVFXLayerIntensity("MeditationAura", CurrentVFXState.MeditationDepth);
    SetVFXLayerIntensity("EnvironmentalNoise", 1.0f - CurrentVFXState.MeditationDepth);
    
    // Deep meditation triggers special effects
    if (CurrentVFXState.MeditationDepth > 0.8f)
    {
        ActivateVFXLayer("DeepMeditation", CurrentVFXState.MeditationDepth);
    }
}

void AConsciousnessVFXManager::EndMeditationVFX()
{
    DeactivateVFXLayer("MeditationAura");
    DeactivateVFXLayer("BreathingRhythm");
    DeactivateVFXLayer("DeepMeditation");
    
    // Restore environmental effects
    SetVFXLayerIntensity("EnvironmentalNoise", 1.0f);
    
    CurrentVFXState.MeditationDepth = 0.0f;
}

void AConsciousnessVFXManager::TriggerTranscendentState()
{
    CurrentVFXState.bInTranscendentState = true;
    
    ActivateVFXLayer("TranscendentLight", 1.0f);
    ActivateVFXLayer("RealityDistortion", 0.7f);
    ActivateVFXLayer("CosmicConnection", 0.8f);
    
    // Temporarily disable normal consciousness effects
    DeactivateVFXLayer("Awareness");
    DeactivateVFXLayer("PositiveResonance");
    DeactivateVFXLayer("NegativeResonance");
}

void AConsciousnessVFXManager::EndTranscendentState()
{
    CurrentVFXState.bInTranscendentState = false;
    
    DeactivateVFXLayer("TranscendentLight");
    DeactivateVFXLayer("RealityDistortion");
    DeactivateVFXLayer("CosmicConnection");
    
    // Restore normal consciousness effects
    SetAwarenessLevel(CurrentVFXState.AwarenessIntensity);
    SetEmotionalResonance(CurrentVFXState.EmotionalResonance);
}

void AConsciousnessVFXManager::SetEnvironmentalResonance(float Resonance)
{
    float ClampedResonance = FMath::Clamp(Resonance, 0.0f, 1.0f);
    SetVFXLayerIntensity("EnvironmentalResonance", ClampedResonance);
}

void AConsciousnessVFXManager::TriggerSynchronicityEffect(const FVector& Location)
{
    // Create a temporary particle effect at the synchronicity location
    if (ParticleComponents.Contains("Synchronicity"))
    {
        UParticleSystemComponent* SyncEffect = ParticleComponents["Synchronicity"];
        SyncEffect->SetWorldLocation(Location);
        SyncEffect->ActivateSystem();
        
        // Auto-deactivate after a short duration
        FTimerHandle SyncTimer;
        GetWorld()->GetTimerManager().SetTimer(SyncTimer, [SyncEffect]()
        {
            if (SyncEffect)
            {
                SyncEffect->DeactivateSystem();
            }
        }, 2.0f, false);
    }
}

void AConsciousnessVFXManager::CreateEnergeticField(const FVector& Center, float Radius, float Intensity)
{
    // Create a dynamic energetic field effect
    ActivateVFXLayer("EnergeticField", Intensity);
    
    // Position the effect at the specified center
    if (ParticleComponents.Contains("EnergeticField"))
    {
        UParticleSystemComponent* FieldEffect = ParticleComponents["EnergeticField"];
        FieldEffect->SetWorldLocation(Center);
        
        // Scale the effect based on radius
        float Scale = Radius / 100.0f; // Normalize to reasonable scale
        FieldEffect->SetWorldScale3D(FVector(Scale));
    }
}

void AConsciousnessVFXManager::UpdatePostProcessParameters()
{
    if (!ConsciousnessParameters)
        return;

    UMaterialParameterCollectionInstance* ParamInstance = GetWorld()->GetParameterCollectionInstance(ConsciousnessParameters);
    if (!ParamInstance)
        return;

    // Update consciousness-related parameters
    ParamInstance->SetScalarParameterValue("AwarenessIntensity", CurrentVFXState.AwarenessIntensity);
    ParamInstance->SetScalarParameterValue("EmotionalResonance", CurrentVFXState.EmotionalResonance);
    ParamInstance->SetScalarParameterValue("SpiritualEnergy", CurrentVFXState.SpiritualEnergy);
    ParamInstance->SetScalarParameterValue("MeditationDepth", CurrentVFXState.MeditationDepth);
    ParamInstance->SetScalarParameterValue("OverallIntensity", CalculateOverallIntensity());
    ParamInstance->SetScalarParameterValue("PulseFrequency", GetPulseFrequency());
    
    // Set consciousness color
    FLinearColor ConsciousnessColor = GetConsciousnessColor();
    ParamInstance->SetVectorParameterValue("ConsciousnessColor", ConsciousnessColor);
    
    // Transcendent state override
    ParamInstance->SetScalarParameterValue("TranscendentState", CurrentVFXState.bInTranscendentState ? 1.0f : 0.0f);
}

void AConsciousnessVFXManager::UpdateParticleEffects()
{
    for (auto& ParticlePair : ParticleComponents)
    {
        UParticleSystemComponent* ParticleComp = ParticlePair.Value;
        if (!ParticleComp)
            continue;

        FString LayerName = ParticlePair.Key;
        
        if (VFXLayers.Contains(LayerName))
        {
            const FVFXLayer& Layer = VFXLayers[LayerName];
            
            // Update particle intensity based on layer intensity
            ParticleComp->SetFloatParameter("Intensity", Layer.Intensity);
            ParticleComp->SetFloatParameter("AwarenessLevel", CurrentVFXState.AwarenessIntensity);
            ParticleComp->SetFloatParameter("EmotionalResonance", CurrentVFXState.EmotionalResonance);
            ParticleComp->SetFloatParameter("SpiritualEnergy", CurrentVFXState.SpiritualEnergy);
        }
    }
}

void AConsciousnessVFXManager::UpdateVFXLayerTransitions(float DeltaTime)
{
    for (auto& LayerPair : VFXLayers)
    {
        FVFXLayer& Layer = LayerPair.Value;
        
        // Smooth transitions for layer intensity
        if (Layer.bIsActive)
        {
            // Fade in
            float TargetIntensity = 1.0f;
            Layer.Intensity = FMath::FInterpTo(Layer.Intensity, TargetIntensity, DeltaTime, Layer.FadeSpeed);
        }
        else
        {
            // Fade out
            Layer.Intensity = FMath::FInterpTo(Layer.Intensity, 0.0f, DeltaTime, Layer.FadeSpeed);
        }
    }
    
    // Handle state transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
        }
    }
}

void AConsciousnessVFXManager::UpdateConsciousnessVisualization()
{
    // Update visual representation based on current consciousness state
    float OverallIntensity = CalculateOverallIntensity();
    
    // Dynamic color shifts based on consciousness state
    FLinearColor BaseColor = GetConsciousnessColor();
    
    // Pulse effects for heightened states
    if (CurrentVFXState.AwarenessIntensity > 0.7f || CurrentVFXState.SpiritualEnergy > 0.7f)
    {
        float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * GetPulseFrequency()) * 0.5f + 0.5f;
        ActivateVFXLayer("ConsciousnessPulse", PulseValue * OverallIntensity);
    }
    else
    {
        DeactivateVFXLayer("ConsciousnessPulse");
    }
}

void AConsciousnessVFXManager::InitializeVFXLayers()
{
    // Initialize all VFX layers
    TArray<FString> LayerNames = {
        "Awareness", "Clarity", "PositiveResonance", "NegativeResonance",
        "SpiritualEnergy", "TransformationBurst", "MeditationAura",
        "BreathingRhythm", "DeepMeditation", "TranscendentLight",
        "RealityDistortion", "CosmicConnection", "HeroAura",
        "WisdomParticles", "ShadowDistortion", "AnimaGlow",
        "SelfMandala", "EnvironmentalResonance", "Synchronicity",
        "EnergeticField", "ConsciousnessPulse", "EnvironmentalNoise"
    };
    
    for (const FString& LayerName : LayerNames)
    {
        FVFXLayer NewLayer;
        NewLayer.LayerName = LayerName;
        NewLayer.Intensity = 0.0f;
        NewLayer.FadeSpeed = 2.0f;
        NewLayer.bIsActive = false;
        
        VFXLayers.Add(LayerName, NewLayer);
    }
}

void AConsciousnessVFXManager::InitializeParticleSystems()
{
    // Create particle system components for each VFX layer
    for (const auto& LayerPair : VFXLayers)
    {
        const FString& LayerName = LayerPair.Key;
        
        UParticleSystemComponent* ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(*LayerName);
        ParticleComp->SetupAttachment(RootComponent);
        ParticleComp->bAutoActivate = false;
        
        ParticleComponents.Add(LayerName, ParticleComp);
    }
}

void AConsciousnessVFXManager::InitializePostProcessMaterials()
{
    // Post-process materials will be set up in Blueprint
    // This function can be overridden in Blueprint to assign specific materials
}

float AConsciousnessVFXManager::CalculateOverallIntensity() const
{
    float TotalIntensity = 0.0f;
    TotalIntensity += CurrentVFXState.AwarenessIntensity * 0.3f;
    TotalIntensity += FMath::Abs(CurrentVFXState.EmotionalResonance) * 0.2f;
    TotalIntensity += CurrentVFXState.SpiritualEnergy * 0.4f;
    TotalIntensity += CurrentVFXState.MeditationDepth * 0.1f;
    
    return FMath::Clamp(TotalIntensity, 0.0f, 1.0f);
}

FLinearColor AConsciousnessVFXManager::GetConsciousnessColor() const
{
    // Base color shifts based on consciousness state
    FLinearColor BaseColor = FLinearColor::White;
    
    switch (CurrentVFXState.CurrentState)
    {
        case EConsciousnessState::Ordinary:
            BaseColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f); // Soft blue-white
            break;
        case EConsciousnessState::Heightened:
            BaseColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm golden
            break;
        case EConsciousnessState::Transcendent:
            BaseColor = FLinearColor(0.9f, 0.7f, 1.0f, 1.0f); // Purple-white
            break;
        case EConsciousnessState::Shadow:
            BaseColor = FLinearColor(0.6f, 0.4f, 0.8f, 1.0f); // Deep purple
            break;
        case EConsciousnessState::Integration:
            BaseColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // Pure white
            break;
    }
    
    // Modulate with emotional resonance
    if (CurrentVFXState.EmotionalResonance > 0.0f)
    {
        // Positive emotions add warmth
        BaseColor.R += CurrentVFXState.EmotionalResonance * 0.2f;
        BaseColor.G += CurrentVFXState.EmotionalResonance * 0.1f;
    }
    else if (CurrentVFXState.EmotionalResonance < 0.0f)
    {
        // Negative emotions add coolness
        BaseColor.B += FMath::Abs(CurrentVFXState.EmotionalResonance) * 0.2f;
        BaseColor.R -= FMath::Abs(CurrentVFXState.EmotionalResonance) * 0.1f;
        BaseColor.G -= FMath::Abs(CurrentVFXState.EmotionalResonance) * 0.1f;
    }
    
    return BaseColor;
}

float AConsciousnessVFXManager::GetPulseFrequency() const
{
    // Base frequency modulated by consciousness state
    float BaseFrequency = 1.0f;
    
    BaseFrequency += CurrentVFXState.AwarenessIntensity * 2.0f;
    BaseFrequency += CurrentVFXState.SpiritualEnergy * 1.5f;
    BaseFrequency += FMath::Abs(CurrentVFXState.EmotionalResonance) * 1.0f;
    
    if (CurrentVFXState.bInTranscendentState)
    {
        BaseFrequency *= 0.5f; // Slower, deeper pulses in transcendent state
    }
    
    return FMath::Clamp(BaseFrequency, 0.1f, 5.0f);
}

void AConsciousnessVFXManager::StartVFXTransition(const FString& FromState, const FString& ToState)
{
    if (bIsTransitioning)
        return;
    
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    TransitionFromState = FromState;
    TransitionToState = ToState;
    
    // Determine transition duration based on state change intensity
    if (FromState.Contains("Transcendent") || ToState.Contains("Transcendent"))
    {
        TransitionDuration = 4.0f; // Longer for transcendent transitions
    }
    else if (FromState.Contains("Shadow") || ToState.Contains("Shadow"))
    {
        TransitionDuration = 3.0f; // Medium for shadow work
    }
    else
    {
        TransitionDuration = 2.0f; // Standard transition
    }
    
    // Trigger transition-specific effects
    ActivateVFXLayer("StateTransition", 1.0f);
    
    // Schedule transition completion
    FTimerHandle TransitionTimer;
    GetWorld()->GetTimerManager().SetTimer(TransitionTimer, [this]()
    {
        DeactivateVFXLayer("StateTransition");
    }, TransitionDuration, false);
}

bool AConsciousnessVFXManager::IsTransitioning() const
{
    return bIsTransitioning;
}