#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Animation/AnimInstance.h"
#include "MotionMatchingSystem.generated.h"

/**
 * Sistema de Motion Matching para personagens e dinossauros
 * Baseado na filosofia de que cada movimento conta uma história
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMotionMatchingSystem : public UAnimInstance
{
    GENERATED_BODY()

public:
    UMotionMatchingSystem();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // === CONFIGURAÇÃO MOTION MATCHING ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> PoseSearchSchema;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<TObjectPtr<UPoseSearchDatabase>> MotionDatabases;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime = 0.2f;
    
    // === ESTADOS EMOCIONAIS ===
    
    UPROPERTY(BlueprintReadWrite, Category = "Emotional State")
    float FearLevel = 0.0f; // 0.0 = calmo, 1.0 = terror
    
    UPROPERTY(BlueprintReadWrite, Category = "Emotional State")
    float TrustLevel = 0.0f; // Para domesticação
    
    UPROPERTY(BlueprintReadWrite, Category = "Emotional State")
    float AlertnessLevel = 0.5f; // Estado de alerta base
    
    // === PARÂMETROS DE MOVIMENTO ===
    
    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    FVector Velocity;
    
    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Speed;
    
    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Direction;
    
    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;
    
    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;
    
    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsRunning;
    
    // === SISTEMA DE PERSONALIDADE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Nervousness = 0.5f; // Afeta micro-movimentos
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Confidence = 0.5f; // Afeta postura
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity = 0.5f; // Afeta movimentos de cabeça
    
    // === FUNÇÕES BLUEPRINT ===
    
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetEmotionalState(float InFear, float InTrust, float InAlertness);
    
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMovementParameters(const FVector& InVelocity, bool bInCrouching, bool bInRunning);
    
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* SelectDatabase();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnEmotionalStateChanged(float Fear, float Trust, float Alertness);

private:
    // Cache para otimização
    UPROPERTY()
    TObjectPtr<UPoseSearchDatabase> CurrentDatabase;
    
    UPROPERTY()
    float LastDatabaseSwitch;
    
    // Histórico para suavização
    TArray<float> FearHistory;
    TArray<float> SpeedHistory;
    
    void UpdateEmotionalBlending(float DeltaTime);
    void SmoothParameters(float DeltaTime);
    float CalculateBlendTime() const;
};