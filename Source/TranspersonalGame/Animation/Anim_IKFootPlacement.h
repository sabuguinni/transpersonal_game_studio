#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Anim_IKFootPlacement.generated.h"

/**
 * Sistema de IK para posicionamento dos pés no terreno
 * Adapta a posição dos pés do personagem ao terreno irregular
 * Essencial para imersão em terrenos procedurais
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    // Posição do pé no espaço world
    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    // Rotação do pé para alinhar com a superfície
    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    // Offset aplicado ao pé (diferença da posição original)
    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float FootOffset;

    // Se o pé está em contacto com o solo
    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        FootOffset = 0.0f;
        bIsGrounded = false;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurações de IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;

    // Nomes dos bones dos pés
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName;

    // Dados dos pés
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootData;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootData;

    // Offset da pelvis para manter os pés no chão
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset;

    // Componentes cached
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetFootIK();

private:
    // Dados interpolados para suavizar movimento
    FAnim_FootIKData TargetLeftFootData;
    FAnim_FootIKData TargetRightFootData;
    float TargetPelvisOffset;

    // Métodos privados
    void CacheComponents();
    void PerformFootTrace(const FName& BoneName, FAnim_FootIKData& OutFootData);
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    void UpdatePelvisOffset();
    void InterpolateFootData(float DeltaTime);
    
    // Trace helpers
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
};