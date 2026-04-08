#include "TranspersonalCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/ConsciousnessSystem.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Aura Mesh Component
    AuraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AuraMesh"));
    AuraMesh->SetupAttachment(RootComponent);
    AuraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AuraMesh->SetCastShadow(false);

    // Create Ethereal Particles Component
    EtherealParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EtherealParticles"));
    EtherealParticles->SetupAttachment(RootComponent);
    EtherealParticles->bAutoActivate = false;

    // Create Inner Light Component
    InnerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("InnerLight"));
    InnerLight->SetupAttachment(RootComponent);
    InnerLight->SetLightColor(FLinearColor(1.0f, 0.8f, 0.3f, 1.0f));
    InnerLight->SetIntensity(1000.0f);
    InnerLight->SetAttenuationRadius(500.0f);
    InnerLight->SetCastShadows(false);

    // Initialize properties
    ConsciousnessLevel = 0.0f;
    TransformationSpeed = 1.0f;
    bRespondsToPlayerConsciousness = true;
    bFloatingMovement = true;
    FloatAmplitude = 50.0f;
    FloatFrequency = 1.0f;
    InteractionRadius = 500.0f;
    bCanCommunicateTelepathically = true;

    // Initialize transformation variables
    TargetScale = FVector(1.0f);
    TargetAuraColor = FLinearColor::White;
    CurrentTransformationProgress = 0.0f;
    FloatTimer = 0.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Store initial Z position for floating movement
    InitialZPosition = GetActorLocation().Z;

    // Find Consciousness System
    ConsciousnessSystemRef = Cast<AConsciousnessSystem>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AConsciousnessSystem::StaticClass())
    );

    // Create dynamic material instance
    if (BaseMaterial)
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (GetMesh())
        {
            GetMesh()->SetMaterial(0, DynamicMaterial);
        }
    }

    // Initialize visual effects based on archetype
    TransformToArchetype(CharacterArchetype);
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update consciousness level from system
    if (bRespondsToPlayerConsciousness && ConsciousnessSystemRef)
    {
        float SystemLevel = ConsciousnessSystemRef->GetConsciousnessLevel();
        if (FMath::Abs(SystemLevel - ConsciousnessLevel) > 0.01f)
        {
            UpdateConsciousnessState(SystemLevel);
        }
    }

    // Handle floating movement
    if (bFloatingMovement)
    {
        FloatTimer += DeltaTime * FloatFrequency;
        float FloatOffset = FMath::Sin(FloatTimer) * FloatAmplitude;
        FVector CurrentLocation = GetActorLocation();
        CurrentLocation.Z = InitialZPosition + FloatOffset;
        SetActorLocation(CurrentLocation);
    }

    // Update visual effects
    UpdateVisualEffects();
}

void ATranspersonalCharacter::UpdateConsciousnessState(float NewLevel)
{
    ConsciousnessLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);

    // Determine consciousness state based on level
    EConsciousnessState NewState = EConsciousnessState::Unconscious;
    if (ConsciousnessLevel >= 0.8f)
        NewState = EConsciousnessState::Unity;
    else if (ConsciousnessLevel >= 0.6f)
        NewState = EConsciousnessState::Transcendent;
    else if (ConsciousnessLevel >= 0.4f)
        NewState = EConsciousnessState::SelfAware;
    else if (ConsciousnessLevel >= 0.2f)
        NewState = EConsciousnessState::Awakening;

    if (NewState != CurrentConsciousnessState)
    {
        CurrentConsciousnessState = NewState;
        OnConsciousnessStateChanged(NewState);
    }

    UpdateVisualEffects();
}

void ATranspersonalCharacter::TransformToArchetype(ECharacterArchetype NewArchetype)
{
    CharacterArchetype = NewArchetype;

    // Set archetype-specific properties
    switch (CharacterArchetype)
    {
        case ECharacterArchetype::SpiritualGuide:
            TargetAuraColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f); // Blue-white
            TargetScale = FVector(1.2f);
            bFloatingMovement = true;
            FloatAmplitude = 30.0f;
            break;

        case ECharacterArchetype::ShadowSelf:
            TargetAuraColor = FLinearColor(0.8f, 0.1f, 0.2f, 1.0f); // Dark red
            TargetScale = FVector(0.9f);
            bFloatingMovement = false;
            break;

        case ECharacterArchetype::WiseElder:
            TargetAuraColor = FLinearColor(1.0f, 0.8f, 0.3f, 1.0f); // Golden
            TargetScale = FVector(1.1f);
            bFloatingMovement = true;
            FloatAmplitude = 20.0f;
            break;

        case ECharacterArchetype::InnerChild:
            TargetAuraColor = FLinearColor(1.0f, 0.6f, 0.8f, 1.0f); // Pink
            TargetScale = FVector(0.8f);
            bFloatingMovement = true;
            FloatAmplitude = 50.0f;
            FloatFrequency = 2.0f;
            break;

        case ECharacterArchetype::CosmicEntity:
            TargetAuraColor = FLinearColor(0.6f, 0.3f, 1.0f, 1.0f); // Purple
            TargetScale = FVector(1.5f);
            bFloatingMovement = true;
            FloatAmplitude = 80.0f;
            FloatFrequency = 0.5f;
            break;
    }

    OnArchetypeTransformation(NewArchetype);
    UpdateVisualEffects();
}

void ATranspersonalCharacter::ActivateEtherealForm(bool bActivate)
{
    if (EtherealParticles)
    {
        if (bActivate)
        {
            EtherealParticles->Activate();
        }
        else
        {
            EtherealParticles->Deactivate();
        }
    }

    // Update material transparency
    if (DynamicMaterial)
    {
        float Opacity = bActivate ? 0.7f : 1.0f;
        DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
    }
}

void ATranspersonalCharacter::SetAuraIntensity(float Intensity)
{
    if (InnerLight)
    {
        InnerLight->SetIntensity(Intensity * 1000.0f);
    }

    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(TEXT("AuraIntensity"), Intensity);
    }
}

void ATranspersonalCharacter::UpdateVisualEffects()
{
    UpdateMaterialParameters();
    UpdateLighting();
    UpdateParticleEffects();

    // Smooth scale transformation
    FVector CurrentScale = GetActorScale3D();
    FVector NewScale = FMath::VInterpTo(CurrentScale, TargetScale, GetWorld()->GetDeltaSeconds(), TransformationSpeed);
    SetActorScale3D(NewScale);
}

void ATranspersonalCharacter::UpdateMaterialParameters()
{
    if (!DynamicMaterial) return;

    // Update consciousness-based parameters
    DynamicMaterial->SetScalarParameterValue(TEXT("ConsciousnessLevel"), ConsciousnessLevel);
    
    // Interpolate aura color
    if (InnerLight)
    {
        FLinearColor CurrentColor = InnerLight->GetLightColor();
        FLinearColor NewColor = FMath::CInterpTo(CurrentColor, TargetAuraColor, GetWorld()->GetDeltaSeconds(), TransformationSpeed);
        InnerLight->SetLightColor(NewColor);
        DynamicMaterial->SetVectorParameterValue(TEXT("AuraColor"), NewColor);
    }

    // Set archetype-specific parameters
    float ArchetypeValue = static_cast<float>(CharacterArchetype) / 4.0f;
    DynamicMaterial->SetScalarParameterValue(TEXT("ArchetypeBlend"), ArchetypeValue);

    // Consciousness state effects
    float StateIntensity = static_cast<float>(CurrentConsciousnessState) / 4.0f;
    DynamicMaterial->SetScalarParameterValue(TEXT("StateIntensity"), StateIntensity);
}

void ATranspersonalCharacter::UpdateLighting()
{
    if (!InnerLight) return;

    // Adjust light intensity based on consciousness level
    float BaseIntensity = 500.0f;
    float ConsciousnessMultiplier = 1.0f + (ConsciousnessLevel * 2.0f);
    InnerLight->SetIntensity(BaseIntensity * ConsciousnessMultiplier);

    // Adjust attenuation radius
    float BaseRadius = 300.0f;
    float RadiusMultiplier = 1.0f + (ConsciousnessLevel * 1.5f);
    InnerLight->SetAttenuationRadius(BaseRadius * RadiusMultiplier);

    // Pulsing effect for higher consciousness states
    if (CurrentConsciousnessState >= EConsciousnessState::Transcendent)
    {
        float PulseValue = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f);
        float PulsedIntensity = InnerLight->Intensity * (0.7f + 0.3f * PulseValue);
        InnerLight->SetIntensity(PulsedIntensity);
    }
}

void ATranspersonalCharacter::UpdateParticleEffects()
{
    if (!EtherealParticles) return;

    // Activate particles based on consciousness level
    bool bShouldActivateParticles = ConsciousnessLevel > 0.3f;
    
    if (bShouldActivateParticles && !EtherealParticles->IsActive())
    {
        EtherealParticles->Activate();
    }
    else if (!bShouldActivateParticles && EtherealParticles->IsActive())
    {
        EtherealParticles->Deactivate();
    }

    // Adjust particle parameters based on consciousness state
    if (EtherealParticles->IsActive())
    {
        // These would be set via particle system parameters
        // Implementation depends on specific particle system setup
    }
}