#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearch.h"
#include "Animation/AnimInstance.h"
#include "MotionMatchingController.generated.h"

UENUM(BlueprintType)
enum class EMotionMatchingDatabase : uint8
{
    Locomotion,
    Interaction,
    Combat,
    Stealth,
    Climbing,
    Emotional
};

USTRUCT(BlueprintType)
struct FMotionMatchingQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DesiredAcceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DesiredSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DesiredDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainComplexity;

    FMotionMatchingQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredAcceleration = FVector::ZeroVector;
        DesiredSpeed = 0.0f;
        DesiredDirection = 0.0f;
        EmotionalState = 0.0f;
        TerrainComplexity = 0.0f;
    }
};

/**
 * Controlador avançado para Motion Matching
 * Gerencia múltiplas databases e queries contextuais
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching Databases
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<EMotionMatchingDatabase, class UPoseSearchDatabase*> AnimationDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* PrimarySchema;

    // Query Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query System")
    float QueryUpdateRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query System")
    float BlendTimeMin = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query System")
    float BlendTimeMax = 0.5f;

    // Emotional Modulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float FearInfluenceOnBlending = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float ExhaustionInfluenceOnSpeed = 0.5f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FMotionMatchingQuery CurrentQuery;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EMotionMatchingDatabase ActiveDatabase;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetActiveDatabase(EMotionMatchingDatabase Database);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateQuery(const FMotionMatchingQuery& NewQuery);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateDynamicBlendTime(float FearLevel, float MovementIntensity);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* GetCurrentDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ModulateQueryWithEmotion(float FearLevel, float ExhaustionLevel);

private:
    void UpdateQueryFromMovement();
    void SelectOptimalDatabase();
    float CalculateTerrainComplexity();

    float LastQueryUpdateTime;
    FVector PreviousVelocity;
    class UAnimationSystemManager* AnimationManager;
};