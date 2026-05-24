#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Anim_TerrainIKSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float DistanceFromGround = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bIsGrounded = false;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsGrounded = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData LeftFoot;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData RightFoot;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bIKEnabled = true;

    FAnim_TerrainIKData()
    {
        LeftFoot = FAnim_IKFootData();
        RightFoot = FAnim_IKFootData();
        PelvisOffset = 0.0f;
        bIKEnabled = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_TerrainIKSystem : public UObject
{
    GENERATED_BODY()

public:
    UAnim_TerrainIKSystem();

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void InitializeIK(USkeletalMeshComponent* SkeletalMesh, UAnimInstance* AnimInstance);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateTerrainIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FAnim_TerrainIKData GetIKData() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetFootBoneName(const FName& LeftFootBone, const FName& RightFootBone);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    bool PerformFootTrace(const FVector& FootLocation, FAnim_IKFootData& OutFootData);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_TerrainIKData IKData;

    UPROPERTY(BlueprintReadOnly, Category = "Component References")
    USkeletalMeshComponent* OwningSkeletalMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Component References")
    UAnimInstance* OwningAnimInstance;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "200.0"))
    float TraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float FootOffset = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (ClampMin = "0.1", ClampMax = "20.0"))
    float InterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    bool bEnableDebugDraw = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    FName PelvisBoneName = TEXT("pelvis");

    // Collision settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
    void UpdateFootIK(const FName& FootBoneName, FAnim_IKFootData& FootData, float DeltaTime);
    void UpdatePelvisOffset(float DeltaTime);
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    FRotator CalculateFootRotation(const FVector& ImpactNormal) const;
    void DrawDebugInfo(const FAnim_IKFootData& FootData, const FVector& TraceStart, const FVector& TraceEnd) const;
};