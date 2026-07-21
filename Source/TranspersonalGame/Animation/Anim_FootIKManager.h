#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Anim_FootIKManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float HipOffset;

    FAnim_FootIKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

/**
 * Componente responsável por gerenciar IK dos pés para adaptação ao terreno
 * Realiza raycasting para detectar superfícies e ajustar posição/rotação dos pés
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_FootIKManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_FootIKManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CONFIGURAÇÕES DE IK ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float MaxHipAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bShowDebugTraces;

    // === NOMES DOS BONES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName;

    // === DADOS DE IK ===
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData IKData;

    // === FUNÇÕES PÚBLICAS ===
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData() const { return IKData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKSettings(float NewTraceDistance, float NewInterpSpeed, float NewFootHeight);

private:
    // === REFERÊNCIAS ===
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    // === DADOS INTERNOS ===
    FAnim_FootIKData TargetIKData;
    FAnim_FootIKData PreviousIKData;

    // === FUNÇÕES PRIVADAS ===
    void InitializeComponent();
    bool PerformFootTrace(const FVector& FootLocation, FVector& OutHitLocation, FVector& OutHitNormal);
    FVector GetFootWorldLocation(FName BoneName) const;
    FRotator CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation) const;
    void UpdateSingleFoot(FName BoneName, FVector& FootLocation, FRotator& FootRotation, float& IKAlpha, float DeltaTime);
    void CalculateHipOffset(float DeltaTime);
    void InterpolateIKData(float DeltaTime);
    void DrawDebugInfo() const;

    // === CONFIGURAÇÕES DE TRACE ===
    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TArray<AActor*> ActorsToIgnore;
};