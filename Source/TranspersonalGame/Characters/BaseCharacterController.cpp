#include "BaseCharacterController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacterController::ABaseCharacterController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create aura mesh component
    AuraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AuraMesh"));
    AuraMesh->SetupAttachment(RootComponent);
    AuraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AuraMesh->SetCastShadow(false);

    // Create energy particle system
    EnergyParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EnergyParticles"));
    EnergyParticles->SetupAttachment(RootComponent);
    EnergyParticles->bAutoActivate = false;

    // Create character light
    CharacterLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CharacterLight"));
    CharacterLight->SetupAttachment(RootComponent);
    CharacterLight->SetIntensity(500.0f);
    CharacterLight->SetLightColor(FLinearColor::White);
    CharacterLight->SetAttenuationRadius(1000.0f);
    CharacterLight->SetCastShadows(false);

    // Initialize visual properties
    VisualProperties.Opacity = 1.0f;
    VisualProperties.AuraColor = FLinearColor::White;
    VisualProperties.AuraIntensity = 0.5f;
    VisualProperties.EnergyFlowRate = 1.0f;
    VisualProperties.bShowGeometricPatterns = false;
    VisualProperties.PatternComplexity = 0.3f;
}

void ABaseCharacterController::BeginPlay()
{
    Super::BeginPlay();

    // Find consciousness system
    ConsciousnessSystem = Cast<AConsciousnessSystem>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AConsciousnessSystem::StaticClass())
    );

    // Create dynamic materials
    if (BaseMaterial)
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        GetMesh()->SetMaterial(0, DynamicMaterial);
    }

    if (AuraMaterial)
    {
        DynamicAuraMaterial = UMaterialInstanceDynamic::Create(AuraMaterial, this);
        AuraMesh->SetMaterial(0, DynamicAuraMaterial);
    }

    // Initialize visual state
    UpdateMaterialParameters();
    UpdateLighting();
    UpdateParticleEffects();
}

void ABaseCharacterController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update state transition
    if (StateTransitionTime > 0.0f)
    {
        StateTransitionTime -= DeltaTime;
        float TransitionProgress = 1.0f - (StateTransitionTime / MaxTransitionDuration);
        
        // Update visual effects based on transition progress
        UpdateMaterialParameters();
    }

    // Check consciousness system for updates
    if (ConsciousnessSystem)
    {
        float CurrentLevel = ConsciousnessSystem->GetConsciousnessLevel();
        if (FMath::Abs(CurrentLevel - LastConsciousnessLevel) > 0.1f)
        {
            RespondToConsciousnessLevel(CurrentLevel);
            LastConsciousnessLevel = CurrentLevel;
        }
    }
}

void ABaseCharacterController::SetCharacterState(ECharacterState NewState)
{
    if (CurrentState != NewState)
    {
        ECharacterState OldState = CurrentState;
        CurrentState = NewState;
        StateTransitionTime = MaxTransitionDuration;

        // Trigger state change event
        OnStateChanged(OldState, NewState);

        // Update visual effects
        UpdateMaterialParameters();
        UpdateLighting();
        UpdateParticleEffects();
    }
}

void ABaseCharacterController::UpdateVisualProperties(const FCharacterVisualProperties& NewProperties)
{
    VisualProperties = NewProperties;
    UpdateMaterialParameters();
    UpdateLighting();
    UpdateParticleEffects();
}

void ABaseCharacterController::RespondToConsciousnessLevel(float ConsciousnessLevel)
{
    // Adjust character appearance based on consciousness level
    float AdjustedLevel = ConsciousnessLevel * ConsciousnessResponseSensitivity;
    
    // Update aura intensity
    VisualProperties.AuraIntensity = FMath::Lerp(0.2f, 1.0f, AdjustedLevel);
    
    // Update energy flow rate
    VisualProperties.EnergyFlowRate = FMath::Lerp(0.5f, 2.0f, AdjustedLevel);
    
    // Show geometric patterns at higher consciousness levels
    VisualProperties.bShowGeometricPatterns = AdjustedLevel > 0.6f;
    VisualProperties.PatternComplexity = FMath::Lerp(0.1f, 1.0f, AdjustedLevel);

    // Trigger consciousness response event
    OnConsciousnessResponse(ConsciousnessLevel);

    // Update visuals
    UpdateMaterialParameters();
    UpdateLighting();
}

void ABaseCharacterController::TriggerTransformation()
{
    // Advance to next state based on archetype
    switch (CurrentState)
    {
        case ECharacterState::Dormant:
            SetCharacterState(ECharacterState::Awakening);
            break;
        case ECharacterState::Awakening:
            SetCharacterState(ECharacterState::Active);
            break;
        case ECharacterState::Active:
            SetCharacterState(ECharacterState::Transforming);
            break;
        case ECharacterState::Transforming:
            SetCharacterState(ECharacterState::Transcendent);
            break;
        default:
            break;
    }
}

void ABaseCharacterController::UpdateMaterialParameters()
{
    if (DynamicMaterial)
    {
        // Update base material parameters
        DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), VisualProperties.Opacity);
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), VisualProperties.AuraColor);
        
        // State-based parameters
        float StateIntensity = 0.5f;
        switch (CurrentState)
        {
            case ECharacterState::Dormant:
                StateIntensity = 0.2f;
                break;
            case ECharacterState::Awakening:
                StateIntensity = 0.5f;
                break;
            case ECharacterState::Active:
                StateIntensity = 0.8f;
                break;
            case ECharacterState::Transforming:
                StateIntensity = 1.2f;
                break;
            case ECharacterState::Transcendent:
                StateIntensity = 1.5f;
                break;
        }
        
        DynamicMaterial->SetScalarParameterValue(TEXT("StateIntensity"), StateIntensity);
        DynamicMaterial->SetScalarParameterValue(TEXT("EnergyFlow"), VisualProperties.EnergyFlowRate);
    }

    if (DynamicAuraMaterial)
    {
        // Update aura material parameters
        DynamicAuraMaterial->SetVectorParameterValue(TEXT("AuraColor"), VisualProperties.AuraColor);
        DynamicAuraMaterial->SetScalarParameterValue(TEXT("AuraIntensity"), VisualProperties.AuraIntensity);
        DynamicAuraMaterial->SetScalarParameterValue(TEXT("ShowPatterns"), VisualProperties.bShowGeometricPatterns ? 1.0f : 0.0f);
        DynamicAuraMaterial->SetScalarParameterValue(TEXT("PatternComplexity"), VisualProperties.PatternComplexity);
    }
}

void ABaseCharacterController::UpdateLighting()
{
    if (CharacterLight)
    {
        // Update light properties based on character state and consciousness level
        float LightIntensity = 500.0f * VisualProperties.AuraIntensity;
        
        switch (CurrentState)
        {
            case ECharacterState::Dormant:
                LightIntensity *= 0.3f;
                break;
            case ECharacterState::Awakening:
                LightIntensity *= 0.6f;
                break;
            case ECharacterState::Active:
                LightIntensity *= 1.0f;
                break;
            case ECharacterState::Transforming:
                LightIntensity *= 1.5f;
                break;
            case ECharacterState::Transcendent:
                LightIntensity *= 2.0f;
                break;
        }

        CharacterLight->SetIntensity(LightIntensity);
        CharacterLight->SetLightColor(VisualProperties.AuraColor);
    }
}

void ABaseCharacterController::UpdateParticleEffects()
{
    if (EnergyParticles)
    {
        // Activate/deactivate particles based on state
        bool bShouldShowParticles = (CurrentState != ECharacterState::Dormant);
        
        if (bShouldShowParticles && !EnergyParticles->IsActive())
        {
            EnergyParticles->Activate();
        }
        else if (!bShouldShowParticles && EnergyParticles->IsActive())
        {
            EnergyParticles->Deactivate();
        }

        // Update particle parameters
        if (EnergyParticles->IsActive())
        {
            // Set spawn rate based on energy flow
            float SpawnRate = 10.0f * VisualProperties.EnergyFlowRate;
            EnergyParticles->SetFloatParameter(TEXT("SpawnRate"), SpawnRate);
            
            // Set color
            EnergyParticles->SetColorParameter(TEXT("ParticleColor"), VisualProperties.AuraColor);
        }
    }
}