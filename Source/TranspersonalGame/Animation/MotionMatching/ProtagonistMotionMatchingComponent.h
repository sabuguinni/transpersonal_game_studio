#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearch.h"
#include "Engine/Engine.h"
#include "ProtagonistMotionMatchingComponent.generated.h"

/**
 * Componente especializado de Motion Matching para o protagonista paleontologista
 * 
 * Implementa a filosofia de Richard Williams: cada movimento tem peso e história
 * Baseado na técnica do RDR2: a forma como o Arthur Morgan anda define quem ele é
 * 
 * O protagonista move-se como um académico cauteloso, não como um herói de acção:
 * - Passos deliberados e observadores
 * - Postura ligeiramente curvada (anos debruçado sobre fósseis)
 * - Movimentos de cabeça frequentes (sempre alerta ao perigo)
 * - Gestos de mão precisos (habituado a trabalho delicado)
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProtagonistMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UProtagonistMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === MOTION MATCHING CONFIGURATION ===
    
    /** Schema de Motion Matching específico para o protagonista */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* ProtagonistSchema;
    
    /** Database principal - movimentos normais */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* NormalMovementDatabase;
    
    /** Database de medo - movimentos quando ameaçado */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* FearMovementDatabase;
    
    /** Database científica - movimentos de investigação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* ScientificMovementDatabase;

    // === PERSONALITY TRAITS ===
    
    /** Nível de cautela (0.0 = despreocupado, 1.0 = extremamente cauteloso) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CautiousnesLevel = 0.7f;
    
    /** Nível de curiosidade científica (afecta movimentos de investigação) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScientificCuriosity = 0.8f;
    
    /** Fadiga física (aumenta ao longo do tempo, afecta postura) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PhysicalFatigue = 0.0f;
    
    /** Nível de stress/medo actual */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentFearLevel = 0.0f;

    // === MOVEMENT CHARACTERISTICS ===
    
    /** Velocidade base de caminhada (mais lenta que um herói típico) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseWalkSpeed = 120.0f;
    
    /** Velocidade máxima de corrida (não-atlética) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxRunSpeed = 350.0f;
    
    /** Frequência de movimentos de cabeça (olhar em volta) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float HeadLookFrequency = 2.5f;
    
    /** Intensidade da postura curvada */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SlouchedPostureIntensity = 0.3f;

public:
    // === ANIMATION FUNCTIONS ===
    
    /** Actualiza o estado emocional e selecciona a database apropriada */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateEmotionalState(float DeltaTime);
    
    /** Calcula o blend weight baseado no contexto actual */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateContextualBlendWeight();
    
    /** Aplica modificadores de personalidade às animações */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ApplyPersonalityModifiers(class UAnimInstance* AnimInstance);
    
    /** Detecta proximidade de dinossauros e ajusta comportamento */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateThreatAwareness();
    
    /** Activa modo científico (ao investigar fósseis/plantas) */
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ActivateScientificMode(bool bActivate);

protected:
    // === INTERNAL FUNCTIONS ===
    
    /** Inicializa o sistema de Motion Matching */
    void InitializeMotionMatchingSystem();
    
    /** Actualiza trajectory baseado no estado emocional */
    void UpdateTrajectoryPrediction(float DeltaTime);
    
    /** Calcula modificadores de postura baseados na fadiga */
    void CalculatePostureModifiers();
    
    /** Detecta superfícies científicas interessantes */
    void DetectScientificInterests();

private:
    // === RUNTIME DATA ===
    
    /** Referência ao Motion Matching node activo */
    UPROPERTY()
    class UMotionMatchingAnimInstance* ActiveMotionMatchingInstance;
    
    /** Database actualmente seleccionada */
    UPROPERTY()
    class UPoseSearchDatabase* CurrentActiveDatabase;
    
    /** Timer para movimentos de cabeça */
    float HeadLookTimer = 0.0f;
    
    /** Última posição conhecida de ameaça */
    FVector LastThreatPosition = FVector::ZeroVector;
    
    /** Estado científico activo */
    bool bScientificModeActive = false;
    
    /** Tempo em modo científico */
    float ScientificModeTime = 0.0f;
    
    /** Cache de dinossauros próximos */
    UPROPERTY()
    TArray<AActor*> NearbyDinosaurs;
    
    /** Modificadores de postura actuais */
    FVector CurrentPostureOffset = FVector::ZeroVector;
    
    /** Histórico de posições para trajectory prediction */
    TArray<FVector> PositionHistory;
    
    /** Máximo de posições no histórico */
    static constexpr int32 MaxPositionHistory = 10;
};

/** Estrutura para dados de trajectory personalizada */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProtagonistTrajectoryData
{
    GENERATED_BODY()

    /** Posição prevista */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PredictedPosition = FVector::ZeroVector;
    
    /** Velocidade prevista */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PredictedVelocity = FVector::ZeroVector;
    
    /** Direcção de facing prevista */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PredictedFacing = FVector::ForwardVector;
    
    /** Nível de cautela previsto */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictedCautiousness = 0.0f;
    
    /** Timestamp desta previsão */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;
};