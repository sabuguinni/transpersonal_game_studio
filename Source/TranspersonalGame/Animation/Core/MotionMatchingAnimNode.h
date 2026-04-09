#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNode_Base.h"
#include "PoseSearch/PoseSearch.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "TranspersonalAnimationTypes.h"
#include "MotionMatchingAnimNode.generated.h"

/**
 * MOTION MATCHING ANIMATION NODE
 * 
 * Nó customizado de Motion Matching que integra personalidade
 * e comportamento específico para o mundo Jurássico.
 * 
 * Cada personagem tem uma "assinatura" de movimento única.
 */
USTRUCT(BlueprintInternalUseOnly)
struct TRANSPERSONALGAME_API FAnimNode_TranspersonalMotionMatching : public FAnimNode_Base
{
    GENERATED_BODY()

public:
    FAnimNode_TranspersonalMotionMatching();

    // === INPUTS ===
    
    /** Pose de entrada para blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (PinShownByDefault))
    FPoseLink Source;
    
    /** Database ativo de Motion Matching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> Database;
    
    /** Schema de configuração */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> Schema;
    
    /** Tempo de blend entre poses */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendTime = 0.2f;
    
    /** Intensidade do Motion Matching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MotionMatchingAlpha = 1.0f;

    // === PERSONALIDADE ===
    
    /** Tipo de personagem (afeta seleção de animações) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ECharacterArchetype CharacterType = ECharacterArchetype::Paleontologist;
    
    /** Estado emocional atual */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EEmotionalState EmotionalState = EEmotionalState::Confident;
    
    /** Modificadores de personalidade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FMovementPersonality PersonalityModifiers;

    // === COMPORTAMENTO DINÂMICO ===
    
    /** Se está em modo de sobrevivência (afeta tensão corporal) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bSurvivalMode = false;
    
    /** Nível de alerta (0.0 = relaxado, 1.0 = máximo alerta) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AlertnessLevel = 0.0f;
    
    /** Se há uma ameaça visível */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bThreatDetected = false;
    
    /** Direção da ameaça (para ajustar postura) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector ThreatDirection = FVector::ZeroVector;

    // === VARIAÇÕES FÍSICAS ===
    
    /** Variações únicas deste indivíduo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variations")
    FPhysicalVariations PhysicalTraits;
    
    /** Peso corporal relativo (afeta animações) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variations", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float BodyWeight = 1.0f;
    
    /** Altura relativa */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Variations", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BodyHeight = 1.0f;

    // === CONFIGURAÇÕES AVANÇADAS ===
    
    /** Usar blending baseado em velocidade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bUseVelocityBasedBlending = true;
    
    /** Threshold de velocidade para mudanças de animação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    float VelocityThreshold = 50.0f;
    
    /** Usar predição de movimento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bUseMovementPrediction = true;
    
    /** Tempo de predição em segundos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float PredictionTime = 0.5f;

    // === DEBUG ===
    
    /** Mostrar informações de debug */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugInfo = false;
    
    /** Cor do debug */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FColor DebugColor = FColor::Green;

public:
    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
    virtual void GatherDebugData(FNodeDebugData& DebugData) override;
    virtual bool HasPreUpdate() const override { return true; }
    virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;

protected:
    // Estado interno
    FPoseSearchQueryTrajector QueryTrajector;
    FPoseSearchResult LastSearchResult;
    float CurrentBlendWeight = 0.0f;
    float BlendTimeRemaining = 0.0f;
    
    // Cache de personalidade
    FMovementPersonality CachedPersonality;
    float PersonalityUpdateTimer = 0.0f;
    
    // Cache de comportamento
    float LastAlertness = 0.0f;
    bool bWasThreatDetected = false;
    FVector LastThreatDirection = FVector::ZeroVector;
    
    // Funções internas
    void UpdatePersonalityCache(const UAnimInstance* AnimInstance);
    void UpdateBehaviorCache(const UAnimInstance* AnimInstance);
    UPoseSearchDatabase* SelectOptimalDatabase() const;
    float CalculatePersonalityBlendTime() const;
    void ApplyPersonalityModifications(FPoseContext& Output) const;
    void ApplyPhysicalVariations(FPoseContext& Output) const;
    void DrawDebugInformation(const UAnimInstance* AnimInstance) const;
    
    // Pose Search helpers
    bool PerformPoseSearch(const UAnimInstance* AnimInstance, FPoseSearchResult& OutResult);
    void BuildQueryTrajector(const UAnimInstance* AnimInstance);
    float CalculateSearchCost(const FPoseSearchResult& Result) const;
    
    // Personalidade helpers
    float GetPersonalitySpeedModifier() const;
    float GetPersonalityWeightModifier() const;
    float GetEmotionalTensionModifier() const;
    FVector GetPosturalAdjustment() const;
    
    // Constantes
    static constexpr float PERSONALITY_UPDATE_FREQUENCY = 0.2f; // 5 updates per second
    static constexpr float MIN_BLEND_TIME = 0.05f;
    static constexpr float MAX_BLEND_TIME = 1.0f;
    static constexpr float THREAT_RESPONSE_SPEED = 5.0f;
};