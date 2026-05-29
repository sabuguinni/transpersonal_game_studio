#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_FootIKController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    float PelvisOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK")
    float IKAlpha;

    FAnim_FootIKData()
    {
        LeftFootOffset = FVector::ZeroVector;
        RightFootOffset = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        PelvisOffset = 0.0f;
        IKAlpha = 1.0f;
    }
};

/**
 * Controlador de IK para pés que adapta a posição dos pés ao terreno
 * Essencial para um jogo de sobrevivência onde o personagem caminha em terreno irregular
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_FootIKController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_FootIKController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração do sistema IK
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    bool bEnableFootIK;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    float TraceDistance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    float IKInterpSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    float CapsuleHalfHeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    FName RightFootSocketName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Foot IK Settings")
    FName PelvisSocketName;

    // Dados de IK calculados
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData FootIKData;

    // Métodos principais
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetFootIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData() const { return FootIKData; }

protected:
    // Métodos internos
    float GetFootOffsetFromGround(const FName& SocketName, float& OutRotation);
    FVector PerformFootTrace(const FVector& StartLocation, float& OutHitDistance, FRotator& OutSurfaceRotation);
    void UpdatePelvisOffset(float LeftFootOffset, float RightFootOffset);

private:
    // Cache de componentes
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Dados internos para interpolação
    float TargetPelvisOffset;
    float CurrentPelvisOffset;
    FVector TargetLeftFootOffset;
    FVector TargetRightFootOffset;
    FVector CurrentLeftFootOffset;
    FVector CurrentRightFootOffset;
    FRotator TargetLeftFootRotation;
    FRotator TargetRightFootRotation;
    FRotator CurrentLeftFootRotation;
    FRotator CurrentRightFootRotation;
};