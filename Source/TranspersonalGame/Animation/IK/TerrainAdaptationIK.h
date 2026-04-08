#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "TerrainAdaptationIK.generated.h"

/**
 * Sistema IK para adaptação a terreno irregular
 * Essencial para mundo pré-histórico com topografia variada
 */
USTRUCT(BlueprintType)
struct FFootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector FootLocation = FVector::ZeroVector;
    
    UPROPERTY(BlueprintReadWrite)
    FRotator FootRotation = FRotator::ZeroRotator;
    
    UPROPERTY(BlueprintReadWrite)
    float IKAlpha = 0.0f;
    
    UPROPERTY(BlueprintReadWrite)
    float DistanceFromGround = 0.0f;
    
    UPROPERTY(BlueprintReadWrite)
    bool bIsPlanted = false;
    
    FFootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsPlanted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTerrainAdaptationIK : public UObject
{
    GENERATED_BODY()

public:
    UTerrainAdaptationIK();

    // === CONFIGURAÇÃO ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float HipAdjustmentSpeed = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;
    
    // === DADOS IK ===
    
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FFootIKData LeftFootIK;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FFootIKData RightFootIK;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float HipOffset = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float CurrentHipOffset = 0.0f;
    
    // === FUNÇÕES PRINCIPAIS ===
    
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateTerrainIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FFootIKData CalculateFootIK(USkeletalMeshComponent* SkeletalMesh, 
                                const FName& FootBoneName, 
                                const FName& ThighBoneName,
                                float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetIKEnabled(bool bEnabled) { bIKEnabled = bEnabled; }
    
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    bool IsIKEnabled() const { return bIKEnabled; }

protected:
    UPROPERTY()
    bool bIKEnabled = true;
    
    // Cache para otimização
    UPROPERTY()
    float LastUpdateTime = 0.0f;
    
    UPROPERTY()
    FVector LastOwnerLocation = FVector::ZeroVector;

private:
    // Funções auxiliares
    FHitResult PerformFootTrace(const FVector& StartLocation, UWorld* World) const;
    float CalculateFootAlpha(float DistanceFromGround) const;
    FRotator CalculateFootRotation(const FVector& SurfaceNormal) const;
    void UpdateHipOffset(float LeftFootDistance, float RightFootDistance, float DeltaTime);
};