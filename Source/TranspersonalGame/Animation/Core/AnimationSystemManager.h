#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

class UMotionMatchingComponent;
class UFootIKComponent;
class UAnimationBlueprint;

/**
 * Central manager for all animation systems in the Transpersonal Game
 * Coordinates Motion Matching, IK systems, and procedural animations
 * Based on RDR2's character-specific movement philosophy
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Animation System Manager"))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Animation Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Systems")
    class UMotionMatchingComponent* MotionMatchingComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Systems")
    class UFootIKComponent* FootIKComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Systems")
    class UProceduralAnimationComponent* ProceduralAnimComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<class UAnimMontage*> CombatMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<class UAnimMontage*> InteractionMontages;

    // Character Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    float CurrentSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsInteracting;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    FVector GroundNormal;

    // Animation Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MotionMatchingBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TerrainAdaptationStrength;

    // Character Personality Settings (RDR2 inspired)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float WalkingCadence; // How rhythmic the walk is

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float PosturalTension; // How tense/relaxed the character appears

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float MovementWeight; // How heavy/light the character feels

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float GestureFrequency; // How often idle gestures occur

public:
    // Core Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeAnimationSystems();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateMovementState(float Speed, float Direction, const FVector& Normal);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCombatMode(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void TriggerInteraction(const FString& InteractionType);

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingSearch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetLocomotionDatabase(class UPoseSearchDatabase* NewDatabase);

    // IK System Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableFootIK(bool bEnable);

    // Procedural Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Procedural Animation")
    void UpdateProceduralBreathing(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Procedural Animation")
    void UpdateProceduralLookAt(const FVector& TargetLocation);

    // Character Personality Functions
    UFUNCTION(BlueprintCallable, Category = "Character Personality")
    void SetCharacterPersonality(float Cadence, float Tension, float Weight, float GestureFreq);

    UFUNCTION(BlueprintCallable, Category = "Character Personality")
    void ApplyPersonalityToAnimation();

    // Terrain Adaptation
    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void UpdateTerrainAdaptation(const FVector& SurfaceNormal, float SurfaceAngle);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawAnimationState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogAnimationPerformance();

private:
    // Internal state tracking
    float LastUpdateTime;
    FVector PreviousLocation;
    FRotator PreviousRotation;
    
    // Performance tracking
    float MotionMatchingCost;
    float IKUpdateCost;
    float ProceduralAnimCost;

    // Internal helper functions
    void CalculateMovementMetrics();
    void UpdateAnimationBlending(float DeltaTime);
    void OptimizePerformance();
};