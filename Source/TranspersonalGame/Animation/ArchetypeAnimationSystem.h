#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimInstance.h"
#include "Characters/CharacterArchetypes.h"
#include "JurassicAnimationAgent.h"
#include "MotionMatching/MotionMatchingController.h"
#include "IK/AdaptiveIKComponent.h"
#include "ArchetypeAnimationSystem.generated.h"

class UPoseSearchDatabase;
class UAnimSequence;
class UBlendSpace;
class UIKRigDefinition;
class UCharacterMovementComponent;
class UAnimMontage;
class UControlRig;

/**
 * ARCHETYPE ANIMATION SYSTEM - AGENTE #10
 * 
 * "Cada movimento conta uma história. Cada gesto revela uma alma."
 * 
 * Sistema avançado de animação que cria linguagem corporal única para cada arquétipo.
 * Inspirado pela filosofia de que a forma como um personagem se move define quem é
 * antes de qualquer palavra ser dita.
 * 
 * CARACTERÍSTICAS ÚNICAS:
 * - Motion Matching contextual baseado em personalidade e estado emocional
 * - IK adaptativo para terreno irregular do mundo Jurássico
 * - Linguagem corporal procedural que evolui com o estado de sobrevivência
 * - Micro-expressões que refletem stress, cansaço e medo
 * - Sistema de adaptação postural baseado em ferimentos e fadiga
 * - Gestos únicos por arquétipo que contam a história do personagem
 */

UENUM(BlueprintType)
enum class EAnimationLayer : uint8
{
    Base                UMETA(DisplayName = "Base Layer"),           // Locomotion principal
    Additive            UMETA(DisplayName = "Additive Layer"),       // Gestos e expressões
    Emotional           UMETA(DisplayName = "Emotional Layer"),      // Reações emocionais
    Survival            UMETA(DisplayName = "Survival Layer"),       // Adaptações de sobrevivência
    Injury              UMETA(DisplayName = "Injury Layer"),         // Compensações por ferimentos
    Fatigue             UMETA(DisplayName = "Fatigue Layer"),        // Efeitos de cansaço
    
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EGestureType : uint8
{
    // Gestos científicos (Paleontologista)
    ObservationPoint        UMETA(DisplayName = "Observation Point"),     // Apontar para observar
    TakeNotes              UMETA(DisplayName = "Take Notes"),             // Fazer anotações
    ExamineObject          UMETA(DisplayName = "Examine Object"),         // Examinar de perto
    MeasureDistance        UMETA(DisplayName = "Measure Distance"),       // Medir distâncias
    
    // Gestos de sobrevivência
    CheckSurroundings      UMETA(DisplayName = "Check Surroundings"),     // Verificar arredores
    ListenCarefully        UMETA(DisplayName = "Listen Carefully"),       // Escutar atentamente
    SignalDanger           UMETA(DisplayName = "Signal Danger"),          // Sinalizar perigo
    HideQuickly            UMETA(DisplayName = "Hide Quickly"),           // Esconder-se rapidamente
    
    // Gestos emocionais
    ShowFear               UMETA(DisplayName = "Show Fear"),              // Demonstrar medo
    ExpressAwe             UMETA(DisplayName = "Express Awe"),            // Expressar admiração
    ShowDetermination      UMETA(DisplayName = "Show Determination"),     // Mostrar determinação
    DisplayExhaustion      UMETA(DisplayName = "Display Exhaustion"),     // Mostrar exaustão
    
    // Gestos sociais (para NPCs)
    FriendlyWave           UMETA(DisplayName = "Friendly Wave"),          // Acenar amigavelmente
    CautiousApproach       UMETA(DisplayName = "Cautious Approach"),      // Aproximação cautelosa
    DefensiveStance        UMETA(DisplayName = "Defensive Stance"),       // Postura defensiva
    OfferHelp              UMETA(DisplayName = "Offer Help"),             // Oferecer ajuda
    
    MAX                    UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral             UMETA(DisplayName = "Neutral"),
    Curious             UMETA(DisplayName = "Curious"),
    Fearful             UMETA(DisplayName = "Fearful"),
    Determined          UMETA(DisplayName = "Determined"),
    Exhausted           UMETA(DisplayName = "Exhausted"),
    Hopeful             UMETA(DisplayName = "Hopeful"),
    Desperate           UMETA(DisplayName = "Desperate"),
    Amazed              UMETA(DisplayName = "Amazed"),
    Cautious            UMETA(DisplayName = "Cautious"),
    Confident           UMETA(DisplayName = "Confident"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Configuração de animação específica para cada arquétipo
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchetypeAnimationConfig
{
    GENERATED_BODY()

    FArchetypeAnimationConfig()
        : Archetype(ECharacterArchetype::Protagonist_Paleontologist)
        , BodyLanguage()
        , bUseMotionMatching(true)
        , bUseAdaptiveIK(true)
        , bUseProceduralGestures(true)
        , bUseEmotionalOverrides(true)
        , BlendTime(0.2f)
        , MotionMatchingWeight(1.0f)
        , IKWeight(1.0f)
        , GestureFrequency(0.5f)
        , EmotionalIntensity(0.7f)
    {}

    // Arquétipo de personagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ECharacterArchetype Archetype;

    // Configuração de linguagem corporal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    FArchetypeBodyLanguage BodyLanguage;

    // Databases de Motion Matching por estado emocional
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<EEmotionalState, TSoftObjectPtr<UPoseSearchDatabase>> EmotionalDatabases;

    // Databases por estado de sobrevivência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<ESurvivalState, TSoftObjectPtr<UPoseSearchDatabase>> SurvivalDatabases;

    // Configuração IK específica
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    TSoftObjectPtr<UIKRigDefinition> IKRigDefinition;

    // Animações específicas do arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TMap<EGestureType, TSoftObjectPtr<UAnimMontage>> GestureAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> IdleVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> WalkVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> RunVariations;

    // Blend Spaces específicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TSoftObjectPtr<UBlendSpace> EmotionalBlendSpace;

    // Control Rigs para expressões faciais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Animation")
    TSoftObjectPtr<UControlRig> FacialControlRig;

    // Configurações de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseAdaptiveIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseProceduralGestures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseEmotionalOverrides;

    // Parâmetros de blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.05", ClampMax = "1.0"))
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MotionMatchingWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float GestureFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalIntensity;
};

/**
 * Estado atual de animação de um personagem
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterAnimationState
{
    GENERATED_BODY()

    FCharacterAnimationState()
        : CurrentEmotionalState(EEmotionalState::Neutral)
        , CurrentSurvivalState(ESurvivalState::Fresh)
        , CurrentStressLevel(EStressLevel::Calm)
        , FatigueLevel(0.0f)
        , InjuryLevel(0.0f)
        , FearLevel(0.0f)
        , ConfidenceLevel(1.0f)
        , LastGestureTime(0.0f)
        , TimeSinceLastMovement(0.0f)
        , bIsInDanger(false)
        , bIsHiding(false)
        , bIsObserving(false)
    {}

    // Estados atuais
    UPROPERTY(BlueprintReadWrite, Category = "Current State")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadWrite, Category = "Current State")
    ESurvivalState CurrentSurvivalState;

    UPROPERTY(BlueprintReadWrite, Category = "Current State")
    EStressLevel CurrentStressLevel;

    // Níveis de condição (0-1)
    UPROPERTY(BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FatigueLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InjuryLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidenceLevel;

    // Timing para gestos procedurais
    UPROPERTY(BlueprintReadWrite, Category = "Timing")
    float LastGestureTime;

    UPROPERTY(BlueprintReadWrite, Category = "Timing")
    float TimeSinceLastMovement;

    // Flags de comportamento
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    bool bIsInDanger;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    bool bIsHiding;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    bool bIsObserving;
};

/**
 * Sistema principal de animação por arquétipo
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchetypeAnimationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Configuração de arquétipos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<ECharacterArchetype, FArchetypeAnimationConfig> ArchetypeConfigs;

    // Data Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
    TSoftObjectPtr<class UArchetypeAnimationDataAsset> AnimationDataAsset;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    void InitializeCharacterAnimation(class ACharacter* Character, ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    void UpdateCharacterAnimationState(class ACharacter* Character, const FCharacterAnimationState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    FArchetypeAnimationConfig GetArchetypeConfig(ECharacterArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    void TriggerContextualGesture(class ACharacter* Character, EGestureType GestureType);

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    void UpdateEmotionalState(class ACharacter* Character, EEmotionalState NewState, float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    void ApplyInjuryCompensation(class ACharacter* Character, float InjuryLevel, const FString& InjuryType);

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation")
    void ApplyFatigueEffects(class ACharacter* Character, float FatigueLevel);

    // Sistema de Motion Matching contextual
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UPoseSearchDatabase* SelectOptimalDatabase(class ACharacter* Character, const FCharacterAnimationState& State) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateBlendTimeForTransition(EEmotionalState FromState, EEmotionalState ToState) const;

    // Sistema de gestos procedurais
    UFUNCTION(BlueprintCallable, Category = "Procedural Gestures")
    bool ShouldTriggerProceduralGesture(class ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Procedural Gestures")
    EGestureType SelectAppropriateGesture(class ACharacter* Character, const FCharacterAnimationState& State) const;

    // Sistema de IK adaptativo
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void UpdateAdaptiveIK(class ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void CalculateFootIKTargets(class ACharacter* Character, FVector& LeftFootTarget, FVector& RightFootTarget) const;

    // Análise de personalidade através de movimento
    UFUNCTION(BlueprintCallable, Category = "Personality Analysis")
    void AnalyzeMovementPersonality(class ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Personality Analysis")
    FArchetypeBodyLanguage GenerateBodyLanguageFromPersonality(ECharacterArchetype Archetype, const FString& PersonalityTraits) const;

private:
    // Cache de estados de personagens
    UPROPERTY()
    TMap<TWeakObjectPtr<class ACharacter>, FCharacterAnimationState> CharacterStates;

    // Cache de configurações carregadas
    UPROPERTY()
    TMap<ECharacterArchetype, FArchetypeAnimationConfig> LoadedConfigs;

    // Timing para otimização
    float LastUpdateTime = 0.0f;
    float UpdateFrequency = 0.033f; // ~30fps para animações

    // Funções auxiliares
    void LoadArchetypeConfiguration(ECharacterArchetype Archetype);
    void UpdateCharacterBodyLanguage(class ACharacter* Character, const FArchetypeBodyLanguage& BodyLanguage, float DeltaTime);
    void ApplyEmotionalOverrides(class ACharacter* Character, EEmotionalState EmotionalState, float Intensity);
    void ProcessSurvivalAdaptations(class ACharacter* Character, ESurvivalState SurvivalState, float DeltaTime);
    float CalculateGestureFrequency(const FCharacterAnimationState& State, const FArchetypeBodyLanguage& BodyLanguage) const;
    bool IsValidGestureForArchetype(ECharacterArchetype Archetype, EGestureType GestureType) const;
};

/**
 * Data Asset para configurações de animação por arquétipo
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchetypeAnimationDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Configurações por arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype Configurations")
    TMap<ECharacterArchetype, FArchetypeAnimationConfig> ArchetypeAnimations;

    // Configurações de linguagem corporal por arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    TMap<ECharacterArchetype, FArchetypeBodyLanguage> BodyLanguageConfigs;

    // Mapeamento de gestos por arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gestures")
    TMap<ECharacterArchetype, TArray<EGestureType>> ArchetypeGestures;

    // Transições emocionais permitidas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Transitions")
    TMap<EEmotionalState, TArray<EEmotionalState>> AllowedEmotionalTransitions;

    // Funções de acesso
    UFUNCTION(BlueprintCallable, Category = "Archetype Animation Data")
    FArchetypeAnimationConfig GetAnimationConfigForArchetype(ECharacterArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation Data")
    FArchetypeBodyLanguage GetBodyLanguageForArchetype(ECharacterArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation Data")
    TArray<EGestureType> GetGesturesForArchetype(ECharacterArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Archetype Animation Data")
    bool CanTransitionToEmotionalState(EEmotionalState FromState, EEmotionalState ToState) const;
};