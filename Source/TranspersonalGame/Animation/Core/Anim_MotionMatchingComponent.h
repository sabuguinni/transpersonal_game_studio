#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Anim_MotionMatchingComponent.generated.h"

// Motion matching state for prehistoric character movement
UENUM(BlueprintType)
enum class EAnim_MotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting")
};

// Motion matching query parameters
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    // Current velocity of the character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity = FVector::ZeroVector;

    // Desired movement direction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector DesiredDirection = FVector::ForwardVector;

    // Current motion state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MotionState MotionState = EAnim_MotionState::Idle;

    // Terrain slope angle (for adaptive movement)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TerrainSlope = 0.0f;

    // Is character on uneven terrain
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsOnUnevenTerrain = false;

    FAnim_MotionQuery()
    {
        Velocity = FVector::ZeroVector;
        DesiredDirection = FVector::ForwardVector;
        MotionState = EAnim_MotionState::Idle;
        TerrainSlope = 0.0f;
        bIsOnUnevenTerrain = false;
    }
};

/**
 * Motion Matching component for fluid, realistic character movement
 * Implements the core animation system for prehistoric humans
 * Each movement feels weighted and intentional, like RDR2's Arthur Morgan
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching database for this character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* MotionDatabase;

    // Current motion query parameters
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionQuery CurrentQuery;

    // Blend time for motion transitions (in seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float BlendTime = 0.3f;

    // Minimum velocity threshold for movement detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float MinMovementThreshold = 5.0f;

    // Maximum search distance for pose matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float MaxSearchDistance = 200.0f;

    // Update the motion query based on character state
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionQuery(const FVector& InVelocity, const FVector& InDesiredDirection, EAnim_MotionState InMotionState);

    // Get the best matching pose from the database
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool FindBestPoseMatch(float& OutPoseTime, int32& OutAnimationIndex);

    // Set the motion state (triggers appropriate animation transitions)
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionState(EAnim_MotionState NewState);

    // Check if character is in a transition state
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInTransition() const;

    // Get current movement speed
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentSpeed() const;

    // Get normalized movement direction
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FVector GetMovementDirection() const;

    // Event called when motion state changes
    UFUNCTION(BlueprintImplementableEvent, Category = "Motion Matching")
    void OnMotionStateChanged(EAnim_MotionState OldState, EAnim_MotionState NewState);

private:
    // Previous motion state for transition detection
    EAnim_MotionState PreviousMotionState;

    // Time since last state change
    float TimeSinceStateChange;

    // Cached character movement component
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Initialize motion matching system
    void InitializeMotionMatching();

    // Update terrain analysis
    void UpdateTerrainAnalysis();

    // Calculate motion state based on current conditions
    EAnim_MotionState CalculateMotionState() const;
};