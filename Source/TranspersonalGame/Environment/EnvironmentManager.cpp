// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "EnvironmentManager.h"
#include "Engine/Engine.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "../TranspersonalGameCharacter.h"
#include "Components/SceneComponent.h"

AEnvironmentManager::AEnvironmentManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    TransitionSpeed = 2.0f;
    ConsciousnessThreshold = 0.3f;
    CurrentEnvironmentState = EEnvironmentState::Neutral;
}

void AEnvironmentManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character and consciousness component
    PlayerCharacter = Cast<ATranspersonalGameCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (PlayerCharacter)
    {
        PlayerConsciousness = PlayerCharacter->FindComponentByClass<UConsciousnessComponent>();
    }
    
    // Initialize environment responses
    InitializeEnvironmentResponses();
    
    // Set initial state
    if (EnvironmentResponses.Contains(CurrentEnvironmentState))
    {
        CurrentResponse = EnvironmentResponses[CurrentEnvironmentState];
        TargetResponse = CurrentResponse;
        UpdateMaterialParameters();
    }
}

void AEnvironmentManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update environment based on player consciousness
    if (PlayerConsciousness)
    {
        float ConsciousnessLevel = PlayerConsciousness->GetConsciousnessLevel();
        float EmotionalResonance = PlayerConsciousness->GetEmotionalResonance();
        float SpiritualAlignment = PlayerConsciousness->GetSpiritualAlignment();
        
        UpdateEnvironmentState(ConsciousnessLevel, EmotionalResonance, SpiritualAlignment);
    }
    
    // Interpolate environment parameters
    InterpolateToTarget(DeltaTime);
}

void AEnvironmentManager::UpdateEnvironmentState(float ConsciousnessLevel, float EmotionalResonance, float SpiritualAlignment)
{
    EEnvironmentState NewState = DetermineEnvironmentState(ConsciousnessLevel, EmotionalResonance, SpiritualAlignment);
    
    if (NewState != CurrentEnvironmentState)
    {
        SetEnvironmentState(NewState);
    }
}

void AEnvironmentManager::SetEnvironmentState(EEnvironmentState NewState)
{
    if (EnvironmentResponses.Contains(NewState))
    {
        CurrentEnvironmentState = NewState;
        TargetResponse = EnvironmentResponses[NewState];
        
        // Trigger Blueprint event
        OnEnvironmentStateChanged(NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Environment state changed to: %d"), (int32)NewState);
    }
}

void AEnvironmentManager::InitializeEnvironmentResponses()
{
    // Neutral State
    FEnvironmentResponse NeutralResponse;
    NeutralResponse.AmbientColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    NeutralResponse.AuraColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.1f);
    NeutralResponse.EmissionIntensity = 0.0f;
    NeutralResponse.FogDensity = 0.02f;
    NeutralResponse.ParticleIntensity = 0.5f;
    EnvironmentResponses.Add(EEnvironmentState::Neutral, NeutralResponse);
    
    // Meditative State
    FEnvironmentResponse MeditativeResponse;
    MeditativeResponse.AmbientColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    MeditativeResponse.AuraColor = FLinearColor(0.3f, 0.6f, 1.0f, 0.3f);
    MeditativeResponse.EmissionIntensity = 0.2f;
    MeditativeResponse.FogDensity = 0.01f;
    MeditativeResponse.ParticleIntensity = 1.0f;
    EnvironmentResponses.Add(EEnvironmentState::Meditative, MeditativeResponse);
    
    // Emotional State
    FEnvironmentResponse EmotionalResponse;
    EmotionalResponse.AmbientColor = FLinearColor(1.0f, 0.7f, 0.8f, 1.0f);
    EmotionalResponse.AuraColor = FLinearColor(1.0f, 0.4f, 0.6f, 0.4f);
    EmotionalResponse.EmissionIntensity = 0.3f;
    EmotionalResponse.FogDensity = 0.03f;
    EmotionalResponse.ParticleIntensity = 1.5f;
    EnvironmentResponses.Add(EEnvironmentState::Emotional, EmotionalResponse);
    
    // Spiritual State
    FEnvironmentResponse SpiritualResponse;
    SpiritualResponse.AmbientColor = FLinearColor(1.0f, 1.0f, 0.8f, 1.0f);
    SpiritualResponse.AuraColor = FLinearColor(1.0f, 0.9f, 0.5f, 0.5f);
    SpiritualResponse.EmissionIntensity = 0.5f;
    SpiritualResponse.FogDensity = 0.005f;
    SpiritualResponse.ParticleIntensity = 2.0f;
    EnvironmentResponses.Add(EEnvironmentState::Spiritual, SpiritualResponse);
    
    // Transcendent State
    FEnvironmentResponse TranscendentResponse;
    TranscendentResponse.AmbientColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    TranscendentResponse.AuraColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.7f);
    TranscendentResponse.EmissionIntensity = 1.0f;
    TranscendentResponse.FogDensity = 0.001f;
    TranscendentResponse.ParticleIntensity = 3.0f;
    EnvironmentResponses.Add(EEnvironmentState::Transcendent, TranscendentResponse);
}

EEnvironmentState AEnvironmentManager::DetermineEnvironmentState(float ConsciousnessLevel, float EmotionalResonance, float SpiritualAlignment)
{
    // Transcendent state - highest priority
    if (ConsciousnessLevel > 0.8f && SpiritualAlignment > 0.7f)
    {
        return EEnvironmentState::Transcendent;
    }
    
    // Spiritual state
    if (SpiritualAlignment > 0.5f && ConsciousnessLevel > 0.4f)
    {
        return EEnvironmentState::Spiritual;
    }
    
    // Emotional state
    if (FMath::Abs(EmotionalResonance) > 0.6f)
    {
        return EEnvironmentState::Emotional;
    }
    
    // Meditative state
    if (ConsciousnessLevel > ConsciousnessThreshold && FMath::Abs(EmotionalResonance) < 0.3f)
    {
        return EEnvironmentState::Meditative;
    }
    
    // Default to neutral
    return EEnvironmentState::Neutral;
}

void AEnvironmentManager::InterpolateToTarget(float DeltaTime)
{
    float Alpha = FMath::Clamp(TransitionSpeed * DeltaTime, 0.0f, 1.0f);
    
    // Interpolate colors
    CurrentResponse.AmbientColor = FMath::Lerp(CurrentResponse.AmbientColor, TargetResponse.AmbientColor, Alpha);
    CurrentResponse.AuraColor = FMath::Lerp(CurrentResponse.AuraColor, TargetResponse.AuraColor, Alpha);
    
    // Interpolate scalars
    CurrentResponse.EmissionIntensity = FMath::Lerp(CurrentResponse.EmissionIntensity, TargetResponse.EmissionIntensity, Alpha);
    CurrentResponse.FogDensity = FMath::Lerp(CurrentResponse.FogDensity, TargetResponse.FogDensity, Alpha);
    CurrentResponse.ParticleIntensity = FMath::Lerp(CurrentResponse.ParticleIntensity, TargetResponse.ParticleIntensity, Alpha);
    
    // Update material parameters
    UpdateMaterialParameters();
}

void AEnvironmentManager::UpdateMaterialParameters()
{
    if (!EnvironmentMPC)
    {
        return;
    }
    
    UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(EnvironmentMPC);
    if (MPCInstance)
    {
        // Update global environment parameters
        MPCInstance->SetVectorParameterValue(FName("EnvironmentAmbientColor"), CurrentResponse.AmbientColor);
        MPCInstance->SetVectorParameterValue(FName("EnvironmentAuraColor"), CurrentResponse.AuraColor);
        MPCInstance->SetScalarParameterValue(FName("EnvironmentEmission"), CurrentResponse.EmissionIntensity);
        MPCInstance->SetScalarParameterValue(FName("EnvironmentFogDensity"), CurrentResponse.FogDensity);
        MPCInstance->SetScalarParameterValue(FName("EnvironmentParticleIntensity"), CurrentResponse.ParticleIntensity);
    }
}