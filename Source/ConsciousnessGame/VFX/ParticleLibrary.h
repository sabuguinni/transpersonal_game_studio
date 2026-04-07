#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Particles/ParticleSystem.h"
#include "ParticleLibrary.generated.h"

USTRUCT(BlueprintType)
struct FConsciousnessParticleEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UParticleSystem* ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AssociatedStates;
};

USTRUCT(BlueprintType)
struct FArchetypeVFXSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FConsciousnessParticleEffect AuraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FConsciousnessParticleEffect ActivationEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FConsciousnessParticleEffect TransitionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class CONSCIOUSNESSGAME_API UParticleLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    // Consciousness State Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness States")
    TMap<FString, FConsciousnessParticleEffect> ConsciousnessStateEffects;

    // Emotional Resonance Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Resonance")
    FConsciousnessParticleEffect PositiveResonanceEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Resonance")
    FConsciousnessParticleEffect NegativeResonanceEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Resonance")
    FConsciousnessParticleEffect NeutralResonanceEffect;

    // Spiritual Energy Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Energy")
    FConsciousnessParticleEffect LowEnergyEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Energy")
    FConsciousnessParticleEffect MediumEnergyEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Energy")
    FConsciousnessParticleEffect HighEnergyEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Energy")
    FConsciousnessParticleEffect TranscendentEnergyEffect;

    // Meditation Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    FConsciousnessParticleEffect MeditationAuraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    FConsciousnessParticleEffect BreathingRhythmEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    FConsciousnessParticleEffect DeepMeditationEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    FConsciousnessParticleEffect MindfulnessEffect;

    // Transformation Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    FConsciousnessParticleEffect TransformationBurstEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    FConsciousnessParticleEffect IntegrationEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    FConsciousnessParticleEffect AwakeningEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
    FConsciousnessParticleEffect RealizationEffect;

    // Archetype Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetypes")
    TMap<FString, FArchetypeVFXSet> ArchetypeEffects;

    // Environmental Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FConsciousnessParticleEffect SynchronicityEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FConsciousnessParticleEffect EnergeticFieldEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FConsciousnessParticleEffect EnvironmentalResonanceEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FConsciousnessParticleEffect CosmicConnectionEffect;

    // Transition Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    FConsciousnessParticleEffect StateTransitionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    FConsciousnessParticleEffect FadeInEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    FConsciousnessParticleEffect FadeOutEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    FConsciousnessParticleEffect MorphEffect;

public:
    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Particle Library")
    FConsciousnessParticleEffect GetEffectByName(const FString& EffectName) const;

    UFUNCTION(BlueprintCallable, Category = "Particle Library")
    FArchetypeVFXSet GetArchetypeEffects(const FString& ArchetypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Particle Library")
    TArray<FString> GetAvailableEffectNames() const;

    UFUNCTION(BlueprintCallable, Category = "Particle Library")
    TArray<FString> GetEffectsForState(const FString& StateName) const;

    UFUNCTION(BlueprintCallable, Category = "Particle Library")
    bool HasEffect(const FString& EffectName) const;

    UFUNCTION(BlueprintCallable, Category = "Particle Library")
    void InitializeDefaultEffects();

private:
    void CreateDefaultConsciousnessEffects();
    void CreateDefaultEmotionalEffects();
    void CreateDefaultSpiritualEffects();
    void CreateDefaultMeditationEffects();
    void CreateDefaultTransformationEffects();
    void CreateDefaultArchetypeEffects();
    void CreateDefaultEnvironmentalEffects();
    void CreateDefaultTransitionEffects();
};