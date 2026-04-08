#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimInstance.h"
#include "Characters/CharacterArchetypes.h"
#include "Characters/JurassicCharacterArtist.h"
#include "MotionMatching/MotionMatchingController.h"
#include "IK/AdaptiveIKComponent.h"
#include "JurassicAnimationAgent.generated.h"

class UPoseSearchDatabase;
class UAnimSequence;
class UBlendSpace;
class UIKRigDefinition;
class UCharacterMovementComponent;
class UAnimMontage;
class UControlRig;

/**
 * JURASSIC ANIMATION AGENT - AGENTE #10
 * 
 * "Cada movimento é uma expressão de personalidade. Cada gesto é uma manifestação da alma."
 * 
 * Este sistema é inspirado pela filosofia de Richard Williams de que animação é ilusão de vida,
 * e pela equipa do RDR2 que descobriu que a forma como um personagem anda diz mais sobre quem é
 * do que qualquer diálogo.
 * 
 * Cada arquétipo tem uma linguagem corporal única que define quem é antes de qualquer palavra.
 * O paleontologista move-se com curiosidade científica.
 * O sobrevivente move-se com cautela constante.
 * O explorador move-se com confiança territorial.
 * 
 * CARACTERÍSTICAS ÚNICAS:
 * - Motion Matching contextual baseado em personalidade
 * - IK adaptativo para terreno irregular do Jurássico
 * - Linguagem corporal procedural baseada em stress/sobrevivência
 * - Micro-expressões que refletem o estado emocional
 * - Adaptação postural baseada em cansaço/ferimentos
 */

UENUM(BlueprintType)
enum class EBodyLanguageArchetype : uint8
{
    // Arquétipos principais com linguagem corporal específica
    ScientificCuriosity     UMETA(DisplayName = "Scientific Curiosity"),      // Paleontologista
    ConstantVigilance       UMETA(DisplayName = "Constant Vigilance"),        // Sobrevivente
    TerritorialConfidence   UMETA(DisplayName = "Territorial Confidence"),    // Explorador
    PredatoryFocus          UMETA(DisplayName = "Predatory Focus"),           // Caçador
    DiplomaticCalculation   UMETA(DisplayName = "Diplomatic Calculation"),    // Comerciante
    SpiritualCentering      UMETA(DisplayName = "Spiritual Centering"),       // Xamã
    MilitaryDiscipline      UMETA(DisplayName = "Military Discipline"),       // Guerreiro
    ArtisticPrecision       UMETA(DisplayName = "Artistic Precision"),        // Artesão
    NaturalAuthority        UMETA(DisplayName = "Natural Authority"),         // Líder
    DefensiveVulnerability  UMETA(DisplayName = "Defensive Vulnerability"),   // Refugiado
    
    MAX                     UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMovementPersonality : uint8
{
    // Personalidades de movimento que afetam como o personagem se move
    Cautious        UMETA(DisplayName = "Cautious"),        // Movimentos hesitantes, verifica constantemente
    Confident       UMETA(DisplayName = "Confident"),       // Movimentos decididos, postura ereta
    Nervous         UMETA(DisplayName = "Nervous"),         // Movimentos rápidos, inquietos
    Relaxed         UMETA(DisplayName = "Relaxed"),         // Movimentos fluídos, sem pressa
    Aggressive      UMETA(DisplayName = "Aggressive"),      // Movimentos bruscos, territoriais
    Graceful        UMETA(DisplayName = "Graceful"),        // Movimentos suaves, controlados
    Exhausted       UMETA(DisplayName = "Exhausted"),       // Movimentos lentos, postura curvada
    Hyperalert      UMETA(DisplayName = "Hyperalert"),      // Movimentos precisos, sempre pronto
    Contemplative   UMETA(DisplayName = "Contemplative"),   // Movimentos pensativos, pausas frequentes
    Spiritual       UMETA(DisplayName = "Spiritual"),       // Movimentos meditativos, centrados
    
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESurvivalState : uint8
{
    // Estados de sobrevivência que afetam a animação
    Fresh           UMETA(DisplayName = "Fresh"),           // Recém-chegado, ainda com energia
    Adapting        UMETA(DisplayName = "Adapting"),        // Aprendendo a sobreviver
    Experienced     UMETA(DisplayName = "Experienced"),     // Já sabe como sobreviver
    Hardened        UMETA(DisplayName = "Hardened"),        // Veterano da sobrevivência
    Desperate       UMETA(DisplayName = "Desperate"),       // Quase no limite
    Broken          UMETA(DisplayName = "Broken"),          // Perdeu a esperança
    
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EStressLevel : uint8
{
    Calm            UMETA(DisplayName = "Calm"),            // Relaxado, sem ameaças
    Aware           UMETA(DisplayName = "Aware"),           // Consciente do ambiente
    Alert           UMETA(DisplayName = "Alert"),           // Detectou algo suspeito
    Tense           UMETA(DisplayName = "Tense"),           // Pronto para reagir
    Panicked        UMETA(DisplayName = "Panicked"),        // Em pânico total
    
    MAX             UMETA(Hidden)
};

/**
 * Configuração de linguagem corporal específica para cada arquétipo
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchetypeBodyLanguage
{
    GENERATED_BODY()

    FArchetypeBodyLanguage()
        : Archetype(EBodyLanguageArchetype::ScientificCuriosity)
        , MovementPersonality(EMovementPersonality::Cautious)
        , PostureWeight(1.0f)
        , GestureFrequency(0.5f)
        , EyeContactLevel(0.7f)
        , PersonalSpaceRadius(150.0f)
        , MovementSpeed(1.0f)
        , TensionLevel(0.3f)
        , HeadTiltAngle(0.0f)
        , ShoulderTension(0.3f)
        , HandGestureStyle(0.5f)
        , BlinkRate(1.0f)
        , BreathingDepth(0.7f)
    {}

    // Arquétipo de linguagem corporal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    EBodyLanguageArchetype Archetype;

    // Personalidade de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EMovementPersonality MovementPersonality;

    // Parâmetros de postura (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PostureWeight;

    // Frequência de gestos (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gestures", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GestureFrequency;

    // Nível de contacto visual (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeContactLevel;

    // Raio de espaço pessoal (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float PersonalSpaceRadius;

    // Multiplicador de velocidade de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float MovementSpeed;

    // Nível de tensão corporal (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionLevel;

    // Micro-detalhes de linguagem corporal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Micro Details", meta = (ClampMin = "-30.0", ClampMax = "30.0"))
    float HeadTiltAngle; // Ângulo de inclinação da cabeça

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Micro Details", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShoulderTension; // Tensão nos ombros

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Micro Details", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HandGestureStyle; // Estilo dos gestos das mãos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Micro Details", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float BlinkRate; // Taxa de pestanejar

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Micro Details", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float BreathingDepth; // Profundidade da respiração
};

/**
 * Configuração de Motion Matching específica para cada arquétipo
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchetypeMotionData
{
    GENERATED_BODY()

    FArchetypeMotionData()
        : bUseMotionMatching(true)
        , bUseAdaptiveIK(true)
        , bUseProceduralGestures(true)
        , BlendTime(0.2f)
        , MotionMatchingWeight(1.0f)
        , IKWeight(1.0f)
    {}

    // Databases de Motion Matching específicos por estado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<ECharacterMovementState, TSoftObjectPtr<UPoseSearchDatabase>> MotionDatabases;

    // Configuração IK específica
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    TSoftObjectPtr<UIKRigDefinition> IKRigDefinition;

    // Animações específicas do arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimSequence>> GestureAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    TArray<TSoftObjectPtr<UAnimMontage>> EmotionalReactions;

    // Blend Spaces específicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TSoftObjectPtr<UBlendSpace> IdleBlendSpace;

    // Flags de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseAdaptiveIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bUseProceduralGestures;

    // Parâmetros de blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MotionMatchingWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKWeight;
};

/**
 * Estado de sobrevivência que afeta a animação
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSurvivalAnimationState
{
    GENERATED_BODY()

    FSurvivalAnimationState()
        : SurvivalState(ESurvivalState::Fresh)
        , StressLevel(EStressLevel::Calm)
        , HealthPercentage(1.0f)
        , FatigueLevel(0.0f)
        , HungerLevel(0.0f)
        , FearLevel(0.0f)
        , DaysInWild(0.0f)
        , bHasRecentTrauma(false)
        , bIsInjured(false)
        , bIsExhausted(false)
    {}

    // Estados principais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ESurvivalState SurvivalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    EStressLevel StressLevel;

    // Parâmetros físicos (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FatigueLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel;

    // Tempo de sobrevivência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    float DaysInWild;

    // Estados especiais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bHasRecentTrauma;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bIsInjured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bIsExhausted;

    // Lista de ferimentos específicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injuries")
    TArray<FString> ActiveInjuries;
};

/**
 * Data Asset que define toda a configuração de animação para um arquétipo
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchetypeAnimationData : public UDataAsset
{
    GENERATED_BODY()

public:
    UArchetypeAnimationData()
        : ArchetypeType(ECharacterArchetype::Protagonist_Paleontologist)
    {}

    // Arquétipo associado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype ArchetypeType;

    // Configuração de linguagem corporal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    FArchetypeBodyLanguage BodyLanguageConfig;

    // Dados de Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FArchetypeMotionData MotionData;

    // Overrides específicos de animação por contexto
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Overrides")
    TMap<FString, TSoftObjectPtr<UAnimSequence>> ContextualAnimations;

    // Control Rigs específicos para este arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Rig")
    TArray<TSoftObjectPtr<UControlRig>> ArchetypeControlRigs;
};

/**
 * Sistema principal de Animation Agent
 * Coordena Motion Matching, IK e linguagem corporal para criar personagens únicos
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicAnimationAgent : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Configuração principal de arquétipos
    UFUNCTION(BlueprintCallable, Category = "Animation Agent")
    void InitializeArchetypeAnimations();

    UFUNCTION(BlueprintCallable, Category = "Animation Agent")
    void SetupCharacterAnimation(ACharacter* Character, ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Animation Agent")
    void UpdateCharacterAnimation(ACharacter* Character, float DeltaTime);

    // Sistema de linguagem corporal
    UFUNCTION(BlueprintCallable, Category = "Body Language")
    void ApplyBodyLanguageToCharacter(ACharacter* Character, const FArchetypeBodyLanguage& BodyLanguage);

    UFUNCTION(BlueprintCallable, Category = "Body Language")
    void UpdateBodyLanguageBasedOnSurvival(ACharacter* Character, const FSurvivalAnimationState& SurvivalState);

    UFUNCTION(BlueprintCallable, Category = "Body Language")
    FArchetypeBodyLanguage GetBodyLanguageForArchetype(ECharacterArchetype Archetype);

    // Motion Matching avançado
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetOptimalDatabaseForContext(
        ECharacterArchetype Archetype, 
        ECharacterMovementState MovementState,
        const FSurvivalAnimationState& SurvivalState
    );

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingWeights(ACharacter* Character, const FArchetypeBodyLanguage& BodyLanguage);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateContextualBlendTime(
        EMovementPersonality Personality, 
        ECharacterMovementState FromState, 
        ECharacterMovementState ToState,
        EStressLevel StressLevel
    );

    // Sistema IK adaptativo
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetupAdaptiveIKForArchetype(ACharacter* Character, ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void UpdateTerrainAdaptationIK(ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void ApplyInjuryCompensationIK(ACharacter* Character, const TArray<FString>& Injuries);

    // Sistema de gestos procedurais
    UFUNCTION(BlueprintCallable, Category = "Procedural Gestures")
    void TriggerContextualGesture(ACharacter* Character, const FString& GestureContext);

    UFUNCTION(BlueprintCallable, Category = "Procedural Gestures")
    void UpdateIdleGestures(ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Procedural Gestures")
    void PlayEmotionalReaction(ACharacter* Character, const FString& EmotionType, float Intensity);

    // Sistema de sobrevivência e adaptação
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void UpdateSurvivalAnimationState(ACharacter* Character, const FSurvivalAnimationState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void ApplyFatigueToAnimation(ACharacter* Character, float FatigueLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void ApplyStressToAnimation(ACharacter* Character, EStressLevel StressLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void ApplyInjuryToAnimation(ACharacter* Character, const FString& InjuryType, float Severity);

    // Integração com Character Artist
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SynchronizeWithCharacterArtist(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ApplyVisualStoryToAnimation(ACharacter* Character, const FCharacterVisualStory& VisualStory);

    // Debugging e ferramentas
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugCharacterAnimation(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void VisualizeBodyLanguage(ACharacter* Character, bool bShowDebugInfo = true);

protected:
    // Dados de configuração dos arquétipos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<ECharacterArchetype, TSoftObjectPtr<UArchetypeAnimationData>> ArchetypeConfigurations;

    // Cache de dados carregados
    UPROPERTY(Transient)
    TMap<ECharacterArchetype, UArchetypeAnimationData*> LoadedArchetypeData;

    // Sistemas auxiliares
    UPROPERTY(Transient)
    class UMotionMatchingController* MotionMatchingController;

    UPROPERTY(Transient)
    class UAdaptiveIKComponent* IKController;

    // Timers para gestos procedurais
    UPROPERTY(Transient)
    TMap<ACharacter*, float> IdleGestureTimers;

    // Estados de sobrevivência ativos
    UPROPERTY(Transient)
    TMap<ACharacter*, FSurvivalAnimationState> CharacterSurvivalStates;

private:
    // Funções auxiliares internas
    void LoadArchetypeData(ECharacterArchetype Archetype);
    void SetupMotionMatchingForCharacter(ACharacter* Character, const FArchetypeMotionData& MotionData);
    void SetupIKForCharacter(ACharacter* Character, const FArchetypeMotionData& MotionData);
    void ApplyPersonalityToMovement(ACharacter* Character, EMovementPersonality Personality);
    void UpdateMicroExpressions(ACharacter* Character, const FArchetypeBodyLanguage& BodyLanguage, float DeltaTime);
    void CalculateProceduralPosture(ACharacter* Character, const FSurvivalAnimationState& SurvivalState);
    
    // Configurações padrão
    void InitializeDefaultArchetypeConfigurations();
    FArchetypeBodyLanguage CreateDefaultBodyLanguageForArchetype(ECharacterArchetype Archetype);
    FArchetypeMotionData CreateDefaultMotionDataForArchetype(ECharacterArchetype Archetype);
};