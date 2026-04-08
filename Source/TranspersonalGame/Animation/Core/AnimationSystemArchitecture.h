#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

/**
 * TRANSPERSONAL GAME STUDIO — ANIMATION SYSTEM ARCHITECTURE
 * 
 * Sistema de animação híbrido para jogo de sobrevivência pré-histórico.
 * Combina Motion Matching para fluidez com IK para adaptação ao terreno.
 * 
 * PRINCÍPIOS FUNDAMENTAIS:
 * 1. VULNERABILIDADE — Jogador sempre parece frágil comparado aos dinossauros
 * 2. PESO FÍSICO — Cada movimento tem consequência e inércia
 * 3. ADAPTAÇÃO — IK de pés adapta a terreno irregular
 * 4. INDIVIDUALIDADE — Cada criatura tem linguagem corporal única
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // JOGADOR
    Paleontologist      UMETA(DisplayName = "Paleontologist - Vulnerable Human"),
    
    // DINOSSAUROS PEQUENOS (domesticáveis)
    Compsognathus      UMETA(DisplayName = "Compsognathus - Skittish Scavenger"),
    Dryosaurus         UMETA(DisplayName = "Dryosaurus - Cautious Herbivore"),
    Hypsilophodon      UMETA(DisplayName = "Hypsilophodon - Alert Runner"),
    
    // DINOSSAUROS MÉDIOS (neutros/territoriais)
    Parasaurolophus    UMETA(DisplayName = "Parasaurolophus - Gentle Giant"),
    Iguanodon          UMETA(DisplayName = "Iguanodon - Defensive Herbivore"),
    Dilophosaurus      UMETA(DisplayName = "Dilophosaurus - Cunning Hunter"),
    
    // DINOSSAUROS GRANDES (apex predators)
    Allosaurus         UMETA(DisplayName = "Allosaurus - Territorial Predator"),
    Tyrannosaurus      UMETA(DisplayName = "Tyrannosaurus - Apex Predator"),
    Spinosaurus        UMETA(DisplayName = "Spinosaurus - Aquatic Hunter")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle               UMETA(DisplayName = "Idle"),
    Walking            UMETA(DisplayName = "Walking"),
    Running            UMETA(DisplayName = "Running"),
    Sprinting          UMETA(DisplayName = "Sprinting - Panic Mode"),
    Crouching          UMETA(DisplayName = "Crouching - Stealth"),
    Crawling           UMETA(DisplayName = "Crawling - Hide"),
    Climbing           UMETA(DisplayName = "Climbing"),
    Swimming           UMETA(DisplayName = "Swimming"),
    Falling            UMETA(DisplayName = "Falling"),
    Landing            UMETA(DisplayName = "Landing"),
    Dodging            UMETA(DisplayName = "Dodging"),
    Stunned            UMETA(DisplayName = "Stunned - Fear Response")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm               UMETA(DisplayName = "Calm"),
    Alert              UMETA(DisplayName = "Alert"),
    Nervous            UMETA(DisplayName = "Nervous"),
    Afraid             UMETA(DisplayName = "Afraid"),
    Panicked           UMETA(DisplayName = "Panicked"),
    Exhausted          UMETA(DisplayName = "Exhausted"),
    Injured            UMETA(DisplayName = "Injured"),
    Confident          UMETA(DisplayName = "Confident - Rare State")
};

/**
 * Estrutura que define a linguagem corporal única de cada arquétipo
 */
USTRUCT(BlueprintType)
struct FBodyLanguageProfile
{
    GENERATED_BODY()

    // Postura base
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpineFlexion = 0.0f; // -1.0 (curvado/submisso) a 1.0 (ereto/dominante)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShoulderTension = 0.0f; // 0.0 (relaxado) a 1.0 (tenso)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeadPosition = 0.0f; // -1.0 (baixo/submisso) a 1.0 (alto/alerta)
    
    // Dinâmica de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementHesitation = 0.0f; // 0.0 (fluído) a 1.0 (hesitante)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StepCaution = 0.0f; // 0.0 (confiante) a 1.0 (cauteloso)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnvironmentalAwareness = 1.0f; // 0.0 (desatento) a 1.0 (hipervigilante)
    
    // Respiração e micro-movimentos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BreathingIntensity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IdleFidgeting = 0.3f; // Pequenos movimentos nervosos
    
    FBodyLanguageProfile()
    {
        SpineFlexion = 0.0f;
        ShoulderTension = 0.0f;
        HeadPosition = 0.0f;
        MovementHesitation = 0.0f;
        StepCaution = 0.0f;
        EnvironmentalAwareness = 1.0f;
        BreathingIntensity = 0.5f;
        IdleFidgeting = 0.3f;
    }
};

/**
 * Sistema de variação procedural para dinossauros únicos
 */
USTRUCT(BlueprintType)
struct FDinosaurVariationData
{
    GENERATED_BODY()

    // Variações físicas que afectam animação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LimbLengthVariation = 1.0f; // 0.8 a 1.2 — afecta stride length
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpineFlexibilityVariation = 1.0f; // 0.7 a 1.3 — afecta curvatura
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeightDistribution = 0.5f; // 0.0 (front-heavy) a 1.0 (back-heavy)
    
    // Personalidade que afecta movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggressiveness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Skittishness = 0.5f;
    
    // Idade e condição física
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Age = 0.5f; // 0.0 (jovem) a 1.0 (velho)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealthCondition = 1.0f; // 0.0 (ferido) a 1.0 (saudável)
    
    FDinosaurVariationData()
    {
        LimbLengthVariation = 1.0f;
        SpineFlexibilityVariation = 1.0f;
        WeightDistribution = 0.5f;
        Aggressiveness = 0.5f;
        Curiosity = 0.5f;
        Skittishness = 0.5f;
        Age = 0.5f;
        HealthCondition = 1.0f;
    }
};

/**
 * Classe base para todos os Animation Blueprints do jogo
 */
UCLASS(Abstract, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

protected:
    // === CORE ANIMATION DATA ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ECharacterArchetype CharacterArchetype;
    
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EMovementState CurrentMovementState;
    
    UPROPERTY(BlueprintReadOnly, Category = "Emotion")
    EEmotionalState CurrentEmotionalState;
    
    UPROPERTY(BlueprintReadOnly, Category = "Body Language")
    FBodyLanguageProfile BodyLanguage;
    
    UPROPERTY(BlueprintReadOnly, Category = "Variation", meta = (ShowOnlyInnerProperties))
    FDinosaurVariationData VariationData;
    
    // === MOTION MATCHING DATA ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Speed;
    
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Direction;
    
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Velocity;
    
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Acceleration;
    
    // === IK DATA ===
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bEnableFootIK;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float PelvisIKOffset;

public:
    // === BLUEPRINT INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void SetCharacterArchetype(ECharacterArchetype NewArchetype);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void SetEmotionalState(EEmotionalState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void ApplyVariationData(const FDinosaurVariationData& NewVariation);
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    virtual FBodyLanguageProfile GetBodyLanguageForArchetype(ECharacterArchetype Archetype) const;
    
    // === ANIMATION EVENTS ===
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMovementStateChanged(EMovementState OldState, EMovementState NewState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnEmotionalStateChanged(EEmotionalState OldState, EEmotionalState NewState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnDangerDetected(float ThreatLevel);

protected:
    // === CORE UPDATES ===
    
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void UpdateMovementData();
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void UpdateIKData();
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    virtual void UpdateEmotionalResponse();
    
    // === HELPER FUNCTIONS ===
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    float CalculateSpeedFromVelocity() const;
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    float CalculateDirectionFromVelocity() const;
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    EMovementState DetermineMovementState() const;
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    float ApplyBodyLanguageModifier(float BaseValue, float Modifier) const;

private:
    // Cache para performance
    UPROPERTY()
    class ACharacter* OwningCharacter;
    
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;
    
    // Timers para emotional state
    float EmotionalStateTimer;
    float LastDangerDetectionTime;
};