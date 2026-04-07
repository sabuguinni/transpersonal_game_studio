#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "../Core/ConsciousnessTypes.h"
#include "ConsciousnessVFXManager.generated.h"

USTRUCT(BlueprintType)
struct FConsciousnessVFXState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AwarenessIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalResonance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualEnergy = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransformationProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState CurrentState = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ActiveArchetypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MeditationDepth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInTranscendentState = false;
};

USTRUCT(BlueprintType)
struct FVFXLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UParticleSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMaterialInterface* PostProcessMaterial;
};

UCLASS(BlueprintType, Blueprintable)
class CONSCIOUSNESSGAME_API AConsciousnessVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    class UPostProcessComponent* ConsciousnessPostProcess;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UMaterialParameterCollection* ConsciousnessParameters;

    // VFX State
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    FConsciousnessVFXState CurrentVFXState;

    // VFX Layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Layers")
    TMap<FString, FVFXLayer> VFXLayers;

    // Particle Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TMap<FString, class UParticleSystemComponent*> ParticleComponents;

    // Post Process Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    TArray<class UMaterialInterface*> ConsciousnessPostProcessMaterials;

    // Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float DefaultTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float IntenseTransitionSpeed = 5.0f;

public:
    // Main Interface
    UFUNCTION(BlueprintCallable, Category = "Consciousness VFX")
    void UpdateConsciousnessState(const FConsciousnessState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness VFX")
    void SetAwarenessLevel(float AwarenessLevel);

    UFUNCTION(BlueprintCallable, Category = "Consciousness VFX")
    void SetEmotionalResonance(float Resonance);

    UFUNCTION(BlueprintCallable, Category = "Consciousness VFX")
    void SetSpiritualEnergy(float Energy);

    UFUNCTION(BlueprintCallable, Category = "Consciousness VFX")
    void TriggerTransformationEffect(float Intensity = 1.0f);

    // VFX Layer Management
    UFUNCTION(BlueprintCallable, Category = "VFX Layers")
    void ActivateVFXLayer(const FString& LayerName, float TargetIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Layers")
    void DeactivateVFXLayer(const FString& LayerName);

    UFUNCTION(BlueprintCallable, Category = "VFX Layers")
    void SetVFXLayerIntensity(const FString& LayerName, float Intensity);

    // Archetype-Specific Effects
    UFUNCTION(BlueprintCallable, Category = "Archetype VFX")
    void ActivateArchetypeVFX(const FString& ArchetypeName);

    UFUNCTION(BlueprintCallable, Category = "Archetype VFX")
    void DeactivateArchetypeVFX(const FString& ArchetypeName);

    // Meditation and Transcendence
    UFUNCTION(BlueprintCallable, Category = "Meditation VFX")
    void StartMeditationVFX(float Depth = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Meditation VFX")
    void UpdateMeditationDepth(float Depth);

    UFUNCTION(BlueprintCallable, Category = "Meditation VFX")
    void EndMeditationVFX();

    UFUNCTION(BlueprintCallable, Category = "Transcendence VFX")
    void TriggerTranscendentState();

    UFUNCTION(BlueprintCallable, Category = "Transcendence VFX")
    void EndTranscendentState();

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void SetEnvironmentalResonance(float Resonance);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void TriggerSynchronicityEffect(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void CreateEnergeticField(const FVector& Center, float Radius, float Intensity);

private:
    // Internal Update Functions
    void UpdatePostProcessParameters();
    void UpdateParticleEffects();
    void UpdateVFXLayerTransitions(float DeltaTime);
    void UpdateConsciousnessVisualization();

    // Helper Functions
    void InitializeVFXLayers();
    void InitializeParticleSystems();
    void InitializePostProcessMaterials();
    
    float CalculateOverallIntensity() const;
    FLinearColor GetConsciousnessColor() const;
    float GetPulseFrequency() const;

    // Transition Management
    void StartVFXTransition(const FString& FromState, const FString& ToState);
    bool IsTransitioning() const;

    // Current transition state
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;
    FString TransitionFromState;
    FString TransitionToState;
};