#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "../Core/ConsciousnessSystem.h"
#include "BaseCharacterController.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    SpiritualGuide      UMETA(DisplayName = "Spiritual Guide"),
    ShadowAspect        UMETA(DisplayName = "Shadow Aspect"),
    WisdomKeeper        UMETA(DisplayName = "Wisdom Keeper"),
    InnerChild          UMETA(DisplayName = "Inner Child"),
    HigherSelf          UMETA(DisplayName = "Higher Self")
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Dormant             UMETA(DisplayName = "Dormant"),
    Awakening           UMETA(DisplayName = "Awakening"),
    Active              UMETA(DisplayName = "Active"),
    Transforming        UMETA(DisplayName = "Transforming"),
    Transcendent        UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct FCharacterVisualProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float Opacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor AuraColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float AuraIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float EnergyFlowRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowGeometricPatterns = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float PatternComplexity = 0.3f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABaseCharacterController : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseCharacterController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterArchetype Archetype = ECharacterArchetype::SpiritualGuide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterState CurrentState = ECharacterState::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterDescription;

    // Visual Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    class UStaticMeshComponent* AuraMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    class UParticleSystemComponent* EnergyParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    class UPointLightComponent* CharacterLight;

    // Material Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* AuraMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* DynamicMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* DynamicAuraMaterial;

    // Visual Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FCharacterVisualProperties VisualProperties;

    // Consciousness Integration
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    class AConsciousnessSystem* ConsciousnessSystem;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetCharacterState(ECharacterState NewState);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateVisualProperties(const FCharacterVisualProperties& NewProperties);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void RespondToConsciousnessLevel(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void TriggerTransformation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void OnStateChanged(ECharacterState OldState, ECharacterState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void OnConsciousnessResponse(float Level);

protected:
    UFUNCTION()
    void UpdateMaterialParameters();

    UFUNCTION()
    void UpdateLighting();

    UFUNCTION()
    void UpdateParticleEffects();

    // State transition timers
    float StateTransitionTime = 0.0f;
    float MaxTransitionDuration = 2.0f;

    // Consciousness response parameters
    float LastConsciousnessLevel = 0.0f;
    float ConsciousnessResponseSensitivity = 1.0f;
};