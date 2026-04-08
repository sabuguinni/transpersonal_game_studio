#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "../Core/ConsciousnessSystem.h"
#include "TranspersonalCharacter.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    SpiritualGuide     UMETA(DisplayName = "Spiritual Guide"),
    ShadowSelf         UMETA(DisplayName = "Shadow Self"),
    WiseElder          UMETA(DisplayName = "Wise Elder"),
    InnerChild         UMETA(DisplayName = "Inner Child"),
    CosmicEntity       UMETA(DisplayName = "Cosmic Entity")
};

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Unconscious        UMETA(DisplayName = "Unconscious"),
    Awakening          UMETA(DisplayName = "Awakening"),
    SelfAware          UMETA(DisplayName = "Self-Aware"),
    Transcendent       UMETA(DisplayName = "Transcendent"),
    Unity              UMETA(DisplayName = "Unity Consciousness")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATranspersonalCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Character Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transpersonal")
    ECharacterArchetype CharacterArchetype = ECharacterArchetype::SpiritualGuide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transpersonal")
    EConsciousnessState CurrentConsciousnessState = EConsciousnessState::Unconscious;

    // Visual Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* AuraMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* EtherealParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* InnerLight;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* DynamicMaterial;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float TransformationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bRespondsToPlayerConsciousness = true;

public:
    // Character Evolution Functions
    UFUNCTION(BlueprintCallable, Category = "Transpersonal")
    void UpdateConsciousnessState(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal")
    void TransformToArchetype(ECharacterArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal")
    void ActivateEtherealForm(bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal")
    void SetAuraIntensity(float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Transpersonal")
    void OnConsciousnessStateChanged(EConsciousnessState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Transpersonal")
    void OnArchetypeTransformation(ECharacterArchetype NewArchetype);

protected:
    // Internal Functions
    void UpdateVisualEffects();
    void UpdateMaterialParameters();
    void UpdateLighting();
    void UpdateParticleEffects();

    // Consciousness System Integration
    UPROPERTY()
    class AConsciousnessSystem* ConsciousnessSystemRef;

    // Animation and Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bFloatingMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FloatAmplitude = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FloatFrequency = 1.0f;

    // Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanCommunicateTelepathically = true;

private:
    float InitialZPosition;
    float FloatTimer;
    FVector TargetScale;
    FLinearColor TargetAuraColor;
    float CurrentTransformationProgress;
};