#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "../Core/TranspersonalAnimationSubsystem.h"
#include "TerrainAdaptationComponent.generated.h"

USTRUCT(BlueprintType)
struct FFootIKData
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

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    ETerrainType TerrainType;

    FFootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        TerrainType = ETerrainType::Flat;
    }
};

USTRUCT(BlueprintType)
struct FTerrainAdaptationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float HipAdjustmentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxHipAdjustment;

    FTerrainAdaptationSettings()
    {
        MaxIKDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        FootTraceDistance = 100.0f;
        FootRadius = 5.0f;
        bEnableFootRotation = true;
        bEnableHipAdjustment = true;
        HipAdjustmentSpeed = 10.0f;
        MaxHipAdjustment = 30.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTerrainAdaptationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTerrainAdaptationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    FFootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    FFootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    float GetHipOffset() const { return CurrentHipOffset; }

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void SetFootBoneNames(const FName& LeftFootBone, const FName& RightFootBone);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void SetIKSettings(const FTerrainAdaptationSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Terrain Adaptation")
    bool IsIKEnabled() const { return bIKEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void SetTerrainAdaptationWeight(float Weight);

    UFUNCTION(BlueprintPure, Category = "Terrain Adaptation")
    float GetTerrainAdaptationWeight() const { return TerrainAdaptationWeight; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FTerrainAdaptationSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FName HipBoneName;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Adaptation")
    FFootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Adaptation")
    FFootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Adaptation")
    float CurrentHipOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Adaptation")
    float TargetHipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bDebugIK;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Adaptation")
    float TerrainAdaptationWeight;

private:
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    class UTranspersonalAnimationSubsystem* AnimationSubsystem;

    FFootIKData CalculateFootIK(const FName& FootBoneName, const FVector& FootLocation);
    void UpdateHipAdjustment(float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootBoneName);
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal);
    void DrawDebugIK();
};