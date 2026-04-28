#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimSequence.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PaleontologistMotionDatabase.generated.h"

/**
 * Motion Database configuration for the Paleontologist character
 * 
 * This defines the complete set of animations and their metadata
 * for Motion Matching to create fluid, contextual movement that
 * reflects the character's emotional state and situation.
 * 
 * Design Notes:
 * - Every animation should convey vulnerability and caution
 * - Fear responses must be immediate and believable
 * - Movements should feel like someone out of their element
 */

UENUM(BlueprintType)
enum class EPaleontologistMovementType : uint8
{
    // Basic Locomotion
    Idle_Calm           UMETA(DisplayName = "Idle - Calm"),
    Idle_Alert          UMETA(DisplayName = "Idle - Alert"),
    Idle_Fearful        UMETA(DisplayName = "Idle - Fearful"),
    
    Walk_Cautious       UMETA(DisplayName = "Walk - Cautious"),
    Walk_Normal         UMETA(DisplayName = "Walk - Normal"),
    Walk_Hurried        UMETA(DisplayName = "Walk - Hurried"),
    
    Run_Escape          UMETA(DisplayName = "Run - Escape"),
    Run_Panic           UMETA(DisplayName = "Run - Panic"),
    
    // Directional Movement
    Strafe_Left         UMETA(DisplayName = "Strafe Left"),
    Strafe_Right        UMETA(DisplayName = "Strafe Right"),
    Backpedal_Slow      UMETA(DisplayName = "Backpedal - Slow"),
    Backpedal_Fast      UMETA(DisplayName = "Backpedal - Fast"),
    
    // Crouching/Hiding
    Crouch_Enter        UMETA(DisplayName = "Crouch - Enter"),
    Crouch_Idle         UMETA(DisplayName = "Crouch - Idle"),
    Crouch_Walk         UMETA(DisplayName = "Crouch - Walk"),
    Crouch_Exit         UMETA(DisplayName = "Crouch - Exit"),
    
    // Observation/Interaction
    Look_Around         UMETA(DisplayName = "Look Around"),
    Peer_Forward        UMETA(DisplayName = "Peer Forward"),
    Listen_Intently     UMETA(DisplayName = "Listen Intently"),
    
    // Tool Interactions
    Gather_Ground       UMETA(DisplayName = "Gather from Ground"),
    Examine_Object      UMETA(DisplayName = "Examine Object"),
    Use_Tool           UMETA(DisplayName = "Use Tool"),
    
    // Fear Responses
    Startle_Small       UMETA(DisplayName = "Startle - Small"),
    Startle_Large       UMETA(DisplayName = "Startle - Large"),
    Freeze_Fear         UMETA(DisplayName = "Freeze - Fear"),
    Cower              UMETA(DisplayName = "Cower"),
    
    // Transitions
    Turn_Left_45        UMETA(DisplayName = "Turn Left 45°"),
    Turn_Right_45       UMETA(DisplayName = "Turn Right 45°"),
    Turn_Left_90        UMETA(DisplayName = "Turn Left 90°"),
    Turn_Right_90       UMETA(DisplayName = "Turn Right 90°"),
    Turn_180           UMETA(DisplayName = "Turn 180°"),
    
    // Recovery
    Stumble_Recover     UMETA(DisplayName = "Stumble Recovery"),
    Fall_Get_Up         UMETA(DisplayName = "Fall and Get Up"),
    
    // Environmental Adaptation
    Step_Over_Small     UMETA(DisplayName = "Step Over Small Obstacle"),
    Step_Over_Large     UMETA(DisplayName = "Step Over Large Obstacle"),
    Duck_Under          UMETA(DisplayName = "Duck Under"),
    Climb_Small         UMETA(DisplayName = "Climb Small Ledge")
};

USTRUCT(BlueprintType)
struct FPaleontologistAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPaleontologistMovementType MovementType = EPaleontologistMovementType::Idle_Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimSequence* AnimationSequence = nullptr;

    // Motion Matching metadata
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevelRequired = 0.0f; // Minimum fear level to trigger this animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevelMax = 1.0f; // Maximum fear level for this animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float MovementSpeed = 1.0f; // Expected movement speed for this animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanLoopSeamlessly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresRootMotion = false;

    // Contextual usage
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOnlyInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOnlyWhenHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresTarget = false; // For observation animations

    // Personality modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NervousnessInfluence = 0.5f; // How much nervousness affects this animation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfidenceInfluence = 0.5f; // How much confidence affects this animation

    // Animation blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.05", ClampMax = "2.0"))
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.05", ClampMax = "2.0"))
    float BlendOutTime = 0.2f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPaleontologistMotionDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPaleontologistMotionDatabase();

    // Complete animation set for the paleontologist
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<FPaleontologistAnimationData> AnimationDatabase;

    // Pose Search Database reference for Motion Matching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    UPoseSearchDatabase* PoseSearchDatabase = nullptr;

    // Motion Matching configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DefaultSampleRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxSearchResults = 10;

    // Fear-based animation selection weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float FearAnimationWeight = 2.0f; // How strongly fear influences animation selection

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float PersonalityWeight = 1.0f; // How strongly personality influences selection

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Animation Database")
    TArray<FPaleontologistAnimationData> GetAnimationsForFearLevel(float FearLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Animation Database")
    TArray<FPaleontologistAnimationData> GetAnimationsForMovementType(EPaleontologistMovementType MovementType) const;

    UFUNCTION(BlueprintCallable, Category = "Animation Database")
    FPaleontologistAnimationData GetBestAnimationForContext(
        float FearLevel, 
        float MovementSpeed, 
        bool bInCombat, 
        bool bHidden,
        float Nervousness,
        float Confidence
    ) const;

    UFUNCTION(BlueprintCallable, Category = "Animation Database")
    void ValidateAnimationDatabase() const;

private:
    float CalculateAnimationScore(
        const FPaleontologistAnimationData& AnimData,
        float FearLevel,
        float MovementSpeed,
        bool bInCombat,
        bool bHidden,
        float Nervousness,
        float Confidence
    ) const;
};