#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "AnimationSystemArchitecture.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO — ANIMATION SYSTEM ARCHITECTURE
 * 
 * Sistema de animação baseado em Motion Matching para criar personagens
 * que se movem como seres humanos reais — com peso, intenção e história.
 * 
 * Inspirado em:
 * - Richard Williams: "Animação é ilusão de vida"
 * - RDR2: "A forma como um personagem anda diz mais sobre quem é do que qualquer diálogo"
 * 
 * PRINCÍPIOS FUNDAMENTAIS:
 * 1. Cada movimento tem propósito — sem movimentos genéricos
 * 2. Peso e resistência em cada gesto
 * 3. História acumulada no corpo de cada personagem
 * 4. Linguagem corporal única que define antes de qualquer palavra
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // PROTAGONISTA
    Paleontologist      UMETA(DisplayName = "Paleontólogo - Protagonista"),
    
    // DINOSSAUROS HERBÍVOROS (Domesticáveis)
    SmallHerbivore      UMETA(DisplayName = "Herbívoro Pequeno"),
    MediumHerbivore     UMETA(DisplayName = "Herbívoro Médio"),
    LargeHerbivore      UMETA(DisplayName = "Herbívoro Grande"),
    
    // DINOSSAUROS CARNÍVOROS (Predadores)
    SmallCarnivore      UMETA(DisplayName = "Carnívoro Pequeno"),
    MediumCarnivore     UMETA(DisplayName = "Carnívoro Médio"),
    LargeCarnivore      UMETA(DisplayName = "Carnívoro Grande"),
    ApexPredator        UMETA(DisplayName = "Predador Apex"),
    
    // ESTADOS ESPECIAIS
    Domesticated        UMETA(DisplayName = "Domesticado"),
    Injured             UMETA(DisplayName = "Ferido"),
    Aggressive          UMETA(DisplayName = "Agressivo"),
    Fearful             UMETA(DisplayName = "Amedrontado")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    // ESTADOS BASE
    Idle                UMETA(DisplayName = "Parado"),
    Walk                UMETA(DisplayName = "Caminhar"),
    Run                 UMETA(DisplayName = "Correr"),
    Sprint              UMETA(DisplayName = "Sprint"),
    
    // ESTADOS DE SOBREVIVÊNCIA
    Sneak               UMETA(DisplayName = "Furtivo"),
    Crouch              UMETA(DisplayName = "Agachado"),
    Crawl               UMETA(DisplayName = "Rastejando"),
    Hide                UMETA(DisplayName = "Escondido"),
    
    // ESTADOS DE INTERAÇÃO
    Gathering           UMETA(DisplayName = "Recolhendo"),
    Building            UMETA(DisplayName = "Construindo"),
    Crafting            UMETA(DisplayName = "Criando"),
    
    // ESTADOS DE COMBATE/PERIGO
    Alert               UMETA(DisplayName = "Alerta"),
    Fleeing             UMETA(DisplayName = "Fugindo"),
    Defending           UMETA(DisplayName = "Defendendo"),
    
    // ESTADOS DINOSSAUROS
    Hunting             UMETA(DisplayName = "Caçando"),
    Feeding             UMETA(DisplayName = "Alimentando"),
    Resting             UMETA(DisplayName = "Descansando"),
    Socializing         UMETA(DisplayName = "Socializando"),
    Patrolling          UMETA(DisplayName = "Patrulhando"),
    Investigating       UMETA(DisplayName = "Investigando")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    // ESTADOS EMOCIONAIS QUE AFETAM MOVIMENTO
    Confident           UMETA(DisplayName = "Confiante"),
    Nervous             UMETA(DisplayName = "Nervoso"),
    Exhausted           UMETA(DisplayName = "Exausto"),
    Determined          UMETA(DisplayName = "Determinado"),
    Panicked            UMETA(DisplayName = "Em Pânico"),
    Curious             UMETA(DisplayName = "Curioso"),
    Aggressive          UMETA(DisplayName = "Agressivo"),
    Submissive          UMETA(DisplayName = "Submisso"),
    Protective          UMETA(DisplayName = "Protetor"),
    Playful             UMETA(DisplayName = "Brincalhão")
};

/**
 * Estrutura que define a personalidade de movimento de cada personagem
 * Cada personagem tem uma assinatura única de movimento
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMovementPersonality
{
    GENERATED_BODY()

    // CARACTERÍSTICAS FÍSICAS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.5, ClampMax = 2.0))
    float BaseSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.5, ClampMax = 2.0))
    float StepLength = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.1, ClampMax = 3.0))
    float BodyWeight = 1.0f;
    
    // CARACTERÍSTICAS COMPORTAMENTAIS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Confidence = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Nervousness = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Aggressiveness = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Cautiousness = 0.7f;
    
    // VARIAÇÕES ÚNICAS (Para dinossauros)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LeftLegLength = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RightLegLength = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpineFlexibility = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeadWeight = 1.0f;
    
    // HISTÓRIA ACUMULADA
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float BattleScars = 0.0f;  // Afeta postura e movimento
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Age = 0.5f;  // Jovem vs Velho
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float DomesticationLevel = 0.0f;  // Para dinossauros domesticados
};

/**
 * Componente principal do sistema de animação
 * Gerencia Motion Matching, IK e personalidade de movimento
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTranspersonalAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalAnimationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // CONFIGURAÇÃO DE PERSONAGEM
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    ECharacterArchetype CharacterArchetype = ECharacterArchetype::Paleontologist;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    FMovementPersonality MovementPersonality;
    
    // MOTION MATCHING
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* InteractionDatabase;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* CombatDatabase;
    
    // ESTADOS ATUAIS
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EMovementState CurrentMovementState = EMovementState::Idle;
    
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEmotionalState CurrentEmotionalState = EEmotionalState::Confident;
    
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentSpeed = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentDirection = 0.0f;
    
    // FUNÇÕES PÚBLICAS
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EMovementState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EEmotionalState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ApplyInjury(float Severity, FName BoneName);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void IncreaseDomestication(float Amount);
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    FVector GetFootPlantTarget(bool bLeftFoot) const;
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementWeight() const;

private:
    // SISTEMA IK
    void UpdateFootIK();
    void UpdateSpineIK();
    
    // PERSONALIDADE
    void ApplyPersonalityToMovement();
    void UpdateEmotionalInfluence();
    
    // TERRAIN ADAPTATION
    FVector LastLeftFootPosition;
    FVector LastRightFootPosition;
    float FootIKAlpha = 1.0f;
};

/**
 * Interface para objetos que podem ser observados pelos dinossauros
 * Implementado pelo protagonista e outros elementos do mundo
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UObservableTarget : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API IObservableTarget
{
    GENERATED_BODY()

public:
    // Retorna o nível de ameaça percebido (0.0 = inofensivo, 1.0 = ameaça máxima)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Observation")
    float GetThreatLevel() const;
    
    // Retorna se o alvo está se movendo furtivamente
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Observation")
    bool IsSneaking() const;
    
    // Retorna a distância de detecção baseada no comportamento atual
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Observation")
    float GetDetectionRadius() const;
    
    // Retorna se o alvo tem comida ou é comida
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Observation")
    bool IsFood() const;
};