#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "AnimationSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimationStateChanged, FName, NewState, FName, PreviousState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMotionMatchingQueryComplete, float, MatchQuality);

/**
 * Sistema central de gestão de animação
 * Coordena Motion Matching, IK adaptativo e linguagem corporal procedural
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === MOTION MATCHING CORE ===
    
    /** Base Pose Search Schema para locomotion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> LocomotionSchema;
    
    /** Database principal de animações de movimento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> PrimaryLocomotionDatabase;
    
    /** Database de animações de interação com ambiente */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> EnvironmentInteractionDatabase;
    
    /** Database de animações de stress/medo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> StressStateDatabase;

    // === ADAPTIVE IK SYSTEM ===
    
    /** Intensidade do IK de pés para adaptação ao terreno */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive IK", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FootIKIntensity = 1.0f;
    
    /** Distância máxima de rastreamento do solo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive IK")
    float GroundTraceDistance = 50.0f;
    
    /** Velocidade de interpolação do IK */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive IK")
    float IKInterpolationSpeed = 10.0f;

    // === BODY LANGUAGE SYSTEM ===
    
    /** Nível atual de stress do personagem (0.0 = calmo, 1.0 = pânico) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StressLevel = 0.0f;
    
    /** Nível de fadiga (afeta postura e velocidade) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FatigueLevel = 0.0f;
    
    /** Personalidade do personagem (afeta animações idle e reações) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Language")
    FName PersonalityArchetype = "Cautious";

    // === DINOSSAURO UNIQUE VARIATIONS ===
    
    /** Seed para variações procedurais de movimento */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Variations")
    int32 MovementVariationSeed = 0;
    
    /** Multiplicador de escala para ajustar animações baseado no tamanho */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Variations")
    float SizeScaleMultiplier = 1.0f;
    
    /** Tipo de personalidade do dinossauro (Aggressive, Passive, Curious, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Variations")
    FName DinosaurPersonality = "Neutral";

    // === EVENTOS ===
    
    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnAnimationStateChanged OnAnimationStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnMotionMatchingQueryComplete OnMotionMatchingQueryComplete;

    // === FUNÇÕES PÚBLICAS ===
    
    /** Actualiza o estado de stress baseado em factores externos */
    UFUNCTION(BlueprintCallable, Category = "Body Language")
    void UpdateStressLevel(float NewStressLevel, float TransitionSpeed = 2.0f);
    
    /** Aplica variação procedural baseada no seed */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Variations")
    void ApplyMovementVariation(int32 NewSeed);
    
    /** Obtém a database apropriada baseada no estado atual */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetCurrentDatabase() const;
    
    /** Força transição para estado específico */
    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void ForceTransitionToState(FName StateName);

private:
    // Estado interno
    FName CurrentAnimationState = "Idle";
    FName PreviousAnimationState = "None";
    float CurrentStressTransition = 0.0f;
    float TargetStressLevel = 0.0f;
    
    // Timers internos
    float LastMotionMatchingQuery = 0.0f;
    float MotionMatchingQueryInterval = 0.1f; // 10 queries por segundo
    
    // Funções internas
    void UpdateStressTransition(float DeltaTime);
    void ProcessMotionMatchingQuery();
    UPoseSearchDatabase* SelectDatabaseByState() const;
};