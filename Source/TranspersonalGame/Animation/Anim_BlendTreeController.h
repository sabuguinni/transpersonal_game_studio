#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "SharedTypes.h"
#include "Anim_BlendTreeController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString NodeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimationAsset* Animation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsLooping;

    FAnim_BlendNode()
    {
        NodeName = TEXT("DefaultNode");
        Animation = nullptr;
        BlendWeight = 1.0f;
        PlayRate = 1.0f;
        bIsLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FAnim_BlendNode> BlendNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float GlobalPlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsActive;

    FAnim_BlendTree()
    {
        TreeName = TEXT("DefaultTree");
        GlobalPlayRate = 1.0f;
        bIsActive = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendTreeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendTreeController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TArray<FAnim_BlendTree> BlendTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    int32 CurrentTreeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* TargetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float BlendTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    bool bAutoUpdateBlending;

    // Movement-based blend parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

public:
    // Blend tree management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AddBlendTree(const FAnim_BlendTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RemoveBlendTree(int32 TreeIndex);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActiveBlendTree(int32 TreeIndex);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_BlendTree GetCurrentBlendTree() const;

    // Node management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AddBlendNode(int32 TreeIndex, const FAnim_BlendNode& NewNode);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendWeight(int32 TreeIndex, int32 NodeIndex, float NewWeight);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdatePlayRate(int32 TreeIndex, int32 NodeIndex, float NewRate);

    // Movement parameter updates
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementParameters(float Speed, float Direction, bool InAir, bool Crouching);

    // Blend space integration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceParameters(class UBlendSpace* BlendSpace, float X, float Y);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpace1DParameter(class UBlendSpace1D* BlendSpace1D, float Value);

    // Animation montage support
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAnimationMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(class UAnimMontage* Montage);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void InitializeWithSkeletalMesh(class USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsBlendTreeValid(int32 TreeIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ResetAllBlendWeights();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PauseAllAnimations();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ResumeAllAnimations();

protected:
    // Internal blend processing
    void ProcessBlendTrees(float DeltaTime);
    void UpdateBlendWeights(float DeltaTime);
    void ApplyBlendToMesh();

    // Movement-based automatic blending
    void UpdateMovementBlending();
    void CalculateLocomotionBlend();

    // Validation
    bool ValidateBlendTree(const FAnim_BlendTree& Tree) const;
    bool ValidateBlendNode(const FAnim_BlendNode& Node) const;
};