#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootSocketName;

    // IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float HipOffset;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData(bool bIsLeftFoot) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetHipOffset() const { return HipOffset; }

private:
    // Character reference
    UPROPERTY()
    class ACharacter* OwningCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;

    // Internal functions
    FAnim_FootIKData CalculateFootIK(const FName& FootSocketName, float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootSocketName) const;
    bool LineTraceFromFoot(const FVector& FootLocation, FHitResult& HitResult) const;
    FRotator CalculateFootRotation(const FHitResult& HitResult) const;
    void UpdateHipOffset(float DeltaTime);
};