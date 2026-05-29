#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "CollisionQueryParams.h"
#include "Anim_IKFootPlacement.generated.h"

class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName FootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName IKBoneName;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float DistanceFromGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float MaxIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float InterpSpeed;

    FAnim_FootIKData()
        : FootBoneName(NAME_None)
        , IKBoneName(NAME_None)
        , FootLocation(FVector::ZeroVector)
        , FootRotation(FRotator::ZeroRotator)
        , IKAlpha(0.0f)
        , DistanceFromGround(0.0f)
        , MaxIKDistance(50.0f)
        , InterpSpeed(15.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootRotationAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_IKSettings()
        : TraceDistance(100.0f)
        , FootHeight(13.0f)
        , InterpSpeed(15.0f)
        , bEnableFootRotation(true)
        , MaxFootRotationAngle(45.0f)
        , TraceChannel(ECC_WorldStatic)
    {}
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    TArray<FAnim_FootIKData> FootData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float HipAdjustment;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float CurrentHipOffset;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComponent;

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void PerformFootTrace(FAnim_FootIKData& FootIKData);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void CalculateHipOffset();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetFootWorldLocation(const FName& BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetupDefaultFootBones();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void AddFootIKData(const FName& FootBoneName, const FName& IKBoneName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void EnableIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsOnUneven Terrain() const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetFootIKAlpha(int32 FootIndex) const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FVector GetFootIKLocation(int32 FootIndex) const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FRotator GetFootIKRotation(int32 FootIndex) const;

private:
    bool bIsInitialized;
    float LastUpdateTime;

    void InitializeComponent();
    FHitResult PerformLineTrace(const FVector& Start, const FVector& End) const;
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
    void UpdateHipOffset(float DeltaTime);
};