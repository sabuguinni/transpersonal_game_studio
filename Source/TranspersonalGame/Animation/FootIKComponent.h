#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootIKComponent.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float LeftFootOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float RightFootOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bIsOnUnevenTerrain = false;
};

UCLASS(ClassGroup = (Animation), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UFootIKComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootIKComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Returns current foot IK data for AnimInstance */
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    FAnim_FootIKData GetFootIKData() const;

    /** Trace distance downward from foot bone to ground */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float TraceDistance = 100.0f;

    /** Interpolation speed for smooth foot placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float InterpSpeed = 15.0f;

    /** Offset from foot bone to actual foot bottom */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootHeight = 10.0f;

    /** Enable/disable foot IK system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    bool bEnableFootIK = true;

    /** Left foot bone name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    FName LeftFootBoneName = FName("foot_l");

    /** Right foot bone name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    FName RightFootBoneName = FName("foot_r");

private:
    FAnim_FootIKData CurrentIKData;

    /** Perform a line trace from foot position downward to find ground */
    bool TraceFootToGround(const FVector& FootWorldLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;

    /** Compute pelvis offset to prevent foot sliding */
    float ComputePelvisOffset(float LeftOffset, float RightOffset) const;

    /** Smoothly interpolate foot offset toward target */
    float InterpFootOffset(float Current, float Target, float DeltaTime) const;

    /** Cached owner skeletal mesh */
    class USkeletalMeshComponent* OwnerMesh = nullptr;
};
