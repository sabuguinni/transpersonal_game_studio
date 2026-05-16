#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "../SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float FootOffset;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        FootOffset = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bShowDebugTraces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName PelvisSocketName;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        MaxFootOffset = 30.0f;
        bEnableFootIK = true;
        bShowDebugTraces = false;
        LeftFootSocketName = TEXT("foot_l");
        RightFootSocketName = TEXT("foot_r");
        PelvisSocketName = TEXT("pelvis");
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled) { IKSettings.bEnableFootIK = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetDebugTraces(bool bShowDebug) { IKSettings.bShowDebugTraces = bShowDebug; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FAnim_IKSettings IKSettings;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float PelvisOffset;

    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

private:
    UFUNCTION()
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, float DeltaTime);

    UFUNCTION()
    FVector PerformFootTrace(const FVector& FootLocation, bool& bHit, FVector& HitNormal);

    UFUNCTION()
    float CalculateFootOffset(const FVector& FootLocation, const FVector& HitLocation);

    UFUNCTION()
    FRotator CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation);

    float PreviousLeftFootOffset;
    float PreviousRightFootOffset;
    float PreviousPelvisOffset;
};