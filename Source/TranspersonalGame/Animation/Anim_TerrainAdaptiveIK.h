#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Anim_TerrainAdaptiveIK.generated.h"

UENUM(BlueprintType)
enum class EAnim_IKBoneType : uint8
{
    LeftFoot        UMETA(DisplayName = "Left Foot"),
    RightFoot       UMETA(DisplayName = "Right Foot"),
    LeftHand        UMETA(DisplayName = "Left Hand"),
    RightHand       UMETA(DisplayName = "Right Hand"),
    Pelvis          UMETA(DisplayName = "Pelvis"),
    Spine           UMETA(DisplayName = "Spine")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKBoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    bool bIsActive;

    FAnim_IKBoneData()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKWeight = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainIKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    float TraceDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    float TraceUpOffset = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    float MaxIKAdjustment = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    float IKInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    float SlopeAdaptationStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    bool bEnablePelvisAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    bool bEnableHandIK = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain IK")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

    FAnim_TerrainIKSettings()
    {
        TraceDistance = 200.0f;
        TraceUpOffset = 50.0f;
        MaxIKAdjustment = 100.0f;
        IKInterpSpeed = 10.0f;
        SlopeAdaptationStrength = 1.0f;
        bEnableFootIK = true;
        bEnablePelvisAdjustment = true;
        bEnableHandIK = false;
        TraceChannel = ECC_WorldStatic;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TerrainAdaptiveIK : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TerrainAdaptiveIK();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_TerrainIKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bones")
    TMap<EAnim_IKBoneType, FAnim_IKBoneData> IKBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float GlobalIKWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugTraces = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowIKTargets = false;

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void EnableIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetIKWeight(float Weight);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetBoneIKWeight(EAnim_IKBoneType BoneType, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateTerrainIK();

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FVector GetFootIKOffset(EAnim_IKBoneType FootType);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FRotator GetFootIKRotation(EAnim_IKBoneType FootType);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    float GetPelvisIKOffset();

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void InitializeIKBones();

private:
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    // Current IK values for smooth interpolation
    float CurrentPelvisOffset;
    TMap<EAnim_IKBoneType, FVector> CurrentIKOffsets;
    TMap<EAnim_IKBoneType, FRotator> CurrentIKRotations;

    // Target IK values from terrain traces
    float TargetPelvisOffset;
    TMap<EAnim_IKBoneType, FVector> TargetIKOffsets;
    TMap<EAnim_IKBoneType, FRotator> TargetIKRotations;

    void PerformFootTrace(EAnim_IKBoneType FootType, FVector& OutLocation, FRotator& OutRotation, bool& bHit);
    void CalculatePelvisAdjustment();
    void InterpolateIKValues(float DeltaTime);
    void UpdateIKBoneTransforms();
    void DrawDebugInfo();
    FVector GetBoneWorldLocation(EAnim_IKBoneType BoneType);
    FName GetBoneNameForType(EAnim_IKBoneType BoneType);
};