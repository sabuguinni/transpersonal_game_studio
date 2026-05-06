#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FVector> BonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    TArray<FVector> BoneVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Weight;

    FAnim_MotionData()
    {
        AnimSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        RootMotionVelocity = FVector::ZeroVector;
        AngularVelocity = 0.0f;
        Weight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionQuery
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FVector DesiredVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float DesiredAngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FVector CurrentPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    FRotator CurrentRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    TArray<FVector> CurrentBonePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float AngularWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
    float PoseWeight;

    FAnim_MotionQuery()
    {
        DesiredVelocity = FVector::ZeroVector;
        DesiredAngularVelocity = 0.0f;
        CurrentPosition = FVector::ZeroVector;
        CurrentRotation = FRotator::ZeroRotator;
        VelocityWeight = 1.0f;
        AngularWeight = 0.5f;
        PoseWeight = 0.3f;
    }
};

/**
 * Componente de Motion Matching para animações fluídas e realistas
 * Baseado em técnicas modernas de IA para seleção de animações
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

    // === BASE DE DADOS DE MOVIMENTO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<UAnimSequence*> TurnAnimations;

    // === CONFIGURAÇÕES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCandidates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugDraw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency;

    // === OSSOS PARA TRACKING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Tracking")
    TArray<FName> TrackedBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Tracking")
    FName RootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Tracking")
    FName LeftFootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Tracking")
    FName RightFootBone;

public:
    // === FUNÇÕES PÚBLICAS ===
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData FindBestMatch(const FAnim_MotionQuery& Query);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddAnimationToDatabase(UAnimSequence* Animation, float SampleRate = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetDesiredMovement(FVector Velocity, float AngularVelocity);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotion() const { return CurrentMotion; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    int32 GetDatabaseSize() const { return MotionDatabase.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void EnableDebugVisualization(bool bEnable);

private:
    // === VARIÁVEIS INTERNAS ===
    FAnim_MotionData CurrentMotion;
    FAnim_MotionQuery CurrentQuery;
    float LastUpdateTime;
    
    // Referências
    class USkeletalMeshComponent* SkeletalMesh;
    class ACharacter* OwnerCharacter;

    // === FUNÇÕES INTERNAS ===
    void UpdateQuery();
    float CalculateMotionCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const;
    float CalculateVelocityCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const;
    float CalculateAngularCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const;
    float CalculatePoseCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const;
    
    void ExtractMotionData(UAnimSequence* Animation, float Time, FAnim_MotionData& OutData);
    void GetBonePositions(UAnimSequence* Animation, float Time, TArray<FVector>& OutPositions);
    void GetBoneVelocities(UAnimSequence* Animation, float Time, TArray<FVector>& OutVelocities);
    
    void DrawDebugInfo();
    void InitializeTrackedBones();
    bool IsValidAnimation(UAnimSequence* Animation) const;
};