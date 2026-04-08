#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "TerrainAdaptationIK.generated.h"

class USkeletalMeshComponent;

/**
 * Sistema de IK para adaptação automática dos pés ao terreno irregular
 * Garante que o paleontologista sempre tenha contato realista com o solo
 * independente da inclinação ou irregularidade do terreno
 */
UCLASS(BlueprintType, Category = "Transpersonal Game|Animation|IK")
class TRANSPERSONALGAME_API UTerrainAdaptationIK : public UObject
{
    GENERATED_BODY()

public:
    UTerrainAdaptationIK();

    /** Atualiza o sistema de IK para ambos os pés */
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateFootIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime);
    
    /** Obtém a localização IK para um pé específico */
    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    FVector GetFootIKLocation(const FName& FootBoneName, USkeletalMeshComponent* SkeletalMesh);
    
    /** Obtém o offset do quadril */
    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    float GetHipOffset() const;
    
    /** Ativa/desativa debug drawing */
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetDebugDrawing(bool bEnabled);

protected:
    /** Atualiza IK para um pé individual */
    void UpdateSingleFootIK(USkeletalMeshComponent* SkeletalMesh, 
        const FName& FootBoneName, const FName& IKBoneName, 
        float& FootOffset, float DeltaTime);
    
    /** Atualiza ajuste do quadril baseado na posição dos pés */
    void UpdateHipAdjustment(USkeletalMeshComponent* SkeletalMesh, float DeltaTime);

public:
    /** Distância máxima que o pé pode ser ajustado */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;
    
    /** Distância do trace para detectar o chão */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootTraceDistance;
    
    /** Velocidade de interpolação do IK */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;
    
    /** Proporção de ajuste do quadril baseado na diferença dos pés */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HipAdjustmentRatio;

protected:
    /** Nomes dos bones */
    UPROPERTY(EditAnywhere, Category = "Bone Names")
    FName LeftFootBoneName;
    
    UPROPERTY(EditAnywhere, Category = "Bone Names")
    FName RightFootBoneName;
    
    UPROPERTY(EditAnywhere, Category = "Bone Names")
    FName LeftIKBoneName;
    
    UPROPERTY(EditAnywhere, Category = "Bone Names")
    FName RightIKBoneName;
    
    UPROPERTY(EditAnywhere, Category = "Bone Names")
    FName HipBoneName;
    
    /** Offsets atuais dos pés */
    float LeftFootIKOffset = 0.0f;
    float RightFootIKOffset = 0.0f;
    float HipIKOffset = 0.0f;
    
    /** Debug drawing */
    bool bEnableDebugDrawing;
};