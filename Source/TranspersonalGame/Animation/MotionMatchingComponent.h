#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/MotionMatchingAnimDatabase.h"
#include "../SharedTypes.h"
#include "MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GroundDistance = 0.0f;

    FAnim_MotionMatchingState()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainAdaptation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector SurfaceNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bOnUnevenTerrain = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float FootIKLeftOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float FootIKRightOffset = 0.0f;

    FAnim_TerrainAdaptation()
    {
        SlopeAngle = 0.0f;
        SurfaceNormal = FVector::UpVector;
        bOnUnevenTerrain = false;
        FootIKLeftOffset = 0.0f;
        FootIKRightOffset = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_TerrainAdaptation TerrainState;

    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UMotionMatchingAnimDatabase* MotionDatabase;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class ACharacter* OwnerCharacter;

    // Animation Instance Reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UAnimInstance* AnimInstance;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateTerrainAdaptation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsOnUnevenTerrain() const;

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void PerformFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    float TraceForFootIK(const FVector& FootLocation, const FVector& TraceDirection) const;

    // Terrain Analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void AnalyzeTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetSurfaceNormal(const FVector& Location) const;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TerrainAnalysisRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SlopeThreshold = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float IKInterpSpeed = 10.0f;

private:
    // Internal state tracking
    FVector LastVelocity;
    float LastUpdateTime;
    bool bInitialized;

    void InitializeComponent();
    void CacheReferences();
};