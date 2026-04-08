#pragma once

#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "PoseSearchSchema_PlayerLocomotion.generated.h"

/**
 * Schema de Motion Matching especializado para locomoção do paleontologista
 * Configurado para capturar movimentos naturais e responsivos
 * com foco em transmitir vulnerabilidade e tensão
 */
UCLASS(BlueprintType, Category = "Transpersonal Game|Animation")
class TRANSPERSONALGAME_API UPoseSearchSchema_PlayerLocomotion : public UPoseSearchSchema
{
    GENERATED_BODY()

public:
    UPoseSearchSchema_PlayerLocomotion();

protected:
    /** Configura o canal de trajetória para movimento do jogador */
    void SetupTrajectoryChannel();
    
    /** Configura o canal de pose para pés e quadril */
    void SetupPoseChannel();
    
    /** Configura o canal de velocidade para movimento do root bone */
    void SetupVelocityChannel();

public:
    /** Peso específico para movimentos de furtividade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth Movement")
    float StealthMovementWeight = 1.5f;
    
    /** Peso específico para movimentos de pânico/fuga */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Panic Movement")
    float PanicMovementWeight = 1.3f;
    
    /** Peso específico para movimentos cautelosos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cautious Movement")
    float CautiousMovementWeight = 1.2f;
};