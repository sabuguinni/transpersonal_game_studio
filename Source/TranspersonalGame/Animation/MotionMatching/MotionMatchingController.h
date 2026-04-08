#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Animation/AnimInstance.h"
#include "../Core/AnimationSystemManager.h"
#include "MotionMatchingController.generated.h"

UENUM(BlueprintType)
enum class EMotionMatchingContext : uint8
{
    Locomotion,
    Interaction,
    Combat,
    Stealth,
    Exploration,
    Social
};

USTRUCT(BlueprintType)
struct FMotionMatchingDatabaseSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> LocomotionDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> InteractionDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> CombatDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> StealthDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> ExplorationDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> SocialDB;
};

USTRUCT(BlueprintType)
struct FTerrainAdaptationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainRoughness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnstableGround = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SurfaceNormal = FVector::UpVector;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Database management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<ECharacterArchetype, FMotionMatchingDatabaseSet> DatabasesByArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> DefaultSchema;

    // Current context
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    EMotionMatchingContext CurrentContext = EMotionMatchingContext::Locomotion;

    // Terrain adaptation
    UPROPERTY(BlueprintReadOnly, Category = "Terrain Adaptation")
    FTerrainAdaptationData TerrainData;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingContext(EMotionMatchingContext NewContext);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetCurrentDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateTerrainAdaptation(const FTerrainAdaptationData& NewTerrainData);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetContextualBlendTime() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool ShouldUseIKFootPlacement() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetTrajectoryPrediction(float TimeAhead) const;

private:
    UPROPERTY()
    TObjectPtr<UAnimationSystemManager> AnimationManager;

    UPROPERTY()
    float ContextTransitionTimer = 0.0f;

    UPROPERTY()
    EMotionMatchingContext PreviousContext = EMotionMatchingContext::Locomotion;

    void UpdateContextBasedOnGameplay();
    void ProcessTerrainAdaptation();
    FMotionMatchingDatabaseSet* GetCurrentDatabaseSet() const;
};