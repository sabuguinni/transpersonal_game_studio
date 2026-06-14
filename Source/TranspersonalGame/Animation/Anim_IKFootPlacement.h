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

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float DistanceFromGround = 0.0f;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDebugDraw = false;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        MaxIKAdjustment = 30.0f;
        bEnableIK = true;
        bDebugDraw = false;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName LeftFootSocketName = TEXT("LeftFootSocket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Names")
    FName RightFootSocketName = TEXT("RightFootSocket");

public:
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled) { IKSettings.bEnableIK = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool IsIKEnabled() const { return IKSettings.bEnableIK; }

protected:
    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, const FName& FootSocketName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FHitResult PerformFootTrace(const FVector& StartLocation) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    void DrawDebugInfo() const;

private:
    void InitializeCharacterReferences();
    FVector GetFootWorldLocation(const FName& BoneName) const;
    float InterpToTarget(float Current, float Target, float InterpSpeed, float DeltaTime) const;
};