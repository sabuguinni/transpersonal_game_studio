#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
// FIXME: Missing header - #include "Animation/PoseSearchDatabase.h"
// DISABLED: #include "Animation/MotionMatchingAnimNodeLibrary.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

/**
 * Core Animation System Manager
 * Manages Motion Matching databases and IK systems for all characters
 * Implements the vulnerability-focused animation philosophy
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* PlayerLocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* PlayerCautionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* PlayerFearDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FString, class UPoseSearchDatabase*> DinosaurDatabases;

    // IK System References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    class UIKRigDefinition* PlayerIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    TMap<FString, class UIKRigDefinition*> DinosaurIKRigs;

    // Animation State Management
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float CurrentTensionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float CurrentFearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsBeingHunted;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateTensionLevel(float NewTension);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateFearLevel(float NewFear);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetHuntedState(bool bHunted);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    UPoseSearchDatabase* GetActivePlayerDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterDinosaurAnimationSet(const FString& SpeciesName, UPoseSearchDatabase* Database, UIKRigDefinition* IKRig);

private:
    void InitializeAnimationDatabases();
    void UpdateAnimationState(float DeltaTime);
    float CalculateMovementTension() const;
    float CalculateEnvironmentalFear() const;
};